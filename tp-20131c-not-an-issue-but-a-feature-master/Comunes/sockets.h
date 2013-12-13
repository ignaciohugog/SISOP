/*
 * sockets.h
 *
 *  Created on: Apr 28, 2013
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_
#include "comunes/log.h"

#define SLEEP_TIME  100000 // 100ms
#define MAX_RETRY  100



/* En base a su ip y puerto crea el socket
 * hace que el SO libere el puerto inmediatamente luego de cerrar el socket
 * por ultimo bindea el socket y lo devuelve.
 */
int* socket_create(int local_port, t_log* logger);
int* socket_create_sin_puerto(t_log* logger);

int socket_close(int *my_socket, t_log* logger);

/*en base al socket del servidor y la cantidad maxima de conexiones hace el listen
 * Tendria que validar?? *
 */
int socket_server_listen(int *socket_server, int max_connections_count, t_log* logger);

/* En base al socket del server, realiza el accept generando el nuevo socket para ese cliente
 * devuelve el socket para el cliente generado
 */
int* socket_server_accept(int *socket_server, t_log* logger);

/* Completa los datos de la estructura sockaddr_in
 * Hace el connect
 */
int socket_client_connect(int *socket_client, char *remote_ip, int remote_port, t_log* logger);

/* Hace un send de bytes
 * no estoy seguro de los retornos
 */
int sendBytes(int *my_socket, void *data, int data_size, t_log* logger) ;

/* Recibe lo que tiene que recibir cargandolo en un buffer y luego retornandolo
 */
void* receiveBytes(int *my_socket, int buffer_size, t_log* logger);

int socket_connect_with_retry(int* socket, char* remote_ip, int port, t_log* logger);

#endif /* SOCKETS_H_ */
