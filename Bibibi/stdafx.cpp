
// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// WeldingRobot.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

CString GetProgramePath()
{
	char currentpath[MAX_PATH];
	GetModuleFileNameA(NULL, currentpath, MAX_PATH);
	CString strPath( currentpath );
	int n = strPath.ReverseFind('\\');	
	return strPath.Left( n - 5 );	//�ú������ص�ǰ���г����·������exe�ļ�����Ŀ¼��һ���ǳ����Debug�ļ���
	//Debug�����Ѿ����š�\��,��ˣ�����뽨��һ�����ļ��У��ļ�������ǰ���üӡ�\\��
	//�磺str += ��Test\\test.txt����Ϊ·��
	//����Ҫע�⣺������ļ���Ҫ��ǰ�˹������ã������ܽ����ĵ�����ĵ�
}
