// MyTaskSchedulerDlg.h: 头文件
//

#pragma once
#include <string>

// CMyTaskSchedulerDlg 对话框
class CMyTaskSchedulerDlg : public CDialogEx
{
	// 构造
public:
	CMyTaskSchedulerDlg(CWnd* pParent = nullptr);	// 标准构造函数

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYTASKSCHEDULER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	// === 按钮点击事件声明 ===
	afx_msg void OnBnClickedBtnTaskA();
	afx_msg void OnBnClickedBtnTaskB();
	afx_msg void OnBnClickedBtnTaskC();
	afx_msg void OnBnClickedBtnTaskD();
	afx_msg void OnBnClickedBtnTaskE();

	// === 列表框事件 ===
	afx_msg void OnLbnSelchangeListLog();

	// === 窗口关闭事件 ===
	virtual void OnCancel();

	// === 自定义函数：更新日志 ===
	void AppendLog(std::string msg);
};