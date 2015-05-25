#ifndef _LOG_H_
#define _LOG_H_

#ifdef	WIN32
#include <Windows.h>
#	ifdef	LOG_EXPORT
#		define	LOG_SPEC __declspec(dllexport)
#	else
#		define	LOG_SPEC	__declspec(dllimport)
#	endif
#else
#	define	LOG_SPEC
#	include <pthread.h>
#	include <unistd.h>
#endif

#define LOG(logger,status, format,args...)	(logger).WriteLog(status ": %s@%s[%u]: " format, __FUNCTION__, __FILE__, __LINE__, ## args)
#define LOG_E(logger,format,args...)	LOG(logger,"error",format,##args)
#define LOG_N(logger,format,args...)	LOG(logger,"noti",format,##args)
#ifdef DEBUG
#define LOG_D(logger,format,args...)	LOG(logger,"debug",format,##args)
#else
#define LOG_D
#endif
#define LOG_W(logger,format,args...)	LOG(logger,"warning",format,##args)
#define LOG_F(logger,format,args...)	LOG(logger,"fatal",format,##args)

class LOG_SPEC CLog {
public:
	int Init (const char *p_pcszLogFileMask, char *p_pszEndOfLine = NULL);
	void Flush ();

#ifdef WIN32
#else
	void SetUGIds (gid_t p_idUId, gid_t p_idGId);
#endif
	void WriteLog (const char *p_pszMsg, ...);
	void Dump (const char *p_pszTitle, const char *p_pszMessage);
	void Dump (const char *p_pszMessage);
public:
	CLog ();
	~CLog ();
private:
	int GetLogFileName (char *p_pszLogFileName, size_t p_stMaxSize);
	bool CheckLogFileName (const char *p_pcszLogFileName);
	int OpenLogFile (const char *p_pcszLogFileName);
#ifdef WIN32
	friend DWORD WINAPI ReCreateFileProc (void *p_pParam);
#else
	friend void * ReCreateFileProc (void *p_pParam);
#endif
private:
#ifdef WIN32
	HANDLE m_iLogFileFD;
	HANDLE m_hReCreateFile;
	double m_dCap;
#else
	int m_iLogFileFD;
	pthread_t m_hReCreateFile;
	gid_t m_idUId;
	gid_t m_idGId;
	/* mutex ��� �������� ������������ ����� ����� �������� ����� */
	pthread_mutex_t m_tMutex;
	/* �������� �������� ����� ����� � �������� */
	unsigned int m_uiWaitInterval;
#endif
	int m_iInit;
	char *m_pszLogFileMask;
	char *m_pszLogFileName;
	char *m_pszEndOfLine;
};

#endif	// _LOG_H_
