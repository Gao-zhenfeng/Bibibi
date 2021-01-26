// BibibiDlg.cpp: 实现文件
//

//#include "stdafx.h"
#include "framework.h"
#include "Bibibi.h"
#include "BibibiDlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "PCAfitting.h"

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

// CBibibiDlg 对话框

CBibibiDlg::CBibibiDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BIBIBI_DIALOG, pParent)
	, Plane()
	, m_img0()
	, m_hengLSPoints(0)
	, m_hengCali(0)
	, m_shuLSPoints(0)
	, m_shuCali(0)
	, m_cornerPoint(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_strCalibrationPath = GetProgramePath() + CALI_PATH;
	m_strTestPath = GetProgramePath() + TEST_PATH;
}

void CBibibiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBibibiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CBibibiDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CBibibiDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &CBibibiDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

// CBibibiDlg 消息处理程序

BOOL CBibibiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CBibibiDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CBibibiDlg::OnPaint()
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
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBibibiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBibibiDlg::SetStatic(CString update)
{
	CString outdate;
	GetDlgItem(IDC_STATIC)->GetWindowText(outdate);
	CString		outputStr = outdate + "\r\n" + update;
	GetDlgItem(IDC_STATIC)->SetWindowText(outputStr);
	char* pch = new char[outputStr.GetLength() + 1];
	pch = outputStr.GetBuffer(outputStr.GetLength() + 1);
	outputStr.ReleaseBuffer();
	cout << pch << endl;
}

//相机标定//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBibibiDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (Calicam(Calicam0))
	{
		Califlags = true;
		CString update = "相机标定完成！";
		SetStatic(update);
	}
}

bool CBibibiDlg::Calicam(CaliProcess& Calicam)
{
	CString filenames;
	CString str;
	CString strnum;
	CString strLeftorRight;
	CString strSaveLeftorRight;
	CString strExtension;
	BeginWaitCursor();

	strLeftorRight = "\\C";
	strSaveLeftorRight = "cam";
	strExtension = ".bmp";

	int nFileCount = NCLAIIMAGES;	//标定所用图像个数，一般为6
	for (int i = 0; i < nFileCount; i++)
	{
		strnum.Format(_T("%d"), i);
		filenames = m_strCalibrationPath + strLeftorRight + strnum + strExtension;
		IplImage* tempProcessImage;
		tempProcessImage = cvLoadImage(filenames, IMREAD_GRAYSCALE);

		if (!tempProcessImage)
		{
			MessageBox("处理标定图像时，未能找到（或载入）图像，请检查！");
			return false;
		}
		Image2Use = cvCreateImage(cvSize(int(tempProcessImage->width), int(tempProcessImage->height)), tempProcessImage->depth, tempProcessImage->nChannels);

		TEllipseCollection m_Ellpco;	//保存从靶标上提取的椭圆信息
		int pointsize;	//椭圆个数
		pointsize = Calicam.CaliProcessNum(tempProcessImage, m_Ellpco, intervalX, intervalY);	//处理标定图像
		if (pointsize != NFEATUREPOINTS)
		{
			MessageBox("图像特征点数量不对，重新选取");
			return  false;
		}
		str.Format("CameraFeaturePicture%d.bmp", i);
		m_strSaveFileName = m_strCalibrationPath + "\\" + strSaveLeftorRight + str;
		cvSaveImage(m_strSaveFileName, Calicam.m_pColorImage);	//保存图像
		str.Format("CameraFeaturePts%d.txt", i);
		m_strSaveFileName = m_strCalibrationPath + "\\" + strSaveLeftorRight + str;
		Calicam.SaveCameraCaliPtsData(Calicam.m_vecFeaturePts, m_strSaveFileName);	//保存数据
		cvReleaseImage(&tempProcessImage);
	}
	//读取数据，进行标定
	if (!Calicam.m_vecFeaturePts.empty())
	{
		Calicam.m_vecFeaturePts.clear();
		vector<C4DPointD>().swap(Calicam.m_vecFeaturePts);
	}
	int nSize = 0;
	int nTemp = 0; //一幅图特征点*n幅图个点
	Calicam.m_pEachPtsCount1 = new int[nFileCount]; //一张图上有多少个点
	for (int i = 0; i < nFileCount; i++)
	{
		str.Format("CameraFeaturePts%d.txt", i);
		m_strSaveFileName = m_strCalibrationPath + "\\" + strSaveLeftorRight + str;;
		Calicam.LoadFeaturePtsData(m_strSaveFileName);	//读取标定数据
		nSize = Calicam.m_vecFeaturePts.size() - nTemp;
		nTemp = Calicam.m_vecFeaturePts.size();
		Calicam.m_pEachPtsCount1[i] = nSize;
	}
	if (Calicam.m_vecFeaturePts.size() <= 0)
	{
		MessageBox("读取文件错误");
		return false;
	}

	str = "Camera_CaliResult.xml";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;

	if (Calicam.ZhangCameraCali(Calicam.m_vecFeaturePts, Image2Use->width, Image2Use->height, nFileCount, Calicam.m_pEachPtsCount1,
		&m_InstrictMat_Calcu, &m_DistCoeffMat_Calcu, &rotation_martix_temp, &trans_vector_temp, m_strSaveFileName, nFileCount))
	{
		reprojectionErr(Calicam.m_vecFeaturePts, rotation_martix_temp, trans_vector_temp, m_InstrictMat_Calcu, m_DistCoeffMat_Calcu);
		EndWaitCursor();
		return true;
	}

	EndWaitCursor();
	return false;
}

bool CBibibiDlg::reprojectionErr(vector<C4DPointD> objPoints, CvMat* Rvec, CvMat* Tvec, CvMat* InstrictMat, CvMat* DistCoeffMat)
{
	double total_err = 0.0;//所有图像的平均误差的总和
	double err = 0.0;//每幅图像的平均误差
	CString update = "单幅图像的重投影误差";
	SetStatic(update);

	vector<Point3f> temPoints;
	vector<Point2f> imgPoints;//旧投影点
	vector<Point2f> imgPoints2;//新投影点
	temPoints.resize(NFEATUREPOINTS);
	imgPoints.resize(NFEATUREPOINTS);
	imgPoints2.resize(NFEATUREPOINTS);

	Mat RvecMat = Rvec;
	Mat TvecMat = Tvec;
	Mat Ins = InstrictMat;
	Mat Dist = DistCoeffMat;
	for (size_t i = 0; i < NCLAIIMAGES; i++)
	{
		Mat rotation_vector = RvecMat.rowRange(i, i + 1).clone();
		Mat trans_vector = TvecMat.rowRange(i, i + 1).clone();
		for (size_t j = 0; j < NFEATUREPOINTS; j++)
		{
			temPoints[j].x = objPoints[i * NFEATUREPOINTS + j].xw;
			temPoints[j].y = objPoints[i * NFEATUREPOINTS + j].yw;
			temPoints[j].z = objPoints[i * NFEATUREPOINTS + j].zw;
			imgPoints[j].x = objPoints[i * NFEATUREPOINTS + j].xf;
			imgPoints[j].y = objPoints[i * NFEATUREPOINTS + j].yf;
		}
		/* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */
		projectPoints(temPoints, rotation_vector, trans_vector, Ins, Dist, imgPoints2);
		/* 计算新的投影点和旧的投影点之间的误差*/
		Mat mat_imgPoints = Mat(1, temPoints.size(), CV_32FC2);;
		Mat mat_imgPoints2 = Mat(1, temPoints.size(), CV_32FC2);
		for (int k = 0; k < temPoints.size(); k++)
		{
			mat_imgPoints.at<Vec2f>(0, k) = Vec2f(imgPoints[k].x, imgPoints[k].y);
			mat_imgPoints2.at<Vec2f>(0, k) = Vec2f(imgPoints2[k].x, imgPoints2[k].y);
		}
		err = norm(mat_imgPoints2, mat_imgPoints, NORM_L2);
		total_err += err /= NFEATUREPOINTS;
		CString str;
		str.Format("%lf", err); //将变量组装到字符串中
		SetStatic(str);
	}

	update = "平均重投影误差";
	SetStatic(update);
	CString str;
	str.Format("%lf", total_err / NCLAIIMAGES);
	SetStatic(str);

	return TRUE;
}

