//#include "stdafx.h"
//#include "pcaa.h"
#include "math.h"
//#include "MatrixZZ.h"
#include "PCAfitting.h"

double PCAfitting::one_data_mean(double* x, int n)
{
	if (n == 0)
		return 0;
	double temp = 0.0;
	for (int i = 0; i < n; i++)
	{
		temp += x[i];
	}
	temp /= n;
	return temp;
}
double PCAfitting::two_data_mean_cov(double* x, double xmean, double* y, double ymean, int n)
{
	double temp = 0.0;
	for (int i = 0; i < n; i++)
	{
		temp += (x[i] - xmean) * (y[i] - ymean);
	}
	temp /= (n - 1);
	return temp;
}

//约化对称矩阵为对称三对角阵的豪斯荷尔德变换法
int Householder(int n, double* a, double* q, double* b, double* c)
{
	int i, j, k, u;
	double h, f, g, h2;
	for (i = 0; i <= n - 1; i++)
		for (j = 0; j <= n - 1; j++)
		{
			u = i * n + j; q[u] = a[u];
		}
	for (i = n - 1; i >= 1; i--)
	{
		h = 0.0;
		if (i > 1)
			for (k = 0; k <= i - 1; k++)
			{
				u = i * n + k; h = h + q[u] * q[u];
			}
		if (h + 1.0 == 1.0)
		{
			c[i] = 0.0;
			if (i == 1) c[i] = q[i * n + i - 1];
			b[i] = 0.0;
		}
		else
		{
			c[i] = sqrt(h);
			u = i * n + i - 1;
			if (q[u] > 0.0) c[i] = -c[i];
			h = h - q[u] * c[i];
			// 			q.SetElem(i+1, i, q[u]-c[i]);
			q[u] = q[u] - c[i];            f = 0.0;
			for (j = 0; j <= i - 1; j++)
			{
				// 				q.SetElem(j+1, i+1,q[i*n+j]/h);
				q[j * n + i] = q[i * n + j] / h;                g = 0.0;
				for (k = 0; k <= j; k++)
					g = g + q[j * n + k] * q[i * n + k];
				if (j + 1 <= i - 1)
					for (k = j + 1; k <= i - 1; k++)
						g = g + q[k * n + j] * q[i * n + k];
				c[j] = g / h;
				f = f + g * q[j * n + i];
			}
			h2 = f / (h + h);
			for (j = 0; j <= i - 1; j++)
			{
				f = q[i * n + j];
				g = c[j] - h2 * f;
				c[j] = g;
				for (k = 0; k <= j; k++)
				{
					u = j * n + k;
					// 					q.SetElem(j+1, k+1, q[u]-f*c[k]-g*q[i*n+k]);
					q[u] = q[u] - f * c[k] - g * q[i * n + k];
				}
			}
			b[i] = h;
		}
	}
	for (i = 0; i <= n - 2; i++) c[i] = c[i + 1];
	c[n - 1] = 0.0;
	b[0] = 0.0;
	for (i = 0; i <= n - 1; i++)
	{
		if ((b[i] != 0.0) && (i - 1 >= 0))
			for (j = 0; j <= i - 1; j++)
			{
				g = 0.0;
				for (k = 0; k <= i - 1; k++)
					g = g + q[i * n + k] * q[k * n + j];
				for (k = 0; k <= i - 1; k++)
				{
					u = k * n + j;
					// 					q.SetElem(k+1, j+1, q[u]-g*q[k*n+i]);
					q[u] = q[u] - g * q[k * n + i];
				}
			}
		u = i * n + i;
		b[i] = q[u];
		// 			q.SetElem(i+1, i+1, 1.0);
		q[u] = 1.0;
		if (i - 1 >= 0)
			for (j = 0; j <= i - 1; j++)
			{
				// 					q.SetElem(i+1, j+1, 0.0);
				// 					q.SetElem(j+1, i+1, 0.0);
				q[i * n + j] = 0.0;
				q[j * n + i] = 0.0;
			}
	}
	return(1);
}

