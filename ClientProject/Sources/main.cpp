// ClientProject/Sources/main.cpp
#include <gio/gio.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <libgen.h>
#include "../../Common/TestInfo.h"
extern "C" {
#include "testservice.h"
}

static GMainLoop *pLoop= NULL;
static GDBusConnection *pConnection = NULL;
static TestServiceOrgExampleITestService *proxy;


static void handleBoolChanged(TestServiceOrgExampleITestService *proxy, gboolean param, gpointer user_data) {
    std::cout << "[Signal] Bool changed: " << param << std::endl;
}
static void handleIntChanged(TestServiceOrgExampleITestService *proxy, gint32 param, gpointer user_data) {
    std::cout << "[Signal] Int changed: " << param << std::endl;
}
static void handleDoubleChanged(TestServiceOrgExampleITestService *proxy, gdouble param, gpointer user_data) {
    std::cout << "[Signal] Double changed: " << param << std::endl;
}
static void handleStringChanged(TestServiceOrgExampleITestService *proxy, const gchar* param, gpointer user_data) {
    std::cout << "[Signal] String changed: " << param << std::endl;
}
static void handleInfoChanged(TestServiceOrgExampleITestService *proxy, GVariant *param, gpointer user_data) {
    gboolean b;
    gint i;
    gdouble d;
    const gchar* s;

    g_variant_get(param, "(bids)", &b, &i, &d, &s);
    g_print("[Signal] Info changed, bool_param:%d, int_param:%d, double_param:%f, string_param:%s.\n", b, i, d, s);
}


bool initDBusCommunication(void){
	
	bool bRet = TRUE;
    GError *pConnError = NULL;
    GError *pProxyError = NULL;
	
	do{
		bRet = TRUE;
		pLoop = g_main_loop_new(NULL,FALSE); 
		
		pConnection = g_bus_get_sync(ORG_EXAMPLE_ITESTSERVICE_BUS, NULL, &pConnError);
		if(pConnError == NULL){
			proxy = test_service_org_example_itest_service_proxy_new_sync(
                    pConnection, G_DBUS_PROXY_FLAGS_NONE,
                    ORG_EXAMPLE_ITESTSERVICE_NAME,        // service name
                    ORG_EXAMPLE_ITESTSERVICE_OBJECT_PATH,       // object path
                    nullptr, &pProxyError);
			if(proxy == 0){
				g_print("initDBusCommunication: Create proxy failed. Reason: %s.\n", pConnError->message);
				g_error_free(pProxyError);
				bRet = FALSE;
			}else{
				g_print("initDBusCommunication: Create proxy successfully. \n");
			}
		}else{
			g_print("initDBusCommunication: Failed to connect to dbus. Reason: %s.\n", pConnError->message);
            g_error_free(pConnError);
            bRet = FALSE;
		}
	}while(bRet == FALSE);
					 
	if(bRet == TRUE){
		g_signal_connect(proxy, "on-test-bool-changed", G_CALLBACK(handleBoolChanged), nullptr);
        g_signal_connect(proxy, "on-test-int-changed", G_CALLBACK(handleIntChanged), nullptr);
        g_signal_connect(proxy, "on-test-double-changed", G_CALLBACK(handleDoubleChanged), nullptr);
        g_signal_connect(proxy, "on-test-string-changed", G_CALLBACK(handleStringChanged), nullptr);
        g_signal_connect(proxy, "on-test-info-changed", G_CALLBACK(handleInfoChanged), nullptr);

	}else{
		g_print("initDBusCommunication: Failed to connect signal.  \n");
	}

	return bRet;
}

static void *run(void* arg)
{
    g_main_loop_run(pLoop);
    return nullptr;
}

void show_menu() {
    std::cout << "\n===== D-Bus Client Menu =====" << std::endl;
    std::cout << "1.  Set Bool (0/1)"           << std::endl;
    std::cout << "2.  Set Int"                  << std::endl;
    std::cout << "3.  Set Double"               << std::endl;
    std::cout << "4.  Set String"               << std::endl;
    std::cout << "5.  Set Info (bool int double string)" << std::endl;
    std::cout << "6.  Get Bool"                 << std::endl;
    std::cout << "7.  Get Int"                  << std::endl;
    std::cout << "8.  Get Double"               << std::endl;
    std::cout << "9.  Get String"               << std::endl;
    std::cout << "10. Get Info"                 << std::endl;
    std::cout << "11. Send File"                << std::endl;
    std::cout << "0.  Exit"                     << std::endl;
    std::cout << "============================" << std::endl;
}

std::string get_basename(const std::string& full_path) {
    char *path_copy = strdup(full_path.c_str());
    std::string result = basename(path_copy);
    free(path_copy);
    return result;
}