//光平面标定///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBibibiDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	IplImage* tempProcessImage;
	CString filenames;
	CString str;
	CString strnum;
	CString strExtension;
	int nFileCount = LASERFILE;//光平面标定所用图像个数，一般为4

	strExtension = ".bmp";

	//CFileDialog dlg(TRUE, ".bmp", "*.bmp", OFN_ALLOWMULTISELECT, "BMP Files(*.bmp)|*.bmp|All Files (*.*)|*.*||", this);
	//dlg.m_ofn.lpstrTitle = "打开光平面标定图片";
	//if (dlg.DoModal() != IDOK)
	//	return;
	//vector<CString> vecDataFileName;
	//POSITION pos = dlg.GetStartPosition();
	//while (pos != NULL)
	//{
	//	CString strTemp;
	//	strTemp = dlg.GetNextPathName(pos);
	//	vecDataFileName.push_back(strTemp);
	//}
	//int nFileCount = vecDataFileName.size();
	//if (nFileCount != 4)
	//{
	//	MessageBox("请同时打开标定所需的4幅图像！");
	//	return;
	//}

	/*if (!m_vecCaliLSPts.empty())
	{
		m_vecCaliLSPts.clear();
		vector<vector<C4DPointD>>().swap(m_vecCaliLSPts);
	}*/

	Mat grayimg;
	Mat tempLaserImg;
	Mat tempCaliImg;
	for (int i = 0; i < nFileCount; i++)
	{
		//filenames = vecDataFileName[i];
		strnum.Format("\\l%d", i);
		filenames = m_strCalibrationPath + strnum + strExtension;
		tempProcessImage = cvLoadImage(filenames, 1);
		tempLaserImg = cvLoadImageM(filenames, 1);
		strnum.Format("\\c%d", i);
		filenames = m_strCalibrationPath + strnum + strExtension;
		tempCaliImg = cvLoadImageM(filenames, 1);

		if (!tempProcessImage)
		{
			MessageBox("处理标定图像时，未能找到（或载入）图像，请检查！");
			return;
		}

		//提取光条中心
		m_img0 = tempLaserImg - tempCaliImg;
		//m_img0 = cvarrToMat(tempProcessImage);
		cvtColor(m_img0, grayimg, CV_BGR2GRAY);
		medianBlur(grayimg, grayimg, 5);//中值滤波
		//adaptiveThreshold(grayimg, grayimg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 25, -1);

		//骨架提取细化/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//threshold(grayimg, grayimg, 120, 255, CV_THRESH_BINARY);
		//Mat structElement0 = getStructuringElement(MORPH_RECT, Size(10, 10), Point(-1, -1));
		//dilate(grayimg, grayimg, structElement0);
		//erode(grayimg, grayimg, structElement0);
		//	//分别提取水平线和垂直线
		//Mat heng, shu;
		//Mat kernel1 = getStructuringElement(MORPH_RECT, Size(grayimg.cols / 32, 1), Point(-1, -1));
		//Mat kernel2 = getStructuringElement(MORPH_RECT, Size(1, grayimg.rows / 32), Point(-1, -1));
		//erode(grayimg, heng, kernel1);
		//dilate(heng, heng, kernel1);
		//erode(grayimg, shu, kernel2);
		//dilate(shu, shu, kernel2);
		//	//条纹细化
		//Mat heng_thin, shu_thin;
		//cvThin(heng, heng_thin, 20);
		//cvThin(shu, shu_thin, 20);

		//灰度重心细化/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Mat structElement0 = getStructuringElement(MORPH_RECT, Size(10, 10), Point(-1, -1));
		dilate(grayimg, grayimg, structElement0);
		erode(grayimg, grayimg, structElement0);
		//分别提取水平线和垂直线
		Mat heng, shu;
		Mat kernel1 = getStructuringElement(MORPH_RECT, Size(grayimg.cols / 32, 1), Point(-1, -1));
		Mat kernel2 = getStructuringElement(MORPH_RECT, Size(1, grayimg.rows / 32), Point(-1, -1));
		erode(grayimg, heng, kernel1);
		dilate(heng, heng, kernel1);
		erode(grayimg, shu, kernel2);
		dilate(shu, shu, kernel2);
		//条纹细化
		Mat heng_thin, shu_thin;
		cvThin_h(heng, heng_thin);
		cvThin_s(shu, shu_thin);

		//线条排序
		linerank(heng_thin, shu_thin);//排序后得到m_hengLSPoints，m_shuLSPoints
			//角点
		m_grid = heng_thin + shu_thin;
		CornerDetect(m_grid);//m_cornerPoint
			//角点排序
		for (size_t ii = 0; ii < m_cornerPoint.size(); ii++)
		{
			PointRank(m_cornerPoint[ii].xf, m_cornerPoint[ii].yf, m_cornerPoint[ii].heng, m_cornerPoint[ii].shu);
		}

		//保存绘制细化后的图像
		str.Format("LSCenterPicture%d.bmp", i);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		IplImage imgTmp = m_img0;
		IplImage* input = cvCloneImage(&imgTmp);
		cvSaveImage(m_strSaveFileName, input);

		//保存光条数据
	//CString str;
	//str.Format("hengLSCenterPts%d.txt", i);
	//m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	//SaveLineData(m_hengLSPoints, m_strSaveFileName);//保存单幅光条中心的数据
	//str.Format("shuLSCenterPts%d.txt", i);
	//m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	//SaveLineData(m_shuLSPoints, m_strSaveFileName);//保存单幅光条中心的数据

		m_hengCali.push_back(m_hengLSPoints);
		m_shuCali.push_back(m_shuLSPoints);			//四幅光条中心的数据
		m_allPoints.push_back(m_cornerPoint);

		m_hengLSPoints.clear();
		m_shuLSPoints.clear();
		m_cornerPoint.clear();
		cvReleaseImage(&tempProcessImage);
	}

	//读取数据，进行标定
	if ((m_hengCali.size() <= 0) || (m_shuCali.size() <= 0))
	{
		MessageBox("读取文件错误");
		return;
	}

	str = ("Camera_CaliResult.xml");
	filenames = m_strCalibrationPath + "\\" + str;

	CvMemStorage* memstroagein = cvCreateMemStorage(0);
	CvFileStorage* fs_readin = cvOpenFileStorage(filenames, memstroagein, CV_STORAGE_READ);
	CvMat* caliresultin = cvCreateMat(3, 3, CV_64FC1);
	CvMat* caliresultdist = cvCreateMat(1, 4, CV_64FC1);
	CvMat* RotVec = cvCreateMat(6, 3, CV_64FC1);
	CvMat* TranVec = cvCreateMat(6, 3, CV_64FC1);

	caliresultin = (CvMat*)cvReadByName(fs_readin, NULL, "intrinsic_matrix1", NULL);
	caliresultdist = (CvMat*)cvReadByName(fs_readin, NULL, "distortion_coeffs1", NULL);
	RotVec = (CvMat*)cvReadByName(fs_readin, NULL, "rotation_vectors1", NULL);
	TranVec = (CvMat*)cvReadByName(fs_readin, NULL, "translation_vectors1", NULL);

	//图像坐标转世界坐标
	vector<vector<cornerPoint>> cornerPoints; //所有角点坐标
	cornerPoints.resize(hengNum * shuNum);
	for (size_t ii = 0; ii < nFileCount; ii++)
	{
		m_cornerPoint = m_allPoints[ii];
		UV2CamCrdn2(m_cornerPoint, caliresultin, caliresultdist, RotVec, TranVec, ii);

		str.Format("cornerPoint%d.txt", ii);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		SaveLineData(m_cornerPoint, m_strSaveFileName);

		for (size_t j = 0; j < m_cornerPoint.size(); j++)
		{
			int h = m_cornerPoint[j].heng;
			int s = m_cornerPoint[j].shu;
			int n = 9 * h + s;

			cornerPoints[n].insert(cornerPoints[n].end(), m_cornerPoint[j]);
		}
	}
	str.Format("allcornerPoint.txt");
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	SaveLineData(cornerPoints, m_strSaveFileName);//保存所有角点数据

	//拟合空间直线
		//求平均点
	vector<cornerPoint> averagePoints;
	for (size_t k = 0; k < hengNum * shuNum; k++)
	{
		cornerPoint Pts;
		vector<cornerPoint> cornerPt = cornerPoints[k];
		Pts.xw = (cornerPt[0].xw + cornerPt[1].xw + cornerPt[2].xw + cornerPt[3].xw) / 4;
		Pts.yw = (cornerPt[0].yw + cornerPt[1].yw + cornerPt[2].yw + cornerPt[3].yw) / 4;
		Pts.zw = (cornerPt[0].zw + cornerPt[1].zw + cornerPt[2].zw + cornerPt[3].zw) / 4;
		Pts.heng = k / 9;
		Pts.shu = k % 9;
		averagePoints.push_back(Pts);
	}

	//求拟合直线方向向量(参数方程形式)	x=x0+Dx*t;y=y0+Dy*t;z=z0+Dz*t
	vector<vector<double>> Direction;//拟合直线方程方向向量
	//Direction.resize(hengNum*shuNum);
	direction(cornerPoints, averagePoints, Direction);
	//保存标定数据
	str.Format("PointsCalib.txt");
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fk = fopen(m_strSaveFileName, "w");
	if (fk)
	{
		for (size_t ii = 0; ii < averagePoints.size(); ii++)
		{
			vector<double> direct = Direction[ii];
			fprintf(fk, "%d\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t\n", \
				averagePoints[ii].heng, averagePoints[ii].shu, averagePoints[ii].xw, averagePoints[ii].yw, averagePoints[ii].zw,
				direct[0], direct[1], direct[2]);
		}
	}
	fclose(fk);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vector<vector<C4DPointD>> hengLSPs;	//所有横线坐标
	hengLSPs.resize(hengNum);
	vector<vector<C4DPointD>> shuLSPs;	//所有竖线坐标
	shuLSPs.resize(shuNum);
	//根据光条中心的图像坐标得到光条中心的世界坐标
	for (size_t ii = 0; ii < nFileCount; ii++)
	{
		m_hengLSPoints = m_hengCali[ii];
		for (size_t iii = 0; iii < m_hengLSPoints.size(); iii++)
		{
			UV2CamCrdn(m_hengLSPoints[iii], caliresultin, caliresultdist, RotVec, TranVec, ii);
			hengLSPs[iii].insert(hengLSPs[iii].end(), m_hengLSPoints[iii].begin(), m_hengLSPoints[iii].end());
		}
		str.Format("heng_Pic%d.txt", ii);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		SaveLineData(m_hengLSPoints, m_strSaveFileName);//保存单幅光条中心的数据

		m_shuLSPoints = m_shuCali[ii];
		for (size_t iii = 0; iii < m_shuLSPoints.size(); iii++)
		{
			UV2CamCrdn(m_shuLSPoints[iii], caliresultin, caliresultdist, RotVec, TranVec, ii);
			shuLSPs[iii].insert(shuLSPs[iii].end(), m_shuLSPoints[iii].begin(), m_shuLSPoints[iii].end());
		}
		str.Format("shu_Pic%d.txt", ii);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		SaveLineData(m_shuLSPoints, m_strSaveFileName);//保存单幅光条中心的数据
	}

	for (size_t j = 0; j < hengLSPs.size(); j++)
	{
		str.Format("heng%d.txt", j);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		SaveLineData2(hengLSPs[j], m_strSaveFileName);//保存单幅光条中心的数据
	}
	for (size_t j = 0; j < shuLSPs.size(); j++)
	{
		str.Format("shu%d.txt", j);
		m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		SaveLineData2(shuLSPs[j], m_strSaveFileName);//保存单幅光条中心的数据
	}

	//拟合平面
	str = "hengPlane.txt";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fp = fopen(m_strSaveFileName, "w");
	if (!fp)
	{
		MessageBox("文件打开失败!");
	}
	for (size_t k = 0; k < hengLSPs.size(); k++)
	{
		//FittingPlane(hengLSPs[k], Plane, m_strSaveFileName);
		cvFitPlane(hengLSPs[k], Plane, m_strSaveFileName);
	}
	str = "shuPlane.txt";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fs = fopen(m_strSaveFileName, "w");
	if (!fs)
	{
		MessageBox("文件打开失败!");
	}
	for (size_t k = 0; k < shuLSPs.size(); k++)
	{
		//FittingPlane(shuLSPs[k], Plane, m_strSaveFileName);
		cvFitPlane(shuLSPs[k], Plane, m_strSaveFileName);
	}

	//误差
	//m_dPlaneError
	double a[9], b[9], c[9], d[9];
	str = "hengPlane.txt";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fr = fopen(m_strSaveFileName, "rt+");
	if (fr)
	{
		for (size_t ii = 0; ii < hengNum; ii++)
		{
			fscanf(fr, "%lf%lf%lf%lf\n", &a[ii], &b[ii], &c[ii], &d[ii]);
		}
		fclose(fr);
	}
	double PlaneErr;
	for (size_t k = 0; k < nFileCount; k++)
	{
		double PlaneErr = 0.0;//单幅误差
		m_hengLSPoints = m_hengCali[k];

		for (size_t kk = 0; kk < m_hengLSPoints.size(); kk++)
		{
			vector<C4DPointD> line = m_hengLSPoints[kk];

			Plane[0] = a[kk];
			Plane[1] = b[kk];
			Plane[2] = c[kk];
			Plane[3] = d[kk];
			for (size_t i = 0; i < line.size(); i++)
			{
				PlaneErr = Plane[0] * line[i].xw + Plane[1] * line[i].yw + Plane[2] * line[i].zw - Plane[3];
			}
		}
	}

	cvReleaseFileStorage(&fs_readin);
	cvReleaseMemStorage(&memstroagein);
	cvReleaseMat(&caliresultin);
	cvReleaseMat(&caliresultdist);
	m_hengLSPoints.clear();
	m_shuLSPoints.clear();
	m_cornerPoint.clear();
	MessageBox("光平面标定完成!");
}

