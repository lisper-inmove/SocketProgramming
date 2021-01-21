#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>	//inet_addr
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_CONNECTIONS 1024
#define MAXLINE 4096

int create_epfd(int *, struct epoll_event, int);
int create_listen_sockfd(int *, struct sockaddr_in *);
int run(int epfd, int fd, struct epoll_event events[]);
void do_something(int epfd, struct epoll_event ev);
void setnonblocking(int);
void new_connection(int, int);
void deal_with_epollin(int, struct epoll_event);
void deal_with_epollout(int, struct epoll_event);

int logfd;

int main(void) {
    int fd, epfd;
    int old_umask = umask(022);
    int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    logfd = open("test.log", O_RDWR | O_CREAT, perm);
    umask(old_umask);
    struct sockaddr_in listen_socket;
    struct epoll_event ev, events[MAX_CONNECTIONS];
    create_listen_sockfd(&fd, &listen_socket);
    create_epfd(&epfd, ev, fd);
    run(epfd, fd, events);
    return 0;
}

int run(int epfd, int fd, struct epoll_event events[]) {
    int n, i;
    struct epoll_event ev;
    while(1) {
        n = epoll_wait(epfd, events, MAX_CONNECTIONS, 1);
        if (n == -1) {
            perror("epoll wait error");
            continue;
        }
        for (i = 0; i < n; i++) {
            struct epoll_event event = events[i];
            int event_fd = event.data.fd;
            if (event_fd == fd) {
                new_connection(epfd, event_fd);
            } else {
                do_something(epfd, event);
            }
        }
    }
}

void do_something(int epfd, struct epoll_event event) {
    if (event.events & EPOLLIN) {
        deal_with_epollin(epfd, event);
    } else if (event.events & EPOLLOUT) {
        deal_with_epollout(epfd, event);
    } else if (event.events & EPOLLHUP) {
        printf("client closed");
    } else if (event.events & EPOLLERR) {
        printf("client EPOLLERR");
    }
}

void deal_with_epollin(int epfd, struct epoll_event ev) {
    while (1) {
        char buf[MAXLINE];
        size_t n = read(ev.data.fd, buf, MAXLINE);
        if (n == -1) {
            if (errno != EAGAIN) {
                perror("read error");
                close(ev.data.fd);
            }
            break;
        } else if (n == 0){
            break;
        } else {
            int x = write(logfd, buf, n);
            perror("write to log file error");
        }
    }
    ev.events = EPOLLOUT | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev);
}

void deal_with_epollout(int epfd, struct epoll_event ev) {
    // char buf[MAXLINE] = "hello random";
    // int n = sizeof(buf) / sizeof(char);
    // write(ev.data.fd, buf, n);
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_MOD, ev.data.fd, &ev);
}

void new_connection(int epfd, int listenfd) {
    struct epoll_event ev;
    struct sockaddr_in clientaddr;
    int socklen = sizeof(clientaddr);
    int connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &socklen);
    if(connfd < 0) {
        perror("connfd < 0");
        exit(1);
    }

    setnonblocking(connfd);

    char *str = inet_ntoa(clientaddr.sin_addr);

    ev.data.fd = connfd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
}

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
    server->sin_port = htons(9002);
    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
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
