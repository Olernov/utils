#ifndef _FILE_LIST_H
#define _FILE_LIST_H

#include "utils/filereader/filereader.h" /* SFileInfo */

#include <string>
#include <map>

#ifdef WIN32
#	ifdef	FILE_LIST_EXPORT
#		define	FILE_LIST_SPEC __declspec(dllexport)
#	else
#		define	FILE_LIST_SPEC __declspec(dllimport)
#	endif
#else
#	define	FILE_LIST_SPEC
#endif

struct FILE_LIST_SPEC SFileListInfo {
	std::string m_strFileType;		/* ��� ������ �������� ������ "file", "ftp", "sftp", "ftps" */
	std::string m_strHost;			/* ��� �����-��������� �������� ������ */
	std::string m_strUserName;		/* ��� ������������ ��� ������� � �����-��������� �������� ������ */
	std::string m_strPassword;		/* ������ ������������ ��� ������� � �����-��������� �������� ������ */
	std::string m_strPath;			/* �����, � ������� ������������ ����� ������ */
	std::multimap<std::string, SFileInfo> *m_pmmapFileList; /* ��������� ��� �������� ������ ������ */
	int m_iLookNestedFold;			/* ���� ������ �� ��������� ������: 0 - �� ������ ����� �� ��������� ������ */
};

class FILE_LIST_SPEC CFileList {
public:
	/* ������� ������ ������ �� �������� ���������� 
	   ���������� 0 � ������ ������ */
	int CreateFileList (SFileListInfo &p_soFileListOpt);
#ifdef _WIN32
	CFileList () { m_hCURLLib = NULL; }
	int CURL_Init (SFileInfo &p_soCURLLibInfo);
	int CURL_Cleanup ();
private:
	HMODULE m_hCURLLib;
#endif
};

#endif /* _FILE_LIST_H */
