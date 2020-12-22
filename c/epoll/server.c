#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include<arpa/inet.h>	//inet_addr


#define MAX_CONNECTIONS 1024

int create_epfd(int *, struct epoll_event, int);
int create_listen_sockfd(int *, struct sockaddr_in *);
int run(int epfd, int fd, struct epoll_event events[]);
void do_something();

int main(void) {
    int fd, epfd;
    struct sockaddr_in listen_socket;
    struct epoll_event ev, events[MAX_CONNECTIONS];
    create_listen_sockfd(&fd, &listen_socket);
    create_epfd(&epfd, ev, fd);
    run(epfd, fd, events);
    return 0;
}

int run(int epfd, int fd, struct epoll_event events[]) {
    int n, i;
    while(1) {
        n = epoll_wait(epfd, events, MAX_CONNECTIONS, 1);
        if (n == -1) {
            perror("epoll wait error");
            return -1;
        }
        for (i = 0; i < n; i++) {
            struct epoll_event event = events[i];
            int event_fd = event.data.fd;
            if (event_fd == fd) {
                printf("new connection\n");
            } else {
                do_something();
            }
        }
    }
}

void do_something() {}

int create_epfd(int *epfd, struct epoll_event ev, int listen_sock_fd) {
    *epfd = epoll_create1(0);
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock_fd;
    int ret = epoll_ctl(*epfd, EPOLL_CTL_ADD, listen_sock_fd, &ev);
    if (ret == -1) {
        perror("epoll ctl error");
        return -1;
    }
}

int create_listen_sockfd(int *fd, struct sockaddr_in *server) {
    int ret;
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(1234);
    ret = bind(*fd, (struct sockaddr *)server, sizeof(*server));
    if (ret == -1) {
        perror("bind socket error");
        return -1;
    }
    ret = listen(*fd, MAX_CONNECTIONS);
    if (ret == -1) {
        perror("listen socket error");
        return -1;
    }
}
