#include <stdio.h>
#include "DBConnection.h"

PGconn *db_connect(void)
{
    PGconn *conn = PQconnectdb(
        "host=localhost "
        "port=5432 "
        "dbname=postgres "
        "user=postgres "
        "password=postgres "
    );

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr,
                "DB connection failed: %s\n",
                PQerrorMessage(conn));

        PQfinish(conn);
        return NULL;
    }

    printf("DB connected\n");

    return conn;
}

PGresult *execute_query(char* query, PGconn* conn) {
    PGresult *res = PQexec(
        conn,
        query
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        // 500を返す
        printf("query failed\n");
        PQclear(res);
        db_disconnect(conn);
        return NULL;
    }

    printf("query executed\n");
    return res;
}

void db_disconnect(PGconn *conn)
{
    if (conn != NULL) {
        PQfinish(conn);
    }
}