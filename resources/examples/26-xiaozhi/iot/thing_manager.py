import ujson

from iot.thing import Thing


class ThingManager:
    _instance = None

    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = ThingManager()
        return cls._instance

    def __init__(self):
        self.things = []
        self.last_states = {}  # 添加状态缓存字典，存储上一次的状态

    def initialize_iot_devices(self):
        """初始化物联网设备.

        注意：倒计时器功能已迁移到MCP工具中，提供更好的AI集成和状态反馈。
        """
        from iot.things.speaker import Speaker
        print("initialize_iot_devices")
        # 添加设备
        self.add_thing(Speaker())

    def add_thing(self, thing):
        self.things.append(thing)

    def get_descriptors_json(self):
        """
        获取所有设备的描述符JSON.
        """
        # 由于get_descriptor_json()是同步方法（返回静态数据），
        # 这里保持简单的同步调用即可
        descriptors = [thing.get_descriptor_json() for thing in self.things]
        return ujson.dumps(descriptors)

    def get_descriptors(self):
        """
        获取所有设备的描述符JSON.
        """
        # 由于get_descriptor_json()是同步方法（返回静态数据），
        # 这里保持简单的同步调用即可
        descriptors = [thing.get_descriptor_json() for thing in self.things]
        return descriptors

    def get_states(self, delta=False):
        """获取所有设备的状态JSON.

        Args:
            delta: 是否只返回变化的部分，True表示只返回变化的部分

        Returns:
            Tuple[bool, list]: 是否有状态变化，以及状态列表
        """
        if not delta:
            self.last_states.clear()

        changed = False

        states_results = [thing.get_state_json() for thing in self.things]

        states = []
        for i, thing in enumerate(self.things):
            state_json = states_results[i]

            if delta:
                # 检查状态是否变化
                is_same = (
                    thing.name in self.last_states
                    and self.last_states[thing.name] == state_json
                )
                if is_same:
                    continue
                changed = True
                self.last_states[thing.name] = state_json

            states.append(state_json)

        return changed, states

    def get_states_json(self, delta=False):
        """获取所有设备的状态JSON.

        Args:
            delta: 是否只返回变化的部分，True表示只返回变化的部分

        Returns:
            Tuple[bool, str]: 是否有状态变化，以及 JSON 字符串
        """
        changed, states = self.get_states(delta=delta)
        return changed, ujson.dumps(states)

    def get_states_json_str(self):
        """
        为了兼容旧代码，保留原来的方法名和返回值类型.
        """
        # get_states_json 为同步方法，不可使用 await（本工程无 asyncio 事件循环）
        _, json_str = self.get_states_json(delta=False)
        return json_str

    def invoke(self, command):
        """调用设备方法.

        Args:
            command: 包含name和method等信息的命令字典
            {"name":"Speaker","method":"setvolume","parameters":{"volume":50}}

        Returns:
            Optional[Any]: 如果找到设备并调用成功，返回调用结果；否则抛出异常
        """
        thing_name = command.get("name")
        for thing in self.things:
            if thing.name == thing_name:
                return thing.invoke(command)

        # 记录错误日志
        print(f"设备不存在: {thing_name}")
        raise ValueError(f"设备不存在: {thing_name}")