//条纹细化（骨架提取
void CBibibiDlg::cvThin(Mat& src, Mat& dst, int intera)
{
	if (src.type() != CV_8UC1)
	{
		printf("只能处理二值或灰度图像\n");
		return;
	}
	//非原地操作时候，copy src到dst
	if (dst.data != src.data)
	{
		src.copyTo(dst);
	}

	int i, j, n;
	int width, height;
	width = src.cols - 1;
	//之所以减1，是方便处理8邻域，防止越界
	height = src.rows - 1;
	int step = src.step;
	int  p2, p3, p4, p5, p6, p7, p8, p9;
	uchar* img;
	bool ifEnd;
	int A1;
	cv::Mat tmpimg;
	//n表示迭代次数
	for (n = 0; n < intera; n++)
	{
		dst.copyTo(tmpimg);
		ifEnd = false;
		img = tmpimg.data;
		for (i = 1; i < height; i++)
		{
			img += step;
			for (j = 1; j < width; j++)
			{
				uchar* p = img + j;
				A1 = 0;
				if (p[0] > 0)
				{
					if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 01模式
					{
						A1++;
					}
					if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 01模式
					{
						A1++;
					}
					if (p[1] == 0 && p[step + 1] > 0) //p4,p5 01模式
					{
						A1++;
					}
					if (p[step + 1] == 0 && p[step] > 0) //p5,p6 01模式
					{
						A1++;
					}
					if (p[step] == 0 && p[step - 1] > 0) //p6,p7 01模式
					{
						A1++;
					}
					if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 01模式
					{
						A1++;
					}
					if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 01模式
					{
						A1++;
					}
					if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 01模式
					{
						A1++;
					}
					p2 = p[-step] > 0 ? 1 : 0;
					p3 = p[-step + 1] > 0 ? 1 : 0;
					p4 = p[1] > 0 ? 1 : 0;
					p5 = p[step + 1] > 0 ? 1 : 0;
					p6 = p[step] > 0 ? 1 : 0;
					p7 = p[step - 1] > 0 ? 1 : 0;
					p8 = p[-1] > 0 ? 1 : 0;
					p9 = p[-step - 1] > 0 ? 1 : 0;
					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
					{
						if ((p2 == 0 || p4 == 0 || p6 == 0) && (p4 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p6=0 && p4*p6*p8==0
						{
							dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
							ifEnd = true;
						}
					}
				}
			}
		}

		dst.copyTo(tmpimg);
		img = tmpimg.data;
		for (i = 1; i < height; i++)
		{
			img += step;
			for (j = 1; j < width; j++)
			{
				A1 = 0;
				uchar* p = img + j;
				if (p[0] > 0)
				{
					if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 01模式
					{
						A1++;
					}
					if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 01模式
					{
						A1++;
					}
					if (p[1] == 0 && p[step + 1] > 0) //p4,p5 01模式
					{
						A1++;
					}
					if (p[step + 1] == 0 && p[step] > 0) //p5,p6 01模式
					{
						A1++;
					}
					if (p[step] == 0 && p[step - 1] > 0) //p6,p7 01模式
					{
						A1++;
					}
					if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 01模式
					{
						A1++;
					}
					if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 01模式
					{
						A1++;
					}
					if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 01模式
					{
						A1++;
					}
					p2 = p[-step] > 0 ? 1 : 0;
					p3 = p[-step + 1] > 0 ? 1 : 0;
					p4 = p[1] > 0 ? 1 : 0;
					p5 = p[step + 1] > 0 ? 1 : 0;
					p6 = p[step] > 0 ? 1 : 0;
					p7 = p[step - 1] > 0 ? 1 : 0;
					p8 = p[-1] > 0 ? 1 : 0;
					p9 = p[-step - 1] > 0 ? 1 : 0;
					if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
					{
						if ((p2 == 0 || p4 == 0 || p8 == 0) && (p2 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p8=0 && p2*p6*p8==0
						{
							dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
							ifEnd = true;
						}
					}
				}
			}
		}
		//如果两个子迭代已经没有可以细化的像素了，则退出迭代
		if (!ifEnd) break;
	}

	/*for (size_t row = 0; row < dst.rows; row++)
	{
		for (size_t col = 0; col < dst.cols; col++)
		{
			int gray = dst.at<uchar>(row, col);
			if (gray == 255)
			{
				m_img0.at<Vec3b>(row, col) = Vec3b(0, 0, 255);
			}
		}
	}*/
}

//条纹细化（灰度重心横线
void CBibibiDlg::cvThin_h(Mat& src, Mat& dst)
{
	if (src.type() != CV_8UC1)
	{
		printf("只能处理灰度图像\n");
		return;
	}
	//非原地操作时候，copy src到dst
	if (dst.data != src.data)
	{
		src.copyTo(dst);
	}

	medianBlur(dst, dst, 5);//中值滤波
	Mat img0;
	cvtColor(dst, img0, CV_GRAY2BGR);

	int gray_th = m_graythred;
	int n = 0;
	int gray, gray_old = 0;
	for (int i = 0; i < dst.cols; i++)
	{
		float sum_value = 0;
		float sum_valuecoor = 0;
		vector<float>current_value;
		vector<float>current_coordinat;

		for (int j = 0; j < dst.rows; j++)
		{
			gray = dst.at<uchar>(j, i);
			if (gray >= gray_th)
			{
				if (gray_old < gray_th)
				{
					n++;
				}
				current_value.push_back(gray);
				current_coordinat.push_back(j);
			}

			if ((gray_old >= gray_th) && (gray < gray_th))
			{
				for (int k = 0; k < current_value.size(); k++)
				{
					sum_valuecoor += current_value[k] * current_coordinat[k];
					sum_value += current_value[k];
				}
				float x = sum_valuecoor / sum_value;
				//circle(img0, Point(i, x), 1, Scalar(0, 0, 255), -1, 8);
				img0.at<Vec3b>(x, i) = Vec3b(0, 0, 255);
				sum_valuecoor = 0;
				sum_value = 0;
				current_value.clear();
				current_coordinat.clear();
			}

			gray_old = gray;
		}
	}

	for (size_t ii = 0; ii < dst.rows; ii++)
	{
		for (size_t jj = 0; jj < dst.cols; jj++)
		{
			if (img0.at<Vec3b>(ii, jj) != Vec3b(0, 0, 255))
				dst.at<uchar>(ii, jj) = uchar(0);
			else
				dst.at<uchar>(ii, jj) = uchar(255);
		}
	}
}

//条纹细化（灰度重心竖线
void CBibibiDlg::cvThin_s(Mat& src, Mat& dst)
{
	if (src.type() != CV_8UC1)
	{
		printf("只能处理灰度图像\n");
		return;
	}
	//非原地操作时候，copy src到dst
	if (dst.data != src.data)
	{
		src.copyTo(dst);
	}

	medianBlur(dst, dst, 5);//中值滤波
	Mat img0;
	cvtColor(dst, img0, CV_GRAY2BGR);

	int gray_th = m_graythred;
	int n = 0;
	int gray, gray_old = 0;
	for (int i = 0; i < dst.rows; i++)
	{
		float sum_value = 0;
		float sum_valuecoor = 0;
		vector<float>current_value;
		vector<float>current_coordinat;

		for (int j = 0; j < dst.cols; j++)
		{
			gray = dst.at<uchar>(i, j);
			if (gray > gray_th)
			{
				if (gray_old <= gray_th)
				{
					n++;
				}
				current_value.push_back(gray);
				current_coordinat.push_back(j);
			}

			if ((gray_old >= gray_th) && (gray < gray_th))
			{
				for (int k = 0; k < current_value.size(); k++)
				{
					sum_valuecoor += current_value[k] * current_coordinat[k];
					sum_value += current_value[k];
				}
				float x = sum_valuecoor / sum_value;
				//circle(img0, Point(i, x), 1, Scalar(0, 0, 255), -1, 8);
				img0.at<Vec3b>(i, x) = Vec3b(0, 0, 255);
				sum_valuecoor = 0;
				sum_value = 0;
				current_value.clear();
				current_coordinat.clear();
			}

			gray_old = gray;
		}
	}

	for (size_t ii = 0; ii < dst.rows; ii++)
	{
		for (size_t jj = 0; jj < dst.cols; jj++)
		{
			if (img0.at<Vec3b>(ii, jj) != Vec3b(0, 0, 255))
				dst.at<uchar>(ii, jj) = uchar(0);
			else
				dst.at<uchar>(ii, jj) = uchar(255);
		}
	}
}

//条纹排序
void CBibibiDlg::linerank(Mat heng_thin, Mat shu_thin)
{
	int gray, heng_num = 0, shu_num = 0;
	int x = 30;		//设置方框范围
	for (size_t i = 0; i < 1024; i++)
	{
		gray = heng_thin.at<uchar>(i, 500);
		if (gray == 255)
		{
			vector<C4DPointD> line;
			for (size_t c = 0; c < 1280; c++)
			{
				for (size_t r = i - x; r < i + x; r++)
				{
					gray = heng_thin.at<uchar>(r, c);
					if (gray == 255)
					{
						C4DPointD p;
						p.xf = c;
						p.yf = r;
						line.push_back(p);
					}
				}
			}
			m_hengLSPoints.push_back(line);
			for (size_t n = 0; n < line.size(); n++)
			{
				/*Point pts;
				pts.x = line[n].xf;
				pts.y = line[n].yf;*/
				m_img0.at<Vec3b>(line[n].yf, line[n].xf) = Vec3b(0, 0, 255);
				//circle(m_img0, pts, 3, Scalar(0, 0, 255), 1, 8, 0);
			}
			vector<C4DPointD>().swap(line);
			heng_num++;
		}
	}
	for (size_t i = 0; i < 1280; i++)
	{
		gray = shu_thin.at<uchar>(512, i);
		if (gray == 255)
		{
			vector<C4DPointD> line;
			for (size_t r = 0; r < 1024; r++)
			{
				for (size_t c = i - x; c < i + x; c++)
				{
					gray = shu_thin.at<uchar>(r, c);
					if (gray == 255)
					{
						C4DPointD p;
						p.xf = c;
						p.yf = r;
						line.push_back(p);
					}
				}
			}
			m_shuLSPoints.push_back(line);
			for (size_t n = 0; n < line.size(); n++)
			{
				/*Point pts;
				pts.x = line[n].xf;
				pts.y = line[n].yf;*/
				m_img0.at<Vec3b>(line[n].yf, line[n].xf) = Vec3b(0, 0, 255);
				//circle(m_img0, pts, 3, Scalar(0, 0, 255), 1, 8, 0);
			}
			vector<C4DPointD>().swap(line);
			shu_num++;
		}
	}
}

//保存光条中心数据
void CBibibiDlg::SaveLineData(vector<vector<C4DPointD>> LSCenterPic, CString strPath)
{
	int nSize = LSCenterPic.size();
	if (nSize == 0)
	{
		MessageBox("数据为空，请检查！");
		return;
	}

	FILE* fp = fopen(strPath, "w");
	if (fp)
	{
		for (int i = 0; i < nSize; i++)
		{
			vector<C4DPointD> LineCenter = LSCenterPic[i];
			int n = LineCenter.size();
			for (int j = 0; j < n; j++)
			{
				fprintf(fp, "%lf\t%lf\t%lf\t%lf\t%lf\t\n", \
					LineCenter[j].xf, LineCenter[j].yf, LineCenter[j].xw, LineCenter[j].yw, LineCenter[j].zw);
			}
		}
		fclose(fp);
	}
}

void CBibibiDlg::SaveLineData2(vector<C4DPointD>& vecPts, CString strPath)
{
	int nSize = vecPts.size();
	if (nSize == 0)
	{
		MessageBox("保存特征点数据时,特征点容器为空，请检查！");
		return;
	}
	FILE* fp = fopen(strPath, "w");
	if (fp)
	{
		for (int i = 0; i < nSize; i++)
		{
			fprintf(fp, "%lf\t%lf\t%lf\t%lf\t%lf\t\n", \
				vecPts[i].xf, vecPts[i].yf, vecPts[i].xw, vecPts[i].yw, vecPts[i].zw);
		}
		fclose(fp);
	}
}

//保存角点数据
void CBibibiDlg::SaveLineData(vector<vector<cornerPoint>> LSCenterPic, CString strPath)
{
	int nSize = LSCenterPic.size();
	if (nSize == 0)
	{
		MessageBox("数据为空，请检查！");
		return;
	}

	FILE* fp = fopen(strPath, "w");
	if (fp)
	{
		for (int i = 0; i < nSize; i++)
		{
			vector<cornerPoint> cornerPts = LSCenterPic[i];
			int n = cornerPts.size();
			for (int j = 0; j < n; j++)
			{
				fprintf(fp, "%lf\t%lf\t%d\t%d\t%lf\t%lf\t%lf\t\n", \
					cornerPts[j].xf, cornerPts[j].yf, cornerPts[j].heng, cornerPts[j].shu,
					cornerPts[j].xw, cornerPts[j].yw, cornerPts[j].zw);
			}
		}
		fclose(fp);
	}
}

void CBibibiDlg::SaveLineData(vector<cornerPoint>& vecPts, CString strPath)
{
	int nSize = vecPts.size();
	if (nSize == 0)
	{
		MessageBox("保存特征点数据时,特征点容器为空，请检查！");
		return;
	}
	FILE* fp = fopen(strPath, "w");
	if (fp)
	{
		for (int ii = 0; ii < nSize; ii++)
		{
			fprintf(fp, "%lf\t%lf\t%d\t%d\t%lf\t%lf\t%lf\t\n", \
				vecPts[ii].xf, vecPts[ii].yf, vecPts[ii].heng, vecPts[ii].shu,
				vecPts[ii].xw, vecPts[ii].yw, vecPts[ii].zw);
		}
		fclose(fp);
	}
}

//图像坐标转为相机坐标
bool CBibibiDlg::UV2CamCrdn(vector<C4DPointD>& LineCenter, CvMat* intrinsic_matrix, CvMat* distortion_coeffs,
	CvMat* rotation_vectors, CvMat* translation_vectors, int nImgIndex)
{
	int nsize = LineCenter.size();
	if (nsize <= 0)
	{
		return FALSE;
	}

	double* u = new double[nsize];
	double* v = new double[nsize];
	ZeroMemory(u, nsize);
	ZeroMemory(v, nsize);

	for (size_t i = 0; i < nsize; i++)
	{
		u[i] = LineCenter[i].xf;
		v[i] = LineCenter[i].yf;
	}
	UndistortPoints(u, v, nsize, intrinsic_matrix, distortion_coeffs);//去畸变

	double plane[4] = { 0.0 };
	double T[3] = { 0.0 };

	CvMat* rotation_vector = cvCreateMat(1, 3, CV_64FC1);
	CvMat* rotation_matrix = cvCreateMat(3, 3, CV_64FC1);
	CvMat* trans_vector = cvCreateMat(1, 3, CV_64FC1);

	cvGetRow(rotation_vectors, rotation_vector, nImgIndex);
	cvGetRow(translation_vectors, trans_vector, nImgIndex);

	cvRodrigues2(rotation_vector, rotation_matrix);//将旋转向量变为旋转矩阵

	T[0] = CV_MAT_ELEM(*trans_vector, double, 0, 0);
	T[1] = CV_MAT_ELEM(*trans_vector, double, 0, 1);
	T[2] = CV_MAT_ELEM(*trans_vector, double, 0, 2);

	plane[0] = CV_MAT_ELEM(*rotation_matrix, double, 0, 2);
	plane[1] = CV_MAT_ELEM(*rotation_matrix, double, 1, 2);
	plane[2] = CV_MAT_ELEM(*rotation_matrix, double, 2, 2);
	plane[3] = -(plane[0] * T[0] + plane[1] * T[1] + plane[2] * T[2]);

	for (int i = 0; i < nsize; i++)
	{
		/*			double xc(0.0), yc(0.0), zc(0.0);*/
		UV2Cam(u[i], v[i], intrinsic_matrix, plane, LineCenter[i].xw, LineCenter[i].yw, LineCenter[i].zw);
	}

	delete[] u; delete[] v;
	cvReleaseMat(&rotation_matrix);
	cvReleaseMat(&rotation_vector);
	cvReleaseMat(&trans_vector);

	return TRUE;
}

bool CBibibiDlg::UV2CamCrdn2(vector<cornerPoint>& cornerPoints, CvMat* intrinsic_matrix, CvMat* distortion_coeffs,
	CvMat* rotation_vectors, CvMat* translation_vectors, int nImgIndex)
{
	int nsize = cornerPoints.size();
	if (nsize <= 0)
	{
		return FALSE;
	}

	double* u = new double[nsize];
	double* v = new double[nsize];
	ZeroMemory(u, nsize);
	ZeroMemory(v, nsize);

	for (size_t i = 0; i < nsize; i++)
	{
		u[i] = cornerPoints[i].xf;
		v[i] = cornerPoints[i].yf;
	}
	UndistortPoints(u, v, nsize, intrinsic_matrix, distortion_coeffs);//去畸变

	double plane[4] = { 0.0 };
	double T[3] = { 0.0 };

	CvMat* rotation_vector = cvCreateMat(1, 3, CV_64FC1);
	CvMat* rotation_matrix = cvCreateMat(3, 3, CV_64FC1);
	CvMat* trans_vector = cvCreateMat(1, 3, CV_64FC1);

	cvGetRow(rotation_vectors, rotation_vector, nImgIndex);
	cvGetRow(translation_vectors, trans_vector, nImgIndex);

	cvRodrigues2(rotation_vector, rotation_matrix);//将旋转向量变为旋转矩阵

	T[0] = CV_MAT_ELEM(*trans_vector, double, 0, 0);
	T[1] = CV_MAT_ELEM(*trans_vector, double, 0, 1);
	T[2] = CV_MAT_ELEM(*trans_vector, double, 0, 2);

	plane[0] = CV_MAT_ELEM(*rotation_matrix, double, 0, 2);
	plane[1] = CV_MAT_ELEM(*rotation_matrix, double, 1, 2);
	plane[2] = CV_MAT_ELEM(*rotation_matrix, double, 2, 2);
	plane[3] = -(plane[0] * T[0] + plane[1] * T[1] + plane[2] * T[2]);

	for (int i = 0; i < nsize; i++)
	{
		/*			double xc(0.0), yc(0.0), zc(0.0);*/
		UV2Cam(u[i], v[i], intrinsic_matrix, plane, cornerPoints[i].xw, cornerPoints[i].yw, cornerPoints[i].zw);
	}

	delete[] u; delete[] v;
	cvReleaseMat(&rotation_matrix);
	cvReleaseMat(&rotation_vector);
	cvReleaseMat(&trans_vector);

	return TRUE;
}

//去畸变
bool CBibibiDlg::UndistortPoints(double* px, double* py, int nCount, const CvMat* intrinsic_matrix, const CvMat* dist_coeffs)
{
	if (px == NULL || py == NULL)
	{
		return FALSE;
	}

	vector<CvPoint2D64f> points;
	points.clear();

	for (int i = 0; i < nCount; i++)
	{
		CvPoint2D64f temp;
		temp.x = px[i];
		temp.y = py[i];
		points.push_back(temp);
	}

	CvMat src = cvMat(nCount, 1, CV_64FC2, &points[0]);

	cvUndistortPoints(&src, &src, intrinsic_matrix, dist_coeffs, NULL, intrinsic_matrix);

	for (int i = 0; i < nCount; i++)
	{
		px[i] = points[i].x;
		py[i] = points[i].y;
	}

	//cvReleaseMat(src);
		//cvReleaseMat(&dst);
	return TRUE;
}

//图像坐标转三维相机坐标系
bool CBibibiDlg::UV2Cam(const double u, const double v, const CvMat* intrinsic_matrix, const double plane[4],
	double& xc, double& yc, double& zc)
{
	double fx = CV_MAT_ELEM(*intrinsic_matrix, double, 0, 0);
	double fy = CV_MAT_ELEM(*intrinsic_matrix, double, 1, 1);
	double u0 = CV_MAT_ELEM(*intrinsic_matrix, double, 0, 2);
	double v0 = CV_MAT_ELEM(*intrinsic_matrix, double, 1, 2);

	return UV2Cam(u, v, fx, fy, u0, v0, plane, xc, yc, zc);
}

bool CBibibiDlg::UV2Cam(const double u, const double v, const double fx, const double fy, const double u0, const double v0, const double plane[4],
	double& xc, double& yc, double& zc)
{
	double a = (u - u0) * plane[0] / fx;
	double b = (v - v0) * plane[1] / fy;
	double c = plane[2];
	double d = plane[3];

	zc = -d / (a + b + c);
	xc = zc * (u - u0) / fx;
	yc = zc * (v - v0) / fy;
	return TRUE;
}

//SVD求拟合直线方向向量
bool CBibibiDlg::direction(vector<vector<cornerPoint>> allcornerPts, vector<cornerPoint> averagePts, vector<vector<double>>& Direct)
{
	for (size_t i = 0; i < PtsNum; i++)
	{
		vector<cornerPoint> cornerPts = allcornerPts[i];
		int M = cornerPts.size();
		MatrixXd J(M, 3);
		for (size_t j = 0; j < M; j++)
		{
			J(j, 0) = cornerPts[j].xw - averagePts[i].xw,
				J(j, 1) = cornerPts[j].yw - averagePts[i].yw,
				J(j, 2) = cornerPts[j].zw - averagePts[i].zw;
		}
		JacobiSVD<MatrixXd> svd(J, ComputeThinU | ComputeThinV);
		MatrixXd V = svd.matrixV();
		MatrixXd U = svd.matrixU();
		MatrixXd A = svd.singularValues();//其中A为Σ对角线元素

		vector<double> D;
		for (size_t j = 0; j < V.rows(); j++)
		{
			D.push_back(V(j, 0));
		}
		Direct.push_back(D);
		D.clear();
	}

	return true;
}

//主元分析法拟合平面
bool CBibibiDlg::FittingPlane(vector<C4DPointD>& LineCenter, double plane[4], CString strPath)
{
	int nsize = LineCenter.size();

	double* xw = new double[nsize];
	double* yw = new double[nsize];
	double* zw = new double[nsize];

	for (int i = 0; i < nsize; i++)
	{
		xw[i] = LineCenter[i].xw;
		yw[i] = LineCenter[i].yw;
		zw[i] = LineCenter[i].zw;
	}

	FittingPlane(xw, yw, zw, nsize, plane, strPath);

	return TRUE;
}

bool CBibibiDlg::FittingPlane(double* xw, double* yw, double* zw, int nCount, double plane[4], CString strPath)
{
	double xmean = PCAfitting::one_data_mean(xw, nCount);
	double ymean = PCAfitting::one_data_mean(yw, nCount);
	double zmean = PCAfitting::one_data_mean(zw, nCount);

	double xycov = PCAfitting::two_data_mean_cov(xw, xmean, yw, ymean, nCount);
	double xzcov = PCAfitting::two_data_mean_cov(xw, xmean, zw, zmean, nCount);
	double yzcov = PCAfitting::two_data_mean_cov(yw, ymean, zw, zmean, nCount);
	double xxcov = PCAfitting::two_data_mean_cov(xw, xmean, xw, xmean, nCount);
	double yycov = PCAfitting::two_data_mean_cov(yw, ymean, yw, ymean, nCount);
	double zzcov = PCAfitting::two_data_mean_cov(zw, zmean, zw, zmean, nCount);

	double covmatrix[9];
	covmatrix[0] = xxcov; covmatrix[4] = yycov; covmatrix[8] = zzcov;
	covmatrix[1] = xycov; covmatrix[3] = xycov;
	covmatrix[2] = xzcov; covmatrix[6] = xzcov;
	covmatrix[5] = yzcov; covmatrix[7] = yzcov;

	double dvalue[3]; double dvector[9];
	PCAfitting::sym_matrix_egin_vector_value(covmatrix, 3, dvalue, dvector);

	double s1[3], s2[3], s3[3];
	int l = 0;
	double temp = dvalue[0];
	for (int i = 0; i < 3; i++)
	{
		if (temp >= dvalue[i])
		{
			temp = dvalue[i];
			l = i;
		}
	}
	int j = 0;
	for (int i = 0; i < 3; i++)
	{
		if (i != l)
		{
			if (j == 0)
			{
				s1[0] = dvector[i]; s1[1] = dvector[i + 3]; s1[2] = dvector[i + 6];
				j++;
			}
			else
			{
				s2[0] = dvector[i]; s2[1] = dvector[i + 3]; s2[2] = dvector[i + 6];
			}
		}
		else
		{
			s3[0] = dvector[i]; s3[1] = dvector[i + 3]; s3[2] = dvector[i + 6];
		}
	}
	double mean[3];
	mean[0] = xmean;
	mean[1] = ymean;
	mean[2] = zmean;
	PCAfitting::one_vector_one_point_plane(s3, mean, plane);

	FILE* fp = fopen(strPath, "a+");
	if (fp)
	{
		fprintf(fp, "%lf\t%lf\t%lf\t%lf\t\n", \
			plane[0], plane[1], plane[2], plane[3]);
		fclose(fp);
	}

	return TRUE;
}

//最小二乘拟合平面
bool CBibibiDlg::cvFitPlane(vector<C4DPointD>& LineCenter, double* plane, CString strPath)
{
	// Estimate geometric centroid.
	vector<double> X_vector;
	vector<double> Y_vector;
	vector<double> Z_vector;
	double xw, yw, zw, xf, yf;
	for (size_t i = 0; i < LineCenter.size(); i++)
	{
		xw = LineCenter[i].xw;
		yw = LineCenter[i].yw;
		zw = LineCenter[i].zw;
		X_vector.push_back(xw);
		Y_vector.push_back(yw);
		Z_vector.push_back(zw);
	}

	CvMat* points = cvCreateMat(X_vector.size(), 3, CV_32FC1);//定义用来存储需要拟合点的矩阵
	for (int i = 0; i < X_vector.size(); ++i)
	{
		points->data.fl[i * 3 + 0] = X_vector[i];//矩阵的值进行初始化   X的坐标值
		points->data.fl[i * 3 + 1] = Y_vector[i];//  Y的坐标值
		points->data.fl[i * 3 + 2] = Z_vector[i];
		//<span style = "font-family: Arial, Helvetica, sans-serif;">//  Z的坐标值</span>
	}

	int nrows = points->rows;
	int ncols = points->cols;
	int type = points->type;
	CvMat* centroid = cvCreateMat(1, ncols, type);
	cvSet(centroid, cvScalar(0));
	for (int c = 0; c < ncols; c++) {
		for (int r = 0; r < nrows; r++)
		{
			centroid->data.fl[c] += points->data.fl[ncols * r + c];
		}
		centroid->data.fl[c] /= nrows;
	}
	// Subtract geometric centroid from each point.
	CvMat* points2 = cvCreateMat(nrows, ncols, type);
	for (int r = 0; r < nrows; r++)
		for (int c = 0; c < ncols; c++)
			points2->data.fl[ncols * r + c] = points->data.fl[ncols * r + c] - centroid->data.fl[c];
	// Evaluate SVD of covariance matrix.
	CvMat* A = cvCreateMat(ncols, ncols, type);
	CvMat* W = cvCreateMat(ncols, ncols, type);
	CvMat* V = cvCreateMat(ncols, ncols, type);
	cvGEMM(points2, points, 1, NULL, 0, A, CV_GEMM_A_T);
	cvSVD(A, W, NULL, V, CV_SVD_V_T);
	// Assign plane coefficients by singular vector corresponding to smallest singular value.
	plane[ncols] = 0;
	for (int c = 0; c < ncols; c++) {
		plane[c] = V->data.fl[ncols * (ncols - 1) + c];
		plane[ncols] += plane[c] * centroid->data.fl[c];
	}

	FILE* fp = fopen(strPath, "a+");
	if (fp)
	{
		fprintf(fp, "%lf\t%lf\t%lf\t%lf\t\n", \
			plane[0], plane[1], plane[2], plane[3]);
		fclose(fp);
	}

	// Release allocated resources.
	cvReleaseMat(&centroid);
	cvReleaseMat(&points2);
	cvReleaseMat(&A);
	cvReleaseMat(&W);
	cvReleaseMat(&V);

	return TRUE;
}

bool CBibibiDlg::Cam2WCrdn(double u, double v,
	const CvMat* rotation_matrix, const CvMat* translation_vector,/*const CvMat* rotation_matrix_01,const CvMat* translation_vector_01,*/
	double& xw, double& yw, double& zw)
{
	CvMat* rotation_matrix_T = cvCreateMat(3, 3, CV_64FC1);
	CvMat* translation_vector_T = cvCreateMat(3, 1, CV_64FC1);
	CvMat* temp_rt = cvCreateMat(3, 1, CV_64FC1);
	CvMat* temp_rt_01 = cvCreateMat(3, 1, CV_64FC1);
	CvMat* object_point = cvCreateMat(3, 1, CV_64FC1);
	CvMat* temp_cc_points = cvCreateMat(3, 1, CV_64FC1);
	CV_MAT_ELEM(*object_point, double, 0, 0) = xw;
	CV_MAT_ELEM(*object_point, double, 1, 0) = yw;
	CV_MAT_ELEM(*object_point, double, 2, 0) = zw;

	//	cvTranspose(rotation_matrix, rotation_matrix_T);
	cvInvert(rotation_matrix, rotation_matrix_T, CV_SVD);
	cvTranspose(translation_vector, translation_vector_T);
	//用R的转置矩阵代替R的逆矩阵，为3*3矩阵，T转置矩阵为3*1矩阵
	cvMatMulAdd(rotation_matrix_T, translation_vector_T, NULL, temp_rt);
	//rt=r转置矩阵乘以t矩阵（t矩阵已经转为3*1矩阵了）
	//cvNot(temp_rt,temp_rt_not);
	cvMatMulAdd(rotation_matrix_T, object_point, NULL, temp_rt_01);
	double ii;
	double iii;
	ii = CV_MAT_ELEM(*temp_rt_01, double, 2, 0);
	iii = CV_MAT_ELEM(*temp_rt, double, 2, 0);
	cvSub(temp_rt_01, temp_rt, temp_cc_points, NULL);

	//	if (count==0)
	{
		xw = CV_MAT_ELEM(*temp_cc_points, double, 0, 0);
		yw = CV_MAT_ELEM(*temp_cc_points, double, 1, 0);
		zw = CV_MAT_ELEM(*temp_cc_points, double, 2, 0);
	}

	cvReleaseMat(&translation_vector_T);
	cvReleaseMat(&rotation_matrix_T);
	cvReleaseMat(&object_point);
	cvReleaseMat(&temp_cc_points);
	cvReleaseMat(&temp_rt);
	//	cvReleaseMat(&rotation_matrix_01_T);
	//	cvReleaseMat(&translation_vector_01_T);
	cvReleaseMat(&temp_rt);
	cvReleaseMat(&temp_rt_01);
	//	cvReleaseMat(&temp_final_point);
	//	cvReleaseMat(&temp_rrtt);

	return TRUE;
}

//测量///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBibibiDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码

	IplImage* tempProcessImage;
	CString filenames;
	CString str;
	CFileDialog dlg(TRUE, ".bmp", "*.bmp", OFN_ALLOWMULTISELECT, "BMP Files(*.bmp)|*.bmp|All Files (*.*)|*.*||", this);
	dlg.m_ofn.lpstrTitle = "打开测量图片";
	if (dlg.DoModal() != IDOK)
		return;
	vector<CString> vecDataFileName;
	POSITION pos = dlg.GetStartPosition();
	while (pos != NULL)
	{
		CString strTemp;
		strTemp = dlg.GetNextPathName(pos);
		vecDataFileName.push_back(strTemp);
	}
	int nFileCount = vecDataFileName.size();
	if (nFileCount != 1)
	{
		MessageBox("请选择1幅图像！");
		return;
	}

	/*if (!m_vecCaliLSPts.empty())
	{
		m_vecCaliLSPts.clear();
		vector<vector<C4DPointD>>().swap(m_vecCaliLSPts);
	}*/

	//Mat m_img0;
	Mat grayimg;

	filenames = vecDataFileName[0];
	tempProcessImage = cvLoadImage(filenames, 1);
	if (!tempProcessImage)
	{
		MessageBox("处理标定图像时，未能找到（或载入）图像，请检查！");
		return;
	}

	//提取光条中心
	m_img0 = cvarrToMat(tempProcessImage);
	cvtColor(m_img0, grayimg, CV_BGR2GRAY);
	medianBlur(grayimg, grayimg, 5);//中值滤波
	//adaptiveThreshold(grayimg, grayimg, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 25, -1);
	threshold(grayimg, grayimg, 120, 255, CV_THRESH_BINARY);
	//分别提取水平线和垂直线
	Mat heng, shu;
	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(grayimg.cols / 32, 1), Point(-1, -1));
	Mat kernel2 = getStructuringElement(MORPH_RECT, Size(1, grayimg.rows / 32), Point(-1, -1));
	erode(grayimg, heng, kernel1);
	dilate(heng, heng, kernel1);
	erode(grayimg, shu, kernel2);
	dilate(shu, shu, kernel2);
	//条纹细化
	Mat heng_thin, shu_thin;
	cvThin(heng, heng_thin, 20);
	cvThin(shu, shu_thin, 20);
	//线排序
	linerank(heng_thin, shu_thin);
	//角点
	m_grid = heng_thin + shu_thin;
	CornerDetect(m_grid);//m_cornerPoint
	//角点排序
	for (size_t i = 0; i < m_cornerPoint.size(); i++)
	{
		PointRank(m_cornerPoint[i].xf, m_cornerPoint[i].yf, m_cornerPoint[i].heng, m_cornerPoint[i].shu);
	}
	str.Format("cornerPoint.txt");
	m_strSaveFileName = m_strTestPath + "\\" + str;
	SaveLineData(m_cornerPoint, m_strSaveFileName);

	str.Format("testPicture.bmp");
	m_strSaveFileName = m_strTestPath + "\\" + str;
	IplImage imgTmp = m_img0;
	IplImage* input = cvCloneImage(&imgTmp);
	cvSaveImage(m_strSaveFileName, input);

	//读取数据
	str = ("Camera_CaliResult.xml");
	filenames = m_strCalibrationPath + "\\" + str;

	CvMemStorage* memstroagein = cvCreateMemStorage(0);
	CvFileStorage* fs_readin = cvOpenFileStorage(filenames, memstroagein, CV_STORAGE_READ);
	CvMat* caliresultin = cvCreateMat(3, 3, CV_64FC1);
	CvMat* caliresultdist = cvCreateMat(1, 4, CV_64FC1);
	CvMat* RotVec = cvCreateMat(6, 3, CV_64FC1);
	CvMat* TranVec = cvCreateMat(6, 3, CV_64FC1);

	caliresultin = (CvMat*)cvReadByName(fs_readin, NULL, "intrinsic_matrix1", NULL);
	caliresultdist = (CvMat*)cvReadByName(fs_readin, NULL, "distortion_coeffs1", NULL);
	RotVec = (CvMat*)cvReadByName(fs_readin, NULL, "rotation_vectors1", NULL);
	TranVec = (CvMat*)cvReadByName(fs_readin, NULL, "translation_vectors1", NULL);

	CvMat* rotation_vector = cvCreateMat(1, 3, CV_64FC1);
	CvMat* rotation_matrix = cvCreateMat(3, 3, CV_64FC1);
	CvMat* trans_vector = cvCreateMat(1, 3, CV_64FC1);

	cvGetRow(RotVec, rotation_vector, 0);
	cvGetRow(TranVec, trans_vector, 0);
	cvRodrigues2(rotation_vector, rotation_matrix);//将旋转向量变为旋转矩阵

	//通过空间直线方程进行三维测量
	int h[81], s[81];
	double x0[81], y0[81], z0[81], d1[81], d2[81], d3[81];
	str = "PointsCalib.txt";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fp = fopen(m_strSaveFileName, "rt+");
	if (fp)
	{
		for (size_t ii = 0; ii < PtsNum; ii++)
		{
			fscanf(fp, "%d\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t\n",
				&h[ii], &s[ii], &x0[ii], &y0[ii], &z0[ii], &d1[ii], &d2[ii], &d3[ii]);
		}
		fclose(fp);
	}

	UV2CCrdn(m_cornerPoint, x0, y0, z0, d1, d2, d3,
		caliresultin, caliresultdist, rotation_matrix, trans_vector);

	str.Format("Result.txt");
	m_strSaveFileName = m_strTestPath + "\\" + str;
	SaveLineData(m_cornerPoint, m_strSaveFileName);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*double *a = new double[hengNum];
	double *b = new double[hengNum];
	double *c = new double[hengNum];
	double *d = new double[hengNum];*/
	double a[9], b[9], c[9], d[9];
	str = "hengPlane.txt";
	m_strSaveFileName = m_strCalibrationPath + "\\" + str;
	FILE* fr = fopen(m_strSaveFileName, "rt+");
	if (fr)
	{
		for (size_t ii = 0; ii < hengNum; ii++)
		{
			fscanf(fr, "%lf%lf%lf%lf\n", &a[ii], &b[ii], &c[ii], &d[ii]);
		}
		fclose(fr);
	}

	str = "cornerPoint.txt";
	m_strSaveFileName = m_strTestPath + "\\" + str;
	FILE* fs = fopen(m_strSaveFileName, "w");
	for (size_t j = 0; j < m_cornerPoint.size(); j++)
	{
		int h = m_cornerPoint[j].heng;
		int s = m_cornerPoint[j].shu;

		//str = "hengPlane.txt";
		//m_strSaveFileName = m_strCalibrationPath + "\\" + str;
		//FILE *fr = fopen(m_strSaveFileName, "r");
		//if (fr)
		//{
		//	for (size_t ii=0; ii<heng; ii++)
		//	{
		//		fscanf(fr, "\n");	//换行
		//	}
		//	fscanf(fr, "%lf%lf%lf%lf", &Plane[0], &Plane[1], &Plane[2], &Plane[3]);
		//	fclose(fr);
		//}
		Plane[0] = a[h];
		Plane[1] = b[h];
		Plane[2] = c[h];
		Plane[3] = d[h];
		UV2WCrdn(m_cornerPoint[j].xf, m_cornerPoint[j].yf, caliresultin, Plane, rotation_matrix, trans_vector,
			m_cornerPoint[j].xw, m_cornerPoint[j].yw, m_cornerPoint[j].zw);

		str = "cornerPoint2.txt";
		m_strSaveFileName = m_strTestPath + "\\" + str;
		FILE* fs = fopen(m_strSaveFileName, "a+");
		if (fs)
		{
			fprintf(fs, "%lf\t%lf\t%d\t%d\t%lf\t%lf\t%lf\t\n", \
				m_cornerPoint[j].xf, m_cornerPoint[j].yf, m_cornerPoint[j].heng, m_cornerPoint[j].shu,
				m_cornerPoint[j].xw, m_cornerPoint[j].yw, m_cornerPoint[j].zw);
			fclose(fs);
		}
	}

	cvReleaseFileStorage(&fs_readin);
	cvReleaseMemStorage(&memstroagein);
	cvReleaseMat(&caliresultin);
	cvReleaseMat(&caliresultdist);
	m_cornerPoint.clear();
	MessageBox("完成!");
}

//角点检测（Shi_Tomasi焦点检测，亚像素角点检测）
bool CBibibiDlg::CornerDetect(Mat grayimg)
{
	//Shi-Tomasi算法
	vector<Point2f> corners;//提供初始角点的坐标位置和精确的坐标的位置
	int maxcorners = 200;
	double qualityLevel = 0.01;  //角点检测可接受的最小特征值
	double minDistance = 30;	//角点之间最小距离
	int blockSize = 3;//计算导数自相关矩阵时指定的领域范围
	double  k = 0.04; //权重系数

	goodFeaturesToTrack(grayimg, corners, maxcorners, qualityLevel, minDistance, Mat(), blockSize, false, k);
	//Mat():表示感兴趣区域；false:表示不用Harris角点检测

	//输出角点信息
	//cout << "角点信息为：" << corners.size() << endl;
	//绘制角点
	RNG rng(12345);
	for (unsigned i = 0; i < corners.size(); i++)
	{
		circle(m_img0, corners[i], 2, Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), -1, 8, 0);
		cout << "角点坐标：" << corners[i] << endl;
	}

	//寻找亚像素角点
	Size winSize = Size(5, 5);  //搜素窗口的一半尺寸
	Size zeroZone = Size(-1, -1);//表示死区的一半尺寸
	//求角点的迭代过程的终止条件，即角点位置的确定
	TermCriteria criteria = TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 40, 0.001);
	//TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 40, 0.001);
	cornerSubPix(grayimg, corners, winSize, zeroZone, criteria);

	//输出角点信息
	//cout << "角点信息为：" << corners.size() << endl;
	//绘制角点
	for (unsigned i = 0; i < corners.size(); i++)
	{
		circle(m_img0, corners[i], 3, Scalar(255, 0, 0), 0.5, 8, 0);
		cout << "角点坐标：" << corners[i] << endl;
	}

	m_cornerPoint.resize(corners.size());
	for (size_t i = 0; i < corners.size(); i++)
	{
		m_cornerPoint[i].xf = corners[i].x;
		m_cornerPoint[i].yf = corners[i].y;
	}

	return TRUE;
}

