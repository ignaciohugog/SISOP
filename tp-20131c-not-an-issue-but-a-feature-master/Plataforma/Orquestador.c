/*
 * Orquestador.c
 *
 *  Created on: 28/04/2013
 *      Author: martinbastero
 */
#include <stdio.h>
#include <comunes/collections/list.h>
#include "planificador.h"
#include "plataforma.h"
#include <stdlib.h>
#include <string.h>
#include "Orquestador.h"
#include <sockets.h>
#include <pthread.h>
#include <Serializadores/Stream.h>
#include <Serializadores/serializadores.h>
#include <comunes/log.h>
#include "comunes_plataforma.h"
#include <unistd.h>
#include "funciones.h"

#define TRUE 1;
#define FALSE 0;
#define PATH_LOG "log_orquestador"

t_log* logger;


int orquestador_main() {
    pthread_t threadPlanificador;
    int port = 8000;
    char ip[16];
    int *socket;
    //int bufferSize = 1024;
    int *socketNuevaConexion;
    int puertoPlanificador = 6000;
    t_list * listaNivelesConectados = NULL;
    t_nivel_deserializado * nivelCola = NULL;
    t_header * header = NULL;
    int puertoPlanificadorNivel;
    int * nivelGlobal;
    int arg[2];
    int posicionNivel;
    int posicionRecurso;
    int cantRecurso;
    int k;
    int size = 0;

    int contPersonaje;
    int cantPersonajesJugando;
    char personaje_en_lista = ' ';

    int contador = 0;
   	char perMat;

    strcpy(ip, "127.0.0.1");

    logger = log_create(PATH_LOG, "Orquestador", true, LOG_LEVEL_INFO);
    listaNivelesConectados = list_create();

    //creo el socket
    socket = socket_create(port, logger);

    //pongo el socket a escuchar
    socket_server_listen(socket, 100, logger);
    log_info(logger, "Se ha creado el socket %d escucha en el puerto %d", *socket, port);

    personajes_jugando = list_create();
    while (1) {
        //pongo el socket a aceptar  conexiones
    	log_trace(logger, "Esperando socketNuevaConexion...");
        socketNuevaConexion = socket_server_accept(socket, logger);
        if (socketNuevaConexion > 0) {
            log_info(logger, "Nueva conexion! Receiving bytes...");
            char* data = receiveBytes(socketNuevaConexion, sizeof(int) * 2, logger);
            if (data != NULL ) {
            	log_debug(logger, " [MAIN] data != NULL....");
            	header = NULL;
                header = (t_header*)deserializar_header(data);
                log_debug(logger, " [MAIN] deserializar_header....");
                // Variables creadas fuera del case porque sino no compila.
                char* buff = NULL;
                personajes *personaje = NULL;
                log_debug(logger, " [MAIN] HEADER TIPO=[%d],Tamanio=[%d]....",header->tipo,header->tamanio);
                switch (header->tipo) {

                case NIVEL:{
                    //Se conecto un nivel;
                    data = NULL;
                    data = receiveBytes(socketNuevaConexion, header->tamanio, logger);

                    t_nivel_deserializado*nivel_Deserializado = nivel_deserialize(data);
                    log_info(logger, "Se conectó el nivel %d IP: %s Puerto %d",
                            nivel_Deserializado->id_nivel,
                            nivel_Deserializado->ip,
                            nivel_Deserializado->puerto);
                    fflush(stdout);

                    //Aca valido si es un nivel nuevo
                    if (validaNivel(nivel_Deserializado->id_nivel, listaNivelesConectados) == 0) {
                        //El nivel es un nivel nuevo. Le asigno un puerto de escucha y lo guardo en la lista de niveles conectados.
                        puertoPlanificador++;
                        nivel_Deserializado->puertoPlanificador = puertoPlanificador;

                        list_add(listaNivelesConectados, nivel_Deserializado);
                        //Guardo el id de nivel y el puerto asignado a los parametros que recibe el hilo planificador
                        arg[1] = puertoPlanificador;
                        arg[0] = nivel_Deserializado->id_nivel;


                        nivelGlobal = malloc(sizeof(int));
                        *nivelGlobal = nivel_Deserializado->id_nivel;
                        list_add(nivelesGlobales, nivelGlobal);

                        //Muestro el nivel que guarde en la lista
                        nivelCola = list_get(listaNivelesConectados, 0);

                        //Generacion del hilo planificador
                        if (pthread_create(&threadPlanificador, NULL, plaificador_main, (int *) arg)) {
                            log_error(logger, "Error al crear el hilo Planificador");
                            return 1;
                        }
                    }
                    else {
                        log_error(logger, "Se intentó conectar el nivel: %d que ya existe en el sistema", nivel_Deserializado->id_nivel);
                        fflush(stdout);
                    }
                    break;
                }

                case PERSONAJE:{
                    //recibo el pedido de un personaje queriendose conectar a un nivel
                    data = NULL;
                    data = receiveBytes(socketNuevaConexion, header->tamanio, logger);

                    //deserializo el paquete
                    personaje = personaje_deserialize(data);

                    //busco el puerto del planificador del nivel donde se quiere conectar
                    puertoPlanificadorNivel = puertoNivel(personaje->nivel_actual, listaNivelesConectados);

                    //hago un control por las dudas de que el nivel no exista o haya fallado la fx
                    if (puertoPlanificadorNivel > 0) {
                        //aca le envio al personaje el puerto del planificador al que se tiene que conectar
                        log_info(logger, "El personaje %c pidió nivel %d",
                                personaje->id_personaje,
                                personaje->nivel_actual);
                        fflush(stdout);

                        //cargo la estructura info Nivel
                        if (funcionInfoNivel(personaje->nivel_actual, listaNivelesConectados, socketNuevaConexion, logger) == 0) {
                            log_error(logger, "Fallo la funcion cargar información nivel");
                        }

                        //Agrego el personaje a la lista de personajes que se estan procesando
                    	pthread_mutex_lock(&mutex_personajes_jugando);
                        if (list_is_empty(personajes_jugando)) {
                            cantPersonajesJugando = 0;
                        }
                        else {
                            cantPersonajesJugando = list_size(personajes_jugando);
                        }
                        personaje_en_lista =FALSE;
                        t_personaje_jugando* per_jugando = NULL;
                        for (contPersonaje = 0; contPersonaje < cantPersonajesJugando; contPersonaje++) {
                            per_jugando = list_get(personajes_jugando, contPersonaje);
                            if (personaje->id_personaje == per_jugando->personaje) {
                                personaje_en_lista = TRUE;
                            }
                        }

                        if (!personaje_en_lista) {
                        	t_personaje_jugando* personaje_jugando = malloc(sizeof(t_personaje_jugando));
                        	personaje_jugando->personaje = personaje->id_personaje;
                        	personaje_jugando->finPlanNiveles = FALSE;

                            list_add(personajes_jugando, personaje_jugando);
                        }
                    	pthread_mutex_unlock(&mutex_personajes_jugando);

                        //agrego personaje a la lista "listaRecovery"
                    	pthread_mutex_lock(&mutex_lista_recovery);
                        list_add(listaRecovery,personaje->id_personaje);
                        pthread_mutex_unlock(&mutex_lista_recovery);
                    }
                    else {
                        log_error(logger, "El personaje %c  se quiso conectar a un nivel inexistente", personaje->id_personaje);
                        fflush(stdout);
                    }
                break;
                }

                case ABANDONO_PJ:{
                	data=NULL;
					log_debug(logger, " Procesando [ABANDONO_PJ]....");
					char personaje[2];
					log_debug(logger, " [ABANDONO_PJ] malloquie personaje....");
					data = receiveBytes(socketNuevaConexion,sizeof(char),logger);
					//data = receiveBytes(socketNuevaConexion,sizeof(char),logger);
					log_debug(logger, " [ABANDONO_PJ] recibi algo....");
					//printf("%p\n", data);

					personaje[0] = data[0];
					log_debug(logger, "[ABANDONO_PJ] Recibi al personaje=[%c]",*data);
					personaje[1] = '\0';

					perMat=mataPersonaje(personaje, logger);

					pthread_mutex_lock(&mutex_lista_bloqueados);
					log_debug(logger, "[ABANDONO_PJ] Orquestador va a ejecutar saca bloq de deadlock...");
					sacaBloqDeDeadLock(perMat, logger);
					log_debug(logger, "[ABANDONO_PJ] Orquestador saco personaje de deadlock...");
					pthread_mutex_unlock(&mutex_lista_bloqueados);
                	break;
                }

                case INTERBLOQUEADOS:{
                	data=NULL;
                	log_info(logger, "[INTERBLOQUEADOS] Orquestador inicio interbloqueados");
                	data= receiveBytes(socketNuevaConexion, header->tamanio, logger);
                	log_info(logger, "[INTERBLOQUEADOS] recibi lista de personajes interbloqueados");
                    perMat=mataPersonaje(data, logger);
                    char*perMat2=malloc(sizeof(char));
                    *perMat2=perMat;

                    log_info(logger, "[INTERBLOQUEADOS] Orquestador envio personaje matado");
                    log_info(logger, "[INTERBLOQUEADOS] sending bytes perMat2...");
                    if(sendBytes(socketNuevaConexion,perMat2,sizeof(char), logger)==-1){
                        log_error(logger,"No realiza send");
                    }

                    pthread_mutex_lock(&mutex_lista_bloqueados);
                    log_info(logger, "[INTERBLOQUEADOS] Orquestador va a ejecutar saca bloq de deadlock...");
                    sacaBloqDeDeadLock(perMat, logger);
                    log_info(logger, "[INTERBLOQUEADOS] Orquestador saco personaje de deadlock...");
                    pthread_mutex_unlock(&mutex_lista_bloqueados);
                    free(perMat2);

                break;
                }

                case FIN_PLAN_NIVELES:{

                    procesar_fin_plan_niveles(socketNuevaConexion, header,logger);
                    break;
                }

                case RECURSOS_LIBERADOS:{
					log_info(logger, "[RECURSOS_LIBERADOS] recibiendo lista de recursos liberados...");
					//Recibo la lista de recursos liberados
					buff = receiveBytes(socketNuevaConexion, header->tamanio, logger);

					size = 0;
					k = 0;
					log_info(logger, "[RECURSOS_LIBERADOS] deserializando lista de recursos liberados...");
					t_recursos* info_recursos = lista_liberados_deserialize(buff, &size);

					log_info(logger, "cantidad de elementos [%d]", info_recursos->recurso_cantidad->elements_count);

					contador = 0;
					char* liberados = malloc(sizeof(char));
					liberados[0] ='\0';

					pthread_mutex_lock(&mutex_lista_bloqueados);
					//lo de abajo capaz puede ir arriba del mutex
					posicionNivel = posNivel(info_recursos->id_nivel);
					log_info(logger, "id nivel Finalizado: [%d]", info_recursos->id_nivel);

					while (k < info_recursos->recurso_cantidad->elements_count) {
						t_recursos_cantidad* info = list_get(info_recursos->recurso_cantidad, k);
						posicionRecurso = posRecurso(info->id_recurso);
						cantRecurso = info->cantidad;
						log_info(logger, "id_recursoLiberado=[%c], cantidadLiberada=[%d]",info->id_recurso,cantRecurso);
						k++;
						marcaPersonajesDesbloqueados(posicionNivel, posicionRecurso, cantRecurso, &liberados, &contador, logger);
					}
					contador++;
					liberados = realloc(liberados,sizeof(char)*contador);
					liberados[contador-1] = '\0';

					log_info(logger, "[RECURSOS_LIBERADOS] Personajes desbloqueados=[%d]...", contador);
					if (contador == 0) {
						log_trace(logger, "[RECURSOS_LIBERADOS] Serializando header_no_libere...");
						t_stream* dataHeaderNoLibere = serializador_header_no_libere();
						log_info(logger, "[RECURSOS_LIBERADOS] Sending NO_LIBERE a Nivel...");
						sendBytes(socketNuevaConexion, dataHeaderNoLibere->data, dataHeaderNoLibere->size, logger);
						log_info(logger, "[RECURSOS_LIBERADOS] Send realizado de NO_LIBERE a Nivel...");
					}
					else {
						log_info(logger, "[RECURSOS_LIBERADOS] Serializando liberados...");
						t_stream * listaLib = serializar_liberados(liberados);
						log_info(logger, "[RECURSOS_LIBERADOS] Enviando lista liberados...");
						sendBytes(socketNuevaConexion, listaLib->data, listaLib->size, logger);
						log_info(logger, "[RECURSOS_LIBERADOS] Send realizado con lista liberados...");
					}

					pthread_mutex_unlock(&mutex_lista_bloqueados);
					log_trace(logger, "[RECURSOS_LIBERADOS] free(liberados)...");
					free(liberados);
					break;
				}

                default:{
                    log_error(logger, "Tipo de dato desconocido [%s]", header->tipo);
                    break;

                }
                }//mal tabeado el switch
            }
            else {
                log_warning(logger, "Se recibio data = NULL...");
            }

            log_trace(logger, "Cerrando socketNuevaConexion=[%d]", *socketNuevaConexion);
            socket_close(socketNuevaConexion, logger);
            log_trace(logger, "socketNuevaConexion cerrado");
        }
        else
        {
        	log_warning(logger, "Fallo server accept con socketNuevaConexion!");
        }
    }

    log_info(logger, "Cerrando conexion del socket: %d", socket);
    socket_close(socket, logger);
    return 1;
}
