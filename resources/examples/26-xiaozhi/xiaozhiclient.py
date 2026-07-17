#!/usr/bin/env python3
"""
小智语音助手 - MicroPython版本
根据通信协议重新实现通信过程
"""

import ujson
import _thread
import time
from config import ConfigManager
from state import (
    DeviceState, SpeechInteractionMode, ListeningMode,
    MessageType, TTSState, ListenState, ListenMode,
    MessageBuilder, MessageParser, session_manager
)
from websocket_client import WebSocketClient
from http_client import HttpClient
from audio_manager import AudioManager
from machine import Pin
from machine import FPIOA
import os
from iot.thing_manager import ThingManager

class XiaoZhiClient:
    """小智语音助手主控制类 - 根据通信协议实现"""
    _instance = None

    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = XiaoZhiClient()
        return cls._instance
       
    def __init__(self):
        self.config = ConfigManager()
        self.ws_client = WebSocketClient()
        self.http_client = HttpClient()
        self.audio_manager = AudioManager.get_instance()
        self.thing_manager = ThingManager.get_instance()
        
        # 回调函数
        self.audio_upload_callback = None
        self.tts_state_callback = None
        self.ws_state_callback = None
        self.llm_callback = None
        self.text_callback = None
        self.status_callback = None
        self.websocket_starting = False
        
        # 会话状态
        self.session_id = ""
        self.websocket_started = False
        self.speech_interaction_mode = SpeechInteractionMode.kSpeechInteractionModeManual
        self.key_pin = None
        self.wake_up_xiaozhi = False
        self.gpio_triggle_thread_run = True
        self.people_wakeup_thread_run = True
        self.wakeup_audio_file = "/sdcard/examples/26-xiaozhi/resource/output_new.opus"
        self.reg_result = False
        self.reg_name = ""
        
    def get_file_info(self, file_path):
        """获取文件大小（字节）"""
        try:
            file_stat = os.stat(file_path)
            file_size = file_stat[6]  # stat 第6位是文件大小（字节）
            print(f"文件 {file_path} 大小：{file_size} 字节")
            return file_size
        except OSError as e:
            print(f"获取文件信息失败：{e}")
            return None
        
    def update_reg_result(self, reg_result, reg_name):
        self.reg_result = reg_result
        self.reg_name = reg_name
                      
    def gpio_triggle_thread(self):
        time.sleep(1)
        gpio_triggle = False
        while(self.gpio_triggle_thread_run):
            if(self.key_pin.value() == 0):
                time.sleep_ms(10)
                if(self.key_pin.value() == 0):
                    self.pin_led.value(1)
                    if not self.websocket_started:
                        self.status_callback("网络连接中，请稍后。。。。")
                        self.connect_to_server()
                        continue
                    
                    if not self.audio_manager.get_recorde_status():
                        self.start_listening(mode=ListenMode.MODE_MANUAL)
                        self.status_callback("等待您的指令")
                        gpio_triggle = True
                        self.audio_manager.start_recording(self.send_audio)
                    time.sleep_ms(100)
            else:
                if gpio_triggle:
                    self.pin_led.value(0)
                    gpio_triggle = False
                    if self.audio_manager.stop_recording() == 0:
                        self.stop_listening()
                        self.status_callback("获取指令结束")
                        time.sleep(1)
                    self.status_callback("等待按键唤醒")
                else:
                    time.sleep_ms(500)
                
            if(self.websocket_started and not self.wake_up_xiaozhi and session_manager.can_send_audio()):
                self.send_iot()
                self.send_iot_state()
                self.wake_up_xiaozhi = True
                self.status_callback("等待按键唤醒")

    def people_wakeup_thread(self):
        time.sleep(1)
        last_reg_result = self.reg_name
        while(self.people_wakeup_thread_run):
            if self.reg_result:
                if not self.websocket_started:
                    time.sleep_ms(100)
                    continue
                if not self.audio_manager.get_recorde_status():
                    if self.reg_name != "" and self.reg_name != last_reg_result:
                        wakeup_text = f"你好小智，我是{self.reg_name}"
                        self.send_wakeup_detect(wakeup_text)
                        last_reg_result = self.reg_name
                        time.sleep_ms(2000)
            else:
                last_reg_result = ""
                time.sleep_ms(500)
                            
    def init_device(self, audio_callback, tts_callback, ws_state_callback,
                   key_pin=21, led_pin=52):
        """初始化设备 - 根据通信协议

        Args:
            audio_callback: 音频下载回调
            tts_callback: TTS 状态回调
            ws_state_callback: WebSocket 状态回调
            key_pin: 对话按键 GPIO 编号（低电平有效），默认 21
            led_pin: 状态指示灯 GPIO 编号，默认 52
        """
        print("初始化小智语音助手设备...")
        print("按键GPIO: %d, LED GPIO: %d" % (key_pin, led_pin))
        fpioa = FPIOA()

        key_func = getattr(FPIOA, "GPIO%d" % key_pin)
        led_func = getattr(FPIOA, "GPIO%d" % led_pin)
        fpioa.set_function(key_pin, key_func)
        fpioa.set_function(led_pin, led_func)

        self.pin_led = Pin(led_pin, Pin.OUT, pull=Pin.PULL_UP, drive=7)
        self.pin_led.off()
        self.key_pin = Pin(key_pin, Pin.IN, pull=Pin.PULL_UP, drive=7)

        self.thing_manager.initialize_iot_devices()
        self.audio_upload_callback = audio_callback
        self.tts_state_callback = tts_callback
        self.ws_state_callback = ws_state_callback
        
        # 加载配置
        if not self.config.load_config():
            print("加载配置失败，使用默认配置")
            return -1
            
        # 设置WebSocket回调
        self.ws_client.set_callbacks(
            self._process_opus_data,
            self._process_text_data,
            self._ws_state_changed
        )
        
        # 设置WebSocket连接参数
        ws_config = self.config.get_websocket_config()
        self.ws_client.set_config(ws_config)
        
        print("设备初始化完成")
        return 0
    def register_update_callback(self, text_callback, llm_callback, status_callback):
        self.text_callback = text_callback
        self.llm_callback = llm_callback
        self.status_callback = status_callback  
        
    def active_device(self):
        """激活设备 - 根据通信协议"""
        print("开始设备激活...")
        
        # 构建激活请求数据
        http_data = {
            "url": self.config.ota_url,
            "post": """{
            "uuid":"%s",
            "application": {
                "name": "xiaozhi_linux_k230",
                "version": "1.0.0"
            },
            "ota": {
            },
            "board": {
                "type": "k230_linux_board",
                "name": "k230_linux_board"
            }
        }""" % self.config.uuid,
            "headers": """{
            "Content-Type": "application/json",
            "Device-Id": "%s",
            "User-Agent": "canaan",
            "Accept-Language": "zh-CN"
        }""" % self.config.mac_address
        }
        
        # 激活码缓冲区
        code_buffer = bytearray(20)
        
        # 不断尝试激活，直到成功
        while True:
            # 调用HTTP客户端进行设备激活
            result = self.http_client.active_device(http_data, code_buffer)
            
            if result == 0:
                # 激活成功
                print("设备激活成功")
                return 0
            elif result == 1:
                # 需要激活码
                active_code = code_buffer.decode().rstrip('\x00')
                if active_code:
                    print("设备需要激活，激活码: %s" % active_code)
                    # 这里可以添加显示激活码的逻辑
                else:
                    print("设备激活失败，需要激活码但未收到")
            else:
                # 激活失败
                print("设备激活失败")
                
            # 等待5秒后重试
            print("5秒后重试激活...")
            time.sleep(5)

    def connect_to_server(self):
        """连接到小智服务器 - 根据通信协议"""
        if self.websocket_starting or self.websocket_started:
            return 0
        self.wake_up_xiaozhi = False
        self.websocket_starting = True
        print("启动WebSocket连接...")
        self.ws_client.start()
        self.websocket_started = True
        return 0
           
    def start_key_trigle_thread(self):
        _thread.start_new_thread(self.gpio_triggle_thread, ())
        
    def start_face_wakeup_thread(self):
        _thread.start_new_thread(self.people_wakeup_thread, ())
        
    def send_audio(self, audio_data, size, user_data=None):
        """发送音频数据 - 根据通信协议"""
        if not self.websocket_started or not session_manager.can_send_audio():
            return 0
        return self.ws_client.send_binary(audio_data, size)
    
    def send_wakeup_audio(self):
        print("self.send_wakeup_audio start")
        with open(self.wakeup_audio_file, 'rb') as f:
                file_size = self.get_file_info(self.wakeup_audio_file)
                buffer = f.read()
                print("self.send_wakeup_audio send_audio")
                self.send_audio(buffer, file_size)
                
                
    def abort_session(self):
        """终止当前会话 - 根据通信协议"""
        if not self.websocket_started:
            return -1
            
        abort_message = MessageBuilder.build_abort_message(
            session_id=self.session_id,
            reason="wake_word_detected"
        )
        
        return self.ws_client.send_text(ujson.dumps(abort_message))
        
    def start_listening(self, mode="auto"):
        """开始监听 - 根据通信协议"""
        if not self.websocket_started:
            return -1
            
        listen_message = MessageBuilder.build_listen_start_message(
            session_id=self.session_id,
            mode=mode
        )
        
        return self.ws_client.send_text(ujson.dumps(listen_message))
    
    def send_iot(self):
        if not self.websocket_started:
            return -1
        descriptors = self.thing_manager.get_descriptors()
        print(descriptors)  
        iot_message = MessageBuilder.build_iot_message(
            session_id=self.session_id,
            descriptors=descriptors
        )
        
        return self.ws_client.send_text(ujson.dumps(iot_message))
 
    def send_iot_state(self, status_json=None):
        if not self.websocket_started:
            return -1
        
        if status_json is None:
            _, states = self.thing_manager.get_states()
            print(states)     
            iot_state_message = MessageBuilder.build_iot_states_message(
                session_id=self.session_id,
                states_data=states
            )
            return self.ws_client.send_text(ujson.dumps(iot_state_message))
        else:
            return self.ws_client.send_text(status_json)
       
    def stop_listening(self):
        """停止监听 - 根据通信协议"""
        print("stop_listening")
        if not self.websocket_started:
            return -1
            
        listen_message = MessageBuilder.build_listen_stop_message(
            session_id=self.session_id
        )
        print(f"send message {listen_message}")
        return self.ws_client.send_text(ujson.dumps(listen_message))
        
    def send_wakeup_detect(self, wakeup_text):
        """发送唤醒词检测 - 根据通信协议"""
        if not self.websocket_started:
            return -1
            
        wakeup_message = MessageBuilder.build_wakeup_detect_message(
            session_id=self.session_id,
            wakeup_text=wakeup_text
        )
        
        return self.ws_client.send_text(ujson.dumps(wakeup_message))
        
    def set_interaction_mode(self, mode):
        """设置交互模式 - 根据通信协议"""
        self.speech_interaction_mode = mode
        session_manager.set_interaction_mode(mode)
        
        # 根据模式设置相应的监听模式
        if mode == SpeechInteractionMode.kSpeechInteractionModeAuto:
            self.listen_mode = ListenMode.MODE_AUTO
        elif mode == SpeechInteractionMode.kSpeechInteractionModeManual:
            self.listen_mode = ListenMode.MODE_MANUAL
        elif mode == SpeechInteractionMode.kSpeechInteractionModeRealtime:
            self.listen_mode = ListenMode.MODE_REALTIME
        elif mode == SpeechInteractionMode.kSpeechInteractionModeAutoWithWakeupWord:
            self.listen_mode = ListenMode.MODE_AUTO
            
        print("交互模式设置为: %d" % mode)
        
    def _process_opus_data(self, data, size):
        """处理接收到的Opus音频数据 - 根据通信协议"""
        self.audio_manager.play_opus_stream(data, size)
        if self.audio_upload_callback:
            self.audio_upload_callback(data, size)
            
    def _process_text_data(self, data, size):
        """处理接收到的文本数据 - 根据通信协议"""
        #try:
        # 使用消息解析器解析消息
        message = MessageParser.parse_message(data)
        self._handle_server_message(message)
        #except Exception as e:
        #    print("处理服务器消息失败: %s" % e)
            
    def _handle_server_message(self, message):
        """处理服务器消息 - 根据通信协议"""
        msg_type = message.get('type')
        
        if msg_type == MessageType.MESSAGE_HELLO:
            self._handle_hello_message(message)
        elif msg_type == MessageType.MESSAGE_TTS:
            self._handle_tts_message(message)
        elif msg_type == MessageType.MESSAGE_STT:
            self._handle_stt_message(message)
        elif msg_type == MessageType.MESSAGE_LLM:
            self._handle_llm_message(message)
        elif msg_type == MessageType.MESSAGE_IOT:
            self._handle_iot_message(message)
        elif msg_type == MessageType.MESSAGE_MCP:
            self._hanle_mcp_message(message)
        else:
            print("收到未知类型消息: %s" % msg_type)
    
    def _handle_iot_message(self, message):
        """{"type":"iot","commands":[{"name":"Speaker","method":"setvolume","parameters":{"volume":50}}],"session_id":"f594920b"}"""
        print(f"收到iot消息 {message}")
        commands = message.get("commands", [])
        if not commands:
            return
        
        for command in commands:
            
            result = self.thing_manager.invoke(command)
            changed, states_json = self.thing_manager.get_states_json(delta=True)
            if changed:
                self.send_iot_state(states_json)
    
    def _hanle_mcp_message(self, message):
        print(f"收到mcp消息 {message}")
        pass
               
    def _handle_hello_message(self, message):
        """处理hello消息 - 根据通信协议"""
        print(f"收到服务器hello消息 {message}")
        
        # 提取音频参数
        audio_params = message.get('audio_params', {})
        sample_rate = audio_params.get('sample_rate', 16000)
        channels = audio_params.get('channels', 1)
        frame_duration = audio_params.get('frame_duration', 60)
        self.session_id = message.get('session_id', "")
        
        print("服务器音频参数: 采样率=%dHz, 通道=%d, 帧长=%dms session_id=%s" % 
              (sample_rate, channels, frame_duration, self.session_id))
        
        # 开始会话
        session_manager.start_session(self.session_id)
        
        # 根据当前交互模式发送开始监听请求
        if self.speech_interaction_mode == SpeechInteractionMode.kSpeechInteractionModeAuto:
            self.start_listening(mode=ListenMode.MODE_AUTO)
        elif self.speech_interaction_mode == SpeechInteractionMode.kSpeechInteractionModeManual:
            self.start_listening(mode=ListenMode.MODE_MANUAL)
        elif self.speech_interaction_mode == SpeechInteractionMode.kSpeechInteractionModeRealtime:
            self.start_listening(mode=ListenMode.MODE_REALTIME)
        elif self.speech_interaction_mode == SpeechInteractionMode.kSpeechInteractionModeAutoWithWakeupWord:
            self.start_listening(mode=ListenMode.MODE_AUTO)
            
    def _handle_tts_message(self, message):
        """处理TTS消息 - 根据通信协议"""
        state = message.get('state')
        text = message.get('text', '')
        
        print("收到TTS消息: state=%s, text=%s" % (state, text))
        
        if state == TTSState.TTS_START:
            # 开始TTS播放，停止录音
            session_manager.set_audio_upload_enabled(False)
            if self.audio_manager.get_recorde_status():
                self.audio_manager.stop_recording()
            self.audio_manager.stop_playing()
            if self.tts_state_callback:
                self.tts_state_callback(1)
                
        elif state == TTSState.TTS_STOP:
            # TTS播放结束，恢复录音
            session_manager.set_audio_upload_enabled(True)
            if self.tts_state_callback:
                self.tts_state_callback(0)
                
            # 根据交互模式决定是否重新开始监听
            if self.speech_interaction_mode in [
                SpeechInteractionMode.kSpeechInteractionModeAuto,
                SpeechInteractionMode.kSpeechInteractionModeAutoWithWakeupWord
            ]:
                # 等待一会避免误触发
                time.sleep(2)
                self.start_listening(mode=ListenMode.MODE_AUTO)
                
        elif state == TTSState.TTS_SENTENCE_START:
            # 句子开始，更新显示文本
            print("TTS句子开始: %s" % text)
            
        if self.text_callback:
            self.text_callback(text)
            
    def _handle_stt_message(self, message):
        """处理STT消息 - 根据通信协议"""
        text = message.get('text', '')
        if self.text_callback:
            self.text_callback(text)
        
    def _handle_llm_message(self, message):
        """处理LLM情感消息 - 根据通信协议"""
        emotion = message.get('emotion', 'neutral')
        print("情感状态: %s" % emotion)
        if self.llm_callback:
            self.llm_callback(emotion)
        
    def _ws_state_changed(self, connected):
        """WebSocket状态变化回调 - 根据通信协议"""
        print("WebSocket连接状态: %s" % ("已连接" if connected else "已断开"))
        if self.ws_state_callback:
            self.ws_state_callback(connected)
            
        if connected:
            # 连接成功，会话开始
            self.websocket_starting = False
            self.websocket_started = True
            session_manager.start_session()
        else:
            # 连接断开，会话结束；停录音避免卡在「已经在录音中」
            session_manager.stop_session()
            self.websocket_started = False
            self.websocket_starting = False
            self.wake_up_xiaozhi = False
            try:
                if self.audio_manager.get_recorde_status():
                    self.audio_manager.stop_recording()
            except:
                pass
            
            
    def deinit_device(self):
        self.gpio_triggle_thread_run = False
        self.people_wakeup_thread_run = False
        try:
            self.ws_client.stop()
        except Exception as e:
            print("ws stop: %s" % e)
        # 给 WS/GPIO/人脸唤醒线程一点时间退出，再关音频
        time.sleep(0.15)
        try:
            self.audio_manager.release_audio()
        except Exception as e:
            print("release_audio: %s" % e)