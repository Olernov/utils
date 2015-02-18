#include "dbpool.h"

#include <string>
#include <semaphore.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
#define LOG_ERR_DESCR(str) 		iFnRes = errno; \
		char mcErrDescr[1024]; \
		if (g_pcoLog) { \
			if (0 != strerror_r (iFnRes, mcErrDescr, sizeof (mcErrDescr))) { \
				mcErrDescr[0] = '\0'; \
			} \
			g_pcoLog->WriteLog ("dbpool: %s: error: %s: code: '%d'; description: '%s'", __func__, str, iFnRes, mcErrDescr); \
		}
#else
#define LOG_ERR_DESCR(str) 		iFnRes = errno; \
		char mcErrDescr[1024]; \
		if (g_pcoLog) { \
			if (NULL == strerror_r (iFnRes, mcErrDescr, sizeof (mcErrDescr))) { \
				mcErrDescr[0] = '\0'; \
			} \
			g_pcoLog->WriteLog ("dbpool: %s: error: %s: code: '%d'; description: '%s'", __func__, str, iFnRes, mcErrDescr); \
		}
#endif

/* ������ ������ ����������� � �� */
static char g_mcRadDBConnTempl[] = "%s/%s@%s";
/* ��������� �� ������ ������� */
static CLog *g_pcoLog = NULL;
/* ��������� �� ������ ������������ */
static CConfig *g_pcoConf = NULL;

/* ������� ��� �������� ���������� ��������� �� ������ ������ ��� �������������� � �� */
sem_t g_tSem;
/* ������ ���� */
static int g_iPoolSize;
/* ������� ���� */
struct SPoolElem {
	otl_connect m_coDBConn;
	bool m_bIsBusy;
	SPoolElem () { m_bIsBusy = false; }
	~SPoolElem () { m_bIsBusy = true; }
};
/* ������ ���� */
static SPoolElem *g_pmsoPool = NULL;
/* ������� ��� ��������� �������� ��������� ���� */
static pthread_mutex_t g_tMutex;

void DisconnectDB (otl_connect &p_coDBConn)
{
	if (p_coDBConn.connected) {
		p_coDBConn.cancel ();
		p_coDBConn.logoff ();
	}
}

int ConnectDB (otl_connect &p_coDBConn)
{
	/* �������� ���������� */
	if (NULL == g_pcoConf) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("%s: dbpool: error: configuration not defined", __func__);
		}
		return -1;
	}

	int iRetVal = 0;
	int iFnRes = 0;
	char mcConnStr[1024];

	std::string
		strDBUser,
		strDBPswd,
		strDBDescr;
	const char *pcszConfParam = NULL;

	/* ����������� ��� ������������ �� �� ������������ */
	pcszConfParam = "db_user";
	iFnRes = g_pcoConf->GetParamValue (pcszConfParam, strDBUser);
	if (iFnRes || 0 == strDBUser.length ()) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: configuration parameter '%s' not defined", __func__, pcszConfParam);
		}
	}

	/* ����������� ������ ������������ �� �� ������������ */
	pcszConfParam = "db_pswd";
	iFnRes = g_pcoConf->GetParamValue (pcszConfParam, strDBPswd);
	if (iFnRes || 0 == strDBPswd.length ()) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: configuration parameter '%s' not defined", __func__, pcszConfParam);
		}
	}

	/* ����������� ���������� �� �� ������������ */
	pcszConfParam = "db_descr";
	iFnRes = g_pcoConf->GetParamValue (pcszConfParam, strDBDescr);
	if (iFnRes || 0 == strDBDescr.length ()) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: configuration parameter '%s' not defined", __func__, pcszConfParam);
		}
	}

	/* ��������� ������ ����������� */
	snprintf (
		mcConnStr,
		sizeof(mcConnStr),
		g_mcRadDBConnTempl,
		strDBUser.c_str(),
		strDBPswd.c_str(),
		strDBDescr.c_str());
	try {
		p_coDBConn.rlogon (mcConnStr);
		p_coDBConn.auto_commit_off ();
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: DB connected successfully", __func__);
		}
	} catch (otl_exception &coOtlExc) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: code: '%d'; description: '%s'", __func__, coOtlExc.code, coOtlExc.msg);
		}
		iRetVal = coOtlExc.code;
	}

	return iRetVal;
}

