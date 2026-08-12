#include <stdio.h>
#include <string.h>
#include "MakeJson.h"

int pgresult_to_json(PGresult *res, char *json) {
    if (res == NULL || json == NULL) {
        return -1;
    }

    strcpy(json, "[");

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < rows; i++) {
        char item[4096];
        strcpy(item, "{");

        for (int j = 0; j < cols; j++) {
            char *column_name = PQfname(res, j);
            char *value = PQgetvalue(res, i, j);

            char field[1024];

            snprintf(
                field,
                sizeof(field),
                "\"%s\":\"%s\"",
                column_name,
                value
            );

            strcat(item, field);

            if (j < cols - 1) {
                strcat(item, ",");
            }
        }

        strcat(item, "}");

        strcat(json, item);

        if (i < rows - 1) {
            strcat(json, ",");
        }
    }

    strcat(json, "]");

    return 0;
}