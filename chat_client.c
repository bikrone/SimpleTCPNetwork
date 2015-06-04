/*
 * chat_client.c
 *
 *  Created on: May 28, 2015
 *      Author: khanh
 */

#include "library/socket_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define SIG_AIO SIGRTMIN+5
#define BUFF_SIZE 10

struct aiocb *cbs[2];
char *buffer[2];

int sock_fd;


void aio_handler(int signal, siginfo_t *info, void *uap) {
	//printf("im here\n");
	int cbIndex = info->si_value.sival_int;
	int data_length = aio_return(cbs[cbIndex]);;

	buffer[cbIndex][data_length] = 0;

	if (cbIndex == 0) {
		// read from input
		write(sock_fd, buffer[0], data_length);

		aio_read(cbs[0]);
	} else if (cbIndex == 1) {
		// read from socket
		if (fputs(buffer[1], stdout) == EOF) {
			err_sys("fputs error");
		}
		aio_read(cbs[1]);
	}
}

void init_aiocb(int sock_fd) {
	cbs[0] = calloc(1, sizeof (struct aiocb));
	cbs[1] = calloc(1, sizeof (struct aiocb));

	memset(cbs[0], 0, sizeof (struct aiocb));
	memset(cbs[1], 0, sizeof (struct aiocb));

	cbs[0]->aio_fildes = 0;
	buffer[0] = calloc(1, BUFF_SIZE);
	cbs[0]->aio_buf = buffer[0];
	cbs[0]->aio_nbytes = BUFF_SIZE;
	cbs[0]->aio_offset = 0;
	cbs[0]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cbs[0]->aio_sigevent.sigev_signo = SIG_AIO;
	cbs[0]->aio_sigevent.sigev_value.sival_int = 0;

	cbs[1]->aio_fildes = sock_fd;
	buffer[1] = calloc(1, BUFF_SIZE);
	cbs[1]->aio_buf = buffer[1];
	cbs[1]->aio_nbytes = BUFF_SIZE;
	cbs[1]->aio_offset = 0;
	cbs[1]->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	cbs[1]->aio_sigevent.sigev_signo = SIG_AIO;
	cbs[1]->aio_sigevent.sigev_value.sival_int = 1;

}

void setup_action() {
	struct sigaction action;
	action.sa_sigaction = aio_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIG_AIO, &action, NULL);
}

void str_cli_asynchronous(int socket_fd) {
	setup_action();
	init_aiocb(socket_fd);

	aio_read(cbs[0]);
	aio_read(cbs[1]);

	while (1) { sleep(1); }
}

void str_cli_non_block(int socket_fd) {
    char data_received[MAXLINE+1], data_send[MAXLINE+1];
    int data_received_length, data_send_length;

    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    fcntl(0, F_SETFL, O_NONBLOCK);

    for (;;) {
        if (fgets(data_send, MAXLINE, stdin) != NULL) {
            if (strcmp(data_send, "quit") == 0) exit(0);
            data_send_length = strlen(data_send);
            writen(socket_fd, data_send, data_send_length);
        }        

        data_received_length = read(socket_fd, data_received, MAXLINE);
        if (data_received_length> 0) {
            data_received[data_received_length] = 0; // mark end of string
            if (fputs(data_received, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
    }
        
}

void str_cli_multiplexing(int socket_fd) {
    // fd set
    fd_set master_read, readfds;
    int fd_max;

    // data received from server
    char data_received[MAXLINE+1], data_send[MAXLINE+1];
    int data_received_length, data_send_length;

    FD_ZERO(&master_read);
    FD_SET(0, &master_read);
    FD_SET(socket_fd, &master_read);
    fd_max = socket_fd;

    for (;;) {
        // copy master to temporary fd set
        readfds = master_read;

        if (select(fd_max+1, &readfds, NULL, NULL, NULL) == -1) {
            err_sys("client: select");
        }

        if (FD_ISSET(0, &readfds)) {
            if (fgets(data_send, MAXLINE, stdin) != NULL) {
            if (strcmp(data_send, "quit") == 0) exit(0);
                data_send_length = strlen(data_send);
                writen(socket_fd, data_send, data_send_length);
            }
        } else if (FD_ISSET(socket_fd, &readfds)) {
            data_received_length = read(socket_fd, data_received, MAXLINE);
            if (data_received_length> 0) {
            data_received[data_received_length] = 0; // mark end of string
                if (fputs(data_received, stdout) == EOF) {
                    err_sys("fputs error");
                }
            }
        }


    }

}

int main(int argc, char **argv) {
    // socket file descriptor
    int socket_fd;

    // server address
    struct sockaddr_in server_address;

    // server ip, default: localhost
    char *server_ip = "127.0.0.1";
    // server port, default: 1337
    char *server_port = "1337";

    if (argc > 1) {
    	server_ip = argv[1];
    	if (argc > 2) {
    	    server_port = argv[2];
    	}
    }

    // generate sockaddr_in server_address from server_ip
    bzero(&server_address, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(server_port));

    if (inet_pton(server_address.sin_family, server_ip, &server_address.sin_addr) <=0) {
    	err_quit("inet_pton error for %s\n", server_ip);
    }

    // create socket file descriptor
    socket_fd = Socket(server_address.sin_family, SOCK_STREAM, 0);
    sock_fd = socket_fd;
    if (socket_fd < 0) {
    	err_sys("socket error");
    }
    

    if (connect(socket_fd, (SA *)&server_address, sizeof server_address)< 0) {
    	err_sys("connect error");
    }
    str_cli_asynchronous(socket_fd);

    exit(0);
}
