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
	int id;
	struct t_info *next;
}   t_info;

fd_set curr_sock, cpy_read, cpy_write;
int user = 0;
char tmp[1000];

t_info *ft_add_server_sock(int fd)
{
	t_info *tab_socket;

	tab_socket = malloc(sizeof(t_info));
	tab_socket->next = NULL;
	tab_socket->id = user;
	tab_socket->socket = fd;
	return tab_socket;
}

void ft_add_back(t_info **tab_sock, t_info *new)
{
	t_info *temp = *tab_sock;

	if (*tab_sock == NULL)
		*tab_sock = new;
	else
	{
		while(temp->next)
			temp = temp->next;
		temp->next = new;
	}
	return ;
}

void send_all_come(t_info *tab, int fd)
{
	t_info *temp = tab;

	while(temp)
	{
		if (temp->socket != 0 && temp->socket != fd && temp->socket != 3)
		{
			send(temp->socket, tmp, 1000, 0);
			bzero(&tmp, sizeof(tmp));
		}
		temp = temp->next;
	}
	return ;
}

void	add_client(int sockfd, struct sockaddr_in cli, int len, t_info **tab_sock)
{
    int connfd;
    connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server accept the client...\n");
	FD_SET(connfd, &curr_sock);
	ft_add_back(tab_sock, ft_add_server_sock(connfd));
	sprintf(tmp, "server: client %d just arrived\n", user);
	send_all_come(*tab_sock, connfd);
	user++;
    return ;
}

void print_client(t_info *tab)
{
	t_info *temp = tab;

	while(temp)
	{
		printf("fd of client = %d\n", temp->socket);
		temp = temp->next;
	}
	return ;
}

void delete_fd(t_info **tab_sock, int fd)
{
	t_info *tmp = *tab_sock;

	while(tmp)
	{
		if (tmp->socket == fd)
		{
			tmp->socket = 0;
			break ;
		}
		tmp = tmp->next;
	}
	return ;
}

int get_id(int fd, t_info *tab)
{
    t_info *temp = tab;

    while (temp)
    {
        if (temp->socket == fd)
            return (temp->id);
        temp = temp->next;
    }
    return (-1);
}

void send_all(char msg[1000], t_info *tab, int fd)
{
	t_info *temp = tab;

	while(temp)
	{
		if (temp->socket != 0 && temp->socket != fd && temp->socket != 3)
		{
			sprintf(tmp, "client %d: %s", get_id(fd, tab), msg);
			printf("tmp = %s\n", tmp);
			send(temp->socket, tmp, 1000, 0);
			bzero(&tmp, sizeof(tmp));
		}
		temp = temp->next;
	}
	return ;
}

void send_all_left(char msg[1000], t_info *tab, int fd)
{
	t_info *temp = tab;

	while(temp)
	{
		if (temp->socket != 0 && temp->socket != fd && temp->socket != 3)
		{
			sprintf(tmp, "server: client %d just left\n", get_id(fd, tab));
			printf("tmp = %s\n", tmp);
			send(temp->socket, tmp, 1000, 0);
			bzero(&tmp, sizeof(tmp));
		}
		temp = temp->next;
	}
	return ;
}

int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 
	t_info *tab_socket;
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
	servaddr.sin_port = htons(10011); 
  
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
	tab_socket = ft_add_server_sock(sockfd);
    FD_ZERO(&curr_sock);
    FD_SET(sockfd, &curr_sock);
	char msg[1000];
	int rec = 0;
	len = sizeof(cli);
	bzero(&msg, sizeof(msg));
	bzero(&tmp, sizeof(tmp));
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
                    add_client(sockfd, cli, len, &tab_socket);
					print_client(tab_socket);
                }
				else
				{
					rec = recv(fd, msg, 1000, 0);
					if (rec == 0)
					{
						printf("user left \n");
						send_all_left(msg, tab_socket, fd);
						delete_fd(&tab_socket ,fd);
						FD_CLR(fd, &curr_sock);
						close(fd);
					}
					else
					{	
						printf("msg = %s", msg);
						send_all(msg, tab_socket, fd);
						bzero(&msg, sizeof(msg));
					}
				}
			}
		}
	}
}