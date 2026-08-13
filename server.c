#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "DBConnection.h"
#include "MakeJson.h"

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

    // 再起動直後でも同じポートを再利用できるようにする */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                    &opt, sizeof(opt)) < 0) {
        perror("ERROR setting SO_REUSEADDR");
        close(sockfd);
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
    while (1) {
        clilen = sizeof(cli_addr);
        new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (new_sockfd < 0) {
            perror("ERROR on accept");
            continue;
        }

        // データを受信する
        memset(buffer, 0, sizeof(buffer));
        n = recv(new_sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (n < 0) {
                perror("ERROR reading from socket");
            }
            close(new_sockfd);
            continue;
        } else {
            buffer[n] = '\0';
        }

        char method[16];
        char path[256];

        sscanf(buffer, "%15s %255s", method, path);
        printf("recv returned: %d bytes\n", n);
        printf("Message from client: %s\n", buffer);

        // ルーティング
        if (strcmp(method, "GET") == 0 &&
        strcmp(path, "/api/tasks") == 0) {
            PGconn *conn = db_connect();
            if (conn == NULL) {
                printf("cannot connect");
                close(new_sockfd);
                continue;
            }
            
            PGresult *res = execute_query("SELECT * from task", conn);
            if (res == NULL) {
                close(new_sockfd);
                continue;
            }

            char json[16384];
            int result = pgresult_to_json(res, json);
            if (result != 0) {
                fprintf(stderr, "ERROR: JSON response is too large\n");
                PQclear(res);
                db_disconnect(conn);
                close(new_sockfd);
                continue;
            }

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

            n = send(new_sockfd, response, strlen(response), 0);
            if (n < 0) {
                perror("ERROR writing to socket");
            }

            PQclear(res);
            db_disconnect(conn);
        }

        close(new_sockfd);
    }

    close(sockfd);

    return 0;
}
