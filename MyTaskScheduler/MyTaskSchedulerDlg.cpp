#include "pch.h"
#include "framework.h"
#include "MyTaskScheduler.h"
#include "MyTaskSchedulerDlg.h"
#include "afxdialogex.h"
#include "SchedulerEngine.h" // 引入核心调度引擎
#include "TaskEngine.h"      // 引入任务定义

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ===========================================================================
// CAboutDlg 对话框 (MFC 默认生成的关于框)
// ===========================================================================
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ===========================================================================
// CMyTaskSchedulerDlg 对话框
// ===========================================================================

CMyTaskSchedulerDlg::CMyTaskSchedulerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MYTASKSCHEDULER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMyTaskSchedulerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

// 消息映射表
BEGIN_MESSAGE_MAP(CMyTaskSchedulerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// === 按钮点击事件映射 ===
	ON_BN_CLICKED(IDC_BTN_TASK_A, &CMyTaskSchedulerDlg::OnBnClickedBtnTaskA)
	ON_BN_CLICKED(IDC_BTN_TASK_B, &CMyTaskSchedulerDlg::OnBnClickedBtnTaskB)
	ON_BN_CLICKED(IDC_BTN_TASK_C, &CMyTaskSchedulerDlg::OnBnClickedBtnTaskC)
	ON_BN_CLICKED(IDC_BTN_TASK_D, &CMyTaskSchedulerDlg::OnBnClickedBtnTaskD)
	ON_BN_CLICKED(IDC_BTN_TASK_E, &CMyTaskSchedulerDlg::OnBnClickedBtnTaskE)
	// === 列表框事件 (防止你双击生成了空函数导致报错) ===
	ON_LBN_SELCHANGE(IDC_LIST_LOG, &CMyTaskSchedulerDlg::OnLbnSelchangeListLog)
END_MESSAGE_MAP()


// ===========================================================================
// 初始化与核心逻辑
// ===========================================================================

BOOL CMyTaskSchedulerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// --- 默认生成的系统菜单代码 ---
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// =========================================================
	// === 项目3 核心逻辑：初始化调度器 ===
	// =========================================================

	// 1. 设置 UI 回调：当后台线程有日志时，调用此 Lambda 更新界面
	TaskScheduler::Instance().SetUICallback([this](std::string msg) {
		// 注意：这个回调是在后台工作线程运行的
		// 必须使用 SendMessage 跨线程通知主窗口更新，直接操作控件(AddString)是不安全的
		if (this->m_hWnd)
		{
			CString strMsg(msg.c_str());
			// 发送 LB_ADDSTRING 消息给列表框
			::SendMessage(::GetDlgItem(this->m_hWnd, IDC_LIST_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strMsg);

			// 自动滚动到最后一行
			int count = (int)::SendMessage(::GetDlgItem(this->m_hWnd, IDC_LIST_LOG), LB_GETCOUNT, 0, 0);
			::SendMessage(::GetDlgItem(this->m_hWnd, IDC_LIST_LOG), LB_SETTOPINDEX, count - 1, 0);
		}
		});

	// 2. 启动后台工作线程
	TaskScheduler::Instance().Start();

	return TRUE;
}

// ===========================================================================
// 按钮事件处理 (Factory + Strategy + Command)
// ===========================================================================

void CMyTaskSchedulerDlg::OnBnClickedBtnTaskA()
{
	// Task A: 备份 (延迟 2秒, 一次性)
	auto task = TaskFactory::CreateTask(TaskType::Backup);
	TaskScheduler::Instance().AddTask(task, 2000, 0);
}

void CMyTaskSchedulerDlg::OnBnClickedBtnTaskB()
{
	// Task B: 计算 (立即开始, 周期 5秒)
	auto task = TaskFactory::CreateTask(TaskType::Matrix);
	TaskScheduler::Instance().AddTask(task, 0, 5000);
}

void CMyTaskSchedulerDlg::OnBnClickedBtnTaskC()
{
	// Task C: HTTP (立即, 一次性)
	auto task = TaskFactory::CreateTask(TaskType::Http);
	TaskScheduler::Instance().AddTask(task, 0, 0);
}

void CMyTaskSchedulerDlg::OnBnClickedBtnTaskD()
{
	// Task D: 提醒 (立即, 周期 3秒 - 演示用)
	auto task = TaskFactory::CreateTask(TaskType::Reminder);
	TaskScheduler::Instance().AddTask(task, 0, 3000);
}

void CMyTaskSchedulerDlg::OnBnClickedBtnTaskE()
{
	// Task E: 统计 (延迟 10秒, 一次性)
	auto task = TaskFactory::CreateTask(TaskType::Stats);
	TaskScheduler::Instance().AddTask(task, 10000, 0);
}

// ===========================================================================
// 辅助函数与系统消息
// ===========================================================================

// 列表框被选中的事件（如果你不小心双击了列表框生成了这个，保留它以防报错）
void CMyTaskSchedulerDlg::OnLbnSelchangeListLog()
{
}

// 辅助日志函数实现 (对应你在头文件里加的声明)
void CMyTaskSchedulerDlg::AppendLog(std::string msg)
{
	// 这个函数供主线程直接调用使用
	CString str(msg.c_str());
	CListBox* pList = (CListBox*)GetDlgItem(IDC_LIST_LOG);
	if (pList) {
		pList->AddString(str);
		// 滚到底部
		int count = pList->GetCount();
		pList->SetTopIndex(count - 1);
	}
}

// 窗口关闭/销毁时
void CMyTaskSchedulerDlg::OnCancel()
{
	// 停止调度器线程，防止关闭窗口时后台线程还在跑导致崩溃
	TaskScheduler::Instance().Stop();

	CDialogEx::OnCancel();
}

void CMyTaskSchedulerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 绘制图标相关代码 (MFC 默认生成)
void CMyTaskSchedulerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CMyTaskSchedulerDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}