#pragma once

#define SQR(a) ((a) * (a)) 

class PCAfitting
{
public:
	// 求一组数据x的平均值
	static double one_data_mean(double* x,int n);
	// 球两组数据x,y的协方差
	static double two_data_mean_cov(double* x,double xdmean,double* y, double ydmean, int n);
	// 求实对称矩阵matrix的特征值dvalue和特征向量dvector
	static void sym_matrix_egin_vector_value(double* matrix, int n, double* dvalue, double* dvector);
	// 借助两个向量dvector1,dvector2和一个点point计算平面方程plane
	static void two_vector_one_point_plane(double* dvector1, double* dvector2, double* point,double* plane);
	static void one_vector_one_point_plane(double* dvector1, double* point,double* plane);
};

void CrossProduct3(double* src1,double* src2,double* dst);
void unit_vector(double* src,int len, double* dst);
double DotProduct( const double* src1, const double* src2, int len );