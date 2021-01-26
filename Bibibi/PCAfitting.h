#pragma once

#define SQR(a) ((a) * (a)) 

class PCAfitting
{
public:
	// ��һ������x��ƽ��ֵ
	static double one_data_mean(double* x,int n);
	// ����������x,y��Э����
	static double two_data_mean_cov(double* x,double xdmean,double* y, double ydmean, int n);
	// ��ʵ�Գƾ���matrix������ֵdvalue����������dvector
	static void sym_matrix_egin_vector_value(double* matrix, int n, double* dvalue, double* dvector);
	// ������������dvector1,dvector2��һ����point����ƽ�淽��plane
	static void two_vector_one_point_plane(double* dvector1, double* dvector2, double* point,double* plane);
	static void one_vector_one_point_plane(double* dvector1, double* point,double* plane);
};

void CrossProduct3(double* src1,double* src2,double* dst);
void unit_vector(double* src,int len, double* dst);
double DotProduct( const double* src1, const double* src2, int len );