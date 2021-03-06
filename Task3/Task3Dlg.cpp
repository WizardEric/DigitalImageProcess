
// Task3Dlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Task3.h"
#include "Task3Dlg.h"
#include "afxdialogex.h"
#include <string.h>
#include "ImageProcess.h"
#include "MyMath.h"
#include "MyImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" void Zoom_host(byte* source, byte* result_buf, int HandleWidth, int HandleHeight, int SourceWidth, int SourceHeight,
	int source_pitch,int source_pixelSize,int handle_pitch,int handle_pixelSize);
extern "C" void Rotate_host(byte* source, byte* result_buf, int HandleWidth, int HandleHeight, int SourceWidth, int SourceHeight, double angle,
	int source_pitch, int source_pixelSize, int handle_pitch, int handle_pixelSize);


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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


// CTask3Dlg 对话框
CTask3Dlg::CTask3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TASK3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//初始化工作
	originImage = NULL;
	handledImage = NULL;
	handledImage_buf = NULL;

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	maxThreadNum = si.dwNumberOfProcessors * 2;

	threadNum = maxThreadNum;


}

void CTask3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAP, mTabControl);
	DDX_Control(pDX, IDC_LIB, mLibComboBox);
	DDX_Control(pDX, IDC_HANDLE, mHandlebutton);
	DDX_Control(pDX, IDC_SHOW_STATUS, mShowStatusEdit);
	DDX_Control(pDX, IDC_PICTURE_PATH, mFilePath);
	DDX_Control(pDX, IDC_THREAD_NUMBER, mThreadNumEdit);
	DDX_Control(pDX, IDC_ORIGIN_PICTURE, mOriginPictureControl);
	DDX_Control(pDX, IDC_HANDLED_PICTURE, mHandledPictureControl);
	DDX_Control(pDX, IDC_THREAD_SLIDER, mThreadNumSlider);
	DDX_Control(pDX, IDC_USE_ORI, mUseOrigin);
	DDX_Control(pDX, IDC_CLEAN, mCleanStatusSpce);
}

BEGIN_MESSAGE_MAP(CTask3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	//用户操作相关
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAP, &CTask3Dlg::OnTcnSelchangeTap) //选项卡更改选项通知
	ON_BN_CLICKED(IDC_OPEN_FILE, &CTask3Dlg::OnBnClickedOpenFile)	//打开文件通知
	ON_BN_CLICKED(IDC_HANDLE, &CTask3Dlg::OnBnClickedHandle)	//开始处理文件通知
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_THREAD_SLIDER, &CTask3Dlg::OnNMCustomdrawThreadSlider)	//用户拖动线程数量条通知
	ON_BN_CLICKED(IDC_CLEAN, &CTask3Dlg::OnBnClickedClean)	//用户点击清除状态栏通知
	ON_CBN_EDITCHANGE(IDC_LIB, &CTask3Dlg::OnCbnEditchangeLib)	//用户更改库选项文本通知
	ON_CBN_SELCHANGE(IDC_LIB, &CTask3Dlg::OnCbnSelchangeLib) //用户更改库选项通知

	//多线程消息同步相关
	ON_MESSAGE(WM_ZOOM, &CTask3Dlg::OnZoomThreadMsgReceived)
	ON_MESSAGE(WM_ROTATE, &CTask3Dlg::OnRotateThreadMsgReceived)
	ON_MESSAGE(WM_DFT, &CTask3Dlg::OnDFTThreadMsgReceived)
	ON_MESSAGE(WM_GAUSSNOISE, &CTask3Dlg::OnGaussNoiseThreadMsgReceived)
	ON_MESSAGE(WM_MEANFILTER, &CTask3Dlg::OnMeanFilterThreadMsgReceived)
	ON_MESSAGE(WM_GAUSSFILTER, &CTask3Dlg::OnGaussFilterThreadMsgReceived)
	ON_MESSAGE(WM_WIENERFILTER, &CTask3Dlg::OnWienerFilterThreadMsgReceived)
	ON_MESSAGE(WM_DFTCUDA, &CTask3Dlg::OnDFTCudaMsgReceived)

	//ON_BN_CLICKED(IDC_CHECK1, &CTask3Dlg::OnBnClickedCheck1)
	ON_EN_CHANGE(IDC_SHOW_STATUS, &CTask3Dlg::OnEnChangeShowStatus)	//自动添加的，暂时无用


END_MESSAGE_MAP()


// CTask3Dlg 消息处理程序