int compute_eigen(int n, double* b, double* c, double* q, double eps, int l)
{
	int i, j, k, m, it, u, v;
	double d, f, h, g, p, r, e, s;
	c[n - 1] = 0.0; d = 0.0; f = 0.0;
	for (j = 0; j <= n - 1; j++)
	{
		it = 0;
		h = eps * (fabs(b[j]) + fabs(c[j]));
		if (h > d) d = h;
		m = j;
		while ((m <= n - 1) && (fabs(c[m]) > d)) m = m + 1;
		if (m != j)
		{
			do
			{
				if (it == l)
				{
					AfxMessageBox("fail");
					return(-1);
				}
				it = it + 1;
				g = b[j];
				p = (b[j + 1] - g) / (2.0 * c[j]);
				r = sqrt(p * p + 1.0);
				if (p >= 0.0) b[j] = c[j] / (p + r);
				else b[j] = c[j] / (p - r);
				h = g - b[j];
				for (i = j + 1; i <= n - 1; i++)
					b[i] = b[i] - h;
				f = f + h; p = b[m]; e = 1.0; s = 0.0;
				for (i = m - 1; i >= j; i--)
				{
					g = e * c[i]; h = e * p;
					if (fabs(p) >= fabs(c[i]))
					{
						e = c[i] / p; r = sqrt(e * e + 1.0);
						c[i + 1] = s * p * r; s = e / r; e = 1.0 / r;
					}
					else
					{
						e = p / c[i]; r = sqrt(e * e + 1.0);
						c[i + 1] = s * c[i] * r;
						s = 1.0 / r; e = e / r;
					}
					p = e * b[i] - s * g;
					b[i + 1] = h + s * (e * g + s * b[i]);
					for (k = 0; k <= n - 1; k++)
					{
						u = k * n + i + 1; v = u - 1;
						h = q[u];
						// 						q.SetElem(k+1, i+2, s*q[v]+e*h);
						// 						q.SetElem(k+1, i+1, e*q[v]-s*h);
						q[u] = s * q[v] + e * h;
						q[v] = e * q[v] - s * h;
					}
				}
				c[j] = s * p; b[j] = e * p;
			} while (fabs(c[j]) > d);
		}
		b[j] = b[j] + f;
	}
	for (i = 0; i <= n - 1; i++)
	{
		k = i; p = b[i];
		if (i + 1 <= n - 1)
		{
			j = i + 1;
			while ((j <= n - 1) && (b[j] <= p))
			{
				k = j; p = b[j]; j = j + 1;
			}
		}
		if (k != i)
		{
			b[k] = b[i]; b[i] = p;
			for (j = 0; j <= n - 1; j++)
			{
				u = j * n + i; v = j * n + k;
				p = q[u];
				// 				q.SetElem(j+1, i+1, q[v]);
				// 				q.SetElem(j+1, k+1, p);
				q[u] = q[v];
				q[v] = p;
			}
		}
	}
	return(1);
}

void PCAfitting::sym_matrix_egin_vector_value(double* matrix, int n, double* dvalue, double* dvector)
{
	double* q = new double[n * n];
	double* b = new double[n];
	double* c = new double[n];

	Householder(n, matrix, q, b, c);
	compute_eigen(n, b, c, q, 0.000001, 60);

	memcpy(dvalue, b, n * sizeof(double));
	memcpy(dvector, q, n * n * sizeof(double));

	delete[] q;
	delete[] b;
	delete[] c;
}
void PCAfitting::two_vector_one_point_plane(double* dvector1, double* dvector2, double* point, double* plane)
{
	double dst[3];

	CrossProduct3(dvector1, dvector2, dst);
	unit_vector(dst, 3, dst);
	memcpy(plane, dst, 3 * sizeof(double));
	plane[3] = DotProduct(plane, point, 3);
	plane[3] = -plane[3];
}

void PCAfitting::one_vector_one_point_plane(double* dvector1, double* point, double* plane)
{
	double dst[3];
	unit_vector(dvector1, 3, dst);
	memcpy(plane, dst, 3 * sizeof(double));
	plane[3] = DotProduct(plane, point, 3);
	plane[3] = -plane[3];
}
//////////////////////////////////////////////////////////////////////////

void CrossProduct3(double* src1, double* src2, double* dst)
{
	dst[2] = src1[0] * src2[1] - src1[1] * src2[0];
	dst[0] = src1[1] * src2[2] - src1[2] * src2[1];
	dst[1] = src1[2] * src2[0] - src1[0] * src2[2];
}

void unit_vector(double* src, int len, double* dst)
{
	double normal = sqrt(DotProduct(src, src, len));
	if (normal == 0.0)
		return;
	for (int i = 0; i < len; i++) dst[i] = src[i] / normal;
}

double DotProduct(const double* src1, const double* src2, int len)
{
	double s = 0.0;
	for (int i = 0; i < len; i++) s += src1[i] * src2[i];
	return s;
}