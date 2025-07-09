// ClientProject/Sources/main.cpp
#include <gio/gio.h>
#include <iostream>
#include <string>
#include "../../Common/Include/TestInfo.h"
extern "C" {
#include "testservice.h"
}

static void on_test_bool_changed(TestServiceOrgExampleITestService *proxy, gboolean param, gpointer user_data) {
    std::cout << "[Signal] Bool changed: " << param << std::endl;
}
static void on_test_int_changed(TestServiceOrgExampleITestService *proxy, gint32 param, gpointer user_data) {
    std::cout << "[Signal] Int changed: " << param << std::endl;
}
static void on_test_double_changed(TestServiceOrgExampleITestService *proxy, gdouble param, gpointer user_data) {
    std::cout << "[Signal] Double changed: " << param << std::endl;
}
static void on_test_string_changed(TestServiceOrgExampleITestService *proxy, const gchar* param, gpointer user_data) {
    std::cout << "[Signal] String changed: " << param << std::endl;
}
static void on_test_info_changed(TestServiceOrgExampleITestService *proxy, gboolean b, gint32 i, gdouble d, const gchar* s, gpointer user_data) {
    std::cout << "[Signal] Info changed: " << b << "," << i << "," << d << "," << s << std::endl;
}

void show_menu() {
    std::cout << "--------- Menu ---------" << std::endl;
    std::cout << "1. Set Bool" << std::endl;
    std::cout << "2. Set Int" << std::endl;
    std::cout << "3. Set Double" << std::endl;
    std::cout << "4. Set String" << std::endl;
    std::cout << "5. Set Info" << std::endl;
    std::cout << "6. Get Bool" << std::endl;
    std::cout << "7. Get Int" << std::endl;
    std::cout << "8. Get Double" << std::endl;
    std::cout << "9. Get String" << std::endl;
    std::cout << "10. Get Info" << std::endl;
    std::cout << "0. Exit" << std::endl;
}

int main() {
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    GDBusConnection *connection = g_bus_get_sync(ORG_EXAMPLE_ITESTSERVICE_BUS, nullptr, nullptr);
    GError *error = nullptr;

    TestServiceOrgExampleITestService *proxy = test_service_org_example_itest_service_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE,
        ORG_EXAMPLE_ITESTSERVICE_NAME,        // service name
        ORG_EXAMPLE_ITESTSERVICE_OBJECT_PATH,       // object path
        nullptr, &error);

    if (!proxy) {
        std::cerr << "Failed to create proxy: " << error->message << std::endl;
        g_error_free(error);
        return 1;
    }

    g_signal_connect(proxy, "on-test-bool-changed", G_CALLBACK(on_test_bool_changed), nullptr);
    g_signal_connect(proxy, "on-test-int-changed", G_CALLBACK(on_test_int_changed), nullptr);
    g_signal_connect(proxy, "on-test-double-changed", G_CALLBACK(on_test_double_changed), nullptr);
    g_signal_connect(proxy, "on-test-string-changed", G_CALLBACK(on_test_string_changed), nullptr);
    g_signal_connect(proxy, "on-test-info-changed", G_CALLBACK(on_test_info_changed), nullptr);

    int choice;
    while (true) {
        show_menu();
        std::cout << "Choose: ";
        std::cin >> choice;
        if (choice == 0) break;

        if (choice == 1) {
            bool val;
            std::cout << "Input bool (0 or 1): "; std::cin >> val;
            gboolean ok = test_service_org_example_itest_service_call_set_test_bool_sync(proxy, val, nullptr, nullptr, nullptr);
            std::cout << "SetTestBool result: " << ok << std::endl;
        } else if (choice == 2) {
            int val;
            std::cout << "Input int: "; std::cin >> val;
            gboolean ok = test_service_org_example_itest_service_call_set_test_int_sync(proxy, val, nullptr, nullptr, nullptr);
            std::cout << "SetTestInt result: " << ok << std::endl;
        } else if (choice == 3) {
            double val;
            std::cout << "Input double: "; std::cin >> val;
            gboolean ok = test_service_org_example_itest_service_call_set_test_double_sync(proxy, val, nullptr, nullptr, nullptr);
            std::cout << "SetTestDouble result: " << ok << std::endl;
        } else if (choice == 4) {
            std::string val;
            std::cout << "Input string: "; std::cin >> val;
            gboolean ok = test_service_org_example_itest_service_call_set_test_string_sync(proxy, val.c_str(), nullptr, nullptr, nullptr);
            std::cout << "SetTestString result: " << ok << std::endl;
        } else if (choice == 5) {
            bool b; int i; double d; std::string s;
            std::cout << "Input bool (0/1), int, double, string: ";
            std::cin >> b >> i >> d >> s;
            gboolean ok = test_service_org_example_itest_service_call_set_test_info_sync(proxy, b, i, d, s.c_str(), nullptr, nullptr, nullptr);
            std::cout << "SetTestInfo result: " << ok << std::endl;
        } else if (choice == 6) {
            gboolean ret;
            test_service_org_example_itest_service_call_get_test_bool_sync(proxy, &ret, nullptr, nullptr);
            std::cout << "GetTestBool: " << ret << std::endl;
        } else if (choice == 7) {
            gint32 ret;
            test_service_org_example_itest_service_call_get_test_int_sync(proxy, &ret, nullptr, nullptr);
            std::cout << "GetTestInt: " << ret << std::endl;
        } else if (choice == 8) {
            gdouble ret;
            test_service_org_example_itest_service_call_get_test_double_sync(proxy, &ret, nullptr, nullptr);
            std::cout << "GetTestDouble: " << ret << std::endl;
        } else if (choice == 9) {
            gchar *ret;
            test_service_org_example_itest_service_call_get_test_string_sync(proxy, &ret, nullptr, nullptr);
            std::cout << "GetTestString: " << ret << std::endl;
            g_free(ret);
        } else if (choice == 10) {
            gboolean b; gint32 i; gdouble d; gchar *s;
            test_service_org_example_itest_service_call_get_test_info_sync(proxy, &b, &i, &d, &s, nullptr, nullptr);
            std::cout << "GetTestInfo: " << b << "," << i << "," << d << "," << s << std::endl;
            g_free(s);
        }
    }

    g_object_unref(proxy);
    g_object_unref(connection);
    g_main_loop_unref(loop);
    return 0;
}
