#ifndef _FILE_READER
#define _FILE_READER

#include <string>
#ifdef WIN32
#	include <Windows.h>
#endif

#ifdef WIN32
#	ifdef	FILE_READER_EXPORT
#		define	FILE_READER_SPEC __declspec(dllexport)
#	else
#		define	FILE_READER_SPEC __declspec(dllimport)
#	endif
#else
#	define	FILE_READER_SPEC
#endif

struct SFileInfo {
	std::string m_strTitle; 
	std::string m_strDir;
	size_t m_stFileSize;
};

class FILE_READER_SPEC CFileReader
{
public:
	/* �������� ����� */
	int OpenDataFile (
		std::string &p_strType,			/* "file", "ftp" or "sftp" */
#ifdef _WIN32
		SFileInfo &p_soCURLLib,			/* ��������� ���������� cURL */
#endif
		SFileInfo &p_soFileInfo,		/* ��������� ������������ ����� */
		std::string *p_pstrHost,		/* ��� ����� (ftp, sftp) */
		std::string *p_pstrUserName,	/* ��� ������������ (ftp, sftp) */
		std::string *p_pstrPassword);	/* ������ ������������ (ftp, sftp) */
	/* ������ ������ �� ������ ���������� 0 � ������ ��������� ������ ���� ����������� ������ */
	int ReadData (
		unsigned char *p_pucData,		/* ����� ��� ������ ������ */
		int &p_iDataSize);				/* ������ ������, � ������ ��������� ���������� ������� ���������� ���������� � ����� ������ */
	/* ��������� ���� */
	int CloseDataFile ();
	const char * GetDir() { return m_soFileInfo.m_strDir.c_str (); }
	const char * GetFileName() { return m_soFileInfo.m_strTitle.c_str (); }
	/* ��������� �� ������ � ����� */
	int IsWritingBufCompl ();
	/* ���� �� ����� */
	int IsBufferEmpty ();
private:
	int AllocateMemBlock (size_t p_stSize);
#ifdef _WIN32
	friend DWORD WINAPI CURL_LoadFile (void *p_pcoThis);
	friend DWORD WINAPI FS_LoadFile (void *p_pcoThis);
#else
	friend void * CURL_LoadFile (void *p_pcoThis);
	friend void * FS_LoadFile (void *p_pcoThis);
#endif
	friend size_t CURL_Write (void *p_pvSrc, size_t p_stSize, size_t p_stCount, CFileReader *p_pcoFileReader);
public:
	CFileReader (void);
	~CFileReader (void);
private:
	int m_hFile;
	SFileInfo m_soFileInfo;
	std::string m_strType;
	unsigned char *m_pmucBuf;		/* ����� ��� ����������� ����������� ����� */
	size_t m_stBufSize;				/* ������ ������ */
	volatile size_t m_stFileSize;	/* ������ ����������������� ����� */
	volatile size_t m_stReadPointer;	/* ������� ��� ������ �� ������ */
	volatile int m_iIsWritingBufCompl;	/* ��������� �� ������ � ����� */
#ifdef _WIN32
	int CURL_Init (SFileInfo &p_soCURLLibInfo);
	HANDLE m_hWriteThread;			/* ���������� ������������ ������ ������ ������ � ����� */
#else
	pthread_t m_hWriteThread;		/* ���������� ������������ ������ ������ ������ � ����� */
#endif
	int m_iCancelWritingBuf;		/* �������� ������ � ����� */
	char *m_pszHost;				/* ��� ���������� �����-��������� */
	char *m_pszUserName;			/* ��� ������������ ������� � �����-��������� */
	char *m_pszPassword;			/* ������ ������������ ������� � �����-��������� */
#ifdef _WIN32
	CRITICAL_SECTION m_soCSBuf;		/* ����������� ������ ��� �������� � ������� */
	HMODULE m_hCURLLib;				/* ���������� ������������ ���������� */
#else
	pthread_mutex_t m_tMutex;		/* ������� ��� �������� � ������� */
#endif
};

#endif /* _FILE_READER */
