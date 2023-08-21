#ifndef UUID5_H_
#define UUID5_H_

#include <string>

namespace uuid {

typedef enum {
    NAMESPACE_DNS = 0,
    NAMESPACE_URL,
    NAMESPACE_OID,
    NAMESPACE_X500
} UUID5_NAMESPACE;

std::string uuid5_generate(const std::string& name, UUID5_NAMESPACE space = NAMESPACE_OID);

} // namespace

#endif /* UUID5_H_*/
 