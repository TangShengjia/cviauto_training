#pragma once
#include <string>
#include <memory>
#include <vector>

// 定义TestInfo结构体
struct TestInfo {
    bool bool_param;
    int int_param;
    double double_param;
    std::string string_param;
};

// ITestListener接口 - 用于广播数据变更
class ITestListener {
public:
    virtual ~ITestListener() = default;
    
    virtual void OnTestBoolChanged(bool param) = 0;
    virtual void OnTestIntChanged(int param) = 0;
    virtual void OnTestDoubleChanged(double param) = 0;
    virtual void OnTestStringChanged(const std::string& param) = 0;
    virtual void OnTestInfoChanged(const TestInfo& param) = 0;
};

// ITestService接口 - 定义服务方法
class ITestService {
public:
    virtual ~ITestService() = default;
    
    // Set接口
    virtual bool SetTestBool(bool param) = 0;
    virtual bool SetTestInt(int param) = 0;
    virtual bool SetTestDouble(double param) = 0;
    virtual bool SetTestString(const std::string& param) = 0;
    virtual bool SetTestInfo(const TestInfo& param) = 0;
    
    // Get接口
    virtual bool GetTestBool() = 0;
    virtual int GetTestInt() = 0;
    virtual double GetTestDouble() = 0;
    virtual std::string GetTestString() = 0;
    virtual TestInfo GetTestInfo() = 0;
    
    // 文件传输接口
    virtual bool SendFile(const unsigned char* file_buf, size_t file_size, const std::string& file_name) = 0;
    
    // 注册和注销监听器
    virtual void RegisterListener(std::shared_ptr<ITestListener> listener) = 0;
    virtual void UnregisterListener(std::shared_ptr<ITestListener> listener) = 0;
};    