"""
状态管理模块
根据通信协议重新定义设备状态、消息类型和监听模式
"""

class DeviceState:
    """设备状态枚举 - 根据通信协议"""
    kDeviceStateUnknown = 0
    kDeviceStateIdle = 1
    kDeviceStateListening = 2
    kDeviceStateSpeaking = 3
    kDeviceStateActivating = 4

class SpeechInteractionMode:
    """语音交互模式枚举 - 根据通信协议"""
    kSpeechInteractionModeManual = 0      # 手动模式
    kSpeechInteractionModeAuto = 1        # 自动模式
    kSpeechInteractionModeRealtime = 2    # 实时模式
    kSpeechInteractionModeAutoWithWakeupWord = 3  # 自动+唤醒词模式

class ListeningMode:
    """监听模式枚举 - 根据通信协议"""
    kListeningModeManualStop = 0   # 手动停止
    kListeningModeAutoStop = 1     # 自动停止
    kListeningModeAlwaysOn = 2     # 持续监听

class MessageType:
    """消息类型枚举 - 根据通信协议"""
    MESSAGE_HELLO = "hello"
    MESSAGE_LISTEN = "listen"
    MESSAGE_TTS = "tts"
    MESSAGE_ABORT = "abort"
    MESSAGE_MCP = "mcp"
    MESSAGE_LLM = "llm"
    MESSAGE_STT = "stt"
    MESSAGE_IOT = 'iot'

class TTSState:
    """TTS状态枚举 - 根据通信协议"""
    TTS_START = "start"
    TTS_STOP = "stop"
    TTS_SENTENCE_START = "sentence_start"

class ListenState:
    """监听状态枚举 - 根据通信协议"""
    LISTEN_START = "start"
    LISTEN_STOP = "stop"
    LISTEN_DETECT = "detect"

class ListenMode:
    """监听模式枚举 - 根据通信协议"""
    MODE_AUTO = "auto"        # 自动停止
    MODE_MANUAL = "manual"    # 手动停止
    MODE_REALTIME = "realtime" # 持续监听

class MessageBuilder:
    """消息构建器 - 根据通信协议构建各种消息"""
    
    @staticmethod
    def build_hello_message():
        """构建hello消息 - 根据通信协议"""
        return {
            "type": "hello",
            "version": 1,
            "transport": "websocket",
            "features" : {
                "mcp": True
            },
            "audio_params": {
                "format": "opus",
                "sample_rate": 16000,
                "channels": 1,
                "frame_duration": 60
            }
        }   
    @staticmethod
    def build_listen_start_message(session_id="", mode="auto"):
        """构建开始监听消息 - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "listen",
            "state": "start",
            "mode": mode
        }
    
    @staticmethod
    def build_listen_stop_message(session_id=""):
        """构建停止监听消息 - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "listen",
            "state": "stop"
        }
    
    @staticmethod
    def build_wakeup_detect_message(session_id="", wakeup_text=""):
        """构建唤醒词检测消息 - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "listen",
            "state": "detect",
            "text": wakeup_text
        }
        
    @staticmethod
    def build_iot_message(session_id="", descriptors=""):
        """构建IOT - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "iot",
            "update": True,
            "descriptors": descriptors
        }
 
    @staticmethod
    def build_iot_states_message(session_id="", states_data=""):
        """构建MCP IOT - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "iot",
            "update": True,
            "states": states_data
        }
               
    @staticmethod
    def build_abort_message(session_id="", reason="wake_word_detected"):
        """构建中止消息 - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "abort",
            "reason": reason
        }
    
    @staticmethod
    def build_mcp_message(session_id="", payload=None):
        """构建MCP消息 - 根据通信协议"""
        return {
            "session_id": session_id,
            "type": "mcp",
            "payload": payload or {}
        }

