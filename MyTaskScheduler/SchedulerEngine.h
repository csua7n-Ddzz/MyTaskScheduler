// =================================================================================
// 项目名称: MyTaskScheduler (Project 3)
// 文件名称: SchedulerEngine.h
// 对应需求: 任务调度、优先队列、线程安全
// =================================================================================
#pragma once
#include "TaskEngine.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

// 定义 UI 回调函数类型 (Observer Pattern 的简化版)
using UINotifyCallback = std::function<void(std::string)>;

// 调度任务封装类 (Decorator/Wrapper)
struct ScheduledTask {
    std::shared_ptr<ITask> task;
    std::chrono::system_clock::time_point runTime; // 执行时间点
    bool isPeriodic;          // 是否周期性
    int intervalMs;           // 周期时间(毫秒)

    // 优先级比较：时间越早优先级越高 (最小堆)
    bool operator>(const ScheduledTask& other) const {
        return runTime > other.runTime;
    }
};

// 任务调度器 (Singleton + Producer-Consumer Pattern)
class TaskScheduler {
public:
    static TaskScheduler& Instance() {
        static TaskScheduler instance;
        return instance;
    }

    // 启动调度器
    void Start() {
        if (m_running) return;
        m_running = true;
        // 启动工作线程
        m_workerThread = std::thread(&TaskScheduler::WorkerLoop, this);
        LogWriter::Instance().Write("Scheduler Started.");
    }

    // 停止调度器
    void Stop() {
        m_running = false;
        m_cv.notify_all(); // 唤醒线程以便退出
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
        LogWriter::Instance().Write("Scheduler Stopped.");
    }

    // 设置 UI 通知回调
    void SetUICallback(UINotifyCallback cb) {
        m_uiCallback = cb;
    }

    // 添加任务
    // delayMs: 延迟多少毫秒执行 (0表示立即)
    // intervalMs: 周期执行间隔 (0表示一次性)
    void AddTask(std::shared_ptr<ITask> task, int delayMs = 0, int intervalMs = 0) {
        ScheduledTask sTask;
        sTask.task = task;
        sTask.runTime = std::chrono::system_clock::now() + std::chrono::milliseconds(delayMs);
        sTask.isPeriodic = (intervalMs > 0);
        sTask.intervalMs = intervalMs;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_taskQueue.push(sTask);
        }
        m_cv.notify_one(); // 唤醒工作线程：有新活儿了！

        // 通知UI
        std::stringstream ss;
        ss << "Scheduled: " << task->GetName() << " (Delay: " << delayMs << "ms)";
        if (m_uiCallback) m_uiCallback(ss.str());
    }

private:
    TaskScheduler() : m_running(false) {}
    ~TaskScheduler() { Stop(); }

    // 工作线程主循环
    void WorkerLoop() {
        while (m_running) {
            ScheduledTask currentTask;
            bool haveTask = false;

            {
                std::unique_lock<std::mutex> lock(m_mutex);

                // 如果队列为空，等待直到有新任务或停止
                if (m_taskQueue.empty()) {
                    m_cv.wait(lock, [this] { return !m_taskQueue.empty() || !m_running; });
                }

                if (!m_running) break;

                // 检查队首任务
                if (!m_taskQueue.empty()) {
                    auto now = std::chrono::system_clock::now();
                    auto topTask = m_taskQueue.top();

                    if (now >= topTask.runTime) {
                        // 时间到了，取出执行
                        currentTask = topTask;
                        m_taskQueue.pop();
                        haveTask = true;
                    }
                    else {
                        // 时间还没到，等待直到时间到或者有新任务插入
                        m_cv.wait_until(lock, topTask.runTime);
                    }
                }
            } // 锁在这里释放，执行任务不需要持锁（提高并发度）

            if (haveTask && currentTask.task) {
                try {
                    // === 执行任务 ===
                    // 通知UI开始
                    if (m_uiCallback) m_uiCallback("Executing: " + currentTask.task->GetName());

                    currentTask.task->Execute(); // 多态调用

                    // 通知UI完成
                    if (m_uiCallback) m_uiCallback("Finished: " + currentTask.task->GetName());

                    // 如果是周期任务，重新加入队列
                    if (currentTask.isPeriodic && m_running) {
                        currentTask.runTime = std::chrono::system_clock::now()
                            + std::chrono::milliseconds(currentTask.intervalMs);
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_taskQueue.push(currentTask);
                        }
                        // 不需要notify，因为线程还在循环里
                    }
                }
                catch (...) {
                    LogWriter::Instance().Write("Exception occurred in task execution!");
                }
            }
        }
    }

    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_workerThread;
    bool m_running;
    UINotifyCallback m_uiCallback;
};