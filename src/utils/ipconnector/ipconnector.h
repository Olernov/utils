#ifdef	WIN32
#	ifdef	IPCONNECTOR_IMPORT
#		define	IPCONNECTOR_SPEC	__declspec(dllimport)
#	else
#		define	IPCONNECTOR_SPEC __declspec(dllexport)
#	endif
#else
#	define	IPCONNECTOR_SPEC
#	include <netinet/in.h>
#endif

class IPCONNECTOR_SPEC CIPConnector {
public:
	CIPConnector ();
	CIPConnector (int p_iReqTimeout);
	~CIPConnector ();
public:
	int Connect (const char *p_pszHostName, unsigned short p_usPort, int p_iProtoType = IPPROTO_TCP);
	int Send (const char *p_mcBuf, int p_iLen);
	int Recv (char *p_mcBuf, int p_iBufSize);
	void DisConnect ();
	/* ������ 0 - ������ ������ ������
	   ������ 1 - ������ ������ ��������������� (��� Windows ������������� ��������, ��� ���������� WSA ���������)
	   ������ 2 - ���������� ����������� */
	int GetStatus () { return m_iStatus; }
private:
	/* ������ ������� */
	int m_iStatus;
	/* ����� ����������� ����������� �������� �������� � ������� � ��������.
	   �������� � ������������ CIPConnector (int p_iReqTimeout). �� ��������� = 5 ��� */
	int m_iRequestTimeout;
#ifdef WIN32
	SOCKET
#else
	int
#endif
	m_sockSock;
private:
	void init (int p_iReqTimeout);
};
