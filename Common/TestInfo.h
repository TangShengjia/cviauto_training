// Common/Include/TestInfo.h
#pragma once
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <gio/gio.h>
#include <glib.h>

#define ORG_EXAMPLE_ITESTSERVICE_BUS        G_BUS_TYPE_SESSION
#define ORG_EXAMPLE_ITESTSERVICE_NAME    "org.example.ITestService"
#define ORG_EXAMPLE_ITESTSERVICE_OBJECT_PATH   "/org/example/ITestService"

struct TestInfo {
    bool bool_param;
    int int_param;
    double double_param;
    std::string string_param;
};

GVariant* to_variant(const TestInfo& info);
void from_variant(GVariant* variant, TestInfo& info);
