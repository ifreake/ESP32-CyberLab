#include "crypto_lab.h"
#include <mbedtls/sha256.h>

String sha256Hex(const String& input) {
    unsigned char digest[32];
    const auto* data = reinterpret_cast<const unsigned char*>(input.c_str());

    int rc = mbedtls_sha256(data, input.length(), digest, 0);
    if (rc != 0) return "";

    static const char hex[] = "0123456789abcdef";
    String output;
    output.reserve(64);
    for (size_t i = 0; i < sizeof(digest); ++i) {
        output += hex[(digest[i] >> 4) & 0x0F];
        output += hex[digest[i] & 0x0F];
    }
    return output;
}
