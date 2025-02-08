import ucryptolib
import collections

print('\n###################### SM4-ECB/CFB/OFB/CBC/CTR Test ##############################')
Sm4 = collections.namedtuple('Sm4', ['key', 'iv', 'pt', 'ct'])
sm4 = [
    # ecb
    Sm4(b'\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10',
        None,
        b'\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb',
        b'\x5e\xc8\x14\x3d\xe5\x09\xcf\xf7\xb5\x17\x9f\x8f\x47\x4b\x86\x19\x2f\x1d\x30\x5a\x7f\xb1\x7d\xf9\x85\xf8\x1c\x84\x82\x19\x23\x04'
        ),

    # cbc
    Sm4(
        b'\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10',
        b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f',
        b'\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb',
        b'\x78\xeb\xb1\x1c\xc4\x0b\x0a\x48\x31\x2a\xae\xb2\x04\x02\x44\xcb\x4c\xb7\x01\x69\x51\x90\x92\x26\x97\x9b\x0d\x15\xdc\x6a\x8f\x6d'
        ),
    
    # cfb
    Sm4(
        b'\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10',
        b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f',
        b'\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb',
        b'\xac\x32\x36\xcb\x86\x1d\xd3\x16\xe6\x41\x3b\x4e\x3c\x75\x24\xb7\x69\xd4\xc5\x4e\xd4\x33\xb9\xa0\x34\x60\x09\xbe\xb3\x7b\x2b\x3f'
        ),

    # ofb
    Sm4(
        b'\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10',
        b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f',
        b'\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xff\xff\xff\xff\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb',
        b'\xac\x32\x36\xcb\x86\x1d\xd3\x16\xe6\x41\x3b\x4e\x3c\x75\x24\xb7\x1d\x01\xac\xa2\x48\x7c\xa5\x82\xcb\xf5\x46\x3e\x66\x98\x53\x9b'
        ),

    # ctr
    Sm4(
        b'\x01\x23\x45\x67\x89\xab\xcd\xef\xfe\xdc\xba\x98\x76\x54\x32\x10',
        b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f',
        b'\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xcc\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xee\xee\xee\xee\xee\xee\xee\xee\xff\xff\xff\xff\xff\xff\xff\xff\xee\xee\xee\xee\xee\xee\xee\xee\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa',
        b'\xac\x32\x36\xcb\x97\x0c\xc2\x07\x91\x36\x4c\x39\x5a\x13\x42\xd1\xa3\xcb\xc1\x87\x8c\x6f\x30\xcd\x07\x4c\xce\x38\x5c\xdd\x70\xc7\xf2\x34\xbc\x0e\x24\xc1\x19\x80\xfd\x12\x86\x31\x0c\xe3\x7b\x92\x2a\x46\xb8\x94\xbe\xe4\xfe\xb7\x9a\x38\x22\x94\x0c\x93\x54\x05'
        )
]

print('********************** Test-1: keybits=128, ptlen=32 ******************')
print('SM4-ECB Encrypt......')
crypto = ucryptolib.sm4(sm4[0].key, 1)
inbuf = sm4[0].pt
outbuf = bytearray(32)
val = crypto.encrypt(inbuf, outbuf)
val0 = sm4[0].ct
print(val0 == val)
print('SM4-ECB Decrypt......')
crypto = ucryptolib.sm4(sm4[0].key, 1)
inbuf = sm4[0].ct
outbuf = bytearray(32)
val = crypto.decrypt(inbuf, outbuf)
val0 = sm4[0].pt
print(val0 == val)

print('********************** Test-2: keybits=128, ivlen=16, ptlen=32 ******************')
print('SM4-CBC Encrypt......')
crypto = ucryptolib.sm4(sm4[1].key, 2, sm4[1].iv)
inbuf = sm4[1].pt
outbuf = bytearray(32)
val = crypto.encrypt(inbuf, outbuf)
val0 = sm4[1].ct
print(val0 == val)
print('SM4-CBC Decrypt......')
crypto = ucryptolib.sm4(sm4[1].key, 2, sm4[1].iv)
inbuf = sm4[1].ct
val = crypto.decrypt(inbuf)
val0 = sm4[1].pt
print(val0 == val)
# outbuf = bytearray(32)
# val = crypto.decrypt(inbuf, outbuf)
# val0 = sm4[1].pt
# print(val0 == val)

print('********************** Test-3: keybits=128, ivlen=16, ptlen=32 ******************')
print('SM4-CFB Encrypt......')
crypto = ucryptolib.sm4(sm4[2].key, 3, sm4[2].iv)
inbuf = sm4[2].pt
outbuf = bytearray(32)
val = crypto.encrypt(inbuf, outbuf)
val0 = sm4[2].ct
print(val0 == val)
print('SM4-CFB Decrypt......')
crypto = ucryptolib.sm4(sm4[2].key, 3, sm4[2].iv)
inbuf = sm4[2].ct
outbuf = bytearray(32)
val = crypto.decrypt(inbuf, outbuf)
val0 = sm4[2].pt
print(val0 == val)

print('********************** Test-4: keybits=128, ivlen=16, ptlen=32 ******************')
print('SM4-OFB Encrypt......')
crypto = ucryptolib.sm4(sm4[3].key, 5, sm4[3].iv)
inbuf = sm4[3].pt
outbuf = bytearray(32)
val = crypto.encrypt(inbuf, outbuf)
val0 = sm4[3].ct
print(val0 == val)
print('SM4-OFB Decrypt......')
crypto = ucryptolib.sm4(sm4[3].key, 5, sm4[3].iv)
inbuf = sm4[3].ct
outbuf = bytearray(32)
val = crypto.decrypt(inbuf, outbuf)
val0 = sm4[3].pt
print(val0 == val)

print('********************** Test-5: keybits=128, ivlen=16, ptlen=64 ******************')
print('SM4-CTR Encrypt......')
crypto = ucryptolib.sm4(sm4[4].key, 6, sm4[4].iv)
inbuf = sm4[4].pt
outbuf = bytearray(64)
val = crypto.encrypt(inbuf, outbuf)
val0 = sm4[4].ct
print(val0 == val)
print('SM4-CTR Decrypt......')
crypto = ucryptolib.sm4(sm4[4].key, 6, sm4[4].iv)
inbuf = sm4[4].ct
outbuf = bytearray(64)
val = crypto.decrypt(inbuf, outbuf)
val0 = sm4[4].pt
print(val0 == val)
