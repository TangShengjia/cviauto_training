// ServerProject/Sources/main.cpp
#include <iostream>
#include <pthread.h>
#include <mutex>
#include <string>
#include <vector>
#include "../../Common/TestInfo.h"
extern "C" {
#include "testservice.h" // gdbus-codegen生成的
}

// 细粒度锁：分别保护不同类型的共享资源
static std::mutex g_infoMutex;        // 保护g_info
static std::mutex g_fileTransferMutex; // 保护文件传输相关变量

static GMainLoop *pLoop = NULL;
static TestServiceOrgExampleITestService *pSkeleton = NULL;

TestInfo g_info;
std::string g_received_filename;
std::string g_expected_md5;
uint32_t g_expected_filesize = 0;
std::vector<uint8_t> g_file_buffer;


static gboolean handleSetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gboolean param, gpointer user_data) {
    g_print("Server handleSetTestBool is call. bool is : %d.\n", param);
    std::lock_guard<std::mutex> lock(g_infoMutex);
    g_info.bool_param = param;
    test_service_org_example_itest_service_emit_on_test_bool_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_bool(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleSetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gint32 param, gpointer user_data) {
    g_print("Server handleSetTestInt is call. int is : %d.\n", param);
    std::lock_guard<std::mutex> lock(g_infoMutex);
    g_info.int_param = param;
    test_service_org_example_itest_service_emit_on_test_int_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_int(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleSetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gdouble param, gpointer user_data) {
    g_print("Server handleSetTestDouble is call. double is : %f.\n", param);
    std::lock_guard<std::mutex> lock(g_infoMutex);
    g_info.double_param = param;
    test_service_org_example_itest_service_emit_on_test_double_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_double(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleSetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, const gchar *param, gpointer user_data) {
    g_print("Server handleSetTestString is call. string is : %s.\n", param);
    std::lock_guard<std::mutex> lock(g_infoMutex);
    g_info.string_param = param;
    test_service_org_example_itest_service_emit_on_test_string_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_string(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleSetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv,
                                  GVariant *param, gpointer user_data) {
    std::lock_guard<std::mutex> lock(g_infoMutex);
    from_variant(param, g_info);
    g_print("Server handleSetTestInfo is call. bool_param: %d, int_param: %d, double_param: %f, string_param: %s.\n", 
            g_info.bool_param, g_info.int_param, g_info.double_param, g_info.string_param.c_str());
    test_service_org_example_itest_service_emit_on_test_info_changed(skeleton, param);
    test_service_org_example_itest_service_complete_set_test_info(skeleton, inv, TRUE);
    return TRUE;
}

static gboolean handleGetTestBool(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    gboolean bool_val;
    {
        std::lock_guard<std::mutex> lock(g_infoMutex);
        bool_val = g_info.bool_param;
    }
    g_print("Server handleGetTestBool is call. bool is : %d.\n", bool_val);
    test_service_org_example_itest_service_complete_get_test_bool(skeleton, inv, bool_val);
    return TRUE;
}

static gboolean handleGetTestInt(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    gint32 int_val;
    {
        std::lock_guard<std::mutex> lock(g_infoMutex);
        int_val = g_info.int_param;
    }
    g_print("Server handleGetTestInt is call. Int is : %d.\n", int_val);
    test_service_org_example_itest_service_complete_get_test_int(skeleton, inv, int_val);
    return TRUE;
}

static gboolean handleGetTestDouble(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    gdouble double_val;
    {
        std::lock_guard<std::mutex> lock(g_infoMutex);
        double_val = g_info.double_param;
    }
    g_print("Server handleGetTestDouble is call. Double is : %f.\n", double_val);
    test_service_org_example_itest_service_complete_get_test_double(skeleton, inv, double_val);
    return TRUE;
}

static gboolean handleGetTestString(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    std::string string_val;
    {
        std::lock_guard<std::mutex> lock(g_infoMutex);
        string_val = g_info.string_param;
    }
    g_print("Server handleGetTestString is call. String is : %s.\n", string_val.c_str());
    test_service_org_example_itest_service_complete_get_test_string(skeleton, inv, string_val.c_str());
    return TRUE;
}

static gboolean handleGetTestInfo(TestServiceOrgExampleITestService *skeleton, GDBusMethodInvocation *inv, gpointer user_data) {
    TestInfo info_copy;
    {
        std::lock_guard<std::mutex> lock(g_infoMutex);
        info_copy = g_info;
    }
    g_print("Server handleGetTestInfo is call. bool_param: %d, int_param: %d, double_param: %f, string_param: %s.\n", 
            info_copy.bool_param, info_copy.int_param, info_copy.double_param, info_copy.string_param.c_str());
    test_service_org_example_itest_service_complete_get_test_info(skeleton, inv, to_variant(info_copy));
    return TRUE;
}

static gboolean handleSendFileMetadata(TestServiceOrgExampleITestService *skeleton,
                                       GDBusMethodInvocation *inv,
                                       const gchar *filename,
                                       guint32 filesize,
                                       const gchar *md5,
                                       gpointer user_data) {
    g_print("Server: Metadata received - file: %s, size: %u, md5: %s\n", filename, filesize, md5);

    std::lock_guard<std::mutex> lock(g_fileTransferMutex);
    g_received_filename = filename;
    g_expected_md5 = md5;
    g_expected_filesize = filesize;
    g_file_buffer.clear();
    g_file_buffer.resize(filesize);  // 预分配缓冲区

    test_service_org_example_itest_service_complete_send_file_metadata(skeleton, inv, TRUE);
    return TRUE;
}


static gboolean handleSendFileNotification(TestServiceOrgExampleITestService *skeleton,
                                           GDBusMethodInvocation *inv,
                                           const gchar *shm_name,
                                           guint32 offset,
                                           guint32 size,
                                           gboolean is_last_chunk,
                                           gpointer user_data) {
    g_print("Server: Receiving chunk from shm: %s, offset: %u, size: %u\n", shm_name, offset, size);

    int fd = shm_open(shm_name, O_RDONLY, 0666);
    if (fd < 0) {
        g_print("shm_open failed: %s\n", strerror(errno));
        test_service_org_example_itest_service_complete_send_file_notification(skeleton, inv, FALSE);
        return TRUE;
    }

    void* ptr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        g_print("mmap failed\n");
        close(fd);
        test_service_org_example_itest_service_complete_send_file_notification(skeleton, inv, FALSE);
        return TRUE;
    }

    {
        std::lock_guard<std::mutex> lock(g_fileTransferMutex);
        memcpy(&g_file_buffer[offset], ptr, size);
    }

    munmap(ptr, size);
    close(fd);

    if (is_last_chunk) {
        std::string received_filename;
        std::string expected_md5;
        std::vector<uint8_t> file_buffer_copy;
        {
            std::lock_guard<std::mutex> lock(g_fileTransferMutex);
            received_filename = g_received_filename;
            expected_md5 = g_expected_md5;
            file_buffer_copy = g_file_buffer;
        }

        // 校验 MD5
        std::string actual_md5 = calculate_md5(file_buffer_copy.data(), file_buffer_copy.size());
        if (actual_md5 != expected_md5) {
            g_print("MD5 mismatch! expected: %s, actual: %s\n", expected_md5.c_str(), actual_md5.c_str());
        } else {
            g_print("File received and MD5 verified. Saving to %s\n", received_filename.c_str());
            FILE* f = fopen(received_filename.c_str(), "wb");
            if (f) {
                fwrite(file_buffer_copy.data(), 1, file_buffer_copy.size(), f);
                fclose(f);
            } else {
                g_print("Failed to open file for writing: %s\n", received_filename.c_str());
            }
        }
    }

    test_service_org_example_itest_service_complete_send_file_notification(skeleton, inv, TRUE);
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

    (void) g_signal_connect(pSkeleton, "handle-send-file-metadata", G_CALLBACK(handleSendFileMetadata), NULL);
    (void) g_signal_connect(pSkeleton, "handle-send-file-notification", G_CALLBACK(handleSendFileNotification), NULL);

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

    pthread_join(tid, nullptr);
    return 0;
}