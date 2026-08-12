"""
配置管理模块
处理设备配置、UUID、服务器地址等
"""

import ujson
import os
import ubinascii
import machine
import urandom
from libs.Network import NetworkManager, mac_address
from state import (
    DeviceState, SpeechInteractionMode, ListeningMode,
    MessageType, TTSState, ListenState, ListenMode,
    MessageBuilder, MessageParser, session_manager
)

class ConfigManager:
    """配置管理器"""
    
    def __init__(self, project_config):
        network_config = project_config["network"]
        xiaozhi_config = project_config["xiaozhi"]
        webtts_config = project_config["webtts"]

        self.config_file = "/data/xiaozhi.cfg"
        self.default_config = {
            "uuid": "",
            "mac_address": "00:00:00:00:00:00",
            "xiaozhi": {
                "xiaozhi_ota_url": xiaozhi_config["ota_url"],
                "xiaozhi_ws_addr": xiaozhi_config["websocket_url"],
            },
            "webtts": {
                "app_id": webtts_config["app_id"],
                "api_key": webtts_config["api_key"],
                "api_secret": webtts_config["api_secret"],
                "sample_rate": webtts_config["sample_rate"],
                "voice_name": webtts_config["voice_name"],
                "speed": webtts_config["speed"],
            },
        }
        
        # 运行时配置
        self.uuid = ""
        self.mac_address = ""
        self.ota_url = ""
        self.ws_addr = ""
        self.ws_hostname = ""
        self.ws_port = ""
        self.ws_path = ""
        self.ws_headers = ""
        self.ws_hello = ""

        # The configuration manager owns the one interface used by HTTP and
        # WebSocket clients.  It never creates a second interface just to read
        # the MAC address.
        self.network_manager = NetworkManager(
            network_type=network_config["type"],
            ssid=network_config["ssid"],
            password=network_config["password"],
            timeout=network_config["timeout"],
            wlan_device=network_config["wlan_device"],
        )
        self.netif = None
        self.network_ip = None
        self.network_mac = None

    def ensure_network(self):
        """Connect or recover the single shared network interface."""
        self.netif, self.network_ip = self.network_manager.connect()
        if self.network_mac is None:
            self.network_mac = mac_address(self.netif)
        return self.netif
        
    def load_config(self):
        """加载配置文件"""
        create_config = False
        try:
            # 尝试读取配置文件
            with open(self.config_file, 'r') as f:
                config_data = ujson.load(f)
                
            # 合并配置
            self._merge_config(config_data)
            # 旧版文件保存过用户可编辑配置，迁移后只保留设备标识。
            if "xiaozhi" in config_data or "webtts" in config_data:
                create_config = True
            
        except Exception as e:
            print("读取配置文件失败: %s" % e)
            # 使用默认配置
            self._merge_config(self.default_config)

            # 生成新的UUID
            self.uuid = self._generate_uuid()
            create_config = True

        try:
            self.ensure_network()
        except Exception as e:
            print("网络尚未就绪: %s" % e)

        # Repair files created before network ownership moved here.
        if not self.mac_address or self.mac_address == self.default_config["mac_address"]:
            self.mac_address = self.network_mac or self._get_device_id()
            create_config = True

        if create_config:
            self.save_config()
            
        # 解析WebSocket地址
        self._parse_websocket_url()
        
        # 构建WebSocket头部
        self._build_websocket_headers()
        
        # 构建hello消息
        self._build_hello_message()
        #self.build_listen_start_message()
        
        print("配置加载完成")
        print("UUID: %s" % self.uuid)
        print("MAC: %s" % self.mac_address)
        print("WebSocket地址: %s" % self.ws_addr)
        
        return True
        
    def save_config(self):
        """保存设备生成的标识信息"""
        try:
            config_data = {
                "uuid": self.uuid,
                "mac_address": self.mac_address,
            }
            
            with open(self.config_file, 'w') as f:
                ujson.dump(config_data, f)
                
            print("配置保存成功")
            return True
            
        except Exception as e:
            print("保存配置失败: %s" % e)
            return False
            
    def _merge_config(self, config_data):
        """合并配置数据"""
        self.uuid = config_data.get("uuid", self.default_config["uuid"])
        self.mac_address = config_data.get("mac_address", self.default_config["mac_address"])
        # User-editable settings always come from run.py. The persisted file
        # only supplies generated device identity.
        self.ota_url = self.default_config["xiaozhi"]["xiaozhi_ota_url"]
        self.ws_addr = self.default_config["xiaozhi"]["xiaozhi_ws_addr"]
        
    def _generate_uuid(self):
        """生成UUID"""
        random_bytes = bytearray([urandom.randint(0, 255) for _ in range(16)])
        random_bytes[6] = (random_bytes[6] & 0x0F) | 0x40  # 0x40 = 0100 0000
        # 第 8 字节的高 2 位设为 10（对应十六进制 8/9/a/b）
        random_bytes[8] = (random_bytes[8] & 0x3F) | 0x80  # 0x80 = 1000 0000

        # 转换为十六进制字符串并按格式拼接
        hex_str = random_bytes.hex()
        uuid = f"{hex_str[:8]}-{hex_str[8:12]}-{hex_str[12:16]}-{hex_str[16:20]}-{hex_str[20:]}"
        return uuid.upper()

    def _get_device_id(self):
        """获取设备ID作为MAC地址的替代"""
        try:
            # 使用设备唯一ID
            device_id = machine.unique_id()
            if device_id:
                mac = ubinascii.hexlify(device_id).decode()
                # 格式化为MAC地址格式
                if len(mac) >= 12:
                    return ':'.join([mac[i:i+2] for i in range(0, 12, 2)])
                else:
                    # 填充到12位
                    mac = mac.ljust(12, '0')
                    return ':'.join([mac[i:i+2] for i in range(0, 12, 2)])
            else:
                return self.default_config["mac_address"]
                
        except Exception as e:
            print("获取设备ID失败: %s" % e)
            return self.default_config["mac_address"]
            
    def _parse_websocket_url(self):
        """解析WebSocket URL"""
        try:
            # 移除协议前缀
            url = self.ws_addr
            if url.startswith("wss://"):
                url = url[6:]
                self.ws_port = "443"
            elif url.startswith("ws://"):
                url = url[5:]
                self.ws_port = "80"
            else:
                # 默认使用wss
                self.ws_port = "443"
                
            # 分割主机名和路径
            parts = url.split('/', 1)
            self.ws_hostname = parts[0]
            
            if len(parts) > 1:
                self.ws_path = '/' + parts[1]
            else:
                self.ws_path = '/'
                
            # 如果主机名包含端口号
            if ':' in self.ws_hostname:
                host_parts = self.ws_hostname.split(':')
                self.ws_hostname = host_parts[0]
                self.ws_port = host_parts[1]
                
        except Exception as e:
            print("解析WebSocket URL失败: %s" % e)
            raise ValueError("run.py 中的 websocket_url 无效")
            
    def _build_websocket_headers(self):
        """构建WebSocket头部"""
        headers = {
            "Authorization": "Bearer test-token",  # 默认token，会在激活后更新
            "Protocol-Version": "1",
            "Device-Id": self.mac_address,
            "Client-Id": self.uuid
        }
        self.ws_headers = ujson.dumps(headers)
        
    def update_websocket_token(self, token):
        """更新WebSocket认证token"""
        try:
            headers = ujson.loads(self.ws_headers)
            headers["Authorization"] = "Bearer " + token
            self.ws_headers = ujson.dumps(headers)
            print("WebSocket token已更新")
            return True
        except Exception as e:
            print("更新WebSocket token失败: %s" % e)
            return False
        
    def _build_hello_message(self):
        """构建hello消息 - 根据通信协议"""
        hello = MessageBuilder.build_hello_message()
        self.ws_hello = ujson.dumps(hello)
        
    def get_webtts_config(self):
        """获取WebTTS配置"""
        return self.default_config["webtts"]
        
    def update_webtts_config(self, config):
        """更新当前运行的WebTTS配置"""
        self.default_config["webtts"].update(config)
        
    def get_websocket_config(self):
        """获取WebSocket配置"""
        return {
            'hostname': self.ws_hostname,
            'port': int(self.ws_port),
            'path': self.ws_path,
            'headers': self.ws_headers,
            'hello': self.ws_hello
        }