bool CBibibiDlg::PointRank(double x, double y, int& heng, int& shu)
{
	//m_hengLSPoints,m_shuLSPoints
	vector<C4DPointD> line;
	double distance;
	double p2p_dis = 100;
	double p2l_dis = 100;
	for (size_t i = 0; i < m_hengLSPoints.size(); i++)
	{
		line = m_hengLSPoints[i];
		for (size_t j = 0; j < line.size(); j++)
		{
			distance = powf((x - line[j].xf), 2) + powf((y - line[j].yf), 2);
			distance = sqrtf(distance);
			if (distance < p2p_dis)
			{
				p2p_dis = distance;
			}
		}
		if (p2p_dis < p2l_dis)
		{
			p2l_dis = p2p_dis;
			heng = i;
		}
	}

	p2p_dis = 100;
	p2l_dis = 100;
	for (size_t i = 0; i < m_shuLSPoints.size(); i++)
	{
		line = m_shuLSPoints[i];
		for (size_t j = 0; j < line.size(); j++)
		{
			distance = powf((x - line[j].xf), 2) + powf((y - line[j].yf), 2);
			distance = sqrtf(distance);
			if (distance < p2p_dis)
			{
				p2p_dis = distance;
			}
		}
		if (p2p_dis < p2l_dis)
		{
			p2l_dis = p2p_dis;
			shu = i;
		}
	}

	return TRUE;
}

