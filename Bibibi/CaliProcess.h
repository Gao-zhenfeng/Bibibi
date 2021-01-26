#pragma once

#include<vector>
#include <opencv2/opencv.hpp>
#include "CvvImage.h"
using namespace std;
using namespace cv;

#include"TEllipse.h"
#include"TEllipseCollection.h"

#define PI 3.141592654				//圆周率π

#define CALIIMAGES 6 //标定所用图像数量
int AutoThres(/*void* img*/IplImage* frame);
struct C4DPointD
{
	double xf, yf, xw, yw, zw;
	C4DPointD(double xfv = 0.0, double yfv = 0.0, \
		double xwv = 0.0, double ywv = 0.0, double zwv = 0.0) : xf(xfv), yf(yfv), xw(xwv), yw(ywv), zw(zwv) {};

	C4DPointD(const C4DPointD& rhs)
	{
		memcpy(this, &rhs, sizeof(C4DPointD));
	}

	C4DPointD& C4DPointD::operator = (const C4DPointD& rhs)
	{
		if (this == &rhs)
			return *this;

		memcpy(this, &rhs, sizeof(C4DPointD));
		return *this;
	}
};

class CaliProcess
{
public:
	CaliProcess();
	~CaliProcess();

	IplImage* m_pTempImage;		//处理过程中使用的临时图像
	IplImage* m_pColorImage;	//显示处理结果的图像（包含光条中心、拐点等信息）
	vector<C4DPointD> m_vecFeaturePts;//相机标定过程中使用的特征点数据
	int* m_pEachPtsCount1;	//每幅图像特征点个数
	CvMat* rotation_vectors;			//旋转向量
	CvMat* translation_vectors;			//平移向量
	CvMat* intrinsic_matrix;	//相机内参矩阵
	CvMat* distortion_coeffs;	//相机畸变系数
	CvMat* object_points;     //M幅图像中k个点的物理坐标
	CvMat* image_points;    //M幅图像中k个点的像素坐标
	CvMat* point_counts; //每幅图像上点的个数
	CvMat* rotation_matrix_Calcu;
	CvMat* trans_vector_Calcu;

	//////标定相关函数////////

	int thresholdCaliProcess;

	//处理标定图像（非对称性圆靶标），pImg为输入的图像，ellpco为靶标上所有（椭）圆的信息，返回值为圆的数量
	//首先进行二值化，然后调用Find_counter寻找所有椭圆，得到椭圆圆心的图像坐标，然后调用Find_Topology
	//根据四个大圆及周围小圆的实际拓扑位置进行排序，建立坐标系，得到所有椭圆圆心的世界坐标。
	int CaliProcessNum(IplImage* pImg, TEllipseCollection& ellpco, double intervalX, double intervalY);

	//寻找图中所有轮廓，并将所有轮廓拟合成椭圆，返回椭圆数目。img1为灰度图，从img1中寻找轮廓，
	//img为彩图，将轮廓在img上画出来，并在轮廓中心（即圆心）画十字叉，显示后可查看圆轮廓提取是否正确
	//ellpco为提取结果，即椭圆信息。
	int Find_counter(IplImage* img, IplImage* img1, TEllipseCollection& ellpco);
	int Find_counter2(IplImage* img, IplImage* img1, TEllipseCollection& ellpco);

	//根据靶标的非对称性分析椭圆的拓扑结构建立坐标系，DX、DY为靶标上相邻两圆圆心X、Y方向的距离。
	void Find_Topology(TEllipseCollection& ellpco, double DX, double DY);

	//保存相机标定使用的特征点数据（所有圆心的图像坐标xf、yf，世界坐标xw、yw、zw），vecPts为数据，strPath为路径。
	void SaveCameraCaliPtsData(vector<C4DPointD>& vecPts, CString strPath);
	//读取相机标定使用的特征点数据，即读取SaveCameraCaliPtsData保存的数据，strFileName为路径。
	bool LoadFeaturePtsData(CString strFileName);

	bool ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount, CvMat* intrinsic_matrix, CvMat* distortion_coeffs, CvMat* rotation_vectors, CvMat* translation_vectors, CString strFilename, CvMat* object_points, CvMat* image_points, CvMat* point_counts);

	//张正友标定。vecPts为特征点数据，nImgWidth、nImgHeight为图像宽度和高度，nTargetCount为图像个数，取4即可，
	//pEachPtsCount为每幅图像特征点数目，接下来四个CvMat*类型形参为内参数矩阵，畸变系数，旋转矩阵和平移矩阵，
	//即标定结果，strFilename为标定结果保存路径。该函数主要是通过调用cvCalibrateCamera2函数实现。
	//bool ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount,
	//	CvMat* intrinsic_matrix, CvMat* distortion_coeffs,
	//	CvMat* rotation_vectors, CvMat* translation_vectors, CString strFilename);

	bool ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount,
		/*CvMat* intrinsic_matrix, CvMat* distortion_coeffs,
		CvMat* rotation_vectors, CvMat* translation_vectors,*/ CString strFilename, int nFileCountreal/*, *//*CvMat *object_points, CvMat* image_points, CvMat* point_counts*/);

	bool ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount,
		CvMat** intrinsic_matrix, CvMat** distortion_coeffs,
		CvMat** rotation_vectors, CvMat** translation_vectors, CString strFilename, int nFileCountreal/*, *//*CvMat *object_points, CvMat* image_points, CvMat* point_counts*/);
};

