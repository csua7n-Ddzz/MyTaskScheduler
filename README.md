[readme.md](https://github.com/user-attachments/files/24478466/readme.md)


# MyTaskScheduler - 高性能多线程任务调度系统

**MyTaskScheduler** 是基于 C++ MFC 开发的通用任务调度系统。该项目演示了如何在 GUI 应用程序中实现**多线程并发**、**优先队列调度**以及**资源竞争管理**。系统采用了生产者-消费者模型，确保在执行高负载任务时，主界面（UI）依然保持流畅响应。

## 🖥️ 项目简介 (Project Overview)

本项目旨在解决单线程应用程序在处理耗时任务时界面卡顿的问题。通过将任务封装并提交给后台的工作线程（Worker Thread），系统实现了计算与界面的分离。

### 核心特性

* **多线程并发 (Concurrency):** UI 线程与工作线程分离，确保界面永远不卡顿。
* **智能调度 (Intelligent Scheduling):** 基于优先队列（Priority Queue），支持立即执行、延迟执行和周期性执行。
* **安全性设计 (Safety):** 采用 RAII 锁管理与“最小临界区”设计，彻底防止死锁。
* **便携性 (Portability):** 自动识别磁盘路径，支持断网保护，确保在任何机器上均可演示。

---

## 🚀 功能清单 (Task Implementation)

项目实现了以下五类典型任务，涵盖了文件 I/O、数学计算、网络请求和跨线程交互：

| 任务 | 名称 | 描述 | 技术亮点 |
| --- | --- | --- | --- |
| **Task A** | **文件备份** (File Backup) | 将日志文件备份至 `D:\Backup` (自动降级至 C 盘) | C++17 `std::filesystem`, 容错路径处理 |
| **Task B** | **矩阵计算** (Matrix Calc) | 200x200 矩阵乘法，CPU 密集型任务 | 验证多线程防卡顿能力 |
| **Task C** | **网络请求** (HTTP Request) | 请求 Github API 获取 Zen 语录 | `URLDownloadToFile`, 断网自动保护机制 |
| **Task D** | **课堂提醒** (Reminder) | 模拟课堂倒计时，弹出提示框 | 跨线程 UI 更新, Win32 API |
| **Task E** | **数据统计** (Statistics) | 生成随机数并计算均值与方差 | 数学运算, 验证优先队列插队逻辑 |

---

## 🛠️ 核心架构与防死锁设计 (Architecture & Deadlock Prevention)

这是本项目最核心的技术实现，位于 `SchedulerEngine.h` 中。

### 1. 生产者-消费者模型

* **UI 线程 (生产者):** 用户点击按钮，调用 `AddTask` 将任务压入队列。
* **Worker 线程 (消费者):** 后台线程循环扫描队列，取出符合时间要求的任务执行。

### 2. 防死锁机制 (Deadlock Prevention) ⭐

为了防止调度器在执行任务时发生死锁（例如任务内部再次调用 `AddTask` 导致的重入死锁），采用了 **"Unlock-before-Execute" (执行前解锁)** 策略：

```cpp
// 伪代码演示
void WorkerLoop() {
    Task currentTask;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 1. 在临界区内仅进行取任务操作
        if (HasTask()) {
            currentTask = queue.top();
            queue.pop();
        }
    } // 2. 离开作用域，锁自动释放 (RAII)

    // 3. 在【无锁状态】下执行用户任务
    // 即使 Execute 内部再次申请锁，也不会造成死锁
    if (currentTask) {
        currentTask->Execute();
    }
}

```

---

## 📖 演示指南 (Demo Guide)

建议按照以下顺序进行演示，以展示系统的最佳性能：

### 第一步：验证智能调度 (Smart Scheduling)

1. 先点击 **Task E (统计)**（设定有 10秒 延迟）。
2. 立即点击 **Task C (HTTP)**（立即执行）。
3. **观察:** 虽然 E 先提交，但 C 会**插队**先执行完毕。这证明了调度器是基于时间优先级的，而非简单的先进先出。

### 第二步：验证多线程性能 (Concurrency)

1. 点击 **Task B (矩阵)**。
2. **操作:** 在日志显示计算完成前，**快速拖动窗口**。
3. **观察:** 窗口移动丝滑，无“未响应”现象。证明后台计算未阻塞 UI。

### 第三步：验证交互与健壮性 (Robustness)

1. 点击 **Task D (提醒)**：观察屏幕弹出的对话框（跨线程 UI）。
2. 点击 **Task A (备份)**：检查 D 盘（或程序根目录）生成的备份文件。即使没有 D 盘，程序也会自动切换路径，不会崩溃。

---

## ⚙️ 环境依赖 (Prerequisites)

* **IDE:** Visual Studio 2019 / 2022
* **SDK:** Windows 10/11 SDK
* **Language Standard:** ISO C++17 Standard (`/std:c++17`)
* **Encoding:** 源代码已优化为英文提示，避免任何 GBK/UTF-8 编码冲突。

---

## 📂 项目结构

* `MyTaskSchedulerDlg.cpp/h`: 主界面逻辑，负责处理按钮点击事件。
* `SchedulerEngine.h`: 调度器核心，包含线程循环和优先队列。
* `TaskEngine.h`: 五个具体任务的实现逻辑（策略模式）。
* `LogUtils.h`: 线程安全的日志记录器（单例模式）。

---

### 作者

