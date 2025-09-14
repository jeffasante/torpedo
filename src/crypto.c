#include <string.h>
#include "torpedo.h"

static void xor_encrypt(uint8_t* data, size_t size, const char* password) {
    if (!password) return;
    
    size_t pass_len = strlen(password);
    for (size_t i = 0; i < size; i++) {
        data[i] ^= password[i % pass_len];
    }
}

torpedo_error_t torpedo_encrypt_data(uint8_t* data, size_t size, const char* password) {
    xor_encrypt(data, size, password);
    return TORPEDO_OK;
}

torpedo_error_t torpedo_decrypt_data(uint8_t* data, size_t size, const char* password) {
    xor_encrypt(data, size, password); // XOR is symmetric
    return TORPEDO_OK;
}