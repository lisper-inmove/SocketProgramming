#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int create_client();

int main(int argc , char *argv[]) {
    create_client();
    return 0;
}

int create_client() {
    struct sockaddr_in server;
    // step 1: create socket
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // step 2: server sockaddr info
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(1234);

    // step 3: connect to server
    connect(socket_desc, (struct sockaddr *)&server, sizeof(server));

    return 0;
}
