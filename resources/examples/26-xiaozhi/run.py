#!/usr/bin/env python3
"""
小智语音助手 - MicroPython版本启动脚本
"""
import sys
sys.path.append("/sdcard/examples/26-xiaozhi")

import time
from xiaozhiclient import XiaoZhiClient
from lvgl_ai import XiaoZhi_UI
import lvgl as lv

# ========================= 用户配置 =========================
# 项目运行参数统一在这里修改，其他文件不需要修改。
PROJECT_CONFIG = {
    "hardware": {
        # 对话按键 GPIO（低电平有效，按住说话 / 松开结束）
        "key_pin": 21,
        # 状态指示灯 GPIO（按键按下时点亮）
        "led_pin": 52,
    },
    "network": {
        # 可选值：default、lan、wifi_sta
        "type": "wifi_sta",
        # Wi-Fi 设备可选值：auto、usb、sdio、spi
        "wlan_device": "auto",
        "timeout": 20,
        "ssid": "TEST",
        "password": "12345678",
    },
    "xiaozhi": {
        "ota_url": "https://api.tenclass.net/xiaozhi/ota/",
        "websocket_url": "wss://api.tenclass.net:443/xiaozhi/v1/",
    },
    "webtts": {
        "app_id": "",
        "api_key": "",
        "api_secret": "",
        "sample_rate": 16000,
        "voice_name": "xiaoyan",
        "speed": 50,
    },
}
# ============================================================


def main(project_config=PROJECT_CONFIG):
    """运行小智项目。"""
    hardware = project_config["hardware"]
    print("=" * 50)
    print("小智语音助手 - MicroPython版本")
    print("按键GPIO=%d, LED GPIO=%d" %
          (hardware["key_pin"], hardware["led_pin"]))
    print("=" * 50)
    
    
    # 回调函数定义
    def audio_download_callback(data, size):
        """音频下载回调"""
        pass
        #print(f"收到服务器音频数据: {size} bytes")
        # 在这里播放音频

    def status_callback(status):
        xiaozhi_gui.Update_status(status)
               
    def tts_state_callback(state):
        """TTS状态回调"""
        if state == 1:
            print("TTS开始播放")
            xiaozhi_gui.set_tts_playing(True)
            xiaozhi_gui.Update_status("说话中")
        else:
            print("TTS播放结束")
            xiaozhi_gui.set_tts_playing(False)
            xiaozhi_gui.Update_status("说话结束")
            
    def ws_state_callback(connected):
        """WebSocket状态回调"""
        if not xiaozhi_gui.ui_alive:
            return
        if connected:
            print("WebSocket连接成功")
            xiaozhi_gui.Update_status("网络已连接")
        else:
            print("WebSocket连接断开")
            xiaozhi_gui.Update_status("网络连接中断")
            time.sleep(1)
            if xiaozhi_gui.ui_alive:
                xiaozhi_gui.Update_status("等待按键唤醒")
            
    def text_callback(text):
        xiaozhi_gui.Update_text(text)
        
    def llm_callback(llm):
        xiaozhi_gui.Update_llm(llm)
        
    xiaozhi_client = XiaoZhiClient.get_instance(project_config)
    xiaozhi_gui = XiaoZhi_UI()
    xiaozhi_gui.user_gui_init()
    
    # 初始化设备
    print("正在初始化设备...")
    if xiaozhi_client.init_device(
            audio_download_callback,
            tts_state_callback,
            ws_state_callback) != 0:
        print("设备初始化失败")
        return
        
    print("设备初始化成功")
    
    # 激活设备
    print("正在激活设备...")
    if xiaozhi_client.active_device() != 0:
        print("设备激活失败")
        return
        
    print("设备激活成功")
    
    # 连接到服务器
    print("正在连接到小智服务器...")
    xiaozhi_client.connect_to_server()
    
    xiaozhi_client.register_update_callback(text_callback, llm_callback, status_callback)
    
    xiaozhi_client.start_key_trigle_thread()
    
    xiaozhi_client.start_face_wakeup_thread()
    
    print("小智语音助手已启动，按Ctrl+C退出")
    
    try:
        # 主循环
        while True:
            xiaozhi_gui.flush_ui_updates()
            time.sleep_ms(lv.task_handler())
            if xiaozhi_gui.get_person():
                xiaozhi_client.connect_to_server()
            reg_result, reg_name =  xiaozhi_gui.get_reg_result()
            xiaozhi_client.update_reg_result(reg_result, reg_name)
    except KeyboardInterrupt:
        print("\n正在关闭小智语音助手...")
    except Exception as e:
        # CanMV IDE 停止按钮抛的是 Exception("IDE interrupt")，不是 KeyboardInterrupt
        if "IDE interrupt" in str(e):
            print("\n正在关闭小智语音助手...")
        else:
            raise
    finally:
        # 停人脸 → 停 WS/音频；Display/LVGL 交给 [mpy] exit, reset，避免 lv already deinited
        try:
            xiaozhi_gui.begin_exit()
        except Exception as e:
            print("begin_exit: %s" % e)
        try:
            xiaozhi_client.deinit_device()
        except Exception as e:
            print("deinit_device: %s" % e)
        try:
            xiaozhi_gui.user_gui_deinit(release_display=False)
        except Exception as e:
            print("user_gui_deinit: %s" % e)
        print("小智语音助手已关闭")

if __name__ == "__main__":
    main()