BOOL CTask3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	/*AllocConsole();*/

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//Tab项添加
	mTabControl.InsertItem(0, _T("旋转和缩放"));
	mTabControl.InsertItem(1, _T("傅里叶变换"));
	mTabControl.InsertItem(2, _T("高斯噪声"));
	mTabControl.InsertItem(3, _T("滤波"));

	//设置各个子对话框的父窗口为Tab Control
	ZoomRotateDlg.Create(IDD_ZOOM_ROTATE, GetDlgItem(IDC_TAP));
	FourierDlg.Create(IDD_FOURIER, GetDlgItem(IDC_TAP));
	GaussNoiseDlg.Create(IDD_GAUSS_NOISE, GetDlgItem(IDC_TAP));
	FilterDlg.Create(IDD_FILTER, GetDlgItem(IDC_TAP));

	//获得TabControl的整个工作区大小
	CRect tabArea;
	mTabControl.GetClientRect(&tabArea);

	//调整子对话框和父窗口的相对位置
	tabArea.top += 22;

	//设置子对话框尺寸并移动到指定位置 
	ZoomRotateDlg.MoveWindow(&tabArea);
	FourierDlg.MoveWindow(&tabArea);
	GaussNoiseDlg.MoveWindow(&tabArea);
	FilterDlg.MoveWindow(&tabArea);

	//分别设置隐藏和显示 
	ZoomRotateDlg.ShowWindow(true);
	FourierDlg.ShowWindow(false);
	GaussNoiseDlg.ShowWindow(false);
	FilterDlg.ShowWindow(false);

	//设置默认的选项卡 
	mTabControl.SetCurSel(0);


	//Slider的相关设置
	mThreadNumSlider.SetRange(1, maxThreadNum);
	mThreadNumSlider.SetPos(maxThreadNum);

	//多线程库的相关设置
	mLibComboBox.InsertString(0, _T("WIN多线程"));
	mLibComboBox.InsertString(1, _T("OpenMP"));
	mLibComboBox.InsertString(2, _T("CUDA"));
	mLibComboBox.SetCurSel(0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTask3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTask3Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		showOrigin();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTask3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTask3Dlg::SetUniversalParams()
{
	threadNum = mThreadNumSlider.GetPos();	//使用多少个线程
	lib = mLibComboBox.GetCurSel();	//选择的多线程库
	startTime = GetTickCount64();	//处理的开始时间
}

ThreadWorkSpace * CTask3Dlg::GetThreadWorkspaces(Operation process)
{
	ThreadWorkSpace* allThreadsWorkspace = new ThreadWorkSpace[threadNum];
	for (int i = 0; i < threadNum; i++)
	{
		//传入每个线程用来回传消息的窗口
		allThreadsWorkspace[i].window = (CWnd*)this;

		//传入用于处理的图片
		if (mUseOrigin.GetCheck() == BST_UNCHECKED)
		{
			//用户选择不使用原图像
			allThreadsWorkspace[i].img = handledImage;
		}
		else
		{
			//用户选择使用原图像
			allThreadsWorkspace[i].img = originImage;
		}

		//默认放入与待处理图像大小相等的图像用作处理
		int HandledImgWidth = allThreadsWorkspace[i].img->GetWidth();
		int HandledImgHeight = allThreadsWorkspace[i].img->GetHeight();

		//计算平均每个线程所需要处理的区域的大小
		long long areaSize = allThreadsWorkspace[i].img->GetWidth() * allThreadsWorkspace[i].img->GetHeight() / threadNum;

		//计算每个线程需要处理的区域
		allThreadsWorkspace[i].startIndex = i * areaSize;
		allThreadsWorkspace[i].endIndex = i != threadNum - 1 ? (i + 1)*areaSize - 1 : allThreadsWorkspace[i].img->GetWidth()*allThreadsWorkspace[i].img->GetHeight() - 1;

		//指向新的一片空间用于本次处理使用
		if (i == 0)
		{
			handledImage_buf = new CImage();
			handledImage_buf->Create(HandledImgWidth, HandledImgHeight, allThreadsWorkspace[i].img->GetBPP());
		}

		//传入处理图像保存的CImage对象		
		allThreadsWorkspace[i].handled = handledImage_buf;

		//传入算法所需要的参数
		if (process == Operation::Zoom)
		{
			zoomParam* param = new zoomParam;
			param->scale = ZoomRotateDlg.getParam();
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::Rotate)
		{
			rotateParam* param = new rotateParam;
			param->angle = ZoomRotateDlg.getParam();
			param->originHeight = allThreadsWorkspace[i].img->GetHeight();
			param->originWidth = allThreadsWorkspace[i].img->GetWidth();
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::DFT)
		{
			DFTParam* param = new DFTParam;
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::GaussNoise)
		{
			GaussNoiseParam* param = new GaussNoiseParam;
			param->means = GaussNoiseDlg.getMeans();
			param->variance = GaussNoiseDlg.getVariance();
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::MeanFilter)
		{
			MeanFilterParam* param = new MeanFilterParam;
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::GaussFilter)
		{
			GaussFilterParam* param = new GaussFilterParam;
			param->variance = FilterDlg.getVariance();
			allThreadsWorkspace[i].ctx = param;
		}
		else if (process == Operation::WienerFilter)
		{
			WienerFilterParam* param = new WienerFilterParam;
			allThreadsWorkspace[i].ctx = param;
		}
	}
	return allThreadsWorkspace;
}

void CTask3Dlg::showOrigin()
{
	if (originImage != NULL)
	{
		int height;
		int width;

		CRect WorkSpaceRect;
		CRect ShowImageRect;

		//记录图片本身的长和宽
		height = originImage->GetHeight();
		width = originImage->GetWidth();

		mOriginPictureControl.GetClientRect(&WorkSpaceRect); //根据GUI中的工作区生成一个矩形对象，左上角为（0，0）
		CDC *pDC = mOriginPictureControl.GetDC();	//设备上下文（Device Contex）->指向图片数据？
		SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);//设置图片拉伸的模式，设置为HalfTone，这个模式下，应用程序必须调用Win32函数 SetBrushOrgEx 设置画笔原点

		if (width <= WorkSpaceRect.Width() && height <= WorkSpaceRect.Width()) //图片完全可以在工作区中以原来的大小放下
		{
			ShowImageRect = CRect(WorkSpaceRect.TopLeft(), CSize(width, height)); //创建另一个矩形，左上角点和工作区一致，使用CSize传入矩形大小
			originImage->StretchBlt(pDC->m_hDC, ShowImageRect, SRCCOPY);//使用SRCCOPY直接将原图片数据传递至展示图片的矩形（ShowImageRect）
		}
		else //图片在工作区中不能以原来的大小放下
		{
			float xScale = (float)WorkSpaceRect.Width() / (float)width; //记录原工作区矩形和图片的长度比例
			float yScale = (float)WorkSpaceRect.Height() / (float)height; //记录原工作区矩形和图片的宽度比例
			float ScaleIndex = (xScale <= yScale ? xScale : yScale); //选出较小的比例

			ShowImageRect = CRect(WorkSpaceRect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex)); //创建一个由原图片按照刚刚获取的较小的比例缩放的，保证能整个放入工作区的矩形
			originImage->StretchBlt(pDC->m_hDC, ShowImageRect, SRCCOPY);//使用SRCCOPY直接将原图片数据传递至展示图片的矩形（ShowImageRect）
		}
		ReleaseDC(pDC); //将上下文释放
	}
}

