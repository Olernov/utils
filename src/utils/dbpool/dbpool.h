#ifndef _DBPOOL_H_
#define _DBPOOL_H_

#define OTL_ORA11G_R2
#define OTL_STL
#define OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE
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

#endif /* _DBPOOL_H_ */
