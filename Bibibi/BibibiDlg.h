// BibibiDlg.h: 头文件
//

#pragma once
#include "pch.h"
#include "framework.h"
#include "Bibibi.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "CaliProcess.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/SVD>

using namespace Eigen;     // 改成这样亦可 using Eigen::MatrixXd;
using namespace std;
using namespace cv;

#define CALI_PATH "Calibration"		//所有数据存储在工程目录的Calibration文件夹内
#define TEST_PATH "Test"

struct cornerPoint
{
public:
	double xf, yf, xw, yw, zw;
	int heng, shu;

	cornerPoint(double dx = 0.0, double dy = 0.0, double cx = 0.0, double cy = 0.0, double cz = 0.0, int h = 0, int s = 0)
		: xf(dx), yf(dy), xw(cx), yw(cy), zw(cz), heng(h), shu(s) {}
	cornerPoint(const cornerPoint& pts)
	{
		//u = pts.u; v = pts.v;
		//heng = pts.heng; shu = pts.shu;
		memcpy(this, &pts, sizeof(cornerPoint));
	}
	cornerPoint& operator = (const cornerPoint& pts)
	{
		if (this == &pts)
		{
			return *this;
		}
		memcpy(this, &pts, sizeof(cornerPoint));
		//u = pts.u; v = pts.v;
		//heng = pts.heng; shu = pts.shu;

		return *this;
	}
};

//typedef struct tag3POINT
//{
//	LONG  x;
//	LONG  y;
//	LONG  z;
//} POINT3, *PPOINT3, NEAR *NPPOINT3, FAR *LPPOINT3;

// CBibibiDlg 对话框
class CBibibiDlg : public CDialogEx
{
	// 构造
public:
	CBibibiDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BIBIBI_DIALOG };
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
	CString m_strSaveFileName;	//保存图像地址
	CString m_strCalibrationPath;//保存数据地址
	CString m_strTestPath;
	IplImage* Image2Use; // 释放

	void SetStatic(CString update);

	//相机标定
	//标定用参数
	int DEFAULT_IMAGE_WIDTH = 1280, DEFAULT_IMAGE_HEIGHT = 1024, NCLAIIMAGES = 6, NFEATUREPOINTS = 81;
	double intervalX = 12, intervalY = 12;
	CvMat* m_InstrictMat_Calcu;
	CvMat* m_DistCoeffMat_Calcu;
	CvMat* trans_vector_temp;
	CvMat* rotation_martix_temp;

	CaliProcess Calicam0; //单目相机
	bool Califlags; //是否做了相机标定标识符

	afx_msg void OnBnClickedButton1();
	bool Calicam(CaliProcess& Calicam0);//相机标定
	bool reprojectionErr(vector<C4DPointD> objPoints, CvMat* Rvec, CvMat* Tvec, CvMat* InstrictMat, CvMat* DistCoeffMat);

	//光平面标定
	int LASERFILE = 4;
	int hengNum = 9, shuNum = 9;//网格横竖条纹数量
	const int PtsNum = 81;//角点数量
	int m_graythred = 120;
	afx_msg void OnBnClickedButton2();
	//vector<Point3d> LineCenter;//光条中心坐标
	double Plane[4];		//光平面标定结果,ax+by+cz+d=0
	double m_dPlaneError;	//光平面标定误差
	Mat m_img0;		//暂存的彩色图片
	Mat m_grid;		//细化后的图像
	vector<vector<C4DPointD>> m_hengLSPoints;         //光平面标定单幅图的光条中心坐标
	vector<vector<vector<C4DPointD>>> m_hengCali;		//光平面标定4幅图的光条中心坐标
	vector<vector<C4DPointD>> m_shuLSPoints;         //光平面标定单幅图的光条中心坐标
	vector<vector<vector<C4DPointD>>> m_shuCali;		//光平面标定4幅图的光条中心坐标
	vector<cornerPoint> m_cornerPoint;		//单幅角点坐标
	vector<vector<cornerPoint>> m_allPoints; //四幅图所有角点坐

	//vector<Point3d> LineExtract(Mat img0);
	void cvThin(Mat& src, Mat& dst, int intera);
	void cvThin_h(Mat& src, Mat& dst);
	void cvThin_s(Mat& src, Mat& dst);
	void linerank(Mat heng_thin, Mat shu_thin);
	void SaveLineData(vector<vector<C4DPointD>> LSCenterPic, CString strPath);
	void SaveLineData2(vector<C4DPointD>& vecPts, CString strPath);
	void SaveLineData(vector<vector<cornerPoint>> LSCenterPic, CString strPath);
	void SaveLineData(vector<cornerPoint>& vecPts, CString strPath);
	bool UV2CamCrdn(vector<C4DPointD>& LineCenter, CvMat* intrinsic_matrix, CvMat* distortion_coeffs, CvMat* rotation_vectors, CvMat* translation_vectors, int nImgIndex);
	bool UV2CamCrdn2(vector<cornerPoint>& cornerPoints, CvMat* intrinsic_matrix, CvMat* distortion_coeffs, CvMat* rotation_vectors, CvMat* translation_vectors, int nImgIndex);
	bool UndistortPoints(double* px, double* py, int nCount, const CvMat* intrinsic_matrix, const CvMat* dist_coeffs);
	bool UV2Cam(const double u, const double v, const CvMat* intrinsic_matrix, const double plane[4], double& xc, double& yc, double& zc);
	bool UV2Cam(const double u, const double v, const double fx, const double fy, const double cx, const double cy, const double plane[4], double& xc, double& yc, double& zc);
	bool direction(vector<vector<cornerPoint>> allcornerPts, vector<cornerPoint> averagePts, vector<vector<double>>& Direct);
	bool FittingPlane(vector<C4DPointD>& LineCenter, double Plane[4], CString strPath);
	bool FittingPlane(double* xw, double* yw, double* zw, int nCount, double plane[4], CString strPath);
	bool cvFitPlane(vector<C4DPointD>& LineCenter, double* plane, CString strPath);
	bool Cam2WCrdn(double u, double v,
		const CvMat* rotation_matrix, const CvMat* translation_vector,/*const CvMat* rotation_matrix_01,const CvMat* translation_vector_01,*/
		double& xw, double& yw, double& zw);

	//三维检测
	afx_msg void OnBnClickedButton3();
	bool CornerDetect(Mat img);
	bool PointRank(double x, double y, int& heng, int& shu);
	bool UV2WCrdn(double u, double v,
		const CvMat* intrinsic_matrix, const double plane[4],
		const CvMat* rotation_matrix, const CvMat* translation_vector,/*const CvMat* rotation_matrix_01,const CvMat* translation_vector_01,*/
		double& xw, double& yw, double& zw);
	bool UV2CCrdn(vector<cornerPoint>& cornerPoints, double* x0, double* y0, double* z0, double* d1, double* d2, double* d3,
		const CvMat* intrinsic_matrix, CvMat* distortion_coeffs, const CvMat* rotation_matrix, const CvMat* translation_vector);
};
