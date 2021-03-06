#pragma once


// GaussNoiseDlg 对话框

class GaussNoiseDlg : public CDialog
{
	DECLARE_DYNAMIC(GaussNoiseDlg)

public:
	GaussNoiseDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~GaussNoiseDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GAUSS_NOISE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit2();
	CEdit mMeans;
	CEdit mVariance;
	inline double getMeans()
	{
		CString param;
		mMeans.GetWindowTextW(param);
		param.Trim();
		return _ttof(param);
	}
	inline double getVariance()
	{
		CString param;
		mVariance.GetWindowTextW(param);
		param.Trim();
		return _ttof(param);
	}
};
