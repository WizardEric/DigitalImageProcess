#pragma once


// FilterDlg 对话框

class FilterDlg : public CDialog
{
	DECLARE_DYNAMIC(FilterDlg)

public:
	FilterDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~FilterDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox mFilterSelect;
	CEdit mFilterMeans;
	CEdit mFilterVariance;
	inline int getFilter()
	{
		return mFilterSelect.GetCurSel();
	}
	inline double getMeans()
	{
		CString param;
		mFilterMeans.GetWindowTextW(param);
		param.Trim();
		return _ttof(param);
	}
	inline double getVariance()
	{
		CString param;
		mFilterVariance.GetWindowTextW(param);
		param.Trim();
		return _ttof(param);
	}

	
	afx_msg void OnCbnSelchangeFilterselect();
};
