#include "ITestService.h"
#include "SharedMemory.h"
#include <iostream>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <glib.h>
#include <gio/gio.h>

// DBus接口定义
static const gchar *test_service_xml =
"<node>"
"  <interface name='com.example.TestService'>"
"    <method name='SetTestBool'>"
"      <arg type='b' name='param' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <method name='SetTestInt'>"
"      <arg type='i' name='param' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <method name='SetTestDouble'>"
"      <arg type='d' name='param' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <method name='SetTestString'>"
"      <arg type='s' name='param' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <method name='SetTestInfo'>"
"      <arg type='(bids)' name='param' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <method name='GetTestBool'>"
"      <arg type='b' name='result' direction='out'/>"
"    </method>"
"    <method name='GetTestInt'>"
"      <arg type='i' name='result' direction='out'/>"
"    </method>"
"    <method name='GetTestDouble'>"
"      <arg type='d' name='result' direction='out'/>"
"    </method>"
"    <method name='GetTestString'>"
"      <arg type='s' name='result' direction='out'/>"
"    </method>"
"    <method name='GetTestInfo'>"
"      <arg type='(bids)' name='result' direction='out'/>"
"    </method>"
"    <method name='SendFile'>"
"      <arg type='s' name='shm_name' direction='in'/>"
"      <arg type='i' name='chunk_size' direction='in'/>"
"      <arg type='i' name='total_size' direction='in'/>"
"      <arg type='s' name='file_name' direction='in'/>"
"      <arg type='b' name='success' direction='out'/>"
"    </method>"
"    <signal name='TestBoolChanged'>"
"      <arg type='b' name='param'/>"
"    </signal>"
"    <signal name='TestIntChanged'>"
"      <arg type='i' name='param'/>"
"    </signal>"
"    <signal name='TestDoubleChanged'>"
"      <arg type='d' name='param'/>"
"    </signal>"
"    <signal name='TestStringChanged'>"
"      <arg type='s' name='param'/>"
"    </signal>"
"    <signal name='TestInfoChanged'>"
"      <arg type='(bids)' name='param'/>"
"    </signal>"
"  </interface>"
"</node>";

class TestServiceImpl : public ITestService {
public:
    TestServiceImpl() {
        // 初始化DBus
        init_dbus();
    }
    
    ~TestServiceImpl() override {
        if (m_connection) {
            g_object_unref(m_connection);
        }
        if (m_object_manager) {
            g_object_unref(m_object_manager);
        }
    }
    
