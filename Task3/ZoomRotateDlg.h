#pragma once


// ZoomRotateDlg 对话框

class ZoomRotateDlg : public CDialog
{
	DECLARE_DYNAMIC(ZoomRotateDlg)

public:
	ZoomRotateDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ZoomRotateDlg();
	int getZorR(); //获取用户的选择
	double getParam();//获取用户的参数
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZOOM_ROTATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CEdit mEditParam;
	CComboBox mZoomOrRotateCombo;
	afx_msg void OnCbnSelchangeZorr();
};
