import ujson


class ValueType:
    BOOLEAN = "boolean"
    NUMBER = "number"
    STRING = "string"
    FLOAT = "float"
    ARRAY = "array"  # 新增
    OBJECT = "object"  # 新增
    LIST = "array"  # LIST 作为 ARRAY 的别名


class Property:
    def __init__(self, name, description, getter):
        self.name = name
        self.description = description
        self.getter = getter

        self.type = ValueType.STRING  # 默认类型
        self._type_determined = False
        self.get_state_value()

    def _determine_type(self, value):
        """
        根据值确定属性类型.
        """
        if isinstance(value, bool):
            self.type = ValueType.BOOLEAN
        elif isinstance(value, int):
            self.type = ValueType.NUMBER
        elif isinstance(value, float):
            self.type = ValueType.FLOAT
        elif isinstance(value, str):
            self.type = ValueType.STRING
        elif isinstance(value, (list, tuple)):
            self.type = ValueType.ARRAY
        elif isinstance(value, dict):
            self.type = ValueType.OBJECT
        else:
            raise TypeError(f"不支持的属性类型: {type(value)}")

    def get_descriptor_json(self):
        return {"description": self.description, "type": self.type}

    def get_state_value(self):
        """
        获取属性值.
        """
        value = self.getter()
        # 如果是第一次调用 getter，确定类型
        if not self._type_determined:
            self._determine_type(value)
            self._type_determined = True
        return value


class Parameter:
    def __init__(self, name, description, type_, required = True):
        self.name = name
        self.description = description
        self.type = type_
        self.required = required
        self.value = None

    def get_descriptor_json(self):
        return {"description": self.description, "type": self.type}

    def set_value(self, value):
        self.value = value

    def get_value(self):
        return self.value


class Method:
    def __init__(
        self,
        name,
        description,
        parameters,
        callback,
    ):
        self.name = name
        self.description = description
        self.parameters = {param.name: param for param in parameters}
        self.callback = callback

    def get_descriptor_json(self):
        return {
            "description": self.description,
            "parameters": {
                name: param.get_descriptor_json()
                for name, param in self.parameters.items()
            },
        }

    def invoke(self, params):
        """
        调用方法.
        """
        # 设置参数值，处理复杂类型
        for name, value in params.items():
            if name in self.parameters:
                param = self.parameters[name]
                # 如果参数类型是STRING，但值是dict或list，转换为JSON字符串（类似C++版本）
                if param.type == ValueType.STRING and isinstance(value, (dict, list)):
                    param.set_value(ujson.dumps(value, ensure_ascii=False))
                else:
                    param.set_value(value)

        # 检查必需参数
        for name, param in self.parameters.items():
            if param.required and param.get_value() is None:
                raise ValueError(f"缺少必需参数: {name}")

        # 调用异步回调函数
        return self.callback(self.parameters)


class Thing:
    def __init__(self, name, description):
        self.name = name
        self.description = description
        self.properties = {}
        self.methods = {}

    def add_property(self, name, description, getter):
        self.properties[name] = Property(name, description, getter)

    def add_method(
        self,
        name,
        description,
        parameters,
        callback,
    ):
        self.methods[name] = Method(name, description, parameters, callback)

    def get_descriptor_json(self):
        return {
            "name": self.name,
            "description": self.description,
            "properties": {
                name: prop.get_descriptor_json()
                for name, prop in self.properties.items()
            },
            "methods": {
                name: method.get_descriptor_json()
                for name, method in self.methods.items()
            },
        }

    def get_state_json(self):
        """
        获取设备状态.
        """
        state = {}
        for name, prop in self.properties.items():
            state[name] = prop.get_state_value()

        return {
            "name": self.name,
            "state": state,
        }

    def invoke(self, command):
        """
        调用方法. {'commands': [{'parameters': {'volume': 20}, 'name': 'Speaker', 'method': 'setvolume'}], 'type': 'iot'}
        """
        method_name = command.get("method")
        if method_name not in self.methods:
            raise ValueError(f"方法不存在: {method_name}")

        parameters = command.get("parameters", {})
        
        return self.methods[method_name].invoke(parameters)
