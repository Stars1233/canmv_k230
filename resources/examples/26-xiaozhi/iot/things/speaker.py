from iot.thing import Thing, Parameter
from audio_manager import AudioManager

class Speaker(Thing):
    def __init__(self):
        super().__init__("Speaker", "音箱")
        print("Speaker init")
        self.volume = 0
        self.audio_manager = AudioManager.get_instance()

        # 定义属性
        self.add_property("volume", "当前音量多少", self.get_volume)
        volume_param = Parameter("volume", "音量大小，0-100之间的整数", "number")
        params = []
        params.append(volume_param)
        # 定义方法
        self.add_method("setvolume", "设置音量", params, self.set_volume)

    def get_volume(self):
        self.volume = self.audio_manager.getVolume()[0]
        return self.volume

    def set_volume(self, params):
        for name, value in params.items():
            self.volume = value.get_value()
            self.audio_manager.setVolume(self.volume)
            print(f"set_volume {self.volume}")
        return {"status": "success", "message": "设置音量成功"}