void CTask3Dlg::showHandled()
{
	if (handledImage_buf != NULL)
	{
		int height;
		int width;

		CRect WorkSpaceRect;
		CRect ShowImageRect;

		//记录图片本身的长和宽
		height = handledImage_buf->GetHeight();
		width = handledImage_buf->GetWidth();

		mHandledPictureControl.GetClientRect(&WorkSpaceRect); //根据GUI中的工作区生成一个矩形对象，左上角为（0，0）
		CDC *pDC = mHandledPictureControl.GetDC();	//设备上下文（Device Contex）->指向图片数据？
		SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);//设置图片拉伸的模式，设置为HalfTone，这个模式下，应用程序必须调用Win32函数 SetBrushOrgEx 设置画笔原点

		if (width <= WorkSpaceRect.Width() && height <= WorkSpaceRect.Height()) //图片完全可以在工作区中以原来的大小放下
		{
			ShowImageRect = CRect(WorkSpaceRect.TopLeft(), CSize(width, height)); //创建另一个矩形，左上角点和工作区一致，使用CSize传入矩形大小
			handledImage_buf->StretchBlt(pDC->m_hDC, ShowImageRect, SRCCOPY);//使用SRCCOPY直接将原图片数据传递至展示图片的矩形（ShowImageRect）
		}
		else //图片在工作区中不能以原来的大小放下
		{
			float xScale = (float)WorkSpaceRect.Width() / (float)width; //记录原工作区矩形和图片的长度比例
			float yScale = (float)WorkSpaceRect.Height() / (float)height; //记录原工作区矩形和图片的宽度比例
			float ScaleIndex = (xScale <= yScale ? xScale : yScale); //选出较小的比例

			ShowImageRect = CRect(WorkSpaceRect.TopLeft(), CSize((int)width*ScaleIndex, (int)height*ScaleIndex)); //创建一个由原图片按照刚刚获取的较小的比例缩放的，保证能整个放入工作区的矩形
			handledImage_buf->StretchBlt(pDC->m_hDC, ShowImageRect, SRCCOPY);//使用SRCCOPY直接将原图片数据传递至展示图片的矩形（ShowImageRect）
		}
		ReleaseDC(pDC); //将上下文释放
	}
	else
	{
		MessageBox(_T("处理生成图片为空！"));
	}
}

void CTask3Dlg::ZoomOrRotate()
{
	if (ZoomRotateDlg.getZorR() == 0)
	{
		//旋转
		if (lib == 0)
		{
			Rotate_WIN();
		}
		else if (lib == 1)
		{
			Rotate_MP();
		}
		else
		{
			Rotate_CUDA();
		}
	}
	else
	{
		//缩放
		if (lib == 0)
		{
			Zoom_WIN();
		}
		else if (lib == 1)
		{
			Zoom_MP();
		}
		else if (lib == 2)
		{
			Zomm_CUDA();
		}
	}
}

void CTask3Dlg::Zoom_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::Zoom);

	//计算出缩放后图片的大小
	int HandledImgWidth = allThreadWorkspace[0].img->GetWidth()*ZoomRotateDlg.getParam();
	int HandledImgHeight = allThreadWorkspace[0].img->GetHeight()*ZoomRotateDlg.getParam();

	//计算每个线程需要处理的区域
	long long areaSize = HandledImgHeight * HandledImgWidth / threadNum;

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, allThreadWorkspace[0].img->GetBPP());


	for (int i = 0; i < threadNum; i++)
	{
		//传入经过缩放后处理图像保存的CImage对象
		allThreadWorkspace[i].handled = handledImage_buf;

		//传入每个线程所处理的区域
		allThreadWorkspace[i].startIndex = i * areaSize;
		allThreadWorkspace[i].endIndex = i != threadNum - 1 ? (i + 1)*areaSize - 1 : HandledImgHeight * HandledImgWidth - 1;

		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::zoom, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::Zoom_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::Zoom);

	//计算出缩放后图片的大小
	int HandledImgWidth = allThreadWorkspace[0].img->GetWidth()*ZoomRotateDlg.getParam();
	int HandledImgHeight = allThreadWorkspace[0].img->GetHeight()*ZoomRotateDlg.getParam();

	//计算每个线程需要处理的区域
	long long areaSize = HandledImgHeight * HandledImgWidth / threadNum;

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, allThreadWorkspace[0].img->GetBPP());

#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{

		//传入经过缩放后处理图像保存的CImage对象
		allThreadWorkspace[i].handled = handledImage_buf;

		//传入每个线程所处理的区域
		allThreadWorkspace[i].startIndex = i * areaSize;
		allThreadWorkspace[i].endIndex = i != threadNum - 1 ? (i + 1)*areaSize - 1 : HandledImgHeight * HandledImgWidth - 1;

		//传入处理函数，多线程进行处理
		ImageProcess::zoom(&allThreadWorkspace[i]);
	}
}

