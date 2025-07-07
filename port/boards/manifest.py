include("$(MPY_DIR)/extmod/asyncio")

# Require some micropython-lib modules.
require("base64")
require("binascii")
require("gzip")
require("hashlib")
require("heapq")
require("mip")
require("umqtt.simple")

require("dht")

# canmv media modules
package("mpp", base_path="../builtin_py", opt=3)
package("media", base_path="../builtin_py", opt=3)
