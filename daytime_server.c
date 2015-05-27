/*
 * daytime_server.c
 *
 *  Created on: May 27, 2015
 *      Author: khanh
 */

#include "library/socket_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    char *port = "1337";
    int listening_fd, connecting_fd;
    struct sockaddr_in server_address;
    struct sockaddr_storage incoming_address;
    socklen_t incoming_address_length;
    char incoming_ip_address[INET6_ADDRSTRLEN];
    char buff[MAXLINE];
    time_t ticks;

    // options configuring
    if (argc > 1) {
	int i;
	for (i=0; i< argc; i++) {
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
	inet_ntop(incoming_address.ss_family, Get_in_addr((SA *)&incoming_address), incoming_ip_address, sizeof incoming_ip_address);
	printf("Got a connection from %s\r\n", incoming_ip_address);
	ticks = time(NULL);
	snprintf(buff, sizeof buff, "%.24s\r\n", ctime(&ticks));
	Write(connecting_fd, buff, strlen(buff));

	Close(connecting_fd);
    }
}