void CTask3Dlg::Zomm_CUDA()
{
	MyImage srcImage;
	CImage* srcImage_buf;
	if (mUseOrigin.GetCheck() == BST_UNCHECKED)
	{
		//用户选择不使用原图像
		srcImage_buf = handledImage;
		srcImage.setImage(handledImage);
	}
	else
	{
		//用户选择使用原图像
		srcImage_buf = originImage;
		srcImage.setImage(originImage);
	}

	//计算出缩放后图形的大小
	int HandledImgWidth = srcImage.getWidth()*ZoomRotateDlg.getParam();
	int HandledImgHeight = srcImage.getHeight()*ZoomRotateDlg.getParam();

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, srcImage.getBPP());

	MyImage handleImage(handledImage_buf);

	byte* srcImage_start= (byte*)srcImage_buf->GetBits() + srcImage_buf->GetPitch()*(srcImage_buf->GetHeight() - 1);
	byte* handleImage_start= (byte*)handledImage_buf->GetBits() + handledImage_buf->GetPitch()*(handledImage_buf->GetHeight() - 1);

	if (srcImage.isColorful())
	{
		Zoom_host(srcImage_start, handleImage_start, handleImage.getWidth(), handleImage.getHeight(), srcImage.getWidth(),
			srcImage.getHeight(),srcImage_buf->GetPitch(),srcImage_buf->GetBPP()/8,handledImage_buf->GetPitch(),handledImage_buf->GetBPP()/8);
	}


	if (handledImage != NULL)
	{
		delete handledImage;
	}
	handledImage = handledImage_buf;
	showHandled();
	endTime = GetTickCount64();

	CString text;
	mShowStatusEdit.GetWindowTextW(text);
	CString timeStr;
	timeStr.Format(_T("缩放,CUDA,参数:%f,处理%d次,耗时:%dms\r\n"), ZoomRotateDlg.getParam(), 1, endTime - startTime);
	text.Append(timeStr);

	mShowStatusEdit.SetWindowTextW(text);
}

void CTask3Dlg::Rotate_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::Rotate);

	//计算出图像的四个边界点的坐标
	double angle = ZoomRotateDlg.getParam();

	point<double> center;
	center.setPoint(allThreadWorkspace[0].img->GetWidth() / 2.0, allThreadWorkspace[0].img->GetHeight() / 2.0);

	//设置原图片的四个端点
	point<double> left_top; left_top.setPoint(0.0, 0.0);
	point<double> right_top; right_top.setPoint((allThreadWorkspace[0].img->GetWidth() - 1)*1.0, 0.0);
	point<double> left_bottom; left_bottom.setPoint(0.0, (allThreadWorkspace[0].img->GetHeight() - 1)*1.0);
	point<double> right_bottom; right_bottom.setPoint((allThreadWorkspace[0].img->GetWidth() - 1)*1.0, (allThreadWorkspace[0].img->GetHeight() - 1)*1.0);

	//经过旋转后的四个端点
	point<double> ro_left_top = rotate_point(left_top, center, angle);
	point<double> ro_left_bottom = rotate_point(left_bottom, center, angle);
	point<double> ro_right_top = rotate_point(right_top, center, angle);
	point<double> ro_right_bottom = rotate_point(right_bottom, center, angle);

	//计算旋转后的图片大小
	int HandledImgHeight = max(std::abs(ro_left_top.y - ro_right_bottom.y), std::abs(ro_left_bottom.y - ro_right_top.y));
	int HandledImgWidth = max(std::abs(ro_left_top.x - ro_right_bottom.x), std::abs(ro_left_bottom.x - ro_right_top.x));

	//计算每个线程需要处理的区域
	long long areaSize = HandledImgHeight * HandledImgWidth / threadNum;

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, allThreadWorkspace[0].img->GetBPP());

	for (int i = 0; i < threadNum; i++)
	{
		//传入经过旋转后处理图像保存的CImage对象
		allThreadWorkspace[i].handled = handledImage_buf;

		//传入原图中心的坐标
		rotateParam* param = new rotateParam;
		param->angle = ZoomRotateDlg.getParam();
		param->originHeight = allThreadWorkspace[i].img->GetHeight();
		param->originWidth = allThreadWorkspace[i].img->GetWidth();
		param->originCenter = center;
		allThreadWorkspace[i].ctx = param;

		//传入每个线程所处理的区域
		allThreadWorkspace[i].startIndex = i * areaSize;
		allThreadWorkspace[i].endIndex = i != threadNum - 1 ? (i + 1)*areaSize - 1 : HandledImgHeight * HandledImgWidth - 1;

		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::rotate, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::Rotate_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::Rotate);

	//计算出图像的四个边界点的坐标
	double angle = ZoomRotateDlg.getParam();

	point<double> center;
	center.setPoint(allThreadWorkspace[0].img->GetWidth() / 2.0, allThreadWorkspace[0].img->GetHeight() / 2.0);

	//设置原图片的四个端点
	point<double> left_top; left_top.setPoint(0.0, 0.0);
	point<double> right_top; right_top.setPoint((allThreadWorkspace[0].img->GetWidth() - 1)*1.0, 0.0);
	point<double> left_bottom; left_bottom.setPoint(0.0, (allThreadWorkspace[0].img->GetHeight() - 1)*1.0);
	point<double> right_bottom; right_bottom.setPoint((allThreadWorkspace[0].img->GetWidth() - 1)*1.0, (allThreadWorkspace[0].img->GetHeight() - 1)*1.0);

	//经过旋转后的四个端点
	point<double> ro_left_top = rotate_point(left_top, center, angle);
	point<double> ro_left_bottom = rotate_point(left_bottom, center, angle);
	point<double> ro_right_top = rotate_point(right_top, center, angle);
	point<double> ro_right_bottom = rotate_point(right_bottom, center, angle);

	//计算旋转后的图片大小
	int HandledImgHeight = max(std::abs(ro_left_top.y - ro_right_bottom.y), std::abs(ro_left_bottom.y - ro_right_top.y));
	int HandledImgWidth = max(std::abs(ro_left_top.x - ro_right_bottom.x), std::abs(ro_left_bottom.x - ro_right_top.x));

	//计算每个线程需要处理的区域
	long long areaSize = HandledImgHeight * HandledImgWidth / threadNum;

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, allThreadWorkspace[0].img->GetBPP());

	rotateParam* param = new rotateParam;
	param->angle = ZoomRotateDlg.getParam();
	param->originHeight = allThreadWorkspace[0].img->GetHeight();
	param->originWidth = allThreadWorkspace[0].img->GetWidth();
	param->originCenter = center;

