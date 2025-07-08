#include "ITestService.h"
#include "SharedMemory.h"
#include <gio/gio.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>

// 客户端实现
class TestServiceClient : public ITestListener {
public:
    TestServiceClient() {
        init_dbus();
    }
    
    ~TestServiceClient() override {
        if (m_proxy) {
            g_object_unref(m_proxy);
        }
        if (m_connection) {
            g_object_unref(m_connection);
        }
    }
    
    // 设置值
    bool SetTestBool(bool param) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SetTestBool",
            g_variant_new("(b)", param),
            G_DBUS_CALL_FLAGS_NONE,
            -1, // 无限超时
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    bool SetTestInt(int param) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SetTestInt",
            g_variant_new("(i)", param),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    bool SetTestDouble(double param) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SetTestDouble",
            g_variant_new("(d)", param),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    bool SetTestString(const std::string& param) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SetTestString",
            g_variant_new("(s)", param.c_str()),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    bool SetTestInfo(const TestInfo& param) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SetTestInfo",
            g_variant_new("(bids)", 
                param.bool_param, 
                param.int_param, 
                param.double_param, 
                param.string_param.c_str()),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    // 获取值
    bool GetTestBool() {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "GetTestBool",
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool value;
            g_variant_get(result, "(b)", &value);
            g_variant_unref(result);
            return value;
        }
        
        return false;
    }
    
    int GetTestInt() {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "GetTestInt",
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            int value;
            g_variant_get(result, "(i)", &value);
            g_variant_unref(result);
            return value;
        }
        
        return 0;
    }
    
    double GetTestDouble() {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "GetTestDouble",
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            double value;
            g_variant_get(result, "(d)", &value);
            g_variant_unref(result);
            return value;
        }
        
        return 0.0;
    }
    
    std::string GetTestString() {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "GetTestString",
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            const gchar* value;
            g_variant_get(result, "(s)", &value);
            std::string str(value);
            g_variant_unref(result);
            return str;
        }
        
        return "";
    }
    
    TestInfo GetTestInfo() {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "GetTestInfo",
            nullptr,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool bool_val;
            int int_val;
            double double_val;
            const gchar* string_val;
            
            g_variant_get(result, "(bids)", 
                &bool_val, &int_val, &double_val, &string_val);
            
            TestInfo info;
            info.bool_param = bool_val;
            info.int_param = int_val;
            info.double_param = double_val;
            info.string_param = string_val;
            
            g_variant_unref(result);
            return info;
        }
        
        return TestInfo();
    }
    
    // 发送文件
    bool SendFile(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << file_path << std::endl;
            return false;
        }
        
        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // 生成唯一的共享内存名称
        std::string shm_name = generate_shm_name();
        
        // 创建共享内存 (1KB)
        const size_t chunk_size = 1024;
        auto shm = SharedMemory::Create(shm_name, chunk_size);
        
        // 计算块数
        int num_chunks = (file_size + chunk_size - 1) / chunk_size;
        
        // 发送文件信息
        bool success = send_file_info(shm_name, chunk_size, file_size, extract_file_name(file_path));
        if (!success) {
            std::cerr << "Failed to send file info" << std::endl;
            return false;
        }
        
        // 分块发送文件
        for (int i = 0; i < num_chunks; ++i) {
            int current_chunk_size = (i == num_chunks - 1) ? 
                file_size - i * chunk_size : chunk_size;
            
            // 读取文件块
            file.read(static_cast<char*>(shm->Data()), current_chunk_size);
            
            // 发送块 (在实际实现中可能需要某种同步机制)
            std::cout << "Sending chunk " << (i + 1) << " of " << num_chunks << std::endl;
            
            // 等待下一个块 (模拟)
            if (i < num_chunks - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        
        file.close();
        std::cout << "File sent successfully" << std::endl;
        
        return true;
    }
    
    // ITestListener接口实现
    void OnTestBoolChanged(bool param) override {
        std::cout << "OnTestBoolChanged: " << (param ? "true" : "false") << std::endl;
    }
    
    void OnTestIntChanged(int param) override {
        std::cout << "OnTestIntChanged: " << param << std::endl;
    }
    
    void OnTestDoubleChanged(double param) override {
        std::cout << "OnTestDoubleChanged: " << param << std::endl;
    }
    
    void OnTestStringChanged(const std::string& param) override {
        std::cout << "OnTestStringChanged: " << param << std::endl;
    }
    
    void OnTestInfoChanged(const TestInfo& param) override {
        std::cout << "OnTestInfoChanged:" << std::endl;
        std::cout << "  Bool: " << (param.bool_param ? "true" : "false") << std::endl;
        std::cout << "  Int: " << param.int_param << std::endl;
        std::cout << "  Double: " << param.double_param << std::endl;
        std::cout << "  String: " << param.string_param << std::endl;
    }
    
private:
    GDBusProxy* m_proxy = nullptr;
    GDBusConnection* m_connection = nullptr;
    guint m_signal_id = 0;
    
    // 初始化DBus
    void init_dbus() {
        GError* error = nullptr;
        
        // 连接到会话总线
        m_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
        if (error) {
            std::cerr << "Failed to connect to session bus: " << error->message << std::endl;
            g_error_free(error);
            return;
        }
        
        // 创建代理
        m_proxy = g_dbus_proxy_new_sync(
            m_connection,
            G_DBUS_PROXY_FLAGS_NONE,
            nullptr,
            "com.example.TestService",
            "/com/example/TestService",
            "com.example.TestService",
            nullptr,
            &error
        );
        
        if (error) {
            std::cerr << "Failed to create proxy: " << error->message << std::endl;
            g_error_free(error);
            return;
        }
        
        // 连接信号
        m_signal_id = g_signal_connect(
            m_proxy,
            "g-signal",
            G_CALLBACK(signal_handler),
            this
        );
    }
    
    // 信号处理函数
    static void signal_handler(
        GDBusProxy* proxy,
        gchar* sender_name,
        gchar* signal_name,
        GVariant* parameters,
        gpointer user_data) {
        
        TestServiceClient* client = static_cast<TestServiceClient*>(user_data);
        
        if (g_strcmp0(signal_name, "TestBoolChanged") == 0) {
            bool param;
            g_variant_get(parameters, "(b)", &param);
            client->OnTestBoolChanged(param);
        }
        else if (g_strcmp0(signal_name, "TestIntChanged") == 0) {
            int param;
            g_variant_get(parameters, "(i)", &param);
            client->OnTestIntChanged(param);
        }
        else if (g_strcmp0(signal_name, "TestDoubleChanged") == 0) {
            double param;
            g_variant_get(parameters, "(d)", &param);
            client->OnTestDoubleChanged(param);
        }
        else if (g_strcmp0(signal_name, "TestStringChanged") == 0) {
            const gchar* param;
            g_variant_get(parameters, "(s)", &param);
            client->OnTestStringChanged(param);
        }
        else if (g_strcmp0(signal_name, "TestInfoChanged") == 0) {
            bool bool_val;
            int int_val;
            double double_val;
            const gchar* string_val;
            
            g_variant_get(parameters, "(bids)", 
                &bool_val, &int_val, &double_val, &string_val);
            
            TestInfo info;
            info.bool_param = bool_val;
            info.int_param = int_val;
            info.double_param = double_val;
            info.string_param = string_val;
            
            client->OnTestInfoChanged(info);
        }
    }
    
    // 发送文件信息
    bool send_file_info(const std::string& shm_name, size_t chunk_size, size_t total_size, const std::string& file_name) {
        GVariant* result = g_dbus_proxy_call_sync(
            m_proxy,
            "SendFile",
            g_variant_new("(siii)", shm_name.c_str(), static_cast<int>(chunk_size), static_cast<int>(total_size), file_name.c_str()),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            nullptr
        );
        
        if (result) {
            bool success;
            g_variant_get(result, "(b)", &success);
            g_variant_unref(result);
            return success;
        }
        
        return false;
    }
    
    // 生成唯一的共享内存名称
    std::string generate_shm_name() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 9999);
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        return "/test_shm_" + std::to_string(timestamp) + "_" + std::to_string(dis(gen));
    }
    
    // 从路径中提取文件名
    std::string extract_file_name(const std::string& path) {
        size_t last_slash = path.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            return path.substr(last_slash + 1);
        }
        return path;
    }
};

