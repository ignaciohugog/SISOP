#include <stdio.h>
#include <stdlib.h>
#include <comunes/log.h>
#include <sockets.h>
#include "personaje.h"
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>

t_log* logger;

int main(int argc, char **argv) {
	create_log();
	char* path;
	if (argc == 2) {
		path = argv[1];
	}

	bool termino = false;
	bool finaliza_nivel = false;
	t_personaje* personaje = configuracion_inicializar(path);

	signal(SIGTERM, handler);
	signal(SIGUSR1, handler);
	signal(SIGINT, handler);
	t_log* logger = recibir_logger();
	personaje->nivel_actual = personaje->planDeNiveles;

	// pj sin plan de niveles-->pasa directo koopa
	t_info_nivel* info_nivel=conectarme_y_pedir_a_orquestador(personaje->nivel_actual);
	personaje_conecta(info_nivel);
	personaje->nodo_recurso_actual=personaje->nivel_actual ->objetivos->head;
	char envioAPlanificador[2];


	while (termino == false) {
	//	recibir turno del planificador
		char* data = receiveBytes(personaje->sk_planificador, sizeof(char), logger);
		if(data != NULL)
			//log_debug(logger, "{%c} [Main] recibi sizeof(char) del planificador...",personaje->id);

		if((data != NULL) && (data[0] == PERSONAJE_MOVETE)){
			log_info(logger, "{%c} [Main] Recibi turno del Planificador del %s...",personaje->id,personaje->nivel_actual->nivel);
			if ((personaje->nivel_actual != NULL)) {
				//calculo movimiento y nivel me mueve
				hacer_movimiento(personaje->sk_nivel, &personaje->nodo_recurso_actual);

				if (llega_a_destino(personaje->actual, personaje->destino)) {
					log_info(logger, "{%c} [Main] Llegue a destino...",personaje->id);
					// le solicita a nivel el recurso y devuelve si me lo otorgo o no, ene sta parte el pj se bloquea-->esta bien!
					char* respuesta_a_solicitud = solicitar_recurso_a_nivel();

					switch (respuesta_a_solicitud[0]) {

						case RECURSO_OTORGADO: {
							//avanzo al proximo recurso o nivel
							char* recurso = (char*) (personaje->nodo_recurso_actual->data);
							log_info(logger, "{%c} [Main] Recibi el recurso: %c ...",personaje->id,*recurso);
							finaliza_nivel = false;
							termino = avanzar_proximo_objetivo(&finaliza_nivel);
							if(!finaliza_nivel){
								//notificar al planificador PERSONAJE_MOVIO
								envioAPlanificador[0] = PERSONAJE_MOVIO;
								log_info(logger, "{%c} [Main] Sending PERSONAJE_MOVIO al Planificador...",personaje->id);
								sendBytes(personaje->sk_planificador,&envioAPlanificador,sizeof(char)*2, logger);
								log_info(logger, "{%c} [Main] Send PERSONAJE_MOVIO al Planificador del %s...",personaje->id,personaje->nivel_actual->nivel);
							}
							break;
						}
						case NO_HAY_RECURSOS: {
							////notificar al planificador PERSONAJE_BLOQUEADO y ID de recurso
							personaje->bloqueado = 1;
							envioAPlanificador[0] = PERSONAJE_BLOQ;
							char* id_rec = (char*) (personaje->nodo_recurso_actual->data);
							envioAPlanificador[1] = *id_rec;
							log_info(logger, "{%c} [Main] No hay recurso: %c, ME ESTOY BLOQUEANDO...",personaje->id,envioAPlanificador[1]);
							log_info(logger, "{%c} [Main] Sending PERSONAJE_BLOQ al Planificador...",personaje->id);
							sendBytes(personaje->sk_planificador,&envioAPlanificador,sizeof(char)*2, logger);
							log_info(logger, "{%c} [Main] Send realizado con PERSONAJE_BLOQ al Planificador...",personaje->id);
							//tengo que hacer un recibe aca asi me bloqueo hasta no aver conseguido el recurso

//							//pollcito
//							struct pollfd pollcito;
//							  pollcito.fd=*personaje->sk_planificador;
//							  pollcito.events=POLLIN;
//							  pollcito.revents=0;
//                                bool boolean=false;
//							while (boolean==false) {
//                             poll(&pollcito, 2,50);//timeout 50
//                             if (pollcito.revents == POLLIN) {
//                                  boolean=true;
							char* data = receiveBytes(personaje->sk_planificador, sizeof(char), logger);
							if(data != NULL){
								switch(data[0]){
									case PERSONAJE_DESBLOQUEATE:{
										log_info(logger, "{%c} [Main] Recibi PERSONAJE_DESBLOQUEATE del Planificador...",personaje->id);
										personaje->bloqueado = 0;
										finaliza_nivel = false;
										termino = avanzar_proximo_objetivo(&finaliza_nivel);
										if(!finaliza_nivel) {
											char envioAPlataforma[2];
											envioAPlataforma[0] = IM_READY;
											envioAPlataforma[1] = personaje->id;
											log_info(logger,"{%c}[Main]: Sending IM_READY ...",personaje->id);
											sendBytes(personaje->sk_planificador,envioAPlataforma,sizeof(char)*2, logger);
											log_info(logger,"{%c}[Main]: Send realizado IM_READY ...",personaje->id);
										}
										break;
									}
									case TE_MATE:{
										log_info(logger,"{%c} [Main] Recibi TE_MATE, me mataron por interbloqueo",personaje->id);
										personaje->bloqueado = 0;
										personaje_morir();
										t_info_nivel* info_nivel = conectarme_y_pedir_a_orquestador(personaje->nivel_actual);
										//usleep(RECONEXION_LUEGO_DE_MUERTE);
										personaje_conecta(info_nivel);
										break;
									}
								}
//							}
//								}
							}
						break;
						}
					}

				}
				else{//No llegue a destino, entonces no solicite nada.
					envioAPlanificador[0] = PERSONAJE_MOVIO;
					sendBytes(personaje->sk_planificador,&envioAPlanificador,sizeof(char)*2, logger);
					log_info(logger, "{%c} [Main] Send PERSONAJE_MOVIO al Planificador del %s...",personaje->id,personaje->nivel_actual->nivel);
				}
			}
		}
		if(personaje->haceSigterm == 1){
			log_debug(logger,"[MAIN] Valor de haceSigterm = %d",personaje->haceSigterm);
			personaje->haceSigterm = funcionDelSigterm();
			log_debug(logger,"[MAIN] Valor de haceSigterm = %d",personaje->haceSigterm);
		}

	}

	destruir_personaje(personaje);
	destruir_log();
	return EXIT_SUCCESS;
}
