
// Task3Dlg.h: 头文件
//

#pragma once
#include "ZoomRotateDlg.h"
#include "GaussNoiseDlg.h"
#include "ImageProcess.h"
#include "FourierDlg.h"
#include "FilterDlg.h"

// CTask3Dlg 对话框
class CTask3Dlg : public CDialogEx
{
// 构造
public:
	CTask3Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASK3_DIALOG };
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
	DECLARE_MESSAGE_MAP();

private:
	//各个子对话框
	ZoomRotateDlg ZoomRotateDlg;
	GaussNoiseDlg GaussNoiseDlg;
	FourierDlg FourierDlg;
	FilterDlg FilterDlg;

	//各类参数
	CImage * originImage;//未处理图像
	CImage * handledImage;//上次处理出来的图像
	CImage * handledImage_buf; //这次处理出来的图像
	int maxThreadNum;	//运行程序的电脑最多支持的线程数
	int threadNum;	//用户选择的处理线程数
	int lib;	//用户选择的多线程库
	unsigned long long startTime;	//开始时间
	unsigned long long endTime;	//结束时间

	//各种内置处理函数
	void SetUniversalParams();	//将通用参数赋值
	ThreadWorkSpace* GetThreadWorkspaces(Operation process);	//获得多个线程的工作区
	void showOrigin();	//打印原图片
	void showHandled();	//打印处理后图片

	//各种图像处理的实现
	void ZoomOrRotate();
	void Zoom_WIN();
	void Zoom_MP();
	void Zomm_CUDA();
	void Rotate_WIN();
	void Rotate_MP();
	void Rotate_CUDA();

	void DFT();
	void DFT_WIN();
	void DFT_MP();
	void DFT_CUDA();

	void GaussNoise();
	void GaussNoise_WIN();
	void GaussNoise_MP();

	void Filter();
	void MeanFilter_WIN();
	void MeanFilter_MP();
	void GaussFilter_WIN();
	void GaussFilter_Mp();
	void WienerFilter_WIN();
	void WienerFilter_MP();
 public:
	//公共功能区
	CEdit mFilePath;	//文件路径
	CSliderCtrl mThreadNumSlider;	//线程数拖拉框
	CStatic mThreadNumEdit;	//线程数的显示框
	CComboBox mLibComboBox; //选择多线程库的下拉框
	CButton mHandlebutton;	//处理图像的按钮
	CTabCtrl mTabControl;	//TabControl的handle
	CButton mUseOrigin;	//是否对原图进行处理
	CButton mCleanStatusSpce;//清空参数输出区域

	//图像区域
	CStatic mOriginPictureControl;	//原始图像控制区域
	CStatic mHandledPictureControl;	//处理后图像控制区域

	//参数、状态显示区域
	CEdit mShowStatusEdit;	//显示参数、状态的编辑框
	void appendEdit(CString text);	//在状态编辑框后面追加内容

	//控件事件
	afx_msg void OnBnClickedHandle();	//点击处理按钮的事件
	afx_msg void OnBnClickedOpenFile();	//打开文件按钮按下的事件
	afx_msg void OnTcnSelchangeTap(NMHDR *pNMHDR, LRESULT *pResult);	//Tap页的切换
	afx_msg void OnNMCustomdrawThreadSlider(NMHDR *pNMHDR, LRESULT *pResult);	//线程数的拖拉变换事件
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnEnChangeShowStatus();
	afx_msg void OnBnClickedClean();	//清空参数输出

	//多线程工作结束消息处理
	afx_msg LRESULT OnZoomThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDFTThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGaussNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMeanFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGaussFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWienerFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnDFTCudaMsgReceived(WPARAM wParam, LPARAM lParam);
	

	afx_msg void OnCbnEditchangeLib();
	afx_msg void OnCbnSelchangeLib();
};