#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{
		//传入经过旋转后处理图像保存的CImage对象
		allThreadWorkspace[i].handled = handledImage_buf;

		//传入原图中心的坐标
		allThreadWorkspace[i].ctx = param;

		//传入每个线程所处理的区域
		allThreadWorkspace[i].startIndex = i * areaSize;
		allThreadWorkspace[i].endIndex = i != threadNum - 1 ? (i + 1)*areaSize - 1 : HandledImgHeight * HandledImgWidth - 1;

		//传入处理函数，多线程进行处理
		ImageProcess::rotate(&allThreadWorkspace[i]);
	}
}

void CTask3Dlg::Rotate_CUDA()
{
	MyImage srcImage;
	CImage* srcImage_buf;
	if (mUseOrigin.GetCheck() == BST_UNCHECKED)
	{
		//用户选择不使用原图像
		srcImage_buf = handledImage;
		srcImage.setImage(handledImage);
	}
	else
	{
		//用户选择使用原图像
		srcImage_buf = originImage;
		srcImage.setImage(originImage);
	}

	double angle = ZoomRotateDlg.getParam();

	point<double> center;
	center.setPoint(srcImage.getWidth() / 2.0, srcImage.getHeight() / 2.0);

	//设置原图片的四个端点
	point<double> left_top; left_top.setPoint(0.0, 0.0);
	point<double> right_top; right_top.setPoint((srcImage.getWidth() - 1)*1.0, 0.0);
	point<double> left_bottom; left_bottom.setPoint(0.0, (srcImage.getHeight() - 1)*1.0);
	point<double> right_bottom; right_bottom.setPoint((srcImage.getWidth() - 1)*1.0, (srcImage.getHeight() - 1)*1.0);

	//经过旋转后的四个端点
	point<double> ro_left_top = rotate_point(left_top, center, angle);
	point<double> ro_left_bottom = rotate_point(left_bottom, center, angle);
	point<double> ro_right_top = rotate_point(right_top, center, angle);
	point<double> ro_right_bottom = rotate_point(right_bottom, center, angle);

	//计算旋转后的图片大小
	int HandledImgHeight = max(std::abs(ro_left_top.y - ro_right_bottom.y), std::abs(ro_left_bottom.y - ro_right_top.y));
	int HandledImgWidth = max(std::abs(ro_left_top.x - ro_right_bottom.x), std::abs(ro_left_bottom.x - ro_right_top.x));

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(HandledImgWidth, HandledImgHeight, srcImage.getBPP());

	MyImage handleImage(handledImage_buf);

	byte* srcImage_start = (byte*)srcImage_buf->GetBits() + srcImage_buf->GetPitch()*(srcImage_buf->GetHeight() - 1);
	byte* handleImage_start = (byte*)handledImage_buf->GetBits() + handledImage_buf->GetPitch()*(handledImage_buf->GetHeight() - 1);

	if (srcImage.isColorful())
	{
		Rotate_host(srcImage_start, handleImage_start, handleImage.getWidth(), handleImage.getHeight(), srcImage.getWidth(), srcImage.getHeight(), angle,
			srcImage_buf->GetPitch(), srcImage_buf->GetBPP() / 8, handledImage_buf->GetPitch(), handledImage_buf->GetBPP() / 8);
	}

	if (handledImage != NULL)
	{
		delete handledImage;
	}
	handledImage = handledImage_buf;
	showHandled();
	endTime = GetTickCount64();

	CString text;
	mShowStatusEdit.GetWindowTextW(text);
	CString timeStr;
	timeStr.Format(_T("旋转,CUDA,参数:%f,处理%d次,耗时:%dms\r\n"), ZoomRotateDlg.getParam(), 1, endTime - startTime);
	text.Append(timeStr);

	mShowStatusEdit.SetWindowTextW(text);
}

void CTask3Dlg::DFT()
{
	if (lib == 0)
	{
		//WIN多线程
		DFT_WIN();
	}
	else if (lib == 1)
	{
		//OpenMP多线程
		DFT_MP();
	}
	else
	{
		DFT_CUDA();
	}
}

void CTask3Dlg::DFT_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::DFT);
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::DFT, &allThreadWorkspace[i]);
	}
}

struct t {
	int threadNum;
	ThreadWorkSpace *allThreadWorkSpace;
};

void CTask3Dlg::DFT_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::DFT);

	t *tt = new t;
	tt->threadNum = threadNum;
	tt->allThreadWorkSpace = allThreadWorkspace;
	AfxBeginThread([](LPVOID param)->UINT
	{
		auto p = (t*)param;
		int threadNum = p->threadNum;
		auto allThreadWorkspace = p->allThreadWorkSpace;
#pragma omp parallel for num_threads(threadNum)
		for (int i = 0; i < threadNum; i++)
		{
			//传入处理函数，多线程进行处理
			ImageProcess::DFT(&allThreadWorkspace[i]);
		}
		delete param;
		return 0;
	}, tt);
}