int db_pool_reconnect (otl_connect &p_coDBConn)
{
	int iRetVal = 0;
	int iFnRes;

	do {
		if (p_coDBConn.connected) {
			DisconnectDB (p_coDBConn);
		}
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: info: trying to reconnect to DB", __func__);
		}
		iFnRes = ConnectDB (p_coDBConn);
		if (iFnRes) {
			/* ������������ � �� �� ������� */
			if (g_pcoLog) {
				g_pcoLog->WriteLog ("dbpool: %s: error: reconnect failed", __func__);
			}
			iRetVal = -200001;
			break;
		}
	} while (0);

	return iRetVal;
}

int db_pool_check (otl_connect &p_coDBConn)
{
	int iRetVal = 0;
	int iFnRes;

	/* ������ ������ �� ��������� � �� */
	if (! p_coDBConn.connected) {
		return -2;
	}

	std::string strDBDummyReq;

	/* ����������� ����� ������������ ������� �� ������������ */
	if (g_pcoConf) {
		iFnRes = g_pcoConf->GetParamValue ("db_dummy_req", strDBDummyReq);
	} else {
		iFnRes = -1;
	}
	if (iFnRes || 0 == strDBDummyReq.length ()) {
		strDBDummyReq = "select to_char(sysdate, 'yyyy') from dual";
	}

	/* ��������� ����������������� ����������� �� ���������� ������� */
	try {
		char mcTime[128];
		otl_stream coStream (1, strDBDummyReq.c_str (), p_coDBConn);
		coStream >> mcTime;
	} catch (otl_exception &coExc) {
		/* ���� ������ ���������� � ������� */
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: connection test failed: error: code: '%s'; description: '%s'", __func__, coExc.code, coExc.msg);
		}
		iRetVal = -3;
	}

	return iRetVal;
}

int db_pool_init (CLog *p_pcoLog, CConfig *p_pcoConf)
{
	/* �������� ��������� �� ������ ������ ������� */
	if (p_pcoLog) {
		g_pcoLog = p_pcoLog;
	} else {
		return -1;
	}
	/* �������� ��������� �� ������ ������ ������������ */
	if (p_pcoConf) {
		g_pcoConf = p_pcoConf;
	} else {
		return -1;
	}

	int iRetVal = 0;
	int iFnRes;

	/* ����������� � ����������� ������ ���� */
	std::string strDBPoolSize;
	const char *pcszConfParam = "db_pool_size";
	iFnRes = g_pcoConf->GetParamValue (pcszConfParam, strDBPoolSize);
	if (iFnRes || 0 == strDBPoolSize.length ()) {
		g_iPoolSize = 1;
		g_pcoLog->WriteLog ("dbpool: %s: info: configuration parameter '%s' not defined, set pool size value to '%d'", __func__, pcszConfParam, g_iPoolSize);
	} else {
		g_iPoolSize = atoi (strDBPoolSize.c_str ());
	}

	/* �������� ������ ��� ���� */
	g_pmsoPool = new SPoolElem [g_iPoolSize];
	for (int iInd = 0; iInd < g_iPoolSize; ++ iInd) {
		iFnRes = ConnectDB (g_pmsoPool[iInd].m_coDBConn);
		if (iFnRes) {
			break;
		}
	}

	/* ��������� ��������� ������������� ������� */
	if (iFnRes) {
		db_pool_deinit ();
		return -2;
	}

	/* �������������� ������� */
	iFnRes = sem_init (&g_tSem, 0, g_iPoolSize);
	if (iFnRes) {
		db_pool_deinit ();
		return -3;
	}

	/* ������������� �������� */
	iFnRes = pthread_mutex_init (&g_tMutex, NULL);
	if (iFnRes) {
		db_pool_deinit ();
		return -4;
	}
	/* �� ������ ������ ����������� �������, ��� ������ ������������� �� ��� ����� ��������� */
	pthread_mutex_unlock (&g_tMutex);

	return iRetVal;
}

void db_pool_deinit ()
{
	/* ����������� �������, ������� ��������� */
	sem_destroy (&g_tSem);

	/* ����������� �������, ������� ��������� */
	pthread_mutex_destroy (&g_tMutex);

	/* ����������� ������ ���� */
	if (g_pmsoPool) {
		for (int iInd = 0; iInd < g_iPoolSize; ++ iInd) {
			DisconnectDB (g_pmsoPool[iInd].m_coDBConn);
		}
		delete [] g_pmsoPool;
		g_pmsoPool = NULL;
	}
}

