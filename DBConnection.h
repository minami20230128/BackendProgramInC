#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <libpq-fe.h>

PGconn *db_connect(void);
PGresult *execute_query(char *query, PGconn *conn);
void db_disconnect(PGconn *conn);

#endif