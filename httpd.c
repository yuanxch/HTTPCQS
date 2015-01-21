#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define HOST "112.124.10.140"
#define PORT 8888
#define MAXFDS 5000 
#define BACKLOG  32
#define EVSIZE 100
#define BUFFER "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\nContent-Type: text/html\r\n\r\nHello"

int epfd;

void setnonblocking(int fd);
void *serv_epoll(void *ptr);



int main(int argc, char *argv[])
{
	int fd, connfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t cliaddr_len;
	struct epoll_event ev;
	pthread_t tid;
	pthread_attr_t attr;

	epfd = epoll_create(MAXFDS);
	if(epfd < 0) {
		perror("epoll_create");
		return -1;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);		
	if(fd < 0) {
		perror("socket");
		return -1;
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	//serv_addr.sin_addr.s_addr = inet_addr(HOST);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	
	if(bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("bind");
		return -1;
	}

	if(listen(fd, BACKLOG) != 0) {
		perror("listen");
		return -1;
	}

	pthread_attr_init(&attr);	
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(pthread_create(&tid, &attr, serv_epoll, NULL) != 0) {
		perror("pthread_create");
		return -1;
	}


	while(connfd = accept(fd, (struct sockaddr *)&cli_addr, &cliaddr_len)) {
		setnonblocking(connfd);
		ev.data.fd = connfd;		
		ev.events = EPOLLIN | EPOLLET;
		epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
		//printf("Connfd:%d\tHost:%s\tPort:%d\n", connfd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port)); 
	}
		
	return 0;
}


//设置非阻塞
void setnonblocking(int fd)
{
	int opts;
	opts = fcntl(fd, F_GETFD);
	if(opts < 0) {
		perror("fcntl");
		return;
	}

	opts = opts | O_NONBLOCK;
	if(fcntl(fd, F_SETFD, opts) < 0) {
		perror("fcntl");
		return;
	}
	
	return;
}

//以epoll模型来读写
void *serv_epoll(void *ptr)
{
	int i, nfds, cfd, ret;
	struct epoll_event ev, events[EVSIZE];
	char buffer[512];

	for(;;) {

		nfds = epoll_wait(epfd, events, EVSIZE, -1);

		for(i=0; i<nfds; i++) {
			if(events[i].events & EPOLLIN) {
				cfd = events[i].data.fd;	
				ret = recv(cfd, buffer, sizeof(buffer), 0);
				ev.data.fd= cfd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, cfd, &ev);
			} else if(events[i].events & EPOLLOUT) {
				cfd = events[i].data.fd;	
				ret = send(cfd, BUFFER, sizeof(BUFFER), 0);
				ev.data.fd = cfd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);
				close(cfd);
			}
		}
	}
	
	return NULL;
}