class OrderRuler
{
public:
	int nid;
	int ncount;
	double area;

public:
	OrderRuler() :nid(0), ncount(0), area(0.0) {};
	OrderRuler(const OrderRuler& theOrderRuler) { memcpy(this, &theOrderRuler, sizeof(OrderRuler)); }

	OrderRuler& operator=(const OrderRuler& theOrderRuler)
	{
		memcpy(this, &theOrderRuler, sizeof(OrderRuler));
		return *this;
	}

	bool operator<(const OrderRuler& v) const
	{
		if (area == v.area && ncount == v.ncount)
			return nid < v.nid;
		else if (area == v.area)
			return ncount < v.ncount;
		return area < v.area;
	}

	bool operator==(const OrderRuler& v) const
	{
		return ncount == v.ncount && area == v.area && nid == v.nid;
	}
};

struct CVect
{
	double x, y;

	CVect(double x1, double y1, double x2, double y2)
	{
		x = x2 - x1;
		y = y2 - y1;
	}

	CVect(TEllipse& p1, TEllipse& p2)
	{
		x = p2.m_dCenterpx - p1.m_dCenterpx;
		y = p2.m_dCenterpy - p1.m_dCenterpy;
	}

	friend double cosangle(const CVect& v1, const CVect& v2);
	friend double sinanglefab(const CVect& v1, const CVect& v2);
};

class DistCompare
{
public:
	int nid1, nid2;
	double dist;

public:
	DistCompare() :nid1(0), nid2(0), dist(0.0) {};
	DistCompare(const DistCompare& theDistCompare) { memcpy(this, &theDistCompare, sizeof(DistCompare)); }

	DistCompare& operator=(const DistCompare& theOrderRuler)
	{
		memcpy(this, &theOrderRuler, sizeof(DistCompare));
		return *this;
	}

	bool operator<(const DistCompare& v) const
	{
		return dist < v.dist;
	}

	bool operator>(const DistCompare& v) const
	{
		return dist > v.dist;
	}

	bool operator==(const DistCompare& v) const
	{
		return dist == v.dist;
	}

	void SetValue(TEllipse& p1, int n1, TEllipse& p2, int n2)
	{
		dist = (p1.m_dCenterpx - p2.m_dCenterpx) * (p1.m_dCenterpx - p2.m_dCenterpx) + (p1.m_dCenterpy - p2.m_dCenterpy) * (p1.m_dCenterpy - p2.m_dCenterpy);
		nid1 = n1; nid2 = n2;
	}

	void SetValue(double x, double y, int npos, TEllipse& p1, TEllipse& p2)
	{
		dist = (x - p1.m_dCenterpx) * (x - p1.m_dCenterpx) + (y - p1.m_dCenterpy) * (y - p1.m_dCenterpy) +
			(x - p2.m_dCenterpx) * (x - p2.m_dCenterpx) + (y - p2.m_dCenterpy) * (y - p2.m_dCenterpy);
		nid1 = npos;
	}

	void SetValue(TEllipse& p1, double x, double y, int npos)
	{
		dist = (x - p1.m_dCenterpx) * (x - p1.m_dCenterpx) + (y - p1.m_dCenterpy) * (y - p1.m_dCenterpy);
		nid1 = npos;
	}

	void SetValue(CVect& v1, int n1, CVect& v2, int n2)
	{
		dist = sinanglefab(v1, v2);
		nid1 = n1; nid2 = n2;
	}
};

class SmallRuler
{
public:
	int ncount;
	TEllipse* bc;

public:
	SmallRuler() :ncount(0), bc(NULL) {};
	SmallRuler(const SmallRuler& theOrderRuler) { memcpy(this, &theOrderRuler, sizeof(SmallRuler)); }

	SmallRuler& operator=(const SmallRuler& theOrderRuler)
	{
		memcpy(this, &theOrderRuler, sizeof(SmallRuler));
		return *this;
	}

	bool operator<(const SmallRuler& v) const
	{
		return ncount < v.ncount;
	}

	bool operator==(const OrderRuler& v) const
	{
		return ncount == v.ncount;
	}

	void SetValue(TEllipse* p1, int n1)
	{
		ncount = n1; bc = p1;
	}
};
