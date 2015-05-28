/*
 * echo_server.c
 *
 *  Created on: May 28, 2015
 *      Author: khanh
 */

#include "library/socket_helper.h"
#include <stdio.h>
#include <stdlib.h>

void echo_to_socket(int fd, char *buff) {
    ssize_t n;
    while (1) {
	while ((n = read(fd, buff, sizeof buff)) > 0) {
	    buff[n] = '\0';
	    write(fd, buff, strlen(buff));
	    printf("%s", buff);
	    if (strcmp(buff, "quit") == 0) {
		exit(0);
	    }
	}

	if (n<0) {
	    if (errno == EINTR) continue;
	    err_sys("server: read error");
	}

	break;
    }

}

int main(int argc, char **argv) {
    char *port = "1337";
    int listening_fd, connecting_fd;
    struct sockaddr_in server_address, incoming_address;
    socklen_t incoming_address_length;
    char incoming_ip_address[INET6_ADDRSTRLEN];
    char buff[MAXLINE];
    pid_t child_pid;

    // options configuring
    if (argc > 1) {
	int i;
    	for (i=0; i< argc; i++) {
    	    // checking port argument
    	    if (strcmp(argv[i], "-p") == 0) {
    		if (++i >= argc) {
    		    err_quit("The open port is not specified");
    		} else {
    		    port = argv[i];
    		}
    	    }
    	}
    }

    // configuring server_address
    bzero(&server_address, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(port));

    // create listening socket file descriptor
    listening_fd = Socket(server_address.sin_family, SOCK_STREAM, 0);

    Bind(listening_fd, (SA *)&server_address, sizeof server_address);

    Listen(listening_fd, LISTENQ);
    printf("Successfully created server listening on port %s\n\n", port);

    incoming_address_length = sizeof incoming_address;

    // Receive connections
    for (;;) {
	connecting_fd = Accept(listening_fd, (SA *)&incoming_address, &incoming_address_length);
	inet_ntop(incoming_address.sin_family, &(incoming_address.sin_addr), incoming_ip_address, sizeof incoming_ip_address);
	printf("Got a connection from %s\r\n", incoming_ip_address);


	// create child process
	if ((child_pid = fork()) == 0) {
	    Close(listening_fd);
	    while (1) {
		echo_to_socket(connecting_fd, buff);

	    }
	}
	Close(connecting_fd);
    }
}