bool CBibibiDlg::UV2WCrdn(double u, double v,
	const CvMat* intrinsic_matrix, const double plane[4],
	const CvMat* rotation_matrix, const CvMat* translation_vector,/*const CvMat* rotation_matrix_01,const CvMat* translation_vector_01,*/
	double& xw, double& yw, double& zw)
{
	UV2Cam(u, v, intrinsic_matrix, plane, xw, yw, zw);
	//通过uv坐标获取摄像机坐标系坐标(没有去畸变)

//
///*	CvMemStorage* memstorage2 = cvCreateMemStorage(0);
//	CvFileStorage* fs_write = cvOpenFileStorage("calibration_result02.xml", memstorage2, CV_STORAGE_WRITE);
//	cvWriteComment(fs_write, "calibration_result", 1);
//
//	cvWrite(fs_write, "intrinsic_matrix", intrinsic_matrix, cvAttrList(NULL,NULL) );
//	cvWrite(fs_write, "distortion_coeffs",rotation_matrix, cvAttrList(NULL,NULL) );
//	cvWrite(fs_write,"aaaa",translation_vector,cvAttrList(NULL,NULL));
//
//	cvReleaseFileStorage(&fs_write);*/
//
//	CvMat *rotation_matrix_T = cvCreateMat(3, 3, CV_64FC1);
//	CvMat *translation_vector_T = cvCreateMat(3, 1, CV_64FC1);
//	CvMat *temp_rt = cvCreateMat(3, 1, CV_64FC1);
//	CvMat *temp_rt_01 = cvCreateMat(3, 1, CV_64FC1);
//	CvMat *object_point = cvCreateMat(3, 1, CV_64FC1);
//	CvMat *temp_cc_points = cvCreateMat(3, 1, CV_64FC1);
//	CV_MAT_ELEM(*object_point, double, 0, 0) = xw;
//	CV_MAT_ELEM(*object_point, double, 1, 0) = yw;
//	CV_MAT_ELEM(*object_point, double, 2, 0) = zw;
//
//	//	cvTranspose(rotation_matrix, rotation_matrix_T);
//	cvInvert(rotation_matrix, rotation_matrix_T, CV_SVD);
//	cvTranspose(translation_vector, translation_vector_T);
//	//用R的转置矩阵代替R的逆矩阵，为3*3矩阵，T转置矩阵为3*1矩阵
//	cvMatMulAdd(rotation_matrix_T, translation_vector_T, NULL, temp_rt);
//	//rt=r转置矩阵乘以t矩阵（t矩阵已经转为3*1矩阵了）
//	//cvNot(temp_rt,temp_rt_not);
//	cvMatMulAdd(rotation_matrix_T, object_point, NULL, temp_rt_01);
//	double ii;
//	double iii;
//	ii = CV_MAT_ELEM(*temp_rt_01, double, 2, 0);
//	iii = CV_MAT_ELEM(*temp_rt, double, 2, 0);
//	cvSub(temp_rt_01, temp_rt, temp_cc_points, NULL);
//
//	//	if (count==0)
//	{
//		xw = CV_MAT_ELEM(*temp_cc_points, double, 0, 0);
//		yw = CV_MAT_ELEM(*temp_cc_points, double, 1, 0);
//		zw = CV_MAT_ELEM(*temp_cc_points, double, 2, 0);
//	}
//
//	cvReleaseMat(&translation_vector_T);
//	cvReleaseMat(&rotation_matrix_T);
//	cvReleaseMat(&object_point);
//	cvReleaseMat(&temp_cc_points);
//	cvReleaseMat(&temp_rt);
//	//	cvReleaseMat(&rotation_matrix_01_T);
//	//	cvReleaseMat(&translation_vector_01_T);
//	cvReleaseMat(&temp_rt);
//	cvReleaseMat(&temp_rt_01);
//	//	cvReleaseMat(&temp_final_point);
//	//	cvReleaseMat(&temp_rrtt);

	return TRUE;
}

