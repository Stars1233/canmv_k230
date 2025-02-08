import ucryptolib
import collections

print('\n###################### AES-GCM Test ##############################')

# Aes = collections.namedtuple('Aes', ['key', 'iv', 'aad', 'pt', 'ct', 'tag'])
# aes = [
#     Aes(b'\xb5\x2c\x50\x5a\x37\xd7\x8e\xda\x5d\xd3\x4f\x20\xc2\x25\x40\xea\x1b\x58\x96\x3c\xf8\xe5\xbf\x8f\xfa\x85\xf9\xf2\x49\x25\x05\xb4',
#         b'\x51\x6c\x33\x92\x9d\xf5\xa3\x28\x4f\xf4\x63\xd7',
#         b'',
#         b'',
#         b'',
#         b'\xbd\xc1\xac\x88\x4d\x33\x24\x57\xa1\xd2\x66\x4f\x16\x8c\x76\xf0'
#         ),

#     Aes(b'\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f',
#         b'\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94',
#         b'\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef',
#         b'\x27',
#         b'\xeb',
#         b'\x63\x35\xe1\xd4\x9e\x89\x88\xea\xc4\x8e\x42\x19\x4e\x5f\x56\xdb'
#         ),
        
#     Aes(b'\x1f\xde\xd3\x2d\x59\x99\xde\x4a\x76\xe0\xf8\x08\x21\x08\x82\x3a\xef\x60\x41\x7e\x18\x96\xcf\x42\x18\xa2\xfa\x90\xf6\x32\xec\x8a',
#         b'\x1f\x3a\xfa\x47\x11\xe9\x47\x4f\x32\xe7\x04\x62',
#         b'',
#         b'\x06\xb2\xc7\x58\x53\xdf\x9a\xeb\x17\xbe\xfd\x33\xce\xa8\x1c\x63\x0b\x0f\xc5\x36\x67\xff\x45\x19\x9c\x62\x9c\x8e\x15\xdc\xe4\x1e\x53\x0a\xa7\x92\xf7\x96\xb8\x13\x8e\xea\xb2\xe8\x6c\x7b\x7b\xee\x1d\x40\xb0',
#         b'\x91\xfb\xd0\x61\xdd\xc5\xa7\xfc\xc9\x51\x3f\xcd\xfd\xc9\xc3\xa7\xc5\xd4\xd6\x4c\xed\xf6\xa9\xc2\x4a\xb8\xa7\x7c\x36\xee\xfb\xf1\xc5\xdc\x00\xbc\x50\x12\x1b\x96\x45\x6c\x8c\xd8\xb6\xff\x1f\x8b\x3e\x48\x0f',
#         b'\x30\x09\x6d\x34\x0f\x3d\x5c\x42\xd8\x2a\x6f\x47\x5d\xef\x23\xeb'
#         ),
        
#     Aes(b'\x24\x50\x1a\xd3\x84\xe4\x73\x96\x3d\x47\x6e\xdc\xfe\x08\x20\x52\x37\xac\xfd\x49\xb5\xb8\xf3\x38\x57\xf8\x11\x4e\x86\x3f\xec\x7f',
#         b'\x9f\xf1\x85\x63\xb9\x78\xec\x28\x1b\x3f\x27\x94',
#         b'\xad\xb5\xec\x72\x0c\xcf\x98\x98\x50\x00\x28\xbf\x34\xaf\xcc\xbc\xac\xa1\x26\xef',
#         b'\x27\xf3\x48\xf9\xcd\xc0\xc5\xbd\x5e\x66\xb1\xcc\xb6\x3a\xd9\x20\xff\x22\x19\xd1\x4e\x8d\x63\x1b\x38\x72\x26\x5c\xf1\x17\xee\x86\x75\x7a\xcc\xb1\x58\xbd\x9a\xbb\x38\x68\xfd\xc0\xd0\xb0\x74\xb5\xf0\x1b\x2c',
#         b'\xeb\x7c\xb7\x54\xc8\x24\xe8\xd9\x6f\x7c\x6d\x9b\x76\xc7\xd2\x6f\xb8\x74\xff\xbf\x1d\x65\xc6\xf6\x4a\x69\x8d\x83\x9b\x0b\x06\x14\x5d\xae\x82\x05\x7a\xd5\x59\x94\xcf\x59\xad\x7f\x67\xc0\xfa\x5e\x85\xfa\xb8',
#         b'\xbc\x95\xc5\x32\xfe\xcc\x59\x4c\x36\xd1\x55\x02\x86\xa7\xa3\xf0'
#         )
# ]