bool send_file(const std::string& filename) {
    std::string file_name_only = get_basename(filename);

    // 读取文件
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        std::cerr << "Failed to open file.\n";
        return false;
    }
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> buffer(filesize);
    fread(buffer.data(), 1, filesize, f);
    fclose(f);

    // 计算 MD5
    std::string md5 = calculate_md5(buffer.data(), filesize);

    // 发送 Metadata
    gboolean meta_ok = test_service_org_example_itest_service_call_send_file_metadata_sync(
        proxy, file_name_only.c_str(), filesize, md5.c_str(), nullptr, nullptr, nullptr);
    if (!meta_ok) {
        std::cerr << "SendFileMetadata failed\n";
        return false;
    }

    const size_t CHUNK_SIZE = 1024;
    const std::string shm_name = "/file_chunk_shm";
    shm_unlink(shm_name.c_str());
    int fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    ftruncate(fd, CHUNK_SIZE);
    void* shm_ptr = mmap(NULL, CHUNK_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);

    for (size_t offset = 0; offset < filesize; offset += CHUNK_SIZE) {
        size_t chunk_size = std::min(CHUNK_SIZE, filesize - offset);
        memcpy(shm_ptr, buffer.data() + offset, chunk_size);

        gboolean chunk_ok = test_service_org_example_itest_service_call_send_file_notification_sync(
            proxy, shm_name.c_str(), offset, chunk_size, (offset + chunk_size == filesize), nullptr, nullptr, nullptr);
        if (!chunk_ok) {
            std::cerr << "SendFileNotification failed at offset " << offset << "\n";
            break;
        }
    }

    munmap(shm_ptr, CHUNK_SIZE);
    close(fd);
    shm_unlink(shm_name.c_str());

    return true;
}


int main() {

    pthread_t tid;

    initDBusCommunication();

    pthread_create(&tid,NULL,run,NULL);

    int choice;
    while (true) {
        usleep(100000);
        show_menu();
        g_print("Choose: ");
        
        // 处理菜单选择输入
        while (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            g_print("Error: Invalid input. Please enter a number.\n");
            g_print("Choose: ");
        }
        
        if (choice == 0) break;

        if (choice == 1) {
            gboolean  val;
            g_print("Input bool (0 or 1): ");
            
            // 处理布尔值输入
            while (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                g_print("Error: Invalid input. Please enter 0 or 1.\n");
                g_print("Input bool (0 or 1): ");
            }
            
            gboolean ok = test_service_org_example_itest_service_call_set_test_bool_sync(proxy, val, nullptr, nullptr, nullptr);
            g_print("SetTestBool result: %d\n", ok);
        } 
        else if (choice == 2) {
            gint val;
            g_print("Input int: ");
            
            // 处理整数输入
            while (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                g_print("Error: Invalid input. Please enter an integer.\n");
                g_print("Input int: ");
            }
            
            gboolean ok = test_service_org_example_itest_service_call_set_test_int_sync(proxy, val, nullptr, nullptr, nullptr);
            g_print("SetTestInt result: %d\n", ok);
        } 
        else if (choice == 3) {
            gdouble val;
            g_print("Input double: ");
            
            // 处理双精度浮点数输入
            while (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                g_print("Error: Invalid input. Please enter a valid number.\n");
                g_print("Input double: ");
            }
            
            gboolean ok = test_service_org_example_itest_service_call_set_test_double_sync(proxy, val, nullptr, nullptr, nullptr);
            g_print("SetTestDouble result: %d\n", ok);
        } 
        else if (choice == 4) {
            std::string val;
            g_print("Input string: ");
            std::cin >> val;
            
            gboolean ok = test_service_org_example_itest_service_call_set_test_string_sync(proxy, val.c_str(), nullptr, nullptr, nullptr);
            g_print("SetTestString result: %d\n", ok);
        } 
        else if (choice == 5) {
            gboolean b; gint i; gdouble d; std::string s;
            g_print("Input bool (0/1), int, double, string: ");
            
            // 处理多个输入值
            while (!(std::cin >> b >> i >> d >> s)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                g_print("Error: Invalid input. Please enter valid values.\n");
                g_print("Input bool (0/1), int, double, string: ");
            }
            
            gboolean ok = test_service_org_example_itest_service_call_set_test_info_sync(proxy, g_variant_new("(bids)", b, i, d, s.c_str()), nullptr, nullptr, nullptr);
            g_print("SetTestInfo result: %d\n", ok);
        } 
        else if (choice == 6) {
            gboolean  ret;
            test_service_org_example_itest_service_call_get_test_bool_sync(proxy, &ret, nullptr, nullptr);
            g_print("GetTestBool: %d\n", ret);
        } 
        else if (choice == 7) {
            gint32 ret;
            test_service_org_example_itest_service_call_get_test_int_sync(proxy, &ret, nullptr, nullptr);
            g_print("GetTestInt: %d\n", ret);
        } 
        else if (choice == 8) {
            gdouble ret;
            test_service_org_example_itest_service_call_get_test_double_sync(proxy, &ret, nullptr, nullptr);
            g_print("GetTestDouble: %f\n", ret);
        } 
        else if (choice == 9) {
            gchar *ret;
            test_service_org_example_itest_service_call_get_test_string_sync(proxy, &ret, nullptr, nullptr);
            g_print("GetTestString: %s\n", ret);
            g_free(ret);
        } 
        else if (choice == 10) {
            gboolean b; gint32 i; gdouble d; gchar *s;
            GVariant* ret;
            test_service_org_example_itest_service_call_get_test_info_sync(proxy, &ret, nullptr, nullptr);
            g_variant_get(ret, "(bids)", &b, &i, &d, &s);
            g_print("GetTestInfo: bool_param:%d, int_param:%d, double_param:%f, string_param:%s.\n", b, i, d, s);
            g_free(s);
        }
        else if (choice == 11) {
            std::string filename;
            g_print("Input file path: ");
            std::cin.ignore(); // 清除缓冲区中的换行符
            std::getline(std::cin, filename);
            
            if (send_file(filename)) {
                g_print("File sent successfully.\n");
            } else {
                g_print("Failed to send file.\n");
            }
        }
        else {
            g_print("Error: Invalid choice. Please enter a number between 0 and 11.\n");
        }
    }

    g_object_unref(proxy);
    g_object_unref(pConnection);
    g_main_loop_unref(pLoop);
    return 0;
}