    // ITestService接口实现
    bool SetTestBool(bool param) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bool_param = param;
        emit_signal("TestBoolChanged", g_variant_new("(b)", param));
        return true;
    }
    
    bool SetTestInt(int param) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_int_param = param;
        emit_signal("TestIntChanged", g_variant_new("(i)", param));
        return true;
    }
    
    bool SetTestDouble(double param) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_double_param = param;
        emit_signal("TestDoubleChanged", g_variant_new("(d)", param));
        return true;
    }
    
    bool SetTestString(const std::string& param) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_string_param = param;
        emit_signal("TestStringChanged", g_variant_new("(s)", param.c_str()));
        return true;
    }
    
    bool SetTestInfo(const TestInfo& param) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_test_info = param;
        emit_signal("TestInfoChanged", g_variant_new("(bids)", 
            param.bool_param, 
            param.int_param, 
            param.double_param, 
            param.string_param.c_str()));
        return true;
    }
    
    bool GetTestBool() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_bool_param;
    }
    
    int GetTestInt() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_int_param;
    }
    
    double GetTestDouble() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_double_param;
    }
    
    std::string GetTestString() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_string_param;
    }
    
    TestInfo GetTestInfo() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_test_info;
    }
    
    bool SendFile(const unsigned char* file_buf, size_t file_size, const std::string& file_name) override {
        // 实际实现中，我们使用共享内存方法，这里只是接口实现
        return false;
    }
    
    // 注册监听器
    void TestServiceImpl::RegisterListener(std::shared_ptr<ITestListener> listener) {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        m_listeners.insert(listener);
    }

    // 注销监听器
    void TestServiceImpl::UnregisterListener(std::shared_ptr<ITestListener> listener) {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        m_listeners.erase(listener);
    }
    
    // 文件接收方法 - 通过共享内存
    bool ReceiveFile(const std::string& shm_name, int chunk_size, int total_size, const std::string& file_name) {
        try {
            auto shm = SharedMemory::Open(shm_name, chunk_size);
            
            // 创建文件
            std::ofstream file("received_" + file_name, std::ios::binary);
            if (!file) {
                return false;
            }
            
            // 计算块数
            int num_chunks = (total_size + chunk_size - 1) / chunk_size;
            
            // 接收所有块
            for (int i = 0; i < num_chunks; ++i) {
                int current_chunk_size = (i == num_chunks - 1) ? 
                    total_size - i * chunk_size : chunk_size;
                
                // 从共享内存读取
                file.write(static_cast<const char*>(shm->Data()), current_chunk_size);
                
                // 等待下一个块（实际实现中可能需要某种同步机制）
                if (i < num_chunks - 1) {
                    // 这里应该有等待机制
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            
            file.close();
            
            // 计算MD5校验和
            std::string md5 = calculate_md5("received_" + file_name);
            std::cout << "File received and saved as: received_" << file_name << std::endl;
            std::cout << "MD5 Checksum: " << md5 << std::endl;
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error receiving file: " << e.what() << std::endl;
            return false;
        }
    }
    
private:
    // 数据存储
    bool m_bool_param = false;
    int m_int_param = 0;
    double m_double_param = 0.0;
    std::string m_string_param;
    TestInfo m_test_info;
    
    // 监听器集合
    std::unordered_set<std::shared_ptr<ITestListener>> m_listeners;
    std::mutex m_mutex;
    
    // DBus相关
    GDBusConnection* m_connection = nullptr;
    GDBusObjectManagerServer* m_object_manager = nullptr;
    guint m_registration_id = 0;
    
    // 初始化DBus
    void init_dbus() {
        GError* error = nullptr;
        
    
        // 连接到会话总线
        m_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
        if (error != nullptr) {
            std::cerr << "Failed to connect to session bus: " << error->message << std::endl;
            g_error_free(error);
            return;
        }
        
        // 创建对象管理器
        m_object_manager = g_dbus_object_manager_server_new("/com/example");
        
        // 创建接口实现
        GDBusInterfaceVTable vtable = {
            method_call_handler,
            nullptr, // get_property
            nullptr, // set_property
            nullptr  // padding
        };
        
        GNodeInfo* node_info = g_dbus_node_info_new_for_xml(test_service_xml, &error);
        if (error) {
            std::cerr << "Failed to parse XML: " << error->message << std::endl;
            g_error_free(error);
            return;
        }
        
        GDBusInterfaceInfo* interface_info = node_info->interfaces[0];
        GDBusInterfaceSkeleton* interface_skeleton = g_dbus_interface_skeleton_new(interface_info->name);
        g_dbus_interface_skeleton_set_vtable(interface_skeleton, &vtable, this, nullptr);
        
        // 创建对象
        GDBusObjectSkeleton* object_skeleton = g_dbus_object_skeleton_new("/com/example/TestService");
        g_dbus_object_skeleton_add_interface(object_skeleton, interface_skeleton);
        g_object_unref(interface_skeleton);
        
        // 添加对象到管理器
        g_dbus_object_manager_server_export(m_object_manager, object_skeleton);
        g_object_unref(object_skeleton);
        
        // 注册服务
        m_registration_id = g_bus_own_name_on_connection(
            m_connection,
            "com.example.TestService",
            G_BUS_NAME_OWNER_FLAGS_NONE,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        
        g_dbus_node_info_unref(node_info);
    }
    
    // 方法调用处理函数
    static void method_call_handler(
        GDBusConnection* connection,
        const gchar* sender,
        const gchar* object_path,
        const gchar* interface_name,
        const gchar* method_name,
        GVariant* parameters,
        GDBusMethodInvocation* invocation,
        gpointer user_data) {
        
        TestServiceImpl* service = static_cast<TestServiceImpl*>(user_data);
        
        if (g_strcmp0(method_name, "SetTestBool") == 0) {
            bool param;
            g_variant_get(parameters, "(b)", &param);
            bool success = service->SetTestBool(param);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        } 
        else if (g_strcmp0(method_name, "SetTestInt") == 0) {
            int param;
            g_variant_get(parameters, "(i)", &param);
            bool success = service->SetTestInt(param);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        }
        else if (g_strcmp0(method_name, "SetTestDouble") == 0) {
            double param;
            g_variant_get(parameters, "(d)", &param);
            bool success = service->SetTestDouble(param);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        }
        else if (g_strcmp0(method_name, "SetTestString") == 0) {
            const gchar* param;
            g_variant_get(parameters, "(s)", &param);
            bool success = service->SetTestString(param);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        }
        else if (g_strcmp0(method_name, "SetTestInfo") == 0) {
            bool bool_val;
            int int_val;
            double double_val;
            const gchar* string_val;
            
            g_variant_get(parameters, "((bids))", 
                &bool_val, &int_val, &double_val, &string_val);
            
            TestInfo info;
            info.bool_param = bool_val;
            info.int_param = int_val;
            info.double_param = double_val;
            info.string_param = string_val;
            
            bool success = service->SetTestInfo(info);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        }
        else if (g_strcmp0(method_name, "GetTestBool") == 0) {
            bool result = service->GetTestBool();
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", result));
        }
        else if (g_strcmp0(method_name, "GetTestInt") == 0) {
            int result = service->GetTestInt();
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", result));
        }
        else if (g_strcmp0(method_name, "GetTestDouble") == 0) {
            double result = service->GetTestDouble();
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(d)", result));
        }
        else if (g_strcmp0(method_name, "GetTestString") == 0) {
            std::string result = service->GetTestString();
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", result.c_str()));
        }
        else if (g_strcmp0(method_name, "GetTestInfo") == 0) {
            TestInfo info = service->GetTestInfo();
            g_dbus_method_invocation_return_value(invocation, 
                g_variant_new("(bids)", 
                    info.bool_param, 
                    info.int_param, 
                    info.double_param, 
                    info.string_param.c_str()));
        }
        else if (g_strcmp0(method_name, "SendFile") == 0) {
            const gchar* shm_name;
            int chunk_size;
            int total_size;
            const gchar* file_name;
            
            g_variant_get(parameters, "(siii)", 
                &shm_name, &chunk_size, &total_size, &file_name);
            
            bool success = service->ReceiveFile(shm_name, chunk_size, total_size, file_name);
            g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", success));
        }
        else {
            g_dbus_method_invocation_return_error(
                invocation,
                G_DBUS_ERROR,
                G_DBUS_ERROR_UNKNOWN_METHOD,
                "Unknown method: %s",
                method_name);
        }
    }
    
    // 发送信号
    void emit_signal(const char* signal_name, GVariant* parameters) {
        if (!m_connection) return;
        
        g_dbus_connection_emit_signal(
            m_connection,
            nullptr, // sender
            "/com/example/TestService",
            "com.example.TestService",
            signal_name,
            parameters,
            nullptr // 不关心结果
        );
    }
    
    // 计算文件MD5
    std::string calculate_md5(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return "";
        }
        
        MD5_CTX md5Context;
        MD5_Init(&md5Context);
        
        char buffer[1024];
        while (file.good()) {
            file.read(buffer, 1024);
            MD5_Update(&md5Context, buffer, file.gcount());
        }
        
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5_Final(digest, &md5Context);
        
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            oss << std::setw(2) << (int)digest[i];
        }
        
        return oss.str();
    }
};

// 服务端主函数
int main() {
    std::cout << "Starting TestService..." << std::endl;
    
    // 创建服务实例
    auto service = std::make_shared<TestServiceImpl>();
    
    // 运行主循环
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    
    // 清理
    g_main_loop_unref(loop);
    
    return 0;
}    