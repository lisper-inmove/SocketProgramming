#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

void setnonblocking(int);

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
    server.sin_port = htons(9002);

    // step 3: connect to server
    connect(socket_desc, (struct sockaddr *)&server, sizeof(server));

    // setnonblocking(socket_desc);
    int i = 0;
    while (1) {
        i++;
	if (i == 10) break;
        char buf[] = "hello world";
        int n = sizeof(buf) / sizeof(char);
        printf("%d\n", n);
        write(socket_desc, buf, n);

        // int ret = read(socket_desc, buf, 1024);
        // perror("read from server");
        // printf("read from server: %s\n", buf);
    }
    sleep(1);

    return 0;
}


void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);

    if(opts < 0) {
        perror("fcntl(sock, GETFL)");
        exit(1);
    }

    opts = opts | O_NONBLOCK;

    if(fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
}
