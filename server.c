#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "DBConnection.h"

#define PORT 3000

int main() {
    int sockfd, new_sockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // ソケットを作成する
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // ソケットにアドレスを割り当てる
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // クライアントからの接続を待つ
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (new_sockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    // データを受信する
    memset(buffer, 0, 256);
    n = recv(new_sockfd, buffer, 255, 0);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    char method[16];
    char path[256];

    sscanf(buffer, "%15s %255s", method, path);

    printf("Message from client: %s\n", buffer);

    // ルーティング
    if (strcmp(method, "GET") == 0 &&
    strcmp(path, "/api/tasks") == 0) {
        PGconn *conn = db_connect();
        if (conn == NULL) {
            printf("cannot connect");
            return 1;
        }

        PGresult *res = execute_query("SELECT * from task", conn);
        if (res == NULL) {
            return 1;
        }

        char json[8192];
        strcpy(json, "[");

        int rows = PQntuples(res);

        for (int i = 0; i < rows; i++) {
            char item[1024];

            char *title = PQgetvalue(res, i, 0);
            char *start_date = PQgetvalue(res, i, 1);
            char *due_date = PQgetvalue(res, i, 2);
            char *task_condition = PQgetvalue(res, i, 3);
            char *memo = PQgetvalue(res, i, 4);
            char *status = PQgetvalue(res, i, 5);

            snprintf(
                item,
                sizeof(item),
                "{\"title\":%s, \"start_date\":%s, \"due_date\":%s, \"task_condition\":%s, \"memo\":%s, \"status\":%s\"}",
                title,
                start_date, 
                due_date,
                task_condition,
                memo,
                status
            );

            strcat(json, item);

            if (i < rows - 1) {
                strcat(json, ",");
            }
        }

        strcat(json, "]");

        char response[16384];

        snprintf(
            response,
            sizeof(response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            strlen(json),
            json
        );

        printf("DB connected!\n");
        printf("response: %s", response);
        send(new_sockfd, response, strlen(response), 0);
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        db_disconnect(conn);
    }

    close(new_sockfd);
    close(sockfd);

    return 0;
}