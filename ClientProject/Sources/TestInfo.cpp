#include "TestInfo.h"
#include <string>
#include <openssl/md5.h>

GVariant* to_variant(const TestInfo& info) {
    return g_variant_new("(bids)",
                         info.bool_param,
                         info.int_param,
                         info.double_param,
                         info.string_param.c_str());
}

void from_variant(GVariant* variant,TestInfo& info) {
    gboolean b;
    gint i;
    gdouble d;
    const gchar* s;
    // 解包结构体
    g_variant_get(variant, "(bids)", &b, &i, &d, &s);

    info.bool_param = b;
    info.int_param = i;
    info.double_param = d;
    info.string_param = s ? std::string(s) : "";
}



std::string calculate_md5(const void* data, size_t len) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)data, len, digest);

    char md5string[33];
    for (int i = 0; i < 16; ++i)
        sprintf(&md5string[i * 2], "%02x", (unsigned int)digest[i]);
    return std::string(md5string);
}