void CTask3Dlg::DFT_CUDA()
{
	ThreadWorkSpace *workspace=new ThreadWorkSpace;

	MyImage srcImage;
	if (mUseOrigin.GetCheck() == BST_UNCHECKED)
	{
		//用户选择不使用原图像
		srcImage.setImage(handledImage);
		workspace->img = handledImage;
	}
	else
	{
		//用户选择使用原图像
		srcImage.setImage(originImage);
		workspace->img = originImage;
	}

	//生成一片新的区域用于处理
	handledImage_buf = new CImage();
	handledImage_buf->Create(srcImage.getWidth(), srcImage.getHeight(), srcImage.getBPP());

	workspace->handled = handledImage_buf;
	workspace->window = (CWnd*)this;

	AfxBeginThread(ImageProcess::DFT_CUDA,workspace);
}

void CTask3Dlg::GaussNoise()
{
	if (lib == 0)
	{
		//WIN多线程
		GaussNoise_WIN();
	}
	else
	{
		//OpenMP多线程
		GaussNoise_MP();
	}
}

void CTask3Dlg::GaussNoise_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::GaussNoise);
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::GaussNoise, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::GaussNoise_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::GaussNoise);

#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		ImageProcess::GaussNoise(&allThreadWorkspace[i]);
	}
}

void CTask3Dlg::Filter()
{
	if (FilterDlg.getFilter() == 0)
	{
		//均值滤波
		if (lib == 0)
		{
			MeanFilter_WIN();
		}
		else
		{
			MeanFilter_MP();
		}
	}
	else if (FilterDlg.getFilter() == 1)
	{
		//高斯滤波
		if (lib == 0)
		{
			GaussFilter_WIN();
		}
		else
		{
			GaussFilter_Mp();
		}
	}
	else if (FilterDlg.getFilter() == 2)
	{
		//维纳滤波
		if (lib == 0)
		{
			WienerFilter_WIN();
		}
		else
		{
			WienerFilter_MP();
		}
	}
}

void CTask3Dlg::MeanFilter_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::MeanFilter);
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::MeanFilter, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::MeanFilter_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::MeanFilter);
#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		ImageProcess::MeanFilter(&allThreadWorkspace[i]);
	}
}

void CTask3Dlg::GaussFilter_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::GaussFilter);
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::GaussFilter, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::GaussFilter_Mp()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::GaussFilter);
#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{
		//传入处理函数，多线程进行处理
		ImageProcess::GaussFilter(&allThreadWorkspace[i]);
	}
}

void CTask3Dlg::WienerFilter_WIN()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::WienerFilter);


	//计算整张图片的噪声方差平均值，放入参数内
	double variance_sum = 0;
	double variance_sum_R = 0;
	double variance_sum_G = 0;
	double variance_sum_B = 0;
	for (long long index = 0; index < allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight(); index++)
	{
		int x = index % allThreadWorkspace[0].img->GetWidth();
		int y = index / allThreadWorkspace[0].img->GetWidth();

		WienerFilterParam* param = (WienerFilterParam*)allThreadWorkspace[0].ctx;

		MyImage originImage(allThreadWorkspace[0].img);
		MyImage handleImage(allThreadWorkspace[0].handled);

		point<int> neighbors[3][3];
		param->getNeighborPoints(neighbors, x, y);

		if (!originImage.isColorful())
		{
			if (x<1 || y<1 || x>originImage.getWidth() - 2 || y>originImage.getHeight() - 2)
			{
				continue;
			}
			else
			{
				double sum = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						sum += originImage.readImage(neighbors[i][j].x, neighbors[i][j].y);
					}
				}
				double local_means = sum / 9.0;

				double variance = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						variance += (originImage.readImage(neighbors[i][j].x, neighbors[i][j].y) - local_means)*(originImage.readImage(neighbors[i][j].x, neighbors[i][j].y) - local_means) / 9.0;
					}
				}
				variance_sum += variance;
			}

		}
		else
		{
			if (x<1 || y<1 || x>originImage.getWidth() - 2 || y>originImage.getHeight() - 2)
			{
				continue;
			}
			else
			{
				double sum_R = 0;
				double sum_G = 0;
				double sum_B = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						sum_R += originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y);
						sum_G += originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y);
						sum_B += originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y);
					}
				}
				double local_means_R = sum_R / 9.0;
				double local_means_G = sum_G / 9.0;
				double local_means_B = sum_B / 9.0;

				double variance_R = 0;
				double variance_G = 0;
				double variance_B = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						variance_R += (originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y) - local_means_R)*(originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y) - local_means_R) / 9.0;
						variance_G += (originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y) - local_means_G)*(originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y) - local_means_G) / 9.0;
						variance_B += (originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y) - local_means_B)*(originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y) - local_means_B) / 9.0;
					}
				}
				variance_sum_R += variance_R;
				variance_sum_G += variance_G;
				variance_sum_B += variance_B;
			}
		}
	}

	double variance = variance_sum / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_R = variance_sum_R / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_G = variance_sum_G / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_B = variance_sum_B / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());

	for (int i = 0; i < threadNum; i++)
	{
		//传入参数
		delete allThreadWorkspace[i].ctx;
		WienerFilterParam* param = new WienerFilterParam;
		param->noise_variance = variance;
		param->noise_variance_R = variance_R;
		param->noise_variance_G = variance_B;
		param->noise_variance_B = variance_B;
		allThreadWorkspace[i].ctx = param;

		//传入处理函数，多线程进行处理
		AfxBeginThread(ImageProcess::WienerFilter, &allThreadWorkspace[i]);
	}
}

