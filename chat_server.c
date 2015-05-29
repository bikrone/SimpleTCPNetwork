/*
 * chat_server.c
 *
 *  Created on: May 29, 2015
 *      Author: khanh
 */

#include "library/socket_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void broadcast_message(char *message, int message_length, int fd_max, fd_set* fds, int itself_fd, int listener) {
    message[message_length] = 0;
    int i;
    for (i=0; i<=fd_max; i++) {
	if (FD_ISSET(i, fds)) {
	    if (i==listener || i == itself_fd) continue;
	    if (writen(i, message, message_length) == -1) {
		perror("server: written");
	    }
	}
    }
}

int main(int argc, char **argv) {
    char *port = "1337";
    struct sockaddr_in server_address, client_address;
    int server_fd, client_fd, fd_max;
    socklen_t client_address_length;
    char client_ip_address[INET6_ADDRSTRLEN];
    char client_message[MAXLINE+1], server_message[MAXLINE+1];
    char *str_tmp;
    int i;

    // file descriptor set
    fd_set master; // master fdset
    fd_set readfds; // temporary fdset used in select functions

    if (argc > 1) {
	port = argv[1];
    }

    // generate server_address
    bzero(&server_address, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(port));

    // create server socket
    server_fd = socket(server_address.sin_family, SOCK_STREAM, 0);

    // initially, maximum fd is the server_fd
    fd_max = server_fd;

    // set the server_fd and stdin into master fd set
    FD_SET(server_fd, &master);
    FD_SET(0, &master);

    Bind(server_fd, (SA *)&server_address, sizeof server_address);

    Listen(server_fd, LISTENQ);

    printf("Server's starting listen on port %s\n\n", port);

    for (;;) {
	// copy master to temporary fd set
	readfds = master;

	if (select(fd_max+1, &readfds, NULL, NULL, NULL) == -1) {
	    err_sys("server: select");
	}

	if (FD_ISSET(server_fd, &readfds)) {
	    // new connection's coming
	    client_address_length = sizeof client_address;
	    bzero(&client_address, client_address_length);

	    // get client fd
	    client_fd = Accept(server_fd, (SA *)&client_address, &client_address_length);

	    if (client_fd == -1) {
		perror("server: accept");
	    } else {
		// add client_fd into master fd set
	    	FD_SET(client_fd, &master);

	    	// update fd_max if needed
	    	if (client_fd > fd_max) {
	    	    fd_max = client_fd;
	    	}

	    	// get the client_ip_address
	    	inet_ntop(client_address.sin_family, &(client_address.sin_addr), client_ip_address, sizeof client_ip_address);
	    	printf("*** We've got a new connection from %s ***\n", client_ip_address);
	    }
	}

	for (i=0; i<=fd_max; i++) {
	    // check active fds
	    if (FD_ISSET(i, &readfds)) {
		if (i == server_fd) {
		    continue;
		} else if (i == 0) {
		    // get input from stdin
		    if (fgets(server_message, MAXLINE, stdin) != NULL) {
			str_tmp = strdup(server_message);
			strcpy(server_message, "Server: ");
			strcat(server_message, str_tmp);
			free(str_tmp);
			broadcast_message(server_message, strlen(server_message), fd_max, &master, i, server_fd);
		    }
		} else {
		    // handle input from client
		    int n;
		    n = read(i, client_message, MAXLINE);
		    if (n<=0) {
			if (n == 0) {
			    // connection closed
			    printf("*** Connection from socket %d hung up ***\n", i);
			} else {
			    // connection error
			    perror("server: read");
			}

			close(i);
			FD_CLR(i, &master);
			continue;
		    }

		    client_message[n] = 0;

		    str_tmp = strdup(client_message);
		    strcpy(client_message, "");
		    snprintf(client_message, MAXLINE, "Socket %d: ", i);
		    strcat(client_message, str_tmp);
		    free(str_tmp);

		    broadcast_message(client_message, strlen(client_message), fd_max, &master, i, server_fd);
		}
	    }
	}
    }

}
