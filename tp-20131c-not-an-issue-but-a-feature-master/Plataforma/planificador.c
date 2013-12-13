/*
 * planificador.c
 *
 *  Created on: 21/04/2013
 *      Author: utnso
 */

#include <stdio.h>
#include <comunes/collections/list.h>
#include <comunes/log.h>
#include <Serializadores/serializadores.h>
#include "planificador.h"
#include "plataforma.h"
#include "comunes_plataforma.h"
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <Serializadores/Stream.h>
#include <sockets.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <comunes/config.h>
#include "funciones.h"

#define BACKLOG 50
#define PATH_LOG "log_planificador"
#define PATH_CONFIG "PLANIFICADOR.cfg"
#define BUF_LEN 256


void* plaificador_main(int * argv) {

	t_list *lista_personajes_ready = NULL;
	//int *pf_ready; //puntero final

	int tiempoEspera;

	int contador_q = 0; //contador de quantum
	//char personaje_mensaje[10];
	//agrego socket server para atenderlo
	t_log* logger = log_create(PATH_LOG, "Planificador", true, LOG_LEVEL_INFO);
	int* socket_server = socket_create(argv[1], logger);
	int return_poll = 0;
	int indice = 0;
	struct pollfd lectores[BACKLOG + 1];
	t_personaje_ready * personaje_ready;
	//t_personaje_ready  *personaje_ready2=NULL ;
	personaje_bloqueado * personajeBloq = NULL;

	char *data;
	char*movimiento = NULL;
	movimiento = malloc(sizeof(char));
	int hacerRR = 1;

	int idNivel = argv[0];
	char idRecurso = 0;

	int posNivelBuscado;
	int posRecursoBuscado;

	int posicionNivel;

	int k;

	t_config *configuracion_planificador = config_create(PATH_CONFIG);
	q = config_get_int_value(configuracion_planificador, "Quantum");
	tiempoEspera = config_get_int_value(configuracion_planificador, "Tiempo");


	pthread_mutex_lock(&mutex_log_planificador);
	log_info(logger, "Se inicia hilo planificador para nivel: %d", idNivel);
	pthread_mutex_unlock(&mutex_log_planificador);
	fflush(stdout);

	//meter en la lista de bloqueados

	lista_personajes_ready = list_create();

	//	inicia escucha
	socket_server_listen(socket_server, BACKLOG, logger);
	//inicializar personajes
	array_inicializar(lectores, BACKLOG);

	//agrego socket server para atenderlo
	lectores[0].fd = *socket_server;

	posicionNivel = posNivel(idNivel);

	while (1) {
		usleep(15000);
		/*por cada iteracion chequeo que no haya personajes en la cola de bloqueados
		 * que haya que pasar a la cola de ready
		 */
		pthread_mutex_lock(&mutex_lista_bloqueados);
		meteBloqEnColaRedy(posicionNivel, &lista_personajes_ready,idNivel, logger);
		pthread_mutex_unlock(&mutex_lista_bloqueados);

		return_poll = poll(lectores, BACKLOG, 100);
		//Abre el socket para escuchar (con poll)
		if(return_poll > 0){

			if (lectores[0].revents == POLLIN) {  //server!
				indice = aceptar_conexion(&lectores[1], &lectores[0].fd, BACKLOG, logger);
				indice++;
				log_info(logger, "Se recibió una nueva conexión");
			} else {
				int i;
				for (i = 1; i < (BACKLOG + 1); i++) {
					if (lectores[i].revents == POLLIN) {
						//log_trace(logger, "Esperando polling data...");
						data = receiveBytes(&lectores[i].fd, sizeof(char)*2, logger);
						//log_trace(logger, "Polling Data recibida...");
						//Se movio pero hay que hacer lo del serialize
						if (data != NULL ) {
							switch (data[0]) {
								case PERSONAJE_MOVIO: {
									log_trace(logger, "Procesando [PERSONAJE_MOVIO] int0=[%d] i %d", data[0], i);
									//Se movió
									contador_q++;
									hacerRR = 1;
									if(lista_personajes_ready->elements_count > 0){
										t_personaje_ready * personaje_ready3 = list_get(lista_personajes_ready, 0);
										log_info(logger,"Se otorgó turno al personaje %c",	personaje_ready3->id_personaje);
									}
									else {
										log_warning(logger,"lista_personajes_ready vacia en PERSONAJE_MOVIO.");
									}

									break;
								}

								case PERSONAJE_FIN_NIVEL: {
									//recibo el idPersonaje que termino el nivel
									log_info(logger,"Procesando [PERSONAJE_FIN_NIVEL] data[1]=[%c]", data[1]);
									char personajeFinNivel=data[1];
									int indicePerFinNivel=0;
									int indiceEliminar=-1;
									t_personaje_ready * personaje_ready3=NULL;

									//busco en que posicion de la lista esta

									for(indicePerFinNivel=0;indicePerFinNivel<lista_personajes_ready->elements_count;indicePerFinNivel++){
										 personaje_ready3 = list_get(lista_personajes_ready, indicePerFinNivel);
										if (personaje_ready3->id_personaje==personajeFinNivel){
											indiceEliminar=indicePerFinNivel;
											break;
										}
									}

									//lo saco de la lista de ready
									if (indiceEliminar!=-1){
										log_trace(logger,"[PERSONAJE_FIN_NIVEL] Removiendo al personaje [%c] de ready", personajeFinNivel);
										personaje_ready3=list_remove(lista_personajes_ready,indiceEliminar);
										free(personaje_ready3);
										log_trace(logger,"[PERSONAJE_FIN_NIVEL] Remove exitoso!!!");
									}
									else{
										log_debug(logger,"[PERSONAJE_FIN_NIVEL] No se encontro al personaje [%c] en Ready!!!!",personajeFinNivel);
									}

									lectores[i].fd = 0;
									hacerRR = 1;
									indice--;
										
								       // socket_close(&lectores[i].fd,logger);
									//log_info(logger,	"El personaje %c se desconectó del nivel %d",	personaje_ready3->id_personaje,idNivel);

									//Lo saco de la lista de recovery
									char perRec = -1;
									int posRec = -1;
									log_trace(logger,"[PERSONAJE_FIN_NIVEL] Buscando al personaje [%c] en Recovery", personajeFinNivel);
									pthread_mutex_lock(&mutex_lista_recovery);
									for (k = 0; k < listaRecovery->elements_count;k++) {
										perRec = list_get(listaRecovery, k);
										if (perRec== personajeFinNivel){
											posRec = k;
											log_trace(logger,"[PERSONAJE_FIN_NIVEL] Personaje [%c] encontrado en Recovery, subind=[%d]", personajeFinNivel,k);
										}

									}

									if (posRec != -1){
										log_trace(logger,"[PERSONAJE_FIN_NIVEL] Eliminando al [%c] de Recovery", personajeFinNivel);
										list_remove(listaRecovery, posRec);
										log_trace(logger,"[PERSONAJE_FIN_NIVEL] Se elimino exitosamente al [%c] de Recovery", personajeFinNivel);
									}

									pthread_mutex_unlock(&mutex_lista_recovery);

									imprimirListaReady(lista_personajes_ready,logger,idNivel);
									break;
								}

								case PERSONAJE_BLOQ: {
									log_info(logger, "Procesando [PERSONAJE_BLOQ]: Id Recurso=[%c]", data[1]);
                                /* si se bloqueo
                                 pasa a la lista de bloqueados:
                                 - mutex para agregar a la lista global
                                 - busqueda por nivel y recurso
                                 - agrega nodo a la lista*/

                                    idRecurso = data[1];

                                    //me guardo el personaje
                                    t_personaje_ready * personaje_ready2 = list_remove(lista_personajes_ready, 0);
                                    personajeBloq = malloc(sizeof(personaje_bloqueado));
                                    personajeBloq->bool_recurso_ok = 0;
                                    personajeBloq->id_personaje = personaje_ready2->id_personaje;
                                    personajeBloq->socket = personaje_ready2->socket;

                                    //busco la posicion del recurso
                                    //aca agrego a la lista de bloqueados del recurso y del nivel
                                    posNivelBuscado = posNivel(idNivel);

                                    posRecursoBuscado = posRecurso(idRecurso);
                                    log_info(logger, "Se bloqueó el personaje %c", personajeBloq->id_personaje);

                                    pthread_mutex_lock(&mutex_lista_bloqueados);
                                    //agrego a la lista de bloqueados correspondiente
                                    list_add((personajesBloqueadosGlobal[posNivelBuscado][posRecursoBuscado]), personajeBloq);
                                    pthread_mutex_unlock(&mutex_lista_bloqueados);
                                    pthread_mutex_lock(&mutex_lista_bloqueados);
                                    imprime_lista_bloqueados(idNivel, logger);
                                    pthread_mutex_unlock(&mutex_lista_bloqueados);

                                    imprimirListaReady(lista_personajes_ready, logger, idNivel);
                                    hacerRR = 1;
                                    free(personaje_ready2);
									break;
								}

								case NUEVO_PERSONAJE: {
									log_info(logger, "Procesando [NUEVO_PERSONAJE] int0=[%d] int1=[%d] i %d", data[0], data[1], i);
									//carga toda la lista de readym/cola de todos los que se conectaron
									personaje_ready = malloc(sizeof(t_personaje_ready));
									personaje_ready->id_personaje = data[1];
									personaje_ready->socket = lectores[i].fd;

									list_add(lista_personajes_ready,personaje_ready);

									log_info(logger, "Se conectó el personaje: %c",data[1]);
									hacerRR=1;
									imprimirListaReady(lista_personajes_ready,logger,idNivel);
									break;
								}
								case IM_READY: {
									log_info(logger, "Procesando [IM_READY] int0=[%d] int1=[%d] i %d", data[0], data[1], i);
									//personaje cambia a la cola de ready
									personaje_ready = malloc(sizeof(t_personaje_ready));
									personaje_ready->id_personaje = data[1];
									personaje_ready->socket = lectores[i].fd;
									list_add(lista_personajes_ready, personaje_ready);
									pthread_mutex_lock(&mutex_lista_bloqueados);
									imprime_lista_bloqueados(idNivel, logger);
									pthread_mutex_unlock(&mutex_lista_bloqueados);
									imprimirListaReady(lista_personajes_ready,logger,idNivel);
									hacerRR=1;
									log_info(logger, "Saliendo [IM_READY]...");
									break;
								}
								default: {
									if(data[0])
										log_trace(logger, "Procesando [CASE DEFAULT] int0=[%d] int1=[%d] i %d", data[0], data[1], i);
									break;
								}
							}

						    free(data);
						}

						lectores[i].revents = 0;
					}
				}
			}
		}
		else if (return_poll < 0) {
			log_error(logger, "Problema con poll");
		}

		//Round Robin
		//log_trace(logger, "Verificando RoundRobin=[%d]...", hacerRR);
		if (hacerRR) {
			//log_trace(logger, "Ejecutando RoundRobin...");
			if (list_is_empty(lista_personajes_ready)){
				//log_trace(logger, "[RR] No hay personajes en ready...");
			}
			else {
				//log_trace(logger, "[RR] Hay personajes en ready!!!");
				if (contador_q >= q) {
					t_personaje_ready * personaje_ready_terminado = list_remove(lista_personajes_ready, 0);
					log_trace(logger, "[RR] Quantum completo para personaje [%c]...", personaje_ready_terminado->id_personaje);
					list_add(lista_personajes_ready, personaje_ready_terminado); //saca del principio de la lista y coloca al final
					contador_q = 0;
				}

				t_personaje_ready * personaje_ready_a_mover = list_get(lista_personajes_ready, 0);
				*movimiento = PERSONAJE_MOVETE;
				usleep(tiempo_espera);
				log_trace(logger, "[RR] Sending PERSONAJE_MOVETE a personaje=[%c]...", personaje_ready_a_mover->id_personaje);
				if (sendBytes(&personaje_ready_a_mover->socket, movimiento, sizeof(char), logger) < 0) {
					log_error(logger, "[RR] Fallo con el envio");
				}

				log_trace(logger, "[RR] Send PERSONAJE_MOVETE realizado!");
				hacerRR = 0;
			}
		}
	}

	return NULL ;
}