void CTask3Dlg::WienerFilter_MP()
{
	ThreadWorkSpace* allThreadWorkspace = GetThreadWorkspaces(Operation::WienerFilter);


	//计算整张图片的噪声方差平均值，放入参数内
	double variance_sum = 0;
	double variance_sum_R = 0;
	double variance_sum_G = 0;
	double variance_sum_B = 0;
	for (long long index = 0; index < allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight(); index++)
	{
		int x = index % allThreadWorkspace[0].img->GetWidth();
		int y = index / allThreadWorkspace[0].img->GetWidth();

		WienerFilterParam* param = (WienerFilterParam*)allThreadWorkspace[0].ctx;

		MyImage originImage(allThreadWorkspace[0].img);
		MyImage handleImage(allThreadWorkspace[0].handled);

		point<int> neighbors[3][3];
		param->getNeighborPoints(neighbors, x, y);

		if (!originImage.isColorful())
		{
			if (x<1 || y<1 || x>originImage.getWidth() - 2 || y>originImage.getHeight() - 2)
			{
				continue;
			}
			else
			{
				double sum = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						sum += originImage.readImage(neighbors[i][j].x, neighbors[i][j].y);
					}
				}
				double local_means = sum / 9.0;

				double variance = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						variance += (originImage.readImage(neighbors[i][j].x, neighbors[i][j].y) - local_means)*(originImage.readImage(neighbors[i][j].x, neighbors[i][j].y) - local_means) / 9.0;
					}
				}
				variance_sum += variance;
			}

		}
		else
		{
			if (x<1 || y<1 || x>originImage.getWidth() - 2 || y>originImage.getHeight() - 2)
			{
				continue;
			}
			else
			{
				double sum_R = 0;
				double sum_G = 0;
				double sum_B = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						sum_R += originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y);
						sum_G += originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y);
						sum_B += originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y);
					}
				}
				double local_means_R = sum_R / 9.0;
				double local_means_G = sum_G / 9.0;
				double local_means_B = sum_B / 9.0;

				double variance_R = 0;
				double variance_G = 0;
				double variance_B = 0;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						variance_R += (originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y) - local_means_R)*(originImage.readImage_R(neighbors[i][j].x, neighbors[i][j].y) - local_means_R) / 9.0;
						variance_G += (originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y) - local_means_G)*(originImage.readImage_G(neighbors[i][j].x, neighbors[i][j].y) - local_means_G) / 9.0;
						variance_B += (originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y) - local_means_B)*(originImage.readImage_B(neighbors[i][j].x, neighbors[i][j].y) - local_means_B) / 9.0;
					}
				}
				variance_sum_R += variance_R;
				variance_sum_G += variance_G;
				variance_sum_B += variance_B;
			}
		}
	}

	double variance = variance_sum / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_R = variance_sum_R / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_G = variance_sum_G / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());
	double variance_B = variance_sum_B / (allThreadWorkspace[0].img->GetWidth()*allThreadWorkspace[0].img->GetHeight());

#pragma omp parallel for num_threads(threadNum)
	for (int i = 0; i < threadNum; i++)
	{
		//传入参数
		delete allThreadWorkspace[i].ctx;
		WienerFilterParam* param = new WienerFilterParam;
		param->noise_variance = variance;
		param->noise_variance_R = variance_R;
		param->noise_variance_G = variance_B;
		param->noise_variance_B = variance_B;
		allThreadWorkspace[i].ctx = param;

		//传入处理函数，多线程进行处理
		ImageProcess::WienerFilter(&allThreadWorkspace[i]);
	}
}


void CTask3Dlg::OnTcnSelchangeTap(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int CurSel = mTabControl.GetCurSel();
	ZoomRotateDlg.ShowWindow(false);
	GaussNoiseDlg.ShowWindow(false);
	FourierDlg.ShowWindow(false);
	FilterDlg.ShowWindow(false);
	switch (CurSel)
	{
	case(0):
		ZoomRotateDlg.ShowWindow(true);
		break;
	case(1):
		FourierDlg.ShowWindow(true);
		break;
	case(2):
		GaussNoiseDlg.ShowWindow(true);
		break;
	case(3):
		FilterDlg.ShowWindow(true);
		break;
	default:
		break;
	}
	*pResult = 0;
}

void CTask3Dlg::OnBnClickedOpenFile()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files （*.*）|*.*||");
	CString filePath("");

	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	//如果用户选择了OK按钮
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
		mFilePath.SetWindowTextW(strFilePath);

		if (originImage != NULL)
		{
			delete originImage;
		}
		originImage = new CImage();
		originImage->Load(strFilePath);

		if (handledImage != NULL)
		{
			delete handledImage;
		}
		handledImage = new CImage();
		handledImage->Load(strFilePath);

		//重新绘制
		this->Invalidate();
	}
}

void CTask3Dlg::appendEdit(CString content)
{
	CString f;
	mShowStatusEdit.GetWindowTextW(f);
	f = f + content + _T("\r\n");
	mShowStatusEdit.SetWindowTextW(f);
}