bool CBibibiDlg::UV2CCrdn(vector<cornerPoint>& cornerPoints, double* x0, double* y0, double* z0, double* d1, double* d2, double* d3,
	const CvMat* intrinsic_matrix, CvMat* distortion_coeffs, const CvMat* rotation_matrix, const CvMat* translation_vector)
{
	int nsize = cornerPoints.size();
	if (nsize <= 0)
	{
		return FALSE;
	}

	double* u = new double[nsize];
	double* v = new double[nsize];
	ZeroMemory(u, nsize);
	ZeroMemory(v, nsize);

	for (size_t i = 0; i < nsize; i++)
	{
		u[i] = cornerPoints[i].xf;
		v[i] = cornerPoints[i].yf;
	}
	UndistortPoints(u, v, nsize, intrinsic_matrix, distortion_coeffs);//去畸变

	for (size_t i = 0; i < nsize; i++)
	{
		cornerPoints[i].xf = u[i];
		cornerPoints[i].yf = v[i];
	}

	double fx = CV_MAT_ELEM(*intrinsic_matrix, double, 0, 0);
	double fy = CV_MAT_ELEM(*intrinsic_matrix, double, 1, 1);
	double u0 = CV_MAT_ELEM(*intrinsic_matrix, double, 0, 2);
	double v0 = CV_MAT_ELEM(*intrinsic_matrix, double, 1, 2);

	int h, s, n;
	//double a, b, c, d, xc, yc, zc1, zc2, zc;
	double a, b, c, d, t, xc, yc, zc;
	for (size_t i = 0; i < nsize; i++)
	{
		h = cornerPoints[i].heng, s = cornerPoints[i].shu;
		n = 9 * h + s;

		//a = ((u[i] - u0) / fx) - (d1[n] / d3[n]);
		//b = x0[n] - ((d1[n] / d3[n])*z0[n]);
		//zc1 = b / a;
		//c = ((v[i] - v0) / fy) - (d2[n] / d3[n]);
		//d = y0[n] - ((d2[n] / d3[n])*z0[n]);
		//zc2 = d / c;
		//zc = (zc1 + zc2) / 2;
		//
		//xc = ((u[i] - u0) / fx)*zc;
		//yc = ((v[i] - v0) / fy)*zc;

		//a = ((u[i] - u0) / (v[i] - v0))*(fy / fx);
		//t = (a*y0[n] - x0[n]) / (d1[n] - (a*d2[n]));
		a = fx * x0[n] * (v[i] - v0);
		b = fx * d1[n] * (v[i] - v0);
		c = fy * y0[n] * (u[i] - u0);
		d = fy * d2[n] * (u[i] - u0);
		t = (a - c) / (d - b);

		xc = x0[n] + (d1[n] * t);
		yc = y0[n] + (d2[n] * t);
		zc = z0[n] + (d3[n] * t);

		cornerPoints[i].xw = xc;
		cornerPoints[i].yw = yc;
		cornerPoints[i].zw = zc;
	}

	delete[] u; delete[] v;

	return TRUE;
}