#ifndef MAKE_JSON_H
#define MAKE_JSON_H

#include <libpq-fe.h>

int pgresult_to_json(PGresult *res, char *json);

#endif