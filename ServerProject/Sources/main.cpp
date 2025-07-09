// ServerProject/Sources/main.cpp
#include <gio/gio.h>
#include <iostream>
#include "../../Common/Include/TestInfo.h"
extern "C" {
#include "testservice.h" // gdbus-codegen生成的
}

// 数据存储
TestInfo g_info;

static gboolean handle_set_test_bool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gboolean param, gpointer user_data) {
    g_info.bool_param = param;
    test_service_org_example_itest_service_emit_on_test_bool_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_bool(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handle_set_test_int(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gint32 param, gpointer user_data) {
    g_info.int_param = param;
    test_service_org_example_itest_service_emit_on_test_int_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_int(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handle_set_test_double(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gdouble param, gpointer user_data) {
    g_info.double_param = param;
    test_service_org_example_itest_service_emit_on_test_double_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_double(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handle_set_test_string(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, const gchar *param, gpointer user_data) {
    g_info.string_param = param;
    test_service_org_example_itest_service_emit_on_test_string_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_string(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handle_set_test_info(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv,
                                  gboolean b, gint32 i, gdouble d, const gchar *s, gpointer user_data) {
    g_info.bool_param   = b;
    g_info.int_param    = i;
    g_info.double_param = d;
    g_info.string_param = s;
    test_service_org_example_itest_service_emit_on_test_info_changed(skeleton, b, i, d, s);
    test_service_org_example_itest_service_complete_set_test_info(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handle_get_test_bool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    test_service_org_example_itest_service_complete_get_test_bool(skeleton, inv, g_info.bool_param);
    return TRUE;
}
static gboolean handle_get_test_int(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    test_service_org_example_itest_service_complete_get_test_int(skeleton, inv, g_info.int_param);
    return TRUE;
}
static gboolean handle_get_test_double(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    test_service_org_example_itest_service_complete_get_test_double(skeleton, inv, g_info.double_param);
    return TRUE;
}
static gboolean handle_get_test_string(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    test_service_org_example_itest_service_complete_get_test_string(skeleton, inv, g_info.string_param.c_str());
    return TRUE;
}
static gboolean handle_get_test_info(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    test_service_org_example_itest_service_complete_get_test_info(skeleton, inv,
        g_info.bool_param, g_info.int_param, g_info.double_param, g_info.string_param.c_str());
    return TRUE;
}

int main() {
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr);

    TestServiceOrgExampleITestService *skeleton = test_service_org_example_itest_service_skeleton_new();

    g_signal_connect(skeleton, "handle-set-test-bool", G_CALLBACK(handle_set_test_bool), nullptr);
    g_signal_connect(skeleton, "handle-set-test-int", G_CALLBACK(handle_set_test_int), nullptr);
    g_signal_connect(skeleton, "handle-set-test-double", G_CALLBACK(handle_set_test_double), nullptr);
    g_signal_connect(skeleton, "handle-set-test-string", G_CALLBACK(handle_set_test_string), nullptr);
    g_signal_connect(skeleton, "handle-set-test-info", G_CALLBACK(handle_set_test_info), nullptr);

    g_signal_connect(skeleton, "handle-get-test-bool", G_CALLBACK(handle_get_test_bool), nullptr);
    g_signal_connect(skeleton, "handle-get-test-int", G_CALLBACK(handle_get_test_int), nullptr);
    g_signal_connect(skeleton, "handle-get-test-double", G_CALLBACK(handle_get_test_double), nullptr);
    g_signal_connect(skeleton, "handle-get-test-string", G_CALLBACK(handle_get_test_string), nullptr);
    g_signal_connect(skeleton, "handle-get-test-info", G_CALLBACK(handle_get_test_info), nullptr);

    // 导出
    guint reg_id = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(skeleton), connection,
        "/org/example/ITestService", nullptr);

    // Console info
    std::cout << "Server ready, waiting for client calls..." << std::endl;

    g_main_loop_run(loop);

    g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(skeleton));
    g_object_unref(skeleton);
    g_object_unref(connection);
    g_main_loop_unref(loop);

    return 0;
}