class MessageParser:
    """消息解析器 - 根据通信协议解析接收到的消息"""
    
    @staticmethod
    def parse_message(message_data):
        """解析消息 - 根据通信协议"""
        #try:
        import ujson
        message = ujson.loads(message_data)
        msg_type = message.get("type", "")
        
        if msg_type == MessageType.MESSAGE_HELLO:
            return MessageParser._parse_hello_message(message)
        elif msg_type == MessageType.MESSAGE_TTS:
            return MessageParser._parse_tts_message(message)
        elif msg_type == MessageType.MESSAGE_LLM:
            return MessageParser._parse_llm_message(message)
        elif msg_type == MessageType.MESSAGE_STT:
            return MessageParser._parse_stt_message(message)
        elif msg_type == MessageType.MESSAGE_IOT:
            return MessageParser._parse_iot_message(message)
        elif msg_type == MessageType.MESSAGE_MCP:
            return {"type": "mcp", "data": message}
        else:
            return {"type": "unknown", "data": message}
                
        #except Exception as e:
        #    print("解析消息失败: %s" % e)
        #    return {"type": "error", "error": str(e)}
    
    @staticmethod
    def _parse_hello_message(message):
        """解析hello消息 - 根据通信协议"""
        result = {
            "type": MessageType.MESSAGE_HELLO,
            "audio_params": message.get("audio_params", {}),
            "session_id": message.get("session_id", "")
        }
        
        # 提取音频参数
        audio_params = message.get("audio_params", {})
        result["sample_rate"] = audio_params.get("sample_rate", 16000)
        result["channels"] = audio_params.get("channels", 1)
        result["frame_duration"] = audio_params.get("frame_duration", 60)
        
        return result
    
    @staticmethod
    def _parse_tts_message(message):
        """解析TTS消息 - 根据通信协议"""
        result = {
            "type": MessageType.MESSAGE_TTS,
            "state": message.get("state", ""),
            "text": message.get("text", "")
        }
        
        # 根据TTS状态设置设备状态
        tts_state = message.get("state", "")
        if tts_state == TTSState.TTS_START:
            result["device_state"] = DeviceState.kDeviceStateSpeaking
        elif tts_state == TTSState.TTS_STOP:
            result["device_state"] = DeviceState.kDeviceStateListening
        elif tts_state == TTSState.TTS_SENTENCE_START:
            result["device_state"] = DeviceState.kDeviceStateSpeaking
            
        return result
    
    @staticmethod
    def _parse_iot_message(message):
        """解析IOT消息 - 根据通信协议"""
        """opcode=1 payload=b'{"type":"iot","commands":[{"name":"Speaker","method":"setvolume","parameters":{"volume":50}}],"session_id":"f594920b"}"""
        result = {
            "type": MessageType.MESSAGE_IOT,
            "commands": message.get("commands", "")
        }
                   
        return result
    
    @staticmethod
    def _parse_llm_message(message):
        """解析LLM情感消息 - 根据通信协议"""
        return {
            "type": MessageType.MESSAGE_LLM,
            "emotion": message.get("emotion", "neutral")
        }
    
    @staticmethod
    def _parse_stt_message(message):
        """解析STT消息 - 根据通信协议"""
        return {
            "type": MessageType.MESSAGE_STT,
            "text": message.get("text", "")
        }

class SessionManager:
    """会话管理器 - 管理会话状态和生命周期"""
    
    def __init__(self):
        self.session_id = ""
        self.is_active = False
        self.audio_upload_enabled = True
        self.current_mode = SpeechInteractionMode.kSpeechInteractionModeAuto
        
    def start_session(self, session_id=""):
        """开始会话"""
        self.session_id = session_id
        self.is_active = True
        self.audio_upload_enabled = True
        print("会话开始: %s" % session_id)
    
    def stop_session(self):
        """停止会话"""
        self.is_active = False
        self.audio_upload_enabled = False
        print("会话结束")
    
    def set_audio_upload_enabled(self, enabled):
        """设置音频上传状态"""
        self.audio_upload_enabled = enabled
    
    def can_send_audio(self):
        """检查是否可以发送音频"""
        return self.is_active and self.audio_upload_enabled
    
    def set_interaction_mode(self, mode):
        """设置交互模式"""
        self.current_mode = mode
        
        # 根据模式设置相应的监听模式
        if mode == SpeechInteractionMode.kSpeechInteractionModeAuto:
            self.listen_mode = ListenMode.MODE_AUTO
        elif mode == SpeechInteractionMode.kSpeechInteractionModeManual:
            self.listen_mode = ListenMode.MODE_MANUAL
        elif mode == SpeechInteractionMode.kSpeechInteractionModeRealtime:
            self.listen_mode = ListenMode.MODE_REALTIME
        elif mode == SpeechInteractionMode.kSpeechInteractionModeAutoWithWakeupWord:
            self.listen_mode = ListenMode.MODE_AUTO

# 全局会话管理器实例
session_manager = SessionManager()