void CTask3Dlg::OnBnClickedHandle()
{
	// TODO: 在此添加控件通知处理程序代码	
	switch (mTabControl.GetCurSel())
	{
	case(0):
	{
		CString text;
		mShowStatusEdit.GetWindowTextW(text);
		CString timeStr;
		timeStr.Format(_T("开始进行缩放或旋转处理\r\n"));
		text.Append(timeStr);
		mShowStatusEdit.SetWindowTextW(text);
		SetUniversalParams();

		ZoomOrRotate();
		break;
	}
	case(1):
	{
		CString text;
		mShowStatusEdit.GetWindowTextW(text);
		CString timeStr;
		timeStr.Format(_T("开始进行傅里叶变换处理\r\n"));
		text.Append(timeStr);
		mShowStatusEdit.SetWindowTextW(text);
		SetUniversalParams();

		DFT();
		break;
	}
	case(2):
	{
		CString text;
		mShowStatusEdit.GetWindowTextW(text);
		CString timeStr;
		timeStr.Format(_T("开始添加高斯噪声\r\n"));
		text.Append(timeStr);
		mShowStatusEdit.SetWindowTextW(text);
		SetUniversalParams();

		GaussNoise();
		break;
	}
	case(3):
	{
		CString text;
		mShowStatusEdit.GetWindowTextW(text);
		CString timeStr;
		timeStr.Format(_T("正在进行滤波\r\n"));
		text.Append(timeStr);
		mShowStatusEdit.SetWindowTextW(text);
		SetUniversalParams();

		Filter();
		break;
	}
	default:
		break;
	}
}

void CTask3Dlg::OnNMCustomdrawThreadSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString num;
	num.Format(_T("%d"), mThreadNumSlider.GetPos());
	mThreadNumEdit.SetWindowTextW(num);
	*pResult = 0;
}

void CTask3Dlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
}


LRESULT CTask3Dlg::OnZoomThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("缩放处理,线程数:%d,参数:%f,处理%d次,耗时:%dms\r\n"), threadNum, ZoomRotateDlg.getParam(), 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				Zoom_WIN();
			}
			else
			{
				//Ope 多线程
				Zoom_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("旋转处理,线程数:%d,参数:%f,处理%d次,耗时:%dms\r\n"), threadNum, ZoomRotateDlg.getParam(), 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				Rotate_WIN();
			}
			else
			{
				//Ope 多线程
				Rotate_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnDFTThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("傅里叶变换,线程数:%d,处理%d次,耗时:%dms\r\n"), threadNum, 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				DFT_WIN();
			}
			else
			{
				//Ope 多线程
				DFT_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnGaussNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("高斯噪声,线程数:%d,均值：%f,方差：%f,处理%d次,耗时:%dms\r\n"), threadNum, GaussNoiseDlg.getMeans(), GaussNoiseDlg.getVariance(), 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				GaussNoise_WIN();
			}
			else
			{
				//Ope 多线程
				GaussNoise_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnMeanFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("平滑线性滤波,线程数:%d,处理%d次,耗时:%dms\r\n"), threadNum, 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				MeanFilter_WIN();
			}
			else
			{
				//Ope 多线程
				MeanFilter_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnGaussFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("高斯滤波,线程数:%d,标准差：%f,处理%d次,耗时:%dms\r\n"), threadNum, FilterDlg.getVariance(), 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				MeanFilter_WIN();
			}
			else
			{
				//Ope 多线程
				MeanFilter_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnWienerFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;

	//成功完成一次线程处理
	if ((int)wParam == 1)
		tempCount++;
	//成功完成一次完整处理
	if (tempCount == threadNum)
	{
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount == 1)
		{
			if (handledImage != NULL)
			{
				delete handledImage;
			}
			handledImage = handledImage_buf;
			showHandled();

			//完成指定次数的处理
			tempProcessCount = 0;
			endTime = GetTickCount64();

			CString text;
			mShowStatusEdit.GetWindowTextW(text);
			CString timeStr;
			timeStr.Format(_T("维纳滤波,线程数:%d,处理%d次,耗时:%dms\r\n"), threadNum, FilterDlg.getVariance(), 1, endTime - startTime);
			text.Append(timeStr);

			mShowStatusEdit.SetWindowTextW(text);
		}
		else
		{
			//尚未完成，继续进行对应函数
			if (lib == 0)
			{
				//WINDOWS 多线程
				WienerFilter_WIN();
			}
			else
			{
				//Ope 多线程
				WienerFilter_MP();
			}
		}
	}
	return LRESULT();
}

LRESULT CTask3Dlg::OnDFTCudaMsgReceived(WPARAM wParam, LPARAM lParam)
{
	if (handledImage != NULL)
	{
		delete handledImage;
	}
	handledImage = handledImage_buf;
	showHandled();
	endTime = GetTickCount64();

	CString text;
	mShowStatusEdit.GetWindowTextW(text);
	CString timeStr;
	timeStr.Format(_T("傅里叶变换,CUDA,处理%d次,耗时:%dms\r\n"), 1, endTime - startTime);
	text.Append(timeStr);

	mShowStatusEdit.SetWindowTextW(text);

	return LRESULT();
}



void CTask3Dlg::OnEnChangeShowStatus()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CTask3Dlg::OnBnClickedClean()
{
	// TODO: 在此添加控件通知处理程序代码
	CString space;
	space.Format(_T(""));
	mShowStatusEdit.SetWindowTextW(space);
}


void CTask3Dlg::OnCbnEditchangeLib()
{

}


void CTask3Dlg::OnCbnSelchangeLib()
{
	if (mLibComboBox.GetCurSel() == 0 || mLibComboBox.GetCurSel() == 1)
	{
		//当用户选择使用WIN多线程和OPENMP时，恢复线程拖动条功能
		mThreadNumSlider.SetPos(1);
		mThreadNumSlider.SetRange(1, maxThreadNum);
		threadNum = mThreadNumSlider.GetPos();
		lib = mLibComboBox.GetCurSel();
	}
	else if (mLibComboBox.GetCurSel() == 2)
	{
		//当用户选择了使用CUDA进行处理,将多线程滑条禁用
		mThreadNumSlider.SetPos(1);
		mThreadNumSlider.SetRange(1, 1);
		threadNum = mThreadNumSlider.GetPos();
		lib = mLibComboBox.GetCurSel();
	}
}
