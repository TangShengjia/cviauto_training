#include "TestInfo.h"
#include <string>

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