//#include "stdafx.h"
#include "CaliProcess.h"

//nFileCount为一次读入几个标定图像，默认为5个
CaliProcess::CaliProcess()
{
	//分配内存，用于保存标定结果

	intrinsic_matrix = cvCreateMat(3, 3, CV_64FC1);
	distortion_coeffs = cvCreateMat(1, 4, CV_64FC1);

	cvSetZero(intrinsic_matrix);
	cvSetZero(distortion_coeffs);
	m_pTempImage = NULL;
	m_pColorImage = NULL;
	//分配内存，用于保存标定结果 目前先这么写，应该把它写进类里。

	rotation_vectors = cvCreateMat(CALIIMAGES, 3, CV_64FC1);
	translation_vectors = cvCreateMat(CALIIMAGES, 3, CV_64FC1);
	cvSetZero(rotation_vectors);
	cvSetZero(translation_vectors);
	thresholdCaliProcess = 80;
}

CaliProcess::~CaliProcess()
{
	/*	cvReleaseMat(&m_ObjectPoints);
		cvReleaseMat(&m_ImagePoints);
		cvReleaseMat(&m_NPoints);*/

	if (m_pColorImage != nullptr)
	{
		cvReleaseImage(&m_pColorImage);
	}
	m_pColorImage = nullptr;
	if (m_pTempImage != nullptr)
	{
		cvReleaseImage(&m_pTempImage);
	}
	m_pTempImage = nullptr;
	cvReleaseMat(&intrinsic_matrix);
	cvReleaseMat(&distortion_coeffs);
	cvReleaseMat(&rotation_vectors);
	cvReleaseMat(&translation_vectors);
}

int CaliProcess::CaliProcessNum(IplImage* pImg, TEllipseCollection& ellpco, double intervalX, double intervalY)//R:传入的是一副原始的图像，和一个TEllipseCollection类对象
{
	if (pImg == NULL)
	{
		return FALSE;
	}
	/*int threshold = AutoThres(pImg);*/
	int threshold = thresholdCaliProcess;

	if (m_pTempImage != NULL)
	{
		cvReleaseImage(&m_pTempImage);
	}
	m_pTempImage = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 1);
	cvThreshold(pImg, m_pTempImage, 40, 255, CV_THRESH_BINARY);			//可改成自适应二值化
	cvSaveImage("threshold_15.bmp", m_pTempImage);//R:存储二值化后的图像
	if (m_pColorImage != NULL)
	{
		cvReleaseImage(&m_pColorImage);
	}
	m_pColorImage = cvCreateImage(cvGetSize(pImg), IPL_DEPTH_8U, 3);
	cvCvtColor(pImg, m_pColorImage, CV_GRAY2RGB);
	int num = Find_counter2(m_pColorImage, m_pTempImage, ellpco);//R:在原图中寻找轮廓，返回的是轮廓的数量，ellpco中是所有椭圆的信息
	if (num <= 0)
	{
		AfxMessageBox("未找到圆轮廓");
		return 0;
	}

	Find_Topology(ellpco, intervalX, intervalY);
	return num;
}

int CaliProcess::Find_counter(IplImage* img, IplImage* img1, TEllipseCollection& ellpco)//R:参数是原图和经过二值化后的图像，，
{
	TEllipse myellipse;
	CvMemStorage* stor;
	CvSeq* contour;
	stor = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), stor);//创建一个点的序列
	int num = cvFindContours(img1, stor, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//获得椭圆轮廓数量 需要使用二值图像
	if (num == 0)
	{
		AfxMessageBox("未获得椭圆轮廓");
		return -1;
	}
	CvSeq* contourDst = contour;
	CvBox2D32f* box = new CvBox2D32f; //R：分配内存给椭圆数据

	CPoint pointTemp;
	vector<CPoint> pointVector;
	double area;
	float* radius = new float[num];
	CvPoint2D32f* ptCenter = new CvPoint2D32f[num];
	//int maxcount=0;
	//double maxarea=0;
	for (int j = 0; contourDst; contourDst = contourDst->h_next, j++)
	{
		int count = contourDst->total; // number of points
		pointVector.clear();
		area = fabs(cvContourArea(contourDst));//获得椭圆的面积
		cvMinEnclosingCircle(contourDst, &ptCenter[j], &radius[j]);//计算轮廓的最小外接圆
		float cd = area / (PI * radius[j] * radius[j]);

		if (count > 50 && (count < 1000) && area < 100000 && area>500 && radius[j] > 1 && radius[j] < 1000 && cd>0.5)//判断标准根据图像大小做调整
		{
			cvDrawContours(img, contourDst, CV_RGB(255, 255, 255), CV_RGB(0, 0, 0), -1, CV_FILLED, 8);//R:绘制椭圆的轮廓
																									  //cvSaveImage("2.bmp",img);
			CvPoint* PointArray = new CvPoint[count];
			CvPoint2D32f* PointArray2D32f = new CvPoint2D32f[count];
			cvCvtSeqToArray(contourDst, PointArray, CV_WHOLE_SEQ);
			//R:将cvPoint点集转化为椭圆点集
			for (int i = 0; i < count; i++)
			{
				PointArray2D32f[i].x = float(PointArray[i].x);
				PointArray2D32f[i].y = float(PointArray[i].y);
			}
			cvFitEllipse(PointArray2D32f, count, box);//R:拟合当前轮廓
			CvPoint center;
			CvSize size;
			center.x = int(box->center.x + 0.5);
			center.y = int(box->center.y + 0.5);
			size.width = int(box->size.width / 2 + 0.5);
			size.height = int(box->size.height / 2 + 0.5);
			myellipse.m_dCenterpx = box->center.x;//像面坐标
			myellipse.m_dCenterpy = box->center.y;//像面坐标
			myellipse.m_dArea = area;
			myellipse.nid = j;//第j个
			ellpco.ellipses.push_back(myellipse);//R:存储了椭圆中心的坐标，长轴，短轴（或者认为是宽和高的一半），面积
			int x = (int)myellipse.m_dCenterpx;
			int y = (int)myellipse.m_dCenterpy;
		}
	}
	num = ellpco.Size();
	cvSaveImage("3.bmp", img);
	cvReleaseMemStorage(&stor);
	return num;//R:返回值是符合要求的椭圆的数量
}

