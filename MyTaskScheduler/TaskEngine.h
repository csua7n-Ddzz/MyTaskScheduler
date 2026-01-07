// =================================================================================
// 项目名称: MyTaskScheduler (Project 3)
// 文件名称: TaskEngine.h
// 编码格式: 请务必保存为 UTF-8 with BOM (带签名)
// =================================================================================
#pragma once
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <random>
#include <sstream>
#include <filesystem> // C++17 文件系统库
#include <fstream>
#include <chrono>     
#include <numeric>    // 用于计算均值
#include "LogUtils.h"

// === Windows 系统 API ===
#include <windows.h>
#include <urlmon.h>   

// 自动链接 urlmon.lib
#pragma comment(lib, "urlmon.lib")

namespace fs = std::filesystem;

// === 1. 抽象任务接口 ===
class ITask {
public:
    virtual ~ITask() {}
    virtual void Execute() = 0;
    virtual std::string GetName() const = 0;
};

// === 2. 具体任务实现 ===

// Task A: 文件备份 (纯净版，移除防死锁测试以避免循环依赖)
class CBackupTask : public ITask {
public:
    void Execute() override {
        LogWriter::Instance().Write("Task A [Backup]: 开始执行文件备份...");

        fs::path sourceFile = fs::current_path() / "scheduler.log";
        fs::path backupDir = "D:\\Backup";

        // 简单的盘符检测
        std::error_code ec;
        if (!fs::exists("D:\\", ec)) {
            backupDir = "C:\\Backup";
        }

        fs::path targetFile = backupDir / ("backup_" + GetTimestamp() + ".txt");

        try {
            if (!fs::exists(backupDir)) {
                fs::create_directories(backupDir);
            }

            if (fs::exists(sourceFile)) {
                fs::copy_file(sourceFile, targetFile, fs::copy_options::overwrite_existing);
                LogWriter::Instance().Write("Task A [Backup]: 备份成功! 保存至 " + targetFile.string());
            }
            else {
                LogWriter::Instance().Write("Task A [Backup]: 源文件尚未生成，跳过备份。");
            }
        }
        catch (const std::exception& e) {
            LogWriter::Instance().Write(std::string("Task A [Backup]: 异常 - ") + e.what());
        }
    }
    std::string GetName() const override { return "File Backup Task"; }

private:
    std::string GetTimestamp() {
        auto now = std::time(nullptr);
        std::stringstream ss;
        ss << now;
        return ss.str();
    }
};

// Task B: 矩阵计算
class CMatrixTask : public ITask {
public:
    void Execute() override {
        int size = 200;
        LogWriter::Instance().Write("Task B [Matrix]: 开始 200x200 矩阵乘法...");

        std::vector<std::vector<double>> matA(size, std::vector<double>(size, 1.0));
        std::vector<std::vector<double>> matB(size, std::vector<double>(size, 2.0));
        std::vector<std::vector<double>> matC(size, std::vector<double>(size, 0.0));

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                for (int k = 0; k < size; ++k) {
                    matC[i][j] += matA[i][k] * matB[k][j];
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;

        std::stringstream ss;
        ss << "Task B [Matrix]: 运算完成。耗时: " << diff.count() << " 秒";
        LogWriter::Instance().Write(ss.str());
    }
    std::string GetName() const override { return "Matrix Calc Task"; }
};

// Task C: HTTP GET Github
class CHttpTask : public ITask {
public:
    void Execute() override {
        LogWriter::Instance().Write("Task C [HTTP]: GET https://api.github.com/zen ...");

        std::wstring url = L"https://api.github.com/zen";
        fs::path savePath = fs::current_path() / "zen.txt";

        if (fs::exists(savePath)) fs::remove(savePath);

        HRESULT hr = URLDownloadToFile(NULL, url.c_str(), savePath.c_str(), 0, NULL);

        if (hr == S_OK) {
            std::ifstream f(savePath);
            std::string content;
            if (std::getline(f, content)) {
                LogWriter::Instance().Write("Task C [HTTP]: 请求成功! Github Zen 说: " + content);
            }
            else {
                LogWriter::Instance().Write("Task C [HTTP]: 请求成功 (文件已保存)");
            }
        }
        else {
            LogWriter::Instance().Write("Task C [HTTP]: 请求超时 (Github 可能无法访问)，演示结束。");
        }
    }
    std::string GetName() const override { return "HTTP Request Task"; }
};

// Task D: 课堂提醒
class CReminderTask : public ITask {
public:
    void Execute() override {
        LogWriter::Instance().Write("Task D [Reminder]: 触发提醒，正在弹出对话框...");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // 注意：L"" 字符串中如果包含中文，文件编码必须与编译器选项匹配
        ::MessageBox(NULL, L"休息 5 分钟！\n(Rest for 5 minutes)", L"课堂提醒", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

        LogWriter::Instance().Write("Task D [Reminder]: 用户已确认休息。");
    }
    std::string GetName() const override { return "Classroom Reminder"; }
};

// Task E: 随机数统计
class CStatsTask : public ITask {
public:
    void Execute() override {
        LogWriter::Instance().Write("Task E [Stats]: 生成 1000 个随机数并计算...");

        std::vector<int> numbers;
        numbers.reserve(1000);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 100);

        long long sum = 0;
        for (int i = 0; i < 1000; ++i) {
            int num = dis(gen);
            numbers.push_back(num);
            sum += num;
        }

        double mean = sum / 1000.0;

        double varianceSum = 0.0;
        for (int num : numbers) {
            varianceSum += (num - mean) * (num - mean);
        }
        double variance = varianceSum / 1000.0;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(4);
        ss << "Task E [Stats]: 均值(Mean) = " << mean << ", 方差(Variance) = " << variance;
        LogWriter::Instance().Write(ss.str());
    }
    std::string GetName() const override { return "Random Stats Task"; }
};

// === 3. 任务枚举 ===
enum class TaskType {
    Backup, Matrix, Http, Reminder, Stats
};

// === 4. 任务工厂 ===
class TaskFactory {
public:
    static std::shared_ptr<ITask> CreateTask(TaskType type) {
        switch (type) {
        case TaskType::Backup:   return std::make_shared<CBackupTask>();
        case TaskType::Matrix:   return std::make_shared<CMatrixTask>();
        case TaskType::Http:     return std::make_shared<CHttpTask>();
        case TaskType::Reminder: return std::make_shared<CReminderTask>();
        case TaskType::Stats:    return std::make_shared<CStatsTask>();
        default: return nullptr;
        }
    }
};