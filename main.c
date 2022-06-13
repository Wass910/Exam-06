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
char tmp[4096], buf[4096], to_send[4096 + 42];
int count = 0;

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
			send(temp->socket, tmp, 4096, 0);
		}
		temp = temp->next;
	}
	bzero(&tmp, sizeof(tmp));
	return ;
}

void	add_client(int sockfd, struct sockaddr_in cli, socklen_t len, t_info **tab_sock)
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

void send_all_left(t_info *tab, int fd)
{
	t_info *temp = tab;

	while(temp)
	{
		if (temp->socket != 0 && temp->socket != fd && temp->socket != 3)
		{
			sprintf(tmp, "server: client %d just left\n", get_id(fd, tab));
			printf("tmp = %s\n", tmp);
			send(temp->socket, tmp, 4096, 0);
			bzero(&tmp, sizeof(tmp));
		}
		temp = temp->next;
	}
	return ;
}

void send_all(char msg[4096], t_info *tab, int fd)
{
	t_info *temp = tab;

	while(temp)
	{
		if (temp->socket != 0 && temp->socket != fd && temp->socket != 3)
		{
			printf("tmp = %s\n", msg);
			send(temp->socket, msg, 4096, 0);
		}
		temp = temp->next;
	}
	return ;
}

void ex_msg(int fd, t_info *tab)
{
    int i = 0;
    int j = 0;

    while (buf[i])
    {
        tmp[j] = buf[i];
        j++;
        if (buf[i] == '\n')
        {
            sprintf(to_send, "client %d: %s", get_id(fd, tab), tmp);
            send_all(to_send, tab, fd);
            j = 0;
            bzero(&tmp, strlen(tmp));
			bzero(&to_send, strlen(to_send));
        }
        i++;
    }
    bzero(&buf, strlen(buf));
}
int main(int argc, char **argv) {
	argc++;
	int sockfd;
	socklen_t len;
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
	servaddr.sin_port = htons(atoi(argv[1])); 
  
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
	char msg[4096];
	len = sizeof(cli);
	bzero(&msg, sizeof(msg));
	bzero(&tmp, sizeof(tmp));
	bzero(&to_send, sizeof(to_send));
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
					int ret_recv = 1000;
					while (ret_recv == 1000 || buf[strlen(buf) - 1] != '\n')
					{
						ret_recv = recv(fd, buf + strlen(buf), 4096, 0);
						if (ret_recv <= 0)
							break ;
					}
					if (ret_recv == 0)
					{
						printf("user left \n");
						send_all_left( tab_socket, fd);
						delete_fd(&tab_socket ,fd);
						FD_CLR(fd, &curr_sock);
						bzero(&buf, sizeof(buf));
						bzero(&msg, sizeof(msg));
						close(fd);
					}
					else
					{	
						ex_msg(fd, tab_socket);
					}
					count = 0;
				}
			}
		}
	}
}