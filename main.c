#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct t_info
{
    int socket;
}   t_info;

fd_set curr_sock, cpy_read, cpy_write;
int user = 0;

void add_client(int sockfd, struct sockaddr_in cli, int len)
{
    int connfd;
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server accept the client...\n");
	user++;
	FD_SET(connfd, &curr_sock);
    return;
}

int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 
	t_info tab_socket;
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(10010); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}
    FD_ZERO(&curr_sock);
    FD_SET(sockfd, &curr_sock);
	char msg[1000];
	int rec = 0;
	len = sizeof(cli);
	rec = recv(3, msg, 1000, 0);
    while(1)
    {
		cpy_read = curr_sock;
        if (select(1024, &cpy_read, NULL, NULL, NULL) < 0)
            continue;
        for (int fd = 0; fd <= sockfd + user; fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {
                if (fd == sockfd)
                {
                    add_client(sockfd, cli, len);
                }
				else
				{
					rec = recv(fd, msg, 1000, 0);
					printf("msg = %s\n", msg);
				}
			}
		}
	}
}