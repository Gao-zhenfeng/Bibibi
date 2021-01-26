
// stdafx.cpp : 只包括标准包含文件的源文件
// WeldingRobot.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

CString GetProgramePath()
{
	char currentpath[MAX_PATH];
	GetModuleFileNameA(NULL, currentpath, MAX_PATH);
	CString strPath( currentpath );
	int n = strPath.ReverseFind('\\');	
	return strPath.Left( n - 5 );	//该函数返回当前运行程序的路径，即exe文件所在目录，一般是程序的Debug文件夹
	//Debug后面已经跟着‘\’,因此，如果想建立一个子文件夹，文件夹名字前不用加“\\”
	//如：str += “Test\\test.txt”作为路径
	//但是要注意：这个子文件夹要提前人工建立好，否则不能建立文档或打开文档
}
