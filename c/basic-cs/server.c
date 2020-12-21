#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write

void create_server();

int main(int argc , char *argv[]) {
    create_server();
    return 0;
}


void create_server() {
    int socket_desc;
    struct sockaddr_in server, client;
    // step 1: create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // step 2: init server sockaddr info
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(1234);

    // step 3: bind socket to sockaddr
    bind(socket_desc, (struct sockaddr *)&server, sizeof(server));

    // step 4: listen for connection
    listen(socket_desc, 5);

    int c = sizeof(struct sockaddr_in);

    // step 5: Process blocked. Waiting for client connect
    accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
}
