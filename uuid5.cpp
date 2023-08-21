#include "uuid5.h"
#include <stdio.h>
#include <openssl/sha.h>

namespace uuid {

std::string uuid5_generate(const std::string& name, UUID5_NAMESPACE space) {
    static unsigned char namespace_uuid[16] = {
        0x6b, 0xa7, 0xb8, 0x12,
        0x9d, 0xad,
        0x11, 0xd1,
        0x80, 0xb4,
        0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
    };

    switch (space)
    {
        case NAMESPACE_DNS:
            namespace_uuid[3] = 0x10;
            break;
        case NAMESPACE_URL:
            namespace_uuid[3] = 0x11;
            break;
        case NAMESPACE_X500:
            namespace_uuid[3] = 0x14;
            break;
        default: //NAMESPACE_OID
            namespace_uuid[3] = 0x12;
            break;
    }

    std::string concat((char*)namespace_uuid, 16);
    concat.append(name);

    unsigned char digest[SHA_DIGEST_LENGTH] = {0};
    SHA1((const unsigned char *)concat.c_str(), 16 + name.length(), digest);

    digest[6] &= 0x0f;
    digest[6] |= 0x50;

    digest[8] &= 0x3f;
    digest[8] |= 0x80;

    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str),
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7],
            digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);

    return std::string(uuid_str);
}

}
