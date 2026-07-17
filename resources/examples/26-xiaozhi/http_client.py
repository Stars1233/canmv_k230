"""
HTTP客户端模块
处理HTTP请求和设备激活
"""

import usocket
import ujson
import ussl
import network

class HttpClient:
    """HTTP客户端"""
    
    def __init__(self):
        self.timeout = 10  # 超时时间（秒）
        self._init_network()
        
    def _init_network(self):
        """初始化网络连接"""
        try:
            # 初始化WLAN
            wlan = network.LAN()
            if not wlan.active():
                print("激活WLAN...")
                wlan.active(True)
                
            # 检查网络连接状态
            if wlan.isconnected():
                print("网络已连接")
                print("IP地址: %s" % wlan.ifconfig()[0])
            else:
                print("警告: 网络未连接，请确保设备已连接到WiFi")
                print("尝试扫描可用网络...")
                networks = wlan.scan()
                if networks:
                    print("发现 %d 个网络" % len(networks))
                else:
                    print("未发现可用网络")
                    
        except Exception as e:
            print("网络初始化失败: %s" % e)
        
    def post(self, url, data, headers=None):
        """发送HTTP POST请求"""
        #try:
            # 解析URL
        host, port, path = self._parse_url(url)
        
        # 获取地址信息
        addr_info = usocket.getaddrinfo(host, port, usocket.AF_INET, usocket.SOCK_STREAM)
        if not addr_info:
            print("无法解析主机名: %s" % host)
            return None
            
        # 使用第一个地址信息
        addr = addr_info[0][-1]
        
        # 创建socket连接
        sock = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
        sock.settimeout(self.timeout)
        
        # 连接服务器
        sock.connect(addr)
        
        # 如果是HTTPS，包装SSL
        if url.startswith('https://'):
            print("启用SSL加密")
            # 类似C++版本，禁用证书验证以避免TLS握手失败
            try:
                # 尝试使用不验证证书的方式
                sock = ussl.wrap_socket(sock, server_hostname=host)
            except Exception as e:
                print("SSL包装失败: %s" % e)
                # 如果失败，尝试其他方式
                try:
                    sock = ussl.wrap_socket(sock)
                except Exception as e2:
                    print("备用SSL包装也失败: %s" % e2)
                    sock.close()
                    return None
        
        # 构建HTTP请求
        request = self._build_http_request('POST', host, path, data, headers)
        
        # 发送请求
        bytes_sent = sock.write(request.encode())
        
        # 接收响应
        response = self._receive_response(sock)
        
        # 关闭连接
        sock.close()
        
        return response
            
        #except Exception as e:
        #    print("HTTP POST请求失败: %s" % e)
        #    return None
            
    def get(self, url, headers=None):
        """发送HTTP GET请求"""
        try:
            # 解析URL
            host, port, path = self._parse_url(url)
            
            print("连接服务器: %s:%d" % (host, port))
            
            # 获取地址信息
            addr_info = usocket.getaddrinfo(host, port, usocket.AF_INET, usocket.SOCK_STREAM)
            if not addr_info:
                print("无法解析主机名: %s" % host)
                return None
                
            # 使用第一个地址信息
            addr = addr_info[0][-1]
            print("解析到的地址: %s" % str(addr))
            
            # 创建socket连接
            sock = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
            sock.settimeout(self.timeout)
            
            # 连接服务器
            print("正在连接到服务器...")
            sock.connect(addr)
            print("连接成功")
            
            # 如果是HTTPS，包装SSL
            if url.startswith('https://'):
                print("启用SSL加密")
                sock = ussl.wrap_socket(sock)
            
            # 构建HTTP请求
            request = self._build_http_request('GET', host, path, None, headers)
            print("HTTP请求长度: %d" % len(request))
            
            # 发送请求
            print("发送HTTP请求...")
            bytes_sent = sock.write(request.encode())
            print("已发送 %d 字节" % bytes_sent)
            
            # 接收响应
            response = self._receive_response(sock)
            
            # 关闭连接
            sock.close()
            print("连接已关闭")
            
            return response
            
        except Exception as e:
            print("HTTP GET请求失败: %s" % e)
            return None
            
    def _parse_url(self, url):
        """解析URL"""
        # 移除协议前缀
        if url.startswith('https://'):
            url = url[8:]
            default_port = 443
        elif url.startswith('http://'):
            url = url[7:]
            default_port = 80
        else:
            # 默认使用HTTP
            default_port = 80
            
        # 分割主机名和路径
        parts = url.split('/', 1)
        host_part = parts[0]
        
        if len(parts) > 1:
            path = '/' + parts[1]
        else:
            path = '/'
            
        # 处理端口号
        if ':' in host_part:
            host, port_str = host_part.split(':', 1)
            port = int(port_str)
        else:
            host = host_part
            port = default_port
            
        return host, port, path
        
    def _build_http_request(self, method, host, path, data=None, headers=None):
        """构建HTTP请求"""
        if headers is None:
            headers = {}
            
        # 默认头部
        default_headers = {
            'Host': host,
            'User-Agent': 'MicroPython-XiaoZhi/1.0',
            'Accept': '*/*',
            'Connection': 'close'
        }
        
        # 合并头部
        for key, value in headers.items():
            default_headers[key] = value
            
        # 如果有数据，添加Content-Type和Content-Length
        if data is not None:
            if 'Content-Type' not in default_headers:
                default_headers['Content-Type'] = 'application/json'
            default_headers['Content-Length'] = str(len(data))
            
        # 构建请求行和头部
        request_lines = ["%s %s HTTP/1.1" % (method, path)]
        for key, value in default_headers.items():
            request_lines.append("%s: %s" % (key, value))
            
        # 添加空行分隔头部和主体
        request_lines.append('')
        
        # 如果有数据，添加到请求中
        if data is not None:
            request_lines.append(data)
            
        return '\r\n'.join(request_lines)
        
    def _receive_response(self, sock):
        """接收HTTP响应"""
        try:
            response_data = b''
            
            # 接收数据直到连接关闭
            while True:
                chunk = sock.read(1024)
                if not chunk:
                    break
                response_data += chunk
                
            # 解析响应
            return self._parse_http_response(response_data.decode())
            
        except Exception as e:
            print("接收HTTP响应失败: %s" % e)
            return None
            
    def _parse_http_response(self, response_text):
        """解析HTTP响应"""
        try:
            # 分割响应头和主体
            parts = response_text.split('\r\n\r\n', 1)
            if len(parts) < 2:
                return None
                
            header_part, body_part = parts
            
            # 解析状态行
            lines = header_part.split('\r\n')
            status_line = lines[0]
            status_parts = status_line.split(' ', 2)
            
            if len(status_parts) < 3:
                return None
                
            status_code = int(status_parts[1])
            status_message = status_parts[2]
            
            # 解析响应头
            headers = {}
            for line in lines[1:]:
                if ':' in line:
                    key, value = line.split(':', 1)
                    headers[key.strip()] = value.strip()
                    
            # 解析响应体
            body = body_part
            content_type = headers.get('Content-Type', '')
            
            # 如果是JSON，尝试解析
            if 'application/json' in content_type:
                try:
                    body = ujson.loads(body)
                except:
                    pass  # 保持原始字符串
                    
            return {
                'status': status_code,
                'message': status_message,
                'headers': headers,
                'body': body
            }
            
        except Exception as e:
            print("解析HTTP响应失败: %s" % e)
            return None
            
    def active_device(self, http_data, code_buffer):
        """设备激活函数（兼容原有接口）"""
        #try:
            # 解析http_data
        url = http_data.get('url', '')
        post_data = http_data.get('post', '')
        headers_json = http_data.get('headers', '{}')
        
        print("激活请求URL: %s" % url)
        print("激活请求数据: %s" % post_data)
        print("激活请求头部: %s" % headers_json)
        
        # 解析headers
        try:
            headers = ujson.loads(headers_json)
        except Exception as e:
            print("解析headers失败: %s" % e)
            headers = {}
            
        # 发送POST请求
        response = self.post(url, post_data, headers)
        
        if response:
            print("激活响应状态: %d" % response.get('status', 0))
            print("激活响应消息: %s" % response.get('message', ''))
            
        if response and response.get('status') == 200:
            # 激活成功
            body = response.get('body', {})
            print("激活响应体: %s" % str(body))
            
            # 检查是否有activation字段
            if isinstance(body, dict) and body.get('activation') and body['activation'].get('code'):
                # 有激活码，需要用户激活
                active_code = body['activation']['code']
                print("收到激活码: %s" % active_code)
                if code_buffer and len(code_buffer) >= len(active_code):
                    code_buffer[:len(active_code)] = active_code.encode()
                return 1  # 需要激活
            else:
                # 已经激活，提取WebSocket token
                if isinstance(body, dict) and body.get('websocket') and body['websocket'].get('token'):
                    token = body['websocket']['token']
                    print("获取到WebSocket token: %s" % token)
                # 已经激活
                return 0  # 已经激活
        else:
            # 激活失败
            return -1  # 失败
                
        #except Exception as e:
        #    print("设备激活失败: %s" % e)
        #    return -1
