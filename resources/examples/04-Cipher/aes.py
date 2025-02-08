import ucryptolib

def test_aes():
    print("Testing AES encryption/decryption...")
    
    # Test AES-128 ECB
    key = b'1234567890abcdef'  # 16-byte key
    plaintext = b'This is 16 bytes'  # 16-byte data
    print("\nTest 1: AES-128 ECB")
    cipher = ucryptolib.aes(key, ucryptolib.MODE_ECB)
    encrypted = cipher.encrypt(plaintext)
    cipher = ucryptolib.aes(key, ucryptolib.MODE_ECB)  # New instance for decryption
    decrypted = cipher.decrypt(encrypted)
    assert decrypted == plaintext, "AES-128 ECB failed"
    print("Passed")

    # Test invalid key length
    print("\nTest 2: Invalid key length")
    try:
        ucryptolib.aes(b'short_key', ucryptolib.MODE_ECB)
        assert False, "Invalid key not detected"
    except ValueError:
        print("Passed")

    # Test unaligned data length
    print("\nTest 3: Unaligned data length")
    cipher = ucryptolib.aes(key, ucryptolib.MODE_ECB)
    try:
        cipher.encrypt(b'short')
        assert False, "Unaligned data not detected"
    except ValueError:
        print("Passed")

    print("\nAll AES tests passed successfully!")

test_aes()