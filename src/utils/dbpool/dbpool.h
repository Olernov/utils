#ifndef _DBPOOL_H_
#define _DBPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OTL_ORA11G_R2
#define OTL_STL
#ifdef _WIN32
#	define OTL_UBIGINT long unsigned int
#	define OTL_BIGINT long int
#endif
#define OTL_STREAM_NO_PRIVATE_UNSIGNED_LONG_OPERATORS
#include "utils/otlv4.h"

#include "utils/log/log.h"
#include "utils/config/config.h"

/* ������������� �������� ���������� */
int db_pool_init (CLog *p_pcoLog, CConfig *p_pcoConf);
/* ��������������� ���� */
void db_pool_deinit ();

/* ������ ��������� �� ������ ������ ��� �������������� � �� */
otl_connect * db_pool_get ();

/* ������������ �������� ������� ������ �������������� � �� */
int db_pool_release (otl_connect *p_pcoDBConn);

/* �������� ����������������� ����������� � �� */
int db_pool_check (otl_connect &p_coDBConn);

/* ��������������� � �� */
int db_pool_reconnect (otl_connect &p_coDBConn);

#ifdef __cplusplus
}
#endif

#endif /* _DBPOOL_H_ */
