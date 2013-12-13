/*
 * main.c
 *
 *  Created on: 22/04/2013
 *      Author: utnso
 */

//export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/Descargas/repo/tp-20131c-not-an-issue-but-a-feature/Comunes/Debug
#include <stdio.h>
#include <stdlib.h>
#include <comunes/log.h>
#include <sockets.h>
#include <unistd.h>
#include "nivel.h"
#include "tad_items.h"
#include <sys/poll.h>
#include <pthread.h>

#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char* data;
t_log* logger;

int main(int argc, char **argv) {

    create_log();

    char* path;
    if (argc == 2) {
        path = argv[1];
    }

    pthread_t threadInter;
    t_nivel* nivel = configuracion_inicializar(path);
    logger = get_logger();

    //me conecto al orquestador y le mando mi data
    conectar_orquestador_y_envio(nivel);

    //creo el socket con el PUERTO que pase a orquestador para la escucha
    log_trace(logger, "[Main] Creando socket con puerto ...");
    int*socket_server = socket_create(nivel->puerto_nivel, logger);
    log_trace(logger, "[Main] Socket con puerto creado ...");

    //inicia gui
    iniciar_gui(nivel);
//gestiona conexiones utilizando poll
    struct pollfd lectores[BACKLOG + 1];
//	inicia escucha
    socket_server_listen(socket_server, BACKLOG, logger);
//inicializar personajes
    array_inicializar(lectores, BACKLOG);
//agrego socket server para atenderlo
    lectores[0].fd = *socket_server;
    int return_poll = 0;
    int indice = 0;
//creo hilo interbloqueo

    pthread_create(&threadInter, NULL, algoritmoDeadLock, nivel);

	while (1) {

		pthread_mutex_lock(&mutex);

		return_poll = poll(lectores, BACKLOG,TIMEOUT );
		if (return_poll == -1){
			log_warning(get_logger(), "[Main] Problema con Poll...");
		}else {
			if (lectores[0].revents == POLLIN) {  //server!
				indice = aceptar_conexion(&lectores[1], &lectores[0].fd,BACKLOG);
				indice++;
				data = receiveBytes(&(lectores[indice].fd), sizeof(char), logger);
				if(data!=NULL){
					CrearPersonaje(&nivel->lista_items, data[0], 1, 1);
					nivel_gui_dibujar(nivel->lista_items);
					cargarPersonajeAEstructuraDeadLock(data[0],nivel->personajes_deadlock,nivel->lista_items);
				}

			} else {
				int i;
				for (i = 1; i < (BACKLOG + 1); i++) {
					if (lectores[i].revents == POLLIN) {
						data = NULL;
						data = receiveBytes(&(lectores[i].fd), (sizeof(int))*2, logger);

						if (data != NULL ) {

							t_header* header=(t_header*)deserializar_header(data);
                              data = NULL;
                              data = receiveBytes(&(lectores[i].fd),header->tamanio, logger);

							switch (header->tipo){
								case BUSCAR_RECURSO:{
									t_posicion pos= buscar_recurso(data[0],nivel->lista_items);
									sendBytes(&lectores[i].fd,&pos,sizeof(t_posicion), logger);
									break;
								}
								case NIVEL_MOVE:{
									t_movimiento* mov =deserealizar_pedido_nivel_move(data);
									data = NULL;
									mover_personaje(mov->id_personaje, nivel->lista_items, mov->x, mov->y);
									
									break;
								}
								case SOLICITAR_RECURSO:{
									t_solicitud* solicitud = deserealizar_pedido_solicitud_recurso(data);
									solicitar_recurso(solicitud,nivel->lista_items,&(lectores[i].fd),&nivel->personajes_deadlock);
				
									break;
								}
								case ABANDONO_PJ:{
									baja_de_personaje(data[0],nivel,&(nivel->lista_items));
									//elimino socket asociado para el poll
									//socket_close(lectores[i].fd);
									lectores[i].fd=0;
									indice--;
                  
                                     break;
								}
								default: {
								    log_trace(get_logger(), "[DEFAULT] Tipo de mensaje indefinido [%d]...", header->tipo);
								    break;
								}
							}
							free(data);
							free(header);

						}
						lectores[i].revents = 0;
					}
				}
			}
		}
		pthread_mutex_unlock(&mutex);
		nivel_gui_dibujar(nivel->lista_items);
	}

	destruir_nivel(nivel);
	return 0;
}
