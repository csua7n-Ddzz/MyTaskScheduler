// =================================================================================
// 项目名称: MyTaskScheduler (Project 3)
// 文件名称: LogUtils.h
// 修复方案: 强制 UTF-8 + BOM (彻底解决记事本乱码)
// =================================================================================
#pragma once
#include <fstream>
#include <mutex>
#include <string>
#include <ctime>
#include <iomanip>
#include <filesystem>

class LogWriter {
public:
    static LogWriter& Instance() {
        static LogWriter instance;
        return instance;
    }

    void Write(const std::string& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_ofs.is_open()) {
            std::time_t t = std::time(nullptr);
            std::tm tm;
            localtime_s(&tm, &t);

            // 写入时间 + 消息
            m_ofs << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                << message << std::endl;
        }
    }

private:
    LogWriter() {
        // 1. 以【覆盖模式】打开，每次运行清空旧日志
        m_ofs.open("scheduler.log", std::ios::out | std::ios::trunc);

        // 2. 【关键】写入 UTF-8 BOM (Byte Order Mark)
        // 这三个特定的字节 (0xEF, 0xBB, 0xBF) 是告诉记事本 "我是UTF-8" 的身份证
        // 加上这个，不管系统是中文还是英文，乱码都会消失
        unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
        m_ofs.write((char*)bom, 3);
    }

    ~LogWriter() {
        if (m_ofs.is_open()) {
            m_ofs.close();
        }
    }

    LogWriter(const LogWriter&) = delete;
    LogWriter& operator=(const LogWriter&) = delete;

    std::ofstream m_ofs;
    std::mutex m_mutex;
};