int CaliProcess::Find_counter2(IplImage* img, IplImage* img1, TEllipseCollection& ellpco)//R:参数是原图和经过二值化后的图像，，
{
	TEllipse myellipse;
	CvMemStorage* stor;
	CvSeq* contour;
	stor = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), stor);//创建一个点的序列
	int num = cvFindContours(img1, stor, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//获得椭圆轮廓数量 需要使用二值图像
	if (num == 0)
	{
		AfxMessageBox("未获得椭圆轮廓");
		return -1;
	}
	CvSeq* contourDst = contour;
	CvBox2D32f* box = new CvBox2D32f; //R：分配内存给椭圆数据

	CPoint pointTemp;
	vector<CPoint> pointVector;
	double area;
	float* radius = new float[num];
	CvPoint2D32f* ptCenter = new CvPoint2D32f[num];

	for (int j = 0; contourDst; contourDst = contourDst->h_next, j++)
	{
		int count = contourDst->total; // number of points
		pointVector.clear();
		area = fabs(cvContourArea(contourDst));//获得椭圆的面积
		cvMinEnclosingCircle(contourDst, &ptCenter[j], &radius[j]);//计算轮廓的最小外接圆
		float cd = area / (PI * radius[j] * radius[j]);

		if (count > 50 && (count < 2000) && area < 200000 && area>500 && radius[j] > 1 && radius[j] < 2000 && cd>0.5)//判断标准根据图像大小做调整
		{
			cvDrawContours(img, contourDst, CV_RGB(255, 255, 255), CV_RGB(0, 0, 0), -1, CV_FILLED, 8);//R:绘制椭圆的轮廓
			cvSaveImage("2.bmp", img);
			CvPoint* PointArray = new CvPoint[count];
			CvPoint2D32f* PointArray2D32f = new CvPoint2D32f[count];
			cvCvtSeqToArray(contourDst, PointArray, CV_WHOLE_SEQ);
			//R:将cvPoint点集转化为椭圆点集
			for (int i = 0; i < count; i++)
			{
				PointArray2D32f[i].x = float(PointArray[i].x);
				PointArray2D32f[i].y = float(PointArray[i].y);
			}
			cvFitEllipse(PointArray2D32f, count, box);//R:拟合当前轮廓
			CvPoint center;
			CvSize size;
			center.x = int(box->center.x + 0.5);
			center.y = int(box->center.y + 0.5);
			size.width = int(box->size.width / 2 + 0.5);
			size.height = int(box->size.height / 2 + 0.5);
			myellipse.m_dCenterpx = box->center.x;//像面坐标
			myellipse.m_dCenterpy = box->center.y;//像面坐标
			myellipse.m_dArea = area;
			myellipse.nid = j;//第j个
			ellpco.ellipses.push_back(myellipse);//R:存储了椭圆中心的坐标，长轴，短轴（或者认为是宽和高的一半），面积
			int x = (int)myellipse.m_dCenterpx;
			int y = (int)myellipse.m_dCenterpy;
		}
	}
	num = ellpco.Size();
	cvSaveImage("3.bmp", img);
	cvReleaseMemStorage(&stor);
	return num;//R:返回值是符合要求的椭圆的数量
}

