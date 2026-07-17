"""
WebSocket客户端模块
处理WebSocket连接和数据传输
"""

import usocket
import ujson
import ssl
import _thread
import ubinascii
import time
import ustruct
import os

class WebSocketClient:
    """WebSocket客户端"""
    
    def __init__(self):
        self.socket = None
        self.connected = False
        self.running = False
        self.thread = None
        
        # 回调函数
        self.binary_callback = None
        self.text_callback = None
        self.state_callback = None
        
        # 配置
        self.config = {}
        
        # 发送队列
        self.send_queue = []
        self.send_lock = _thread.allocate_lock()
        # 复用 poll，禁止每次 new（会耗尽 DFS fd）
        self._poll = None
        self._poll_sock = None
        
    def set_callbacks(self, binary_cb, text_cb, state_cb):
        """设置回调函数"""
        self.binary_callback = binary_cb
        self.text_callback = text_cb
        self.state_callback = state_cb
        
    def set_config(self, config):
        """设置配置"""
        self.config = config
        
    def start(self):
        """启动WebSocket连接"""
        if self.running:
            print("WebSocket已经在运行")
            return 0
            
        self.running = True
        
        # 启动连接线程
        self.thread = _thread.start_new_thread(self._connection_thread, ())
        
        return 0
        
    def stop(self):
        """停止WebSocket连接（主入口，触发完整资源释放）"""
        if not self.running and not self.connected:
            print("WebSocket已处于关闭状态")
            return 0
            
        print("开始关闭WebSocket连接...")
        # 1. 标记状态为停止，阻止后续操作
        self.running = False
        self.connected = False
        
        # 2. 清空发送队列（加锁保护，避免并发冲突）
        with self.send_lock:
            self.send_queue.clear()
        
        # 3. 统一释放资源
        self._clear_resources()
        
        print("WebSocket连接已完全关闭")
        return 0
            
    def _clear_resources(self):
        """核心资源释放方法（统一处理所有资源）"""
        # 3.1 关闭socket（含协议级关闭帧）
        if self.socket:
            try:
                # 发送WebSocket标准关闭帧（ opcode=0x8，状态码1000表示正常关闭）
                self._send_close_frame(close_code=1000, reason="client主动关闭")
            except Exception as e:
                print("发送关闭帧失败: %s" % e)
            
            try:
                # 关闭底层socket（SSL socket同样适用）
                self.socket.close()
                print("Socket已关闭")
            except Exception as e:
                print("关闭socket异常: %s" % e)
            
            self.socket = None
        
        self._poll = None
        self._poll_sock = None
        
        # 3.3 通知外部状态变化
        if self.state_callback:
            self.state_callback(False)
        
        # 3.4 重置线程引用（线程会自然退出）
        self.thread = None

    def _send_close_frame(self, close_code=1000, reason="normal closure"):
        """发送WebSocket标准关闭帧（符合RFC 6455协议）"""
        if not self.socket:
            return
        
        # 构建关闭帧payload：2字节大端序关闭码 + UTF-8编码的原因描述
        payload = ustruct.pack(">H", close_code)  # 关闭码（1000=正常关闭，1001=客户端离开等）
        if reason:
            payload += reason.encode("utf-8")
        
        # 发送关闭帧（opcode=0x8表示关闭）
        self._send_websocket_frame(self.socket, payload, opcode=0x8)
        # 短暂等待，确保关闭帧被发送（避免socket立即关闭导致数据丢失）
        time.sleep(0.1)
                           
    def send_binary(self, data, size):
        """发送二进制数据"""
        if not self.connected or not self.socket:
            return -1

        # 将数据添加到发送队列；音频积压时丢最旧二进制，避免撑爆发送缓冲
        with self.send_lock:
            if len(self.send_queue) >= 40:
                kept = []
                dropped = 0
                for item in self.send_queue:
                    if item[0] == 'binary' and dropped < 10:
                        dropped += 1
                        continue
                    kept.append(item)
                self.send_queue = kept
            self.send_queue.append(('binary', data[:size]))

        return 0
        
    def send_text(self, data, size=None):
        """发送文本数据"""
        if not self.connected or not self.socket:
            return -1
            
        if size is None:
            size = len(data)
            
        # 将数据添加到发送队列
        with self.send_lock:
            self.send_queue.append(('text', data[:size]))
            
        return 0
    
    def _connection_thread(self):
        """连接线程：发送失败/断线后自动重连，不让异常打死线程"""
        while self.running:
            try:
                if not self.connected:
                    if self._connect():
                        self.connected = True
                        if self.state_callback:
                            self.state_callback(True)
                    else:
                        time.sleep(5)
                        continue

                self._process_messages()
            except Exception as e:
                print("WebSocket连接线程异常: %s" % e)
                self._soft_disconnect()
                if self.running:
                    time.sleep(2)

    def _soft_disconnect(self):
        """连接异常时的软断开：关 socket、清队列、通知上层，保留 running 以便重连"""
        was_connected = self.connected
        self.connected = False
        with self.send_lock:
            self.send_queue = []
        sock = self.socket
        self.socket = None
        self._poll = None
        self._poll_sock = None
        if sock:
            try:
                sock.close()
            except:
                pass
        if was_connected and self.state_callback:
            try:
                self.state_callback(False)
            except:
                pass
            
    def _connect(self):
        """建立WebSocket连接"""
        try:
            hostname = self.config.get('hostname', '')
            port = int(self.config.get('port', 443))
            path = self.config.get('path', '/')

            print("连接WebSocket: %s:%d%s" % (hostname, port, path))

            addr_info = usocket.getaddrinfo(hostname, port, usocket.AF_INET, usocket.SOCK_STREAM)
            if not addr_info:
                print("无法解析主机名: %s" % hostname)
                return False

            addr = addr_info[0][-1]
            print("解析到的地址: %s" % str(addr))

            sock = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
            sock.settimeout(10)

            print("正在连接到服务器...")
            sock.connect(addr)
            print("连接成功")

            if port == 443:
                print("启用SSL加密")
                sock = ssl.wrap_socket(
                    sock,
                    server_hostname=hostname
                )
                print("TLS 握手成功，加密通道已建立")

            if not self._websocket_handshake(sock, hostname, path):
                sock.close()
                return False

            self.socket = sock
            self._poll = None
            self._poll_sock = None
            print("WebSocket连接成功")
            return True

        except Exception as e:
            print("WebSocket连接失败: %s" % e)
            return False
            
    def _websocket_handshake(self, sock, host, path):
        """WebSocket握手"""
        #try:
            # 生成WebSocket key
        key = ubinascii.b2a_base64(os.urandom(16))[:-1].decode()
        
        # 构建握手请求
        handshake = "GET %s HTTP/1.1\r\n" % path
        handshake += "Host: %s\r\n" % host
        handshake += "Upgrade: websocket\r\n"
        handshake += "Connection: Upgrade\r\n"
        handshake += "Sec-WebSocket-Key: %s\r\n" % key
        handshake += "Sec-WebSocket-Version: 13\r\n"
        
        # 添加自定义头部
        headers_json = self.config.get('headers', '{}')
        try:
            headers = ujson.loads(headers_json)
            for header_key, header_value in headers.items():
                handshake += "%s: %s\r\n" % (header_key, header_value)
        except:
            pass
            
        handshake += "\r\n"
        
        print(f"发送WebSocket握手请求... {handshake}")
        # 发送握手请求
        sock.write(handshake.encode())
        
        # 接收握手响应
        response = b''
        while b'\r\n\r\n' not in response:
            chunk = sock.read(1024)
            if not chunk:
                break
            response += chunk
            
        print("收到握手响应，长度: %d" % len(response))
        response_str = response.decode()
        print(f"RTSP响应:\n{response_str}")           
        
            
        # 检查握手是否成功
        if b'101 Switching Protocols' in response and b'Upgrade: websocket' in response:
            print("WebSocket握手成功")
            #self._send_heartbeat()
            # 立即发送hello消息（关键步骤）
            hello = self.config.get("hello", "")
            if hello:
                print("发送hello消息: %s" % hello)
                self._send_websocket_frame(sock, hello, opcode=0x1)  # 文本帧        
            #else:
            #    print("警告: 没有配置hello消息")
                
            return True
        else:
            print("WebSocket握手失败")
            print("响应内容: %s" % response.decode('utf-8', errors='ignore'))
            return False
                
        #except Exception as e:
        #    print("WebSocket握手异常: %s" % e)
        #    return False
            
    def _process_messages(self):
        """处理消息：每轮最多收几帧，然后必 sleep，避免 FD/CPU 打满"""
        if not self.connected or not self.socket:
            raise OSError("websocket not connected")

        for _ in range(6):
            if not (self.socket and self._socket_ready()):
                break
            opcode, payload = self._receive_websocket_frame(self.socket)
            if opcode is None:
                # 对端关闭或读失败，触发重连
                raise OSError("websocket receive failed")
            self._handle_websocket_frame(opcode, payload)

        if not self._process_send_queue():
            raise OSError("websocket send failed")

        self._send_heartbeat()
        time.sleep(0.01)

    def _send_heartbeat(self):
        """发送心跳包保持连接"""
        try:
            if not self.connected or not self.socket:
                return
            if hasattr(self, '_last_heartbeat'):
                if time.time() - self._last_heartbeat > 1:
                    if not self._send_websocket_frame(self.socket, '', opcode=0x9):
                        raise OSError("heartbeat send failed")
                    self._last_heartbeat = time.time()
            else:
                self._last_heartbeat = time.time()
        except Exception as e:
            raise e

    def _socket_ready(self):
        """检查socket是否可读（复用同一个 poll，避免反复 register 泄漏 fd）"""
        try:
            import uselect
            if self._poll is None:
                self._poll = uselect.poll()
                self._poll.register(self.socket, uselect.POLLIN)
                self._poll_sock = self.socket
            elif self._poll_sock is not self.socket:
                try:
                    if self._poll_sock is not None:
                        self._poll.unregister(self._poll_sock)
                except:
                    pass
                self._poll.register(self.socket, uselect.POLLIN)
                self._poll_sock = self.socket
            events = self._poll.poll(0)
            return len(events) > 0
        except:
            return False

    def _process_send_queue(self):
        """处理发送队列。成功返回 True，发送失败返回 False。"""
        with self.send_lock:
            if not self.send_queue:
                return True
            queue = self.send_queue
            self.send_queue = []

        for msg_type, data in queue:
            if not self.socket:
                return False
            opcode = 0x2 if msg_type == 'binary' else 0x1
            if not self._send_websocket_frame(self.socket, data, opcode=opcode):
                return False
        return True
            
    def _send_websocket_frame(self, sock, payload, opcode=0x1):
        """发送WebSocket帧。成功返回 True，失败返回 False（不向外抛，避免打死连接线程）。"""
        try:
            if not sock:
                return False
            payload = payload.encode() if isinstance(payload, str) else payload
            payload_len = len(payload)

            frame = bytearray()
            frame.append(0x80 | opcode)  # FIN=1, opcode

            if payload_len <= 125:
                frame.append(payload_len | 0x80)
            elif payload_len <= 65535:
                frame.append(126|0x80)
                frame.extend(payload_len.to_bytes(2, 'big'))
            else:
                frame.append(127|0x80)
                frame.extend(payload_len.to_bytes(8, 'big'))

            mask = os.urandom(4)
            frame.extend(mask)
            for i in range(payload_len):
                frame.append(payload[i] ^ mask[i % 4])

            sock.write(bytes(frame))
            return True

        except Exception as e:
            print("发送WebSocket帧失败: %s" % e)
            return False
            
    def _receive_websocket_frame(self, sock):
        """接收WebSocket帧"""
        try:
            # 读取帧头
            header = sock.read(2)
            if len(header) < 2:
                print("接收帧头失败：返回")
                return None, None
                
            byte1, byte2 = header[0], header[1]
            
            fin = (byte1 & 0x80) != 0
            opcode = byte1 & 0x0F
            masked = (byte2 & 0x80) != 0
            payload_len = byte2 & 0x7F
            
            # 读取扩展长度
            if payload_len == 126:
                ext_len = sock.read(2)
                if len(ext_len) < 2:
                    print("接收扩展长度失败（126）")
                    return None, None
                payload_len = int.from_bytes(ext_len, 'big')
            elif payload_len == 127:
                ext_len = sock.read(8)
                if len(ext_len) < 8:
                    print("接收扩展长度失败（127）")
                    return None, None
                payload_len = int.from_bytes(ext_len, 'big')
                
            # 读取掩码键
            if masked:
                mask_key = sock.read(4)
                if len(mask_key) < 4:
                    print("接收掩码失败")
                    return None, None
            else:
                mask_key = None
                
            # 读取payload
            payload = b''
            remaining = payload_len
            while remaining > 0:
                chunk = sock.read(min(remaining, 4096))
                if not chunk:
                    break
                payload += chunk
                remaining -= len(chunk)
                
            if len(payload) != payload_len:
                return None, None
                
            # 解掩码
            if masked and mask_key:
                payload = bytes(payload[i] ^ mask_key[i % 4] for i in range(len(payload)))
            
            #print(f"接收WebSocket帧：opcode={opcode} payload={payload}")    
            return opcode, payload
            
        except Exception as e:
            print("接收WebSocket帧失败: %s" % e)
            return None, None
            
    def _handle_websocket_frame(self, opcode, payload):
        """处理WebSocket帧"""
    #try:
        if opcode == 0x1:  # 文本帧
            if self.text_callback:
                self.text_callback(payload.decode(), len(payload))
                
        elif opcode == 0x2:  # 二进制帧
            if self.binary_callback:
                self.binary_callback(payload, len(payload))
                
        elif opcode == 0x8:  # 关闭帧
            self.connected = False
            close_code, reason = self.handle_close_frame(payload)
            print(f"收到WebSocket关闭帧：码={close_code}，原因={reason}")
            
        elif opcode == 0x9:  # Ping帧
            # 回复Pong帧
            self._send_websocket_frame(self.socket, payload, opcode=0xA)
            
        elif opcode == 0xA:  # Pong帧
            # 忽略Pong帧
            pass
                
        #except Exception as e:
        #    print("处理WebSocket帧异常: %s" % e)
            
            
    def handle_close_frame(self, payload):
        """解析关闭帧 Payload：2字节关闭码 + 可选原因"""
        self.stop()
        if len(payload) >= 2:
            # 解析 2 字节大端序关闭码（struct 格式 ">H"：大端序无符号短整型）
            close_code = ustruct.unpack(">H", payload[:2])[0]
            # 解析后续 UTF-8 原因描述
            reason = payload[2:].decode("utf-8", errors="ignore") if len(payload) > 2 else "无"
            return close_code, reason
        return None, "无"