# print('********************** Test-1: ivlen=12, ptlen=0, aadlen=0 ******************')
# print('GCM Encrypt......')
# # init cipher object(aes-gcm)
# crypto = ucryptolib.aes(aes[0].key, 0, aes[0].iv, aes[0].aad)
# # input data = plaintext
# inbuf = aes[0].pt
# outbuf = bytearray(16)
# # outbuf = ciphertext + tag
# val = crypto.encrypt(inbuf, outbuf)
# val0 = aes[0].ct + aes[0].tag
# print(val0 == val)
# print('GCM Decrypt......')
# crypto = ucryptolib.aes(aes[0].key, 0, aes[0].iv, aes[0].aad)
# # input data = ciphertext + tag
# inbuf = aes[0].ct + aes[0].tag
# # outbuf = plaintext
# val = crypto.decrypt(inbuf)
# # special case
# print(val[:1] == b'\x00')
# # val0 = aes[0].pt
# # print(val0 == val)

# print('********************** Test-2: ivlen=12, ptlen=1, aadlen=20 ******************')
# print('GCM Encrypt......')
# # init cipher object(aes-gcm)
# crypto = ucryptolib.aes(aes[1].key, 0, aes[1].iv, aes[1].aad)
# # input data = plaintext
# inbuf = aes[1].pt
# outbuf = bytearray(17)
# # outbuf = ciphertext + tag
# val = crypto.encrypt(inbuf, outbuf)
# val0 = aes[1].ct + aes[1].tag
# print(val0 == val)
# print('GCM Decrypt......')
# crypto = ucryptolib.aes(aes[1].key, 0, aes[1].iv, aes[1].aad)
# # input data = ciphertext + tag
# inbuf = aes[1].ct + aes[1].tag
# outbuf = bytearray(1)
# # outbuf = plaintext
# val = crypto.decrypt(inbuf, outbuf)
# val0 = aes[1].pt
# print(val0 == val)

# print('********************** Test-3: ivlen=12, ptlen=51, aadlen=0 ******************')
# print('GCM Encrypt......')
# # init cipher object(aes-gcm)
# crypto = ucryptolib.aes(aes[2].key, 0, aes[2].iv, aes[2].aad)
# # input data = plaintext
# inbuf = aes[2].pt
# outbuf = bytearray(67)
# # outbuf = ciphertext + tag
# val = crypto.encrypt(inbuf, outbuf)
# val0 = aes[2].ct + aes[2].tag
# print(val0 == val)
# print('GCM Decrypt......')
# crypto = ucryptolib.aes(aes[2].key, 0, aes[2].iv, aes[2].aad)
# # input data = ciphertext + tag
# inbuf = aes[2].ct + aes[2].tag
# outbuf = bytearray(51)
# # outbuf = plaintext
# val = crypto.decrypt(inbuf, outbuf)
# val0 = aes[2].pt
# print(val0 == val)

# print('********************** Test-4: ivlen=12, ptlen=51, aadlen=20 ******************')
# print('GCM Encrypt......')
# # init cipher object(aes-gcm)
# crypto = ucryptolib.aes(aes[3].key, 0, aes[3].iv, aes[3].aad)
# # input data = plaintext
# inbuf = aes[3].pt
# outbuf = bytearray(67)
# # outbuf = ciphertext + tag
# val = crypto.encrypt(inbuf, outbuf)
# val0 = aes[3].ct + aes[3].tag
# print(val0 == val)
# print('GCM Decrypt......')
# crypto = ucryptolib.aes(aes[3].key, 0, aes[3].iv, aes[3].aad)
# # input data = ciphertext + tag
# inbuf = aes[3].ct + aes[3].tag
# val = crypto.decrypt(inbuf)
# val0 = aes[3].pt
# print(val[:51] == val0)
# # outbuf = bytearray(51)
# # val = crypto.decrypt(inbuf, outbuf)
# # val0 = aes[3].pt
# # print(val0 == val)
