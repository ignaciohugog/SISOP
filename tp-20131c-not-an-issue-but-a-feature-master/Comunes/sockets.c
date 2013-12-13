/*
 * sockets.c
 *
 *  Created on: Apr 28, 2013
 *      Author: utnso
 */

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "sockets.h"

int* socket_create(int local_port, t_log* logger) {
	int *newSocket;
	int optval;
	int *result = malloc(sizeof(int));
	newSocket = malloc(sizeof(int));
	if((*newSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_error(logger, "[socket_create] Fallo la creacion del socket!!!");
		exit(EXIT_FAILURE);
		return newSocket;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	optval = 1;
	if((*result = setsockopt(*newSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1)
	{
		log_error(logger, "[socket_create] Fallo setsockopt!!!");
		exit(EXIT_FAILURE);
		return result;
	}

	struct sockaddr_in local_address;
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = INADDR_ANY;
	local_address.sin_port = htons(local_port);
	memset(&(local_address.sin_zero), '\0', 8);
	if((*result = bind(*newSocket,(struct sockaddr*) &local_address, sizeof(local_address))) == -1){
		log_error(logger, "[socket_create] Fallo bind!!!");
		exit(EXIT_FAILURE);
		return result;
	}

	return newSocket;
}

int* socket_create_sin_puerto(t_log* logger){
	int *newSocket;
	struct sockaddr_in local_address;
	int optval;
	newSocket = malloc(sizeof(int));
	if((*newSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_error(logger, "[socket_create_sin_puerto] Fallo la creacion del socket!!!");
		exit(EXIT_FAILURE);
		return newSocket;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	optval = 1;
	int *result = malloc(sizeof(int));
	if((*result = setsockopt(*newSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) == -1) {
		log_error(logger, "[socket_create_sin_puerto] Fallo setsockopt!!!");
		exit(EXIT_FAILURE);
		return result;
	}

	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_address.sin_zero), '\0', 8);
	/*if((result = bind(*newSocket, (struct sockaddr*) &local_address, sizeof(local_address))) == -1){
		log_error(logger, "[socket_create_sin_puerto] Fallo bind!!!");
		exit(EXIT_FAILURE);
		return result;
	}*/

	return newSocket;
}

int socket_close(int *my_socket, t_log* logger) {
	int result = 0;
	if((result = close(*my_socket)) == -1){
		log_error(logger, "[socket_close] Fallo al cerrar socket!!!");
		exit(EXIT_FAILURE);
	}
	else {
		free(my_socket);
	}

	return result;
}

int socket_server_listen(int *socket_server, int max_connections_count, t_log* logger) {
	 int result = 0;
	 if((result = listen(*socket_server, max_connections_count)) == -1){
		 exit(EXIT_FAILURE);
		 log_error(logger, "[socket_server_listen] Fallo listen!!!");
	 }

	 return result;
}

int* socket_server_accept(int *socket_server, t_log* logger) {
	int *remote_client_socket;
	struct sockaddr_in *remote_address;
	int addrlen;
	remote_client_socket = malloc(sizeof(int));
	addrlen = sizeof(struct sockaddr_in);
	remote_address = malloc(sizeof(struct sockaddr_in));
	//tengo que poner remote addres o null?, addrlen o 0 ?
	if((*remote_client_socket = accept(*socket_server, (struct sockaddr *)remote_address, (void *) &addrlen)) == -1) {
		log_error(logger, "[socket_server_accept] Fallo accept!!!");
		exit(EXIT_FAILURE);
	}

	return remote_client_socket;
}

int socket_client_connect(int *socket_client, char *remote_ip, int remote_port, t_log* logger) {
	struct sockaddr_in remote_address;
	remote_address.sin_family = AF_INET;
	remote_address.sin_addr.s_addr = inet_addr(remote_ip);
	remote_address.sin_port = htons(remote_port);

	return connect(*socket_client, (struct sockaddr*) &remote_address, sizeof(remote_address));
}

int sendBytes(int *my_socket, void *data, int data_size, t_log* logger) {
	int enviados = 0;
	int total_enviados = 0;
	setsockopt(*my_socket, SOL_SOCKET, SO_SNDBUF, &data_size, sizeof(int));
	while(total_enviados < data_size) {
		enviados = send(*my_socket, data + total_enviados, data_size - total_enviados, 0);
		switch (enviados) {
			case 0:
				log_info(logger, "[sendBytes] Se completo el envio. [%d]", total_enviados);
				return 1;
			case -1:
				log_error(logger, "[sendBytes] Fallo sendBytes!!! result=[-1]");
				exit(EXIT_FAILURE);
			default:
				total_enviados += enviados;
				log_trace(logger, "[sendBytes] Send parcial. [%d/%d]", total_enviados, data_size);
				break;
		}
	}

	return 1;
}

void* receiveBytes(int *my_socket, int buffer_size, t_log* logger) {
	int total_recibidos = 0;
	int recibidos = 0;
	void *buffer = malloc(buffer_size);
	int i = 1;
	while(total_recibidos < buffer_size) {
		recibidos = recv(*my_socket, buffer + total_recibidos, buffer_size - total_recibidos, MSG_WAITALL);
		if(recibidos > 0) {
			total_recibidos += recibidos;
			//log_trace(logger, "[receiveBytes] Entre en RECIBIDOS [%d]",i);
			log_trace(logger, "[receiveBytes] Receive parcial. [%d/%d]", total_recibidos, buffer_size);
		}
		else if(recibidos == 0) {
			//log_trace(logger, "[receiveBytes] Entre en CERO [%d]",i);
			break;
		}
		else {
			log_error(logger, "[receiveBytes] Fallo receiveBytes!!! result=[-1]");
			exit(EXIT_FAILURE);
		}
		i++;
	}

	/*if(total_recibidos < 1) {
		free(buffer);
		return NULL;
	}*/

	return buffer;
}

int socket_connect_with_retry(int* socket, char* remote_ip, int port, t_log* logger) {
    int connect_result = -1;
    int current_retry = 0;
    while (current_retry < MAX_RETRY && connect_result < 0) {
        connect_result = socket_client_connect(socket, remote_ip, port, logger);
        current_retry++;
        if (connect_result < 0) {
            usleep(SLEEP_TIME);
            log_warning(logger, "[socket_connect_with_retry] Intentando reconectar [%d/%d]...", current_retry, MAX_RETRY);
        }
    }

    return connect_result;
}