void CaliProcess::Find_Topology(TEllipseCollection& ellpco, double DX, double DY)
{
	vector<TEllipse> FourBigCircle;
	vector<TEllipse> SmallCircle;
	vector<TEllipse> ellipses(ellpco.ellipses);//括号前的是括号里的一个副本
	ellpco.Clear();

	// 2. 对图像边长和面积进行排序，找到调变点，查找调变点的信息 进行噪声剔出
	{
		//////////////////////////////////////////////////////////////////////////
		set<OrderRuler> CircleLengthOrder;
		OrderRuler temp;
		std::vector<TEllipse>::iterator iter = ellipses.begin();
		double area;
		int nsize = ellipses.size();

		while (iter != ellipses.end())
		{
			temp.area = iter->m_dArea;
			temp.nid = iter->nid;
			CircleLengthOrder.insert(temp);
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////

		std::set<OrderRuler>::iterator iterj = CircleLengthOrder.begin();
		for (int i = 0; i < nsize - 4; i++)
		{
			iter = ellipses.begin();
			while (iter != ellipses.end())
			{
				if (iter->nid == iterj->nid)
				{
					SmallCircle.push_back(*iter);
					break;
				}
				iter++;
			}
			iterj++;
		}

		for (int i = 0; i < 4; i++)
		{
			iter = ellipses.begin();
			while (iter != ellipses.end())
			{
				if (iter->nid == iterj->nid)
				{
					FourBigCircle.push_back(*iter);
					break;
				}
				iter++;
			}
			iterj++;
		}
	}

	if (FourBigCircle.size() != 4)
	{
		AfxMessageBox("大圆标记没有找到，请仔细检查图像");
	}

	// 3. 为四个大圆排序，编号
	{
		int norigin = 0, npid1 = 1, npid2 = 2, npid3 = 3;

		std::vector<TEllipse> FourBigCircleTemp;
		FourBigCircleTemp.reserve(100);

		std::vector<TEllipse>::iterator iter = FourBigCircle.begin();
		while (iter != FourBigCircle.end())
		{
			FourBigCircleTemp.push_back(*iter);
			iter++;
		}

		CVect v1(FourBigCircleTemp[0], FourBigCircleTemp[1]);
		CVect v2(FourBigCircleTemp[0], FourBigCircleTemp[2]);
		CVect v3(FourBigCircleTemp[0], FourBigCircleTemp[3]);

		// 大圆0是斜线以外的点
		if (sinanglefab(v1, v2) > 0.052 && sinanglefab(v1, v3) > 0.052 && sinanglefab(v2, v3) > 0.052)		// 3度
		{
			npid1 = 0;
			set<DistCompare> distvector;
			DistCompare disttemp;
			disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
				CVect(FourBigCircleTemp[0], FourBigCircleTemp[1]), 1);
			distvector.insert(disttemp);
			disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
				CVect(FourBigCircleTemp[0], FourBigCircleTemp[2]), 2);
			distvector.insert(disttemp);
			disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
				CVect(FourBigCircleTemp[0], FourBigCircleTemp[3]), 3);
			distvector.insert(disttemp);
			std::set<DistCompare>::iterator iterd1 = distvector.begin();
			npid3 = iterd1->nid2;
			iterd1++;
			norigin = iterd1->nid2;
			iterd1++;
			npid2 = iterd1->nid2;
		}
		// 大圆0是斜线上的点
		else
		{
			if (sinanglefab(v1, v2) < 0.052)
			{
				npid1 = 3;
				set<DistCompare> distvector;
				DistCompare disttemp;
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
					CVect(FourBigCircleTemp[3], FourBigCircleTemp[1]), 1);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
					CVect(FourBigCircleTemp[3], FourBigCircleTemp[2]), 2);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), -1,
					CVect(FourBigCircleTemp[3], FourBigCircleTemp[0]), 0);
				distvector.insert(disttemp);
				std::set<DistCompare>::iterator iterd1 = distvector.begin();
				npid3 = iterd1->nid2;
				iterd1++;
				norigin = iterd1->nid2;
				iterd1++;
				npid2 = iterd1->nid2;
			}
			else if (sinanglefab(v1, v3) < 0.052)
			{
				npid1 = 2;
				set<DistCompare> distvector;
				DistCompare disttemp;
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[2], FourBigCircleTemp[0]), 0);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[2], FourBigCircleTemp[1]), 1);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[1], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[2], FourBigCircleTemp[3]), 3);
				distvector.insert(disttemp);
				std::set<DistCompare>::iterator iterd1 = distvector.begin();
				npid3 = iterd1->nid2;
				iterd1++;
				norigin = iterd1->nid2;
				iterd1++;
				npid2 = iterd1->nid2;
			}

			else if (sinanglefab(v2, v3) < 0.052)
			{
				npid1 = 1;
				set<DistCompare> distvector;
				DistCompare disttemp;
				disttemp.SetValue(CVect(FourBigCircleTemp[2], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[1], FourBigCircleTemp[0]), 0);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[2], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[1], FourBigCircleTemp[2]), 2);
				distvector.insert(disttemp);
				disttemp.SetValue(CVect(FourBigCircleTemp[2], FourBigCircleTemp[3]), -1,
					CVect(FourBigCircleTemp[1], FourBigCircleTemp[3]), 3);
				distvector.insert(disttemp);
				std::set<DistCompare>::iterator iterd1 = distvector.begin();
				npid3 = iterd1->nid2;
				iterd1++;
				norigin = iterd1->nid2;
				iterd1++;
				npid2 = iterd1->nid2;
			}
		}
		// 四个点按照原点，最远点，两近点（离远点最近在前）的顺序排列好
		FourBigCircleTemp[norigin].nxid = 0;
		FourBigCircleTemp[norigin].nyid = 0;
		FourBigCircleTemp[npid1].nxid = 2;
		FourBigCircleTemp[npid1].nyid = 0;
		FourBigCircleTemp[npid2].nxid = 1;
		FourBigCircleTemp[npid2].nyid = 1;
		FourBigCircleTemp[npid3].nxid = -1;
		FourBigCircleTemp[npid3].nyid = -1;

		FourBigCircle.clear();

		FourBigCircle.push_back(FourBigCircleTemp[norigin]);
		FourBigCircle.push_back(FourBigCircleTemp[npid1]);
		FourBigCircle.push_back(FourBigCircleTemp[npid2]);
		FourBigCircle.push_back(FourBigCircleTemp[npid3]);
	}
	// 4. 查找双轴
	CVect vxy45(FourBigCircle[0], FourBigCircle[2]);
	TEllipse xAxisHelpPoint, yAxisHelpPoint, yAxisPoint;
	vector<TEllipse> xAxisPointVect;
	vector<TEllipse> yAxisHelpPointVect;
	vector<TEllipse> xAxisHelpPointVect;
	vector<TEllipse> yAxisPointVect;

	{
		//////////////////////////////////////////////////////////////////////////
		{
			CVect vx(FourBigCircle[0], FourBigCircle[1]);
			vector<TEllipse> nxAxisPointVect;
			double sinangleh = 0.08;		// 3度
			std::vector<TEllipse>::iterator iter = SmallCircle.begin();

			while (iter != SmallCircle.end())
			{
				if (sinanglefab(vx, CVect(FourBigCircle[0], *iter)) < sinangleh)
				{
					xAxisPointVect.push_back(*iter);
					iter = SmallCircle.erase(iter);
					continue;
				}
				iter++;
			}

			iter = xAxisPointVect.begin();
			while (iter != xAxisPointVect.end())
			{
				if (acos(cosangle(vxy45, CVect(FourBigCircle[0], *iter))) > 1.57)
				{
					nxAxisPointVect.push_back(*iter);
					iter = xAxisPointVect.erase(iter);
					continue;
				}
				iter++;
			}
			//
			xAxisPointVect.push_back(FourBigCircle[1]);
			// order the axis point
			set<DistCompare> distset;
			std::set<DistCompare>::iterator iterd;
			iter = xAxisPointVect.begin();
			int ncount = 0;
			DistCompare disttemp;
			while (iter != xAxisPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[0], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			yAxisHelpPoint = (xAxisPointVect[iterd->nid2]);//(1，0)
			while (iterd != distset.end())
			{
				xAxisPointVect[iterd->nid2].nxid = ncount;
				xAxisPointVect[iterd->nid2].nyid = 0;
				iterd++; ncount++;
			}

			iter = nxAxisPointVect.begin();
			distset.clear();
			ncount = 0;
			while (iter != nxAxisPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[0], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			while (iterd != distset.end())
			{
				nxAxisPointVect[iterd->nid2].nxid = -ncount;
				nxAxisPointVect[iterd->nid2].nyid = 0;
				iterd++; ncount++;
			}
			iter = nxAxisPointVect.begin();
			while (iter != nxAxisPointVect.end())
			{
				xAxisPointVect.push_back(*iter);
				iter++;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		{
			vector<TEllipse> nyAxisHelpPointVect;

			CVect vy(yAxisHelpPoint, FourBigCircle[2]);
			double sinangleh = 0.08;		// 3度
			std::vector<TEllipse>::iterator iter = SmallCircle.begin();
			while (iter != SmallCircle.end())
			{
				if (sinanglefab(vy, CVect(FourBigCircle[2], *iter)) < sinangleh)
				{
					yAxisHelpPointVect.push_back(*iter);
					iter = SmallCircle.erase(iter);
					continue;
				}
				iter++;
			}
			iter = yAxisHelpPointVect.begin();
			while (iter != yAxisHelpPointVect.end())
			{
				if (acos(cosangle(vxy45, CVect(FourBigCircle[2], *iter))) > 1.57)
				{
					nyAxisHelpPointVect.push_back(*iter);
					iter = yAxisHelpPointVect.erase(iter);
					continue;
				}
				iter++;
			}
			yAxisHelpPointVect.push_back(FourBigCircle[2]);
			set<DistCompare> distset;
			std::set<DistCompare>::iterator iterd;
			iter = yAxisHelpPointVect.begin();
			int ncount = 0;
			DistCompare disttemp;
			while (iter != yAxisHelpPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[1], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			while (iterd != distset.end())
			{
				yAxisHelpPointVect[iterd->nid2].nxid = 1;
				yAxisHelpPointVect[iterd->nid2].nyid = ncount;
				iterd++; ncount++;
			}
			iter = nyAxisHelpPointVect.begin();
			distset.clear();
			ncount = 0;
			while (iter != nyAxisHelpPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[2], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			xAxisHelpPoint = (nyAxisHelpPointVect[iterd->nid2]);
			while (iterd != distset.end())
			{
				nyAxisHelpPointVect[iterd->nid2].nxid = 1;
				nyAxisHelpPointVect[iterd->nid2].nyid = -ncount;
				iterd++; ncount++;
			}
			iter = nyAxisHelpPointVect.begin();
			while (iter != nyAxisHelpPointVect.end())
			{
				yAxisHelpPointVect.push_back(*iter);
				iter++;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		{
			vector<TEllipse> nxAxisHelpPointVect;
			CVect vx(FourBigCircle[3], xAxisHelpPoint);
			double sinangleh = 0.08;		// 3度
			std::vector<TEllipse>::iterator iter = SmallCircle.begin();
			while (iter != SmallCircle.end())
			{
				if (sinanglefab(vx, CVect(FourBigCircle[3], *iter)) < sinangleh)
				{
					xAxisHelpPointVect.push_back(*iter);
					iter = SmallCircle.erase(iter);
					continue;
				}
				iter++;
			}
			iter = xAxisHelpPointVect.begin();
			while (iter != xAxisHelpPointVect.end())
			{
				if (acos(cosangle(vxy45, CVect(FourBigCircle[3], *iter))) > 1.57)
				{
					nxAxisHelpPointVect.push_back(*iter);
					iter = xAxisHelpPointVect.erase(iter);
					continue;
				}
				iter++;
			}
			xAxisHelpPointVect.push_back(xAxisHelpPoint);
			// order the axis point
			set<DistCompare> distset;
			std::set<DistCompare>::iterator iterd;
			iter = xAxisHelpPointVect.begin();
			int ncount = 0;
			DistCompare disttemp;
			while (iter != xAxisHelpPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[3], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 0;
			yAxisPoint = (xAxisHelpPointVect[iterd->nid2]);//(0,-1)
			while (iterd != distset.end())
			{
				xAxisHelpPointVect[iterd->nid2].nxid = ncount;
				xAxisHelpPointVect[iterd->nid2].nyid = -1;
				iterd++; ncount++;
			}
			//
			nxAxisHelpPointVect.push_back(FourBigCircle[3]);
			iter = nxAxisHelpPointVect.begin();
			distset.clear();
			ncount = 0;
			while (iter != nxAxisHelpPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[3], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			while (iterd != distset.end())
			{
				nxAxisHelpPointVect[iterd->nid2].nxid = -ncount;
				nxAxisHelpPointVect[iterd->nid2].nyid = -1;
				iterd++; ncount++;
			}
			iter = nxAxisHelpPointVect.begin();
			while (iter != nxAxisHelpPointVect.end())
			{
				xAxisHelpPointVect.push_back(*iter);
				iter++;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		{
			vector<TEllipse> nyAxisPointVect;

			CVect vy(yAxisPoint, FourBigCircle[0]);
			double sinangleh = 0.08;		// 3度
			std::vector<TEllipse>::iterator iter = SmallCircle.begin();
			while (iter != SmallCircle.end())
			{
				if (sinanglefab(vy, CVect(FourBigCircle[0], *iter)) < sinangleh)
				{
					yAxisPointVect.push_back(*iter);
					iter = SmallCircle.erase(iter);
					continue;
				}
				iter++;
			}
			iter = yAxisPointVect.begin();
			while (iter != yAxisPointVect.end())
			{
				if (acos(cosangle(vxy45, CVect(FourBigCircle[0], *iter))) > 1.57)
				{
					nyAxisPointVect.push_back(*iter);
					iter = yAxisPointVect.erase(iter);
					continue;
				}
				iter++;
			}
			set<DistCompare> distset;
			std::set<DistCompare>::iterator iterd;
			iter = yAxisPointVect.begin();
			int ncount = 0;
			DistCompare disttemp;
			while (iter != yAxisPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[0], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			while (iterd != distset.end())
			{
				yAxisPointVect[iterd->nid2].nxid = 0;
				yAxisPointVect[iterd->nid2].nyid = ncount;
				iterd++; ncount++;
			}
			nyAxisPointVect.push_back(yAxisPoint);
			iter = nyAxisPointVect.begin();
			distset.clear();
			ncount = 0;
			while (iter != nyAxisPointVect.end())
			{
				disttemp.SetValue(FourBigCircle[0], -1, *iter, ncount);
				distset.insert(disttemp);
				iter++; ncount++;
			}
			iterd = distset.begin();
			ncount = 1;
			xAxisHelpPoint = (nyAxisPointVect[iterd->nid2]);
			while (iterd != distset.end())
			{
				nyAxisPointVect[iterd->nid2].nxid = 0;
				nyAxisPointVect[iterd->nid2].nyid = -ncount;
				iterd++; ncount++;
			}
			iter = nyAxisPointVect.begin();
			while (iter != nyAxisPointVect.end())
			{
				yAxisPointVect.push_back(*iter);
				iter++;
			}
		}
	}

	// 5.确定双轴的顺序
	vector<TEllipse> FinalCircle;
	{
		//////////////////////////////////////////////////////////////////////////
		set<SmallRuler> orderruler;
		SmallRuler temporder;
		std::vector<TEllipse>::iterator iter = xAxisPointVect.begin();
		while (iter != xAxisPointVect.end())
		{
			temporder.SetValue(&(*iter), iter->nxid);
			orderruler.insert(temporder);
			iter++;
		}
		vector<TEllipse> tempbc;
		std::set<SmallRuler>::iterator iterruler = orderruler.begin();
		while (iterruler != orderruler.end())
		{
			tempbc.push_back(*(iterruler->bc));
			iterruler++;
		}
		xAxisPointVect.clear();
		iter = tempbc.begin();
		while (iter != tempbc.end())
		{
			if (iter->nxid == 0 || iter->nxid == 1)
				FinalCircle.push_back(*iter);
			else
				xAxisPointVect.push_back(*iter);
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////
		tempbc.clear();
		orderruler.clear();
		iter = xAxisHelpPointVect.begin();
		while (iter != xAxisHelpPointVect.end())
		{
			temporder.SetValue(&(*iter), iter->nxid);
			orderruler.insert(temporder);
			iter++;
		}
		iterruler = orderruler.begin();
		while (iterruler != orderruler.end())
		{
			tempbc.push_back(*(iterruler->bc));
			iterruler++;
		}
		xAxisHelpPointVect.clear();
		iter = tempbc.begin();
		while (iter != tempbc.end())
		{
			if (iter->nxid == 0 || iter->nxid == 1)
				FinalCircle.push_back(*iter);
			else
				xAxisHelpPointVect.push_back(*iter);
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////
		tempbc.clear();
		orderruler.clear();
		iter = yAxisPointVect.begin();
		while (iter != yAxisPointVect.end())
		{
			temporder.SetValue(&(*iter), iter->nyid);
			orderruler.insert(temporder);
			iter++;
		}
		iterruler = orderruler.begin();
		while (iterruler != orderruler.end())
		{
			tempbc.push_back(*(iterruler->bc));
			iterruler++;
		}
		yAxisPointVect.clear();
		iter = tempbc.begin();
		while (iter != tempbc.end())
		{
			if (iter->nyid != 0 && iter->nyid != -1)
				yAxisPointVect.push_back(*iter);
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////
		tempbc.clear();
		orderruler.clear();
		iter = yAxisHelpPointVect.begin();
		while (iter != yAxisHelpPointVect.end())
		{
			temporder.SetValue(&(*iter), iter->nyid);
			orderruler.insert(temporder);
			iter++;
		}
		iterruler = orderruler.begin();
		while (iterruler != orderruler.end())
		{
			tempbc.push_back(*(iterruler->bc));
			iterruler++;
		}
		yAxisHelpPointVect.clear();
		iter = tempbc.begin();
		while (iter != tempbc.end())
		{
			if (iter->nyid != 0 && iter->nyid != -1)
				yAxisHelpPointVect.push_back(*iter);
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////
	}

	// 5.确定其他小圆的坐标
	{
		std::vector<TEllipse>::iterator iter = xAxisPointVect.begin();
		std::vector<TEllipse>::iterator iter2;
		std::vector<TEllipse>::iterator iters = SmallCircle.begin();
		double sinangleh = 0.05;		// 3度
										//////////////////////////////////////////////////////////////////////////
		while (iter != xAxisPointVect.end())
		{
			iter2 = xAxisHelpPointVect.begin();
			BOOL bFind = FALSE;
			while (iter2 != xAxisHelpPointVect.end())
			{
				if (iter->nxid == iter2->nxid)
				{
					bFind = TRUE;
					break;
				}
				iter2++;
			}
			if (bFind)
			{
				iters = SmallCircle.begin();
				while (iters != SmallCircle.end())
				{
					if (sinanglefab(CVect(*iter, *iter2), CVect(*iter, *iters)) < sinangleh)
						iters->nxid = iter->nxid;
					iters++;
				}
			}
			iter++;
		}
		//////////////////////////////////////////////////////////////////////////
		iter = yAxisPointVect.begin();
		while (iter != yAxisPointVect.end())
		{
			iter2 = yAxisHelpPointVect.begin();
			BOOL bFind = FALSE;
			while (iter2 != yAxisHelpPointVect.end())
			{
				if (iter->nyid == iter2->nyid)
				{
					bFind = TRUE;
					break;
				}
				iter2++;
			}
			if (bFind)
			{
				iters = SmallCircle.begin();
				while (iters != SmallCircle.end())
				{
					if (sinanglefab(CVect(*iter, *iter2), CVect(*iter, *iters)) < sinangleh)
						iters->nyid = iter->nyid;
					iters++;
				}
			}
			iter++;
		}
	}

	// 6.总结最后圆
	{
		std::vector<TEllipse>::iterator iter = FourBigCircle.begin();
		FinalCircle.push_back(FourBigCircle[0]);
		iter = xAxisPointVect.begin();
		while (iter != xAxisPointVect.end())
		{
			FinalCircle.push_back(*iter);
			iter++;
		}
		iter = xAxisHelpPointVect.begin();
		while (iter != xAxisHelpPointVect.end())
		{
			FinalCircle.push_back(*iter);
			iter++;
		}
		iter = yAxisPointVect.begin();
		while (iter != yAxisPointVect.end())
		{
			FinalCircle.push_back(*iter);
			iter++;
		}
		iter = yAxisHelpPointVect.begin();
		while (iter != yAxisHelpPointVect.end())
		{
			FinalCircle.push_back(*iter);
			iter++;
		}
		iter = SmallCircle.begin();
		while (iter != SmallCircle.end())
		{
			FinalCircle.push_back(*iter);
			iter++;
		}
	}

	// 7. 修正多余的圆
	{
		CVect vx(FourBigCircle[0], FourBigCircle[1]);
		CVect vy(FourBigCircle[0], yAxisPoint);
		double sinangleh = 0.05;		// 3度
		std::vector<TEllipse>::iterator iter = FinalCircle.begin();
		while (iter != FinalCircle.end())
		{
			if (iter->nxid == 0)
			{
				if (sinanglefab(vy, CVect(FourBigCircle[0], *iter)) > sinangleh)
				{
					iter = FinalCircle.erase(iter);
					continue;
				}
			}
			if (iter->nyid == 0)
			{
				if (sinanglefab(vx, CVect(FourBigCircle[0], *iter)) > sinangleh)
				{
					iter = FinalCircle.erase(iter);
					continue;
				}
			}
			iter++;
		}
	}

	// out put the data
	std::vector<TEllipse>::iterator iter = ellipses.begin();
	iter = FinalCircle.begin();
	while (iter != FinalCircle.end())
	{
		iter->m_dXw = (iter->nxid) * DX;
		iter->m_dYw = (iter->nyid) * DY;
		ellpco.Push(*iter);
		iter++;
	}
	iter = FinalCircle.begin();
	if (!m_vecFeaturePts.empty())
	{
		m_vecFeaturePts.clear();
		vector<C4DPointD>().swap(m_vecFeaturePts);
	}
	int circle_num = FinalCircle.size();
	C4DPointD tempPts;
	tempPts.xf = 0;
	tempPts.yf = 0;
	tempPts.xw = 0;
	tempPts.yw = 0;
	tempPts.zw = 0;
	for (int i = 0; i < circle_num; i++)
	{
		m_vecFeaturePts.push_back(tempPts);
	}
	double xf, yf, xw, yw, zw;
	vector<C4DPointD>::iterator iter1 = m_vecFeaturePts.begin();
	while (iter != FinalCircle.end())
	{
		iter1->xf = iter->m_dCenterpx;
		iter1->yf = iter->m_dCenterpy;
		iter1->xw = iter->nxid * DX;
		iter1->yw = iter->nyid * DY;
		iter1->zw = 0;
		iter++;
		iter1++;
	}

	// draw the element
	iter1 = m_vecFeaturePts.begin();
	CString str;
	CString str2;
	CvFont fontMX = cvFont(2.5, 1.5);
	cvInitFont(&fontMX, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5, 0, 1, 8);
	while (iter1 != m_vecFeaturePts.end())
	{
		xf = (int)iter1->xf;
		yf = (int)iter1->yf;
		xw = iter1->xw;
		yw = iter1->yw;
		/*cvLine(m_pColorImage, cvPoint(xf - 5, yf), cvPoint(xf + 5, yf), CV_RGB(255, 0, 255), 0.5);
		cvLine(m_pColorImage, cvPoint(xf, yf - 5), cvPoint(xf, yf + 5), CV_RGB(255, 0, 255), 0.5);*/
		cvLine(m_pColorImage, cvPoint(xf - 25, yf), cvPoint(xf + 25, yf), CV_RGB(255, 0, 255), 0.5);
		cvLine(m_pColorImage, cvPoint(xf, yf - 25), cvPoint(xf, yf + 25), CV_RGB(255, 0, 255), 0.5);

		str.Format("%.0f", xw);
		cvPutText(m_pColorImage, str, cvPoint(xf + 15, yf + 25), &fontMX, CV_RGB(255, 0, 0));
		str2.Format("%.0f", yw);
		cvPutText(m_pColorImage, str2, cvPoint(xf + 15, yf + 45), &fontMX, CV_RGB(255, 0, 0));
		iter1++;
	}
	//cvSaveImage("4.bmp",m_pColorImage);
}

void CaliProcess::SaveCameraCaliPtsData(vector<C4DPointD>& vecPts, CString strPath)
{
	int nSize = vecPts.size();
	if (nSize == 0)
	{
		//	MessageBox("标定保存特征点数据时,特征点容器为空，请检查！");
		AfxMessageBox(_T("标定保存特征点数据时,特征点容器为空，请检查！"));
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

bool CaliProcess::LoadFeaturePtsData(CString strFileName)
{
	FILE* fp = fopen(strFileName, "r");
	if (fp)
	{
		double xw, yw, zw, xf, yf;
		while (!feof(fp))
		{
			fscanf(fp, "%lf\t%lf\t%lf\t%lf\t%lf\n", &xf, &yf, &xw, &yw, &zw);
			C4DPointD temp(xf, yf, xw, yw, zw);  //此处初始化的时候，要和结构体中变量定义的顺序一致
			m_vecFeaturePts.push_back(temp);
		}
		fclose(fp);
		return TRUE;
	}
	else
		return FALSE;
}

vector<cv::Point3f> Generate3DPoints()
{
	vector<cv::Point3f> points;

	float x, y, z;

	x = .5; y = .5; z = -.5;
	points.push_back(cv::Point3f(x, y, z));

	x = .5; y = .5; z = .5;
	points.push_back(cv::Point3f(x, y, z));

	x = -.5; y = .5; z = .5;
	points.push_back(cv::Point3f(x, y, z));

	x = -.5; y = .5; z = -.5;
	points.push_back(cv::Point3f(x, y, z));

	x = .5; y = -.5; z = -.5;
	points.push_back(cv::Point3f(x, y, z));

	x = -.5; y = -.5; z = -.5;
	points.push_back(cv::Point3f(x, y, z));

	x = -.5; y = -.5; z = .5;
	points.push_back(cv::Point3f(x, y, z));

	for (unsigned int i = 0; i < points.size(); ++i)
	{
		cout << points[i] << endl << endl;
	}

	return points;
}

bool CaliProcess::ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount,
	/*CvMat* intrinsic_matrix, CvMat* distortion_coeffs,
	CvMat* rotation_vectors, CvMat* translation_vectors,*/ CString strFilename, int nFileCountreal/*,*/ /*CvMat *object_points, CvMat* image_points, CvMat* point_counts*/)
{
	if (nTargetCount != CALIIMAGES)
	{
		MessageBox(NULL, "标定类常量参数错误，更改参数重新标定", "提示", MB_OK);
		return true;
	}

	int nPointsCount = vecPts.size();//所有标定图像上的特征点个数
									 // 	int nEachPtsCount = nPointsCount / nTargetCount;
	CvMat* object_points = cvCreateMat(nPointsCount, 3, CV_64FC1);//保存特征点的空间坐标
	CvMat* image_points = cvCreateMat(nPointsCount, 2, CV_64FC1);//保存特征点的图像坐标
																 //	CvMat* project_points	= cvCreateMat(nPointsCount, 2, CV_64FC1);
	CvMat* point_counts = cvCreateMat(nTargetCount, 1, CV_32SC1);

	cvSetZero(object_points);
	cvSetZero(image_points);

	//	point_counts = cvCreateMat(nFileCountreal, 1, CV_32SC1);//每幅图像上特征点的个数
	cvSetZero(point_counts);
	/////////

	CvSize image_size;

	image_size.width = nImgWidth;
	image_size.height = nImgHeight;

	for (int i = 0; i < nPointsCount; i++)//保存特征点的空间坐标和图像坐标
	{
		CV_MAT_ELEM(*object_points, double, i, 0) = vecPts[i].xw;
		CV_MAT_ELEM(*object_points, double, i, 1) = vecPts[i].yw;
		CV_MAT_ELEM(*object_points, double, i, 2) = vecPts[i].zw;

		CV_MAT_ELEM(*image_points, double, i, 0) = vecPts[i].xf;
		CV_MAT_ELEM(*image_points, double, i, 1) = vecPts[i].yf;
	}

	//标相机
	for (int i = 0; i < nTargetCount; i++)//保存每幅图像的特征点个数
	{
		CV_MAT_ELEM(*point_counts, int, i, 0) = pEachPtsCount[i];
	}

	int flags;
	cvCalibrateCamera2(object_points, image_points,
		point_counts, image_size,
		intrinsic_matrix, distortion_coeffs,
		rotation_vectors, translation_vectors,
		flags = 0);

	CvMemStorage* memstorage = cvCreateMemStorage(0);//创建一个内存存储器，为0时默认内存为64K
	CvFileStorage* fs_write = cvOpenFileStorage(strFilename, memstorage, CV_STORAGE_WRITE);//打开存储文件
																						   //	cvWriteComment(fs_write, "calibration result", 1);

	cvWrite(fs_write, "intrinsic_matrix1", intrinsic_matrix, cvAttrList(NULL, NULL));//写入对象，如矩阵
	cvWrite(fs_write, "distortion_coeffs1", distortion_coeffs, cvAttrList(NULL, NULL));
	cvWrite(fs_write, "rotation_vectors1", rotation_vectors, cvAttrList(NULL, NULL));
	cvWrite(fs_write, "translation_vectors1", translation_vectors, cvAttrList(NULL, NULL));

	cvReleaseFileStorage(&fs_write);
	cvReleaseMemStorage(&memstorage);

	return TRUE;
}

bool CaliProcess::ZhangCameraCali(vector<C4DPointD>& vecPts, int nImgWidth, int nImgHeight, int nTargetCount, const int* pEachPtsCount,
	CvMat** intrinsic_matrixout, CvMat** distortion_coeffsout,
	CvMat** rotation_vectorsout, CvMat** translation_vectorsout, CString strFilename, int nFileCountreal/*,*/ /*CvMat *object_points, CvMat* image_points, CvMat* point_counts*/)
{
	if (nTargetCount != CALIIMAGES)
	{
		MessageBox(NULL, "标定类常量参数错误，更改参数重新标定", "提示", MB_OK);
		return true;
	}

	int nPointsCount = vecPts.size();//所有标定图像上的特征点个数
									 // 	int nEachPtsCount = nPointsCount / nTargetCount;
	CvMat* object_points = cvCreateMat(nPointsCount, 3, CV_64FC1);//保存特征点的空间坐标
	CvMat* image_points = cvCreateMat(nPointsCount, 2, CV_64FC1);//保存特征点的图像坐标
																 //	CvMat* project_points	= cvCreateMat(nPointsCount, 2, CV_64FC1);
	CvMat* point_counts = cvCreateMat(nTargetCount, 1, CV_32SC1);

	cvSetZero(object_points);
	cvSetZero(image_points);

	//	point_counts = cvCreateMat(nFileCountreal, 1, CV_32SC1);//每幅图像上特征点的个数
	cvSetZero(point_counts);
	/////////

	CvSize image_size;

	image_size.width = nImgWidth;
	image_size.height = nImgHeight;

	for (int i = 0; i < nPointsCount; i++)//保存特征点的空间坐标和图像坐标
	{
		CV_MAT_ELEM(*object_points, double, i, 0) = vecPts[i].xw;
		CV_MAT_ELEM(*object_points, double, i, 1) = vecPts[i].yw;
		CV_MAT_ELEM(*object_points, double, i, 2) = vecPts[i].zw;

		CV_MAT_ELEM(*image_points, double, i, 0) = vecPts[i].xf;
		CV_MAT_ELEM(*image_points, double, i, 1) = vecPts[i].yf;
	}

	//标相机
	for (int i = 0; i < nTargetCount; i++)//保存每幅图像的特征点个数
	{
		CV_MAT_ELEM(*point_counts, int, i, 0) = pEachPtsCount[i];
	}

	int flags;
	cvCalibrateCamera2(object_points, image_points,
		point_counts, image_size,
		intrinsic_matrix, distortion_coeffs,
		rotation_vectors, translation_vectors,
		flags = 0);

	*intrinsic_matrixout = cvCloneMat(intrinsic_matrix);
	*distortion_coeffsout = cvCloneMat(distortion_coeffs);
	*rotation_vectorsout = cvCloneMat(rotation_vectors);
	*translation_vectorsout = cvCloneMat(translation_vectors);

	CvMemStorage* memstorage = cvCreateMemStorage(0);//创建一个内存存储器，为0时默认内存为64K
	CvFileStorage* fs_write = cvOpenFileStorage(strFilename, memstorage, CV_STORAGE_WRITE);

	cvWrite(fs_write, "intrinsic_matrix1", intrinsic_matrix, cvAttrList(NULL, NULL));//写入对象，如矩阵
	cvWrite(fs_write, "distortion_coeffs1", distortion_coeffs, cvAttrList(NULL, NULL));
	cvWrite(fs_write, "rotation_vectors1", rotation_vectors, cvAttrList(NULL, NULL));
	cvWrite(fs_write, "translation_vectors1", translation_vectors, cvAttrList(NULL, NULL));

	cvReleaseFileStorage(&fs_write);
	cvReleaseMemStorage(&memstorage);

	return TRUE;
}

double cosangle(const CVect& v1, const CVect& v2)
{
	return (v1.x * v2.x + v1.y * v2.y) / (sqrt(v1.x * v1.x + v1.y * v1.y) * sqrt(v2.x * v2.x + v2.y * v2.y));
}

double sinanglefab(const CVect& v1, const CVect& v2)
{
	double cosanglev = (v1.x * v2.x + v1.y * v2.y) / (sqrt(v1.x * v1.x + v1.y * v1.y) * sqrt(v2.x * v2.x + v2.y * v2.y));
	return sqrt(1.0 - cosanglev * cosanglev);
}

int AutoThres(IplImage* frame)
{
	//IplImage* frame = reinterpret_cast<IplImage*>(img);
#define GrayScale 256   //frame灰度级
	int width = frame->width;
	int height = frame->height;
	int pixelCount[GrayScale] = { 0 };
	float pixelPro[GrayScale] = { 0 };
	int pixelSum = width * height, threshold = 0;
	//uchar* data = (uchar*)frame->imageData;

	//统计每个灰度级中像素的个数
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			//pixelCount[(int)data[i * width + j]]++;
			pixelCount[(int)cvGet2D(frame, i, j).val[0]]++;
		}
	}

	//计算每个灰度级的像素数目占整幅图像的比例
	for (int i = 0; i < GrayScale; i++)
	{
		pixelPro[i] = (float)pixelCount[i] / pixelSum;
	}
	//遍历灰度级[0,255],寻找合适的threshold
	float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax = 0;
	for (int i = 0; i < GrayScale; i++)
	{
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0;
		for (int j = 0; j < GrayScale; j++)
		{
			if (j <= i)   //背景部分
			{
				w0 += pixelPro[j];
				u0tmp += j * pixelPro[j];
			}
			else   //前景部分
			{
				w1 += pixelPro[j];
				u1tmp += j * pixelPro[j];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		deltaTmp = (float)(w0 * w1 * pow((u0 - u1), 2));
		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = i;
		}
	}
	return threshold;
}