otl_connect * db_pool_get ()
{
	otl_connect * pcoRetVal = NULL;

	int iFnRes;
	timespec soTimeSpec;
	timeval soTimeVal;

	/* ����������� ������� ����� */
	iFnRes = gettimeofday (&soTimeVal, NULL);
	if (iFnRes) {
		return NULL;
	}

	/* ��������� ����� �� �������� �� ����� ����� ������������ �������� */
	soTimeSpec.tv_sec = soTimeVal.tv_sec + 1;
	soTimeSpec.tv_nsec = soTimeVal.tv_usec * 1000;

	/* ���� ������������ ������� ��� �������������� � �� */
	iFnRes = sem_timedwait (&g_tSem, &soTimeSpec);
	/* ���� �� ����� �������� �������� ������ */
	if (iFnRes) {
		LOG_ERR_DESCR ("sem_timedwait error accurred");
		return NULL;
	}

	/* ����������� ������� ����� */
	iFnRes = gettimeofday (&soTimeVal, NULL);
	if (iFnRes) {
		return NULL;
	}

	/* ��������� ����� �� �������� �� ����� ����� ������������ �������� */
	soTimeSpec.tv_sec = soTimeVal.tv_sec + 1;
	soTimeSpec.tv_nsec = soTimeVal.tv_usec * 1000;

	/* ������ � ����������� ������ */
	iFnRes = pthread_mutex_timedlock (&g_tMutex, &soTimeSpec);
	/* ���� �� ����� �������� �������� ������ */
	if (iFnRes) {
		LOG_ERR_DESCR ("pthread_mutex_timedlock error accurred");
		/* ���������� ������� */
		sem_post (&g_tSem);
		return NULL;
	}

	/* ���� ��������� ������� */
	for (int iInd = 0; iInd < g_iPoolSize; ++ iInd) {
		/* ���� ���� ��������� ������� */
		if (! g_pmsoPool[iInd].m_bIsBusy) {
			g_pmsoPool[iInd].m_bIsBusy = true;
			pcoRetVal = & (g_pmsoPool[iInd].m_coDBConn);
		}
	}
	/* �� ������ ������ �������� �������� ��������� */
	if (NULL == pcoRetVal) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: unexpected error occurred: there is no free db connections but semaphore has allowed to pass through it", __func__);
		}
	}

	/* ����������� ������� */
	pthread_mutex_unlock (&g_tMutex);

	return pcoRetVal;
}

int db_pool_release (otl_connect *p_pcoDBConn)
{
	int iRetVal;
	int iFnRes;
	timespec soTimeSpec;
	timeval soTimeVal;

	/* ����������� ������� ����� */
	iFnRes = gettimeofday (&soTimeVal, NULL);
	if (iFnRes) {
		return NULL;
	}

	/* ��������� ����� �� �������� �� ����� ����� ������������ �������� */
	soTimeSpec.tv_sec = soTimeVal.tv_sec + 1;
	soTimeSpec.tv_nsec = soTimeVal.tv_usec * 1000;

	/* ������ � ����������� ������ */
	iFnRes = pthread_mutex_timedlock (&g_tMutex, &soTimeSpec);
	/* ���� �� ����� �������� �������� ������ */
	if (iFnRes) {
		LOG_ERR_DESCR ("pthread_mutex_timedlock error accurred");
		return -1;
	}

	int iInd = 0;
	/* ���� ��������������� ������� */
	for (; iInd < g_iPoolSize; ++ iInd) {
		/* ���� ���� ������� ������� */
		if (&(g_pmsoPool[iInd].m_coDBConn) == p_pcoDBConn) {
			break;
		}
	}
	/* �� ������ ������ �������� �������� */
	if (g_iPoolSize == iInd) {
		if (g_pcoLog) {
			g_pcoLog->WriteLog ("dbpool: %s: error: unexpected error occurred: db coonector not found", __func__);
		}
	} else {
		g_pmsoPool[iInd].m_bIsBusy = false;
	}

	/* ����������� ������� */
	iFnRes = pthread_mutex_unlock (&g_tMutex);
	/* ���� �� ����� �������� �������� ������ */
	if (iFnRes) {
		LOG_ERR_DESCR ("pthread_mutex_unlock error accurred");
	}

	/* ����������� ������� �������� */
	iFnRes = sem_post (&g_tSem);
	/* ���� �� ����� �������� �������� ������ */
	if (iFnRes) {
		LOG_ERR_DESCR ("sem_post error accurred");
	}

	return iRetVal;
}
