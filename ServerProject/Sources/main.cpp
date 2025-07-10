// ServerProject/Sources/main.cpp
#include <iostream>
#include <pthread.h>
#include "../../Common/TestInfo.h"
extern "C" {
#include "testservice.h" // gdbus-codegen生成的
}

static GMainLoop *pLoop = NULL;
static TestServiceOrgExampleITestService *pSkeleton = NULL;

TestInfo g_info;


static gboolean handleSetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gboolean param, gpointer user_data) {
    g_print("Server handleSetTestBool is call. bool is : %d.\n", param);
    g_info.bool_param = param;
    test_service_org_example_itest_service_emit_on_test_bool_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_bool(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handleSetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gint32 param, gpointer user_data) {
    g_print("Server handleSetTestInt is call. int is : %d.\n", param);
    g_info.int_param = param;
    test_service_org_example_itest_service_emit_on_test_int_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_int(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handleSetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gdouble param, gpointer user_data) {
    g_print("Server handleSetTestDouble is call. double is : %f.\n", param);
    g_info.double_param = param;
    test_service_org_example_itest_service_emit_on_test_double_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_double(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handleSetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, const gchar *param, gpointer user_data) {
    g_print("Server handleSetTestString is call. string is : %s.\n", param);
    g_info.string_param = param;
    test_service_org_example_itest_service_emit_on_test_string_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_string(skeleton, inv, TRUE);
    return TRUE;
}
static gboolean handleSetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv,
                                  GVariant *param, gpointer user_data) {
    
    from_variant(param, g_info);
    g_print("Server handleGetTestInfo is call. bool_param: %d,int_param: %d,double_param: %f,string_param: %s.\n", g_info.bool_param, g_info.int_param, g_info.double_param, g_info.string_param.c_str());
    test_service_org_example_itest_service_emit_on_test_info_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_info(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleGetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    g_print("Server handleGetTestBool is call. bool is : %d.\n", g_info.bool_param);
    test_service_org_example_itest_service_complete_get_test_bool(skeleton, inv, g_info.bool_param);
    return TRUE;
}
static gboolean handleGetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    g_print("Server handleGetTestInt is call. Int is : %d.\n", g_info.int_param);
    test_service_org_example_itest_service_complete_get_test_int(skeleton, inv, g_info.int_param);
    return TRUE;
}
static gboolean handleGetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    g_print("Server handleGetTestDouble is call. Double is : %f.\n", g_info.double_param);
    test_service_org_example_itest_service_complete_get_test_double(skeleton, inv, g_info.double_param);
    return TRUE;
}
static gboolean handleGetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    g_print("Server handleGetTestString is call. String is : %s.\n", g_info.string_param.c_str());
    test_service_org_example_itest_service_complete_get_test_string(skeleton, inv, g_info.string_param.c_str());
    return TRUE;
}
static gboolean handleGetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    g_print("Server handleGetTestInfo is call. bool_param: %d,int_param: %d,double_param: %f,string_param: %s.\n", g_info.bool_param, g_info.int_param, g_info.double_param, g_info.string_param.c_str());
    test_service_org_example_itest_service_complete_get_test_info(skeleton, inv, to_variant(g_info));
    return TRUE;
}



static void GBusAcquired_Callback(GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data){
    
    GError *pError = NULL;

    pSkeleton = test_service_org_example_itest_service_skeleton_new();

    // 连接所有方法处理函数
    (void) g_signal_connect(pSkeleton, "handle-set-test-bool", G_CALLBACK(handleSetTestBool), NULL);
    (void) g_signal_connect(pSkeleton, "handle-set-test-int", G_CALLBACK(handleSetTestInt), NULL);
    (void) g_signal_connect(pSkeleton, "handle-set-test-double", G_CALLBACK(handleSetTestDouble), NULL);
    (void) g_signal_connect(pSkeleton, "handle-set-test-string", G_CALLBACK(handleSetTestString), NULL);
    (void) g_signal_connect(pSkeleton, "handle-set-test-info", G_CALLBACK(handleSetTestInfo), NULL);
    
    (void) g_signal_connect(pSkeleton, "handle-get-test-bool", G_CALLBACK(handleGetTestBool), NULL);
    (void) g_signal_connect(pSkeleton, "handle-get-test-int", G_CALLBACK(handleGetTestInt), NULL);
    (void) g_signal_connect(pSkeleton, "handle-get-test-double", G_CALLBACK(handleGetTestDouble), NULL);
    (void) g_signal_connect(pSkeleton, "handle-get-test-string", G_CALLBACK(handleGetTestString), NULL);
    (void) g_signal_connect(pSkeleton, "handle-get-test-info", G_CALLBACK(handleGetTestInfo), NULL);

    (void) g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(pSkeleton),
                                                connection,
                                                ORG_EXAMPLE_ITESTSERVICE_OBJECT_PATH,
                                                &pError);

    if(pError == 0){
        g_print("skeleton export successfully. \n");
    }else{
        g_print("Error: Failed to export object. Reason: %s.\n", pError->message);
        g_error_free(pError);
        g_main_loop_quit(pLoop);
        return;
    }
}


static void GBusNameAcquired_Callback (GDBusConnection *connection,
                             const gchar *name,
                             gpointer user_data){
    g_print("GBusNameAcquired_Callback, Acquired bus name: %s \n", ORG_EXAMPLE_ITESTSERVICE_NAME);
}

static void GBusNameLost_Callback (GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data){
    if (connection == NULL)
    {
        g_print("GBusNameLost_Callback, Error: Failed to connect to dbus. \n");
    }else{
        g_print("GBusNameLost_Callback, Error: Failed to get dbus name : %s\n", ORG_EXAMPLE_ITESTSERVICE_NAME);
}
    g_main_loop_quit(pLoop);
}


bool initDBusCommunicationForServer(void){
    
    bool bRet = TRUE;

    pLoop = g_main_loop_new(nullptr, FALSE); 

    guint own_id = 
        g_bus_own_name (ORG_EXAMPLE_ITESTSERVICE_BUS,
                    ORG_EXAMPLE_ITESTSERVICE_NAME,
                    G_BUS_NAME_OWNER_FLAGS_NONE,
                    &GBusAcquired_Callback,
                    &GBusNameAcquired_Callback,
                    &GBusNameLost_Callback,
                    NULL,
                    NULL);

    return bRet;
}

static void *run(void* arg)
{
    g_main_loop_run(pLoop);
    return nullptr;
}

int main() {
    pthread_t tid;
    initDBusCommunicationForServer();

    pthread_create(&tid, NULL, run, NULL);

    while(1){
	    /* code */
    }
    return 0;
}