// 客户端主函数
int main() {
    std::cout << "Starting TestService Client..." << std::endl;
    
    // 创建客户端实例
    TestServiceClient client;
    
    // 简单的命令行界面
    while (true) {
        std::cout << "\n=== TestService Client Menu ===" << std::endl;
        std::cout << "1. Set Boolean" << std::endl;
        std::cout << "2. Set Integer" << std::endl;
        std::cout << "3. Set Double" << std::endl;
        std::cout << "4. Set String" << std::endl;
        std::cout << "5. Set TestInfo" << std::endl;
        std::cout << "6. Get All Values" << std::endl;
        std::cout << "7. Send File" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter choice: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // 忽略换行符
        
        switch (choice) {
            case 1: {
                std::cout << "Enter boolean value (1/0): ";
                bool value;
                std::cin >> value;
                std::cin.ignore();
                client.SetTestBool(value);
                break;
            }
            case 2: {
                std::cout << "Enter integer value: ";
                int value;
                std::cin >> value;
                std::cin.ignore();
                client.SetTestInt(value);
                break;
            }
            case 3: {
                std::cout << "Enter double value: ";
                double value;
                std::cin >> value;
                std::cin.ignore();
                client.SetTestDouble(value);
                break;
            }
            case 4: {
                std::cout << "Enter string value: ";
                std::string value;
                std::getline(std::cin, value);
                client.SetTestString(value);
                break;
            }
            case 5: {
                TestInfo info;
                std::cout << "Enter boolean value (1/0): ";
                std::cin >> info.bool_param;
                std::cout << "Enter integer value: ";
                std::cin >> info.int_param;
                std::cout << "Enter double value: ";
                std::cin >> info.double_param;
                std::cin.ignore();
                std::cout << "Enter string value: ";
                std::getline(std::cin, info.string_param);
                client.SetTestInfo(info);
                break;
            }
            case 6: {
                std::cout << "\n=== Current Values ===" << std::endl;
                std::cout << "Bool: " << (client.GetTestBool() ? "true" : "false") << std::endl;
                std::cout << "Int: " << client.GetTestInt() << std::endl;
                std::cout << "Double: " << client.GetTestDouble() << std::endl;
                std::cout << "String: " << client.GetTestString() << std::endl;
                
                TestInfo info = client.GetTestInfo();
                std::cout << "\nTestInfo:" << std::endl;
                std::cout << "  Bool: " << (info.bool_param ? "true" : "false") << std::endl;
                std::cout << "  Int: " << info.int_param << std::endl;
                std::cout << "  Double: " << info.double_param << std::endl;
                std::cout << "  String: " << info.string_param << std::endl;
                break;
            }
            case 7: {
                std::cout << "Enter file path: ";
                std::string file_path;
                std::getline(std::cin, file_path);
                client.SendFile(file_path);
                break;
            }
            case 0:
                std::cout << "Exiting..." << std::endl;
                return 0;
            default:
                std::cout << "Invalid choice!" << std::endl;
        }
    }
    
    return 0;
}    