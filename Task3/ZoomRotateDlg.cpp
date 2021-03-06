// ZoomRotateDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Task3.h"
#include "ZoomRotateDlg.h"
#include "afxdialogex.h"


// ZoomRotateDlg 对话框

IMPLEMENT_DYNAMIC(ZoomRotateDlg, CDialog)

ZoomRotateDlg::ZoomRotateDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_ZOOM_ROTATE, pParent)
{
	
}

ZoomRotateDlg::~ZoomRotateDlg()
{
}

int ZoomRotateDlg::getZorR()
{
	return mZoomOrRotateCombo.GetCurSel();
}

double ZoomRotateDlg::getParam()
{
	CString param;
	mEditParam.GetWindowTextW(param);
	param.Trim();
	return _ttof(param);
}



void ZoomRotateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EditParam, mEditParam);
	DDX_Control(pDX, IDC_ZorR, mZoomOrRotateCombo);
}

BOOL ZoomRotateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	mZoomOrRotateCombo.InsertString(0,_T("旋转"));
	mZoomOrRotateCombo.InsertString(1, _T("缩放"));
	mZoomOrRotateCombo.SetCurSel(0);

	return 0;
}


BEGIN_MESSAGE_MAP(ZoomRotateDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_ZorR, &ZoomRotateDlg::OnCbnSelchangeZorr)
END_MESSAGE_MAP()


// ZoomRotateDlg 消息处理程序


void ZoomRotateDlg::OnCbnSelchangeZorr()
{
	// TODO: 在此添加控件通知处理程序代码

}


