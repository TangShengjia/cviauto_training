#pragma once
#include <string>
#include <memory>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

class SharedMemory {
public:
    // 创建共享内存
    static std::shared_ptr<SharedMemory> Create(const std::string& name, size_t size) {
        return std::shared_ptr<SharedMemory>(new SharedMemory(name, size, true));
    }
    
    // 打开现有共享内存
    static std::shared_ptr<SharedMemory> Open(const std::string& name, size_t size) {
        return std::shared_ptr<SharedMemory>(new SharedMemory(name, size, false));
    }
    
    ~SharedMemory() {
        if (m_data) {
            munmap(m_data, m_size);
        }
        if (m_created && m_fd >= 0) {
            close(m_fd);
            shm_unlink(m_name.c_str());
        }
    }
    
    void* Data() const { return m_data; }
    size_t Size() const { return m_size; }
    
private:
    SharedMemory(const std::string& name, size_t size, bool create)
        : m_name(name), m_size(size), m_created(create) {
        
        // 创建或打开共享内存对象
        if (create) {
            m_fd = shm_open(m_name.c_str(), O_CREAT | O_RDWR, 0666);
            if (m_fd == -1) {
                throw std::runtime_error("Failed to create shared memory");
            }
            
            // 设置共享内存大小
            if (ftruncate(m_fd, size) == -1) {
                close(m_fd);
                throw std::runtime_error("Failed to set shared memory size");
            }
        } else {
            m_fd = shm_open(m_name.c_str(), O_RDWR, 0666);
            if (m_fd == -1) {
                throw std::runtime_error("Failed to open shared memory");
            }
        }
        
        // 映射共享内存
        m_data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        if (m_data == MAP_FAILED) {
            close(m_fd);
            throw std::runtime_error("Failed to map shared memory");
        }
    }
    
    std::string m_name;
    size_t m_size;
    int m_fd;
    void* m_data;
    bool m_created;
};    