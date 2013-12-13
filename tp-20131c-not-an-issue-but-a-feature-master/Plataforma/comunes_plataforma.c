/*
 * comunes_plataforma.c
 *
 *  Created on: 21/05/2013
 *      Author: utnso
 */

#include <stdio.h>
#include <comunes/collections/list.h>
#include <comunes/log.h>
#include <comunes/config.h>
#include "planificador.h"
#include "plataforma.h"
#include <stdlib.h>
#include <string.h>
#include "Orquestador.h"
#include <sockets.h>
#include <pthread.h>
#include <Serializadores/Stream.h>
#include <Serializadores/serializadores.h>
#include "funciones.h"

/*
 int * buscar_lista_personajes_bloqueados(int nivel, char recurso) {
 int cantElementos;
 int i;
 int si_encontro_nivel = 0;
 //int si_encontro_recurso = 0;
 nivel_bloqueado *nivelCola;
 recurso_bloqueados *recursoCola;



 //busca la cantidad de elementos en el nivel
 cantElementos = list_size(lista_niveles_bloqueados);

 //recorre la lista de niveles buscando nivel
 for (i = 0; i < cantElementos; i++) {

 nivelCola = list_get(lista_niveles_bloqueados, i);

 if (nivelCola->id_nivel == nivel) {
 //el nivel ya fue cargado
 //obtengo el puntero de la cola de recursos
 lista_recursos_bloqueados = nivelCola->p_recurso;
 si_encontro_nivel = 1;
 }

 }

 //si encuentra nivel busca recurso
 if (si_encontro_nivel == 1) {
 cantElementos = list_size(lista_recursos_bloqueados);

 i = 0;
 for (i = 0; i < cantElementos; i++) {

 recursoCola = list_get(lista_recursos_bloqueados, i);


 if (recursoCola-> == nivel) {
 //el nivel ya fue cargado
 //retorno el puntero a la lista de personajes de ese recurso en se nivel
 return recursoCola.p_personaje;

 }

 }

 }// Sino encuentra nivel, crea lista recursos y lista nivel
 else {

 //		t_list *lista_recursos_bloqueados;
 //		t_list *lista_personajes_bloqueados;
 recursoCola = malloc(sizeof(recurso_bloqueados));
 recursoCola->id_recurso= recurso;
 recursoCola->p_personaje = lista_personajes_bloqueados;


 nivelCola = malloc(sizeof(nivel_bloqueado));
 nivelCola->id_nivel = nivel;
 nivelCola->p_recurso = lista_recursos_bloqueados;

 }

 }
 */

char posRecurso(char recurso) {

	int cantElementos;
	int i;
	char*recursoAgregar;
	char * data;

//primero pregunto si la lista de recursos esta vacia

	recursoAgregar = malloc(sizeof(char));

	*recursoAgregar = recurso;

	if (list_is_empty(recursosGlobales)) {
		//agrego el recurso a la lista
		//pthread_mutex_lock( &mutex_lista_bloqueados );
		list_add(recursosGlobales, recursoAgregar);
		//pthread_mutex_unlock( &mutex_lista_bloqueados );
	}

	//ahora busco en que posicion de la lista esta

	cantElementos = list_size(recursosGlobales);

	for (i = 0; i < cantElementos; i++) {

		data = list_get(recursosGlobales, i);

		if (*data == recurso) {

			return i;
		}
	}

	if (i == cantElementos) {
		//agrego el recurso a la lista
		list_add(recursosGlobales, recursoAgregar);
		cantElementos = list_size(recursosGlobales);
		//vuelvo a buscar en la lista de recursos la pos del recurso en la lista
		for (i = 0; i < cantElementos; i++) {

			data = list_get(recursosGlobales, i);

			if (*data == recurso) {

				return i;
			}
		}
	}

	return 0;

}

int posNivel(int Nivel) {

	int cantElementos;
	int i;
	char*nivelAgregar;
	char * data;

//primero pregunto si la lista de recursos esta vacia

	nivelAgregar = malloc(sizeof(int));

	*nivelAgregar = Nivel;

	if (list_is_empty(nivelesGlobales)) {
		//agrego el recurso a la lista
		list_add(recursosGlobales, nivelAgregar);
	}

	//ahora busco en que posicion de la lista esta

	cantElementos = list_size(nivelesGlobales);

	for (i = 0; i < cantElementos; i++) {

		data = list_get(nivelesGlobales, i);

		if (*data == Nivel) {

			return i;
		}
	}

	if (i == cantElementos) {
		//agrego el recurso a la lista
		list_add(nivelesGlobales, Nivel);

		//vuelvo a buscar en la lista de recursos la pos del recurso en la lista
		for (i = 0; i < cantElementos; i++) {

			data = list_get(nivelesGlobales, i);

			if (*data == Nivel) {

				return i;
			}
		}
	}

	return 0;

}

int posicion_recurso(char id_recurso) {

	int cantElementos;
	char *recurso;
	int flag = 0;
	int i = 0;
	char *idRec_ptr = NULL;

	idRec_ptr = malloc(sizeof(char));

	cantElementos = list_size(recursosGlobales);

	for (i = 0; i < cantElementos; i++) {
		recurso = list_get(recursosGlobales, i);

		if (*recurso == id_recurso) {
			//el nivel ya fue cargado
			//devuelvo pid
			return i;
			flag = 1;
		}
	}
	if (flag != 1) {
		*idRec_ptr = id_recurso;
		list_add(recursosGlobales, idRec_ptr);
		return i;
	}

	return -1;
}

int meteBloqEnColaRedy(int posNivel, t_list**colaReady, int idNivel, t_log* logger) {
	int contadorRecurso;
	int contadorPersonaje;
	int cantElementos;
	int index;
	t_list *lista_personajes;
	personaje_bloqueado *personaje;
	t_personaje_ready * personaje_ready;
	char data[1];

	for (contadorRecurso = 0; contadorRecurso < VAR; contadorRecurso++) {
		lista_personajes =
				personajesBloqueadosGlobal[posNivel][contadorRecurso];

		//cantElementos = list_size(lista_personajes);
		cantElementos = lista_personajes->elements_count;

		index = 0;
		for (contadorPersonaje = 0; contadorPersonaje < cantElementos;contadorPersonaje++) {
			personaje = list_get(lista_personajes, index);

			if (personaje->bool_recurso_ok == 1) {

				log_info(logger, "[meteBloqEnColaRedy] Enviando PERSONAJE_DESBLOQUEATE a personaje=[%c]", personaje->id_personaje);
				//Aca le mando al personaje el mensaje para que se desbloquee
				data[0] = PERSONAJE_DESBLOQUEATE;
				sendBytes(&personaje->socket, data, sizeof(char), logger);
				log_info(logger, "[meteBloqEnColaRedy] PERSONAJE_DESBLOQUEATE a personaje=[%c] Enviado!!!", personaje->id_personaje);
				//personaje cambia a la cola de ready
				/*personaje_ready = malloc(sizeof(t_personaje_ready));
				personaje_ready->id_personaje = personaje->id_personaje;
				personaje_ready->socket = personaje->socket;
				list_add(*colaReady, personaje_ready);*/
				//list_remove_and_destroy_element(lista_personajes,contadorPersonaje, (void*)personaje);
				list_remove(lista_personajes, index);

				imprime_lista_bloqueados(idNivel, logger);


			} else
				index++;

		}
	}

	return 1;
}

void read_config_file(t_log* logger, char* path) {
	t_config *configuracion_planificador = config_create(path);
	tiempo_espera = config_get_int_value(configuracion_planificador, "Tiempo");
	char* tempCFG = config_get_string_value(configuracion_planificador,
			"koopa_config");
	int configLen = strlen(tempCFG);
	koopa_config = malloc(configLen + 1);
	// Necesario porque sino se queda apuntando a la estructura del config.
	// Luego al hacer destroy se perderia la info.
	strcpy(koopa_config, tempCFG);
	config_destroy(configuracion_planificador);
	log_info(logger, "Se ha cargado el tiempo de espera de %d", tiempo_espera);
}

int sacaBloqDeDeadLock(char personajeMate , t_log* logger) {
    log_info(logger, "Ejecutando [sacaBloqDeDeadLock]...");
	int contadorRecurso;
	int contadorPersonaje;
	int cantElementos;
	int contNiveles;
	int index;
	t_list *lista_personajes;
	personaje_bloqueado *personaje;
	t_personaje_ready * personaje_ready;
	char *data;

	for (contadorRecurso = 0; contadorRecurso < VAR; contadorRecurso++) {

		for (contNiveles = 0; contNiveles < VAR;	contNiveles++) {

			lista_personajes =personajesBloqueadosGlobal[contNiveles][contadorRecurso];

			cantElementos = lista_personajes->elements_count;
			index = 0;
			for (index = 0; index < cantElementos; index++) {

				personaje = list_get(lista_personajes, index);

				if (personaje->id_personaje == personajeMate) {

					//personaje cambia a la cola de ready

					personaje_ready = malloc(sizeof(t_personaje_ready));
					personaje_ready->id_personaje = personaje->id_personaje;
					personaje_ready->socket = personaje->socket;
					data = malloc(sizeof(char));
					*data = TE_MATE;
					log_trace(logger,"[sacaBloqDeDeadLock] ABRIENDO EL MUTEX RECOVERY");
					//lo saca de la lista de recovery
					pthread_mutex_lock(&mutex_lista_recovery);

					int cantPersonajesRecovery = list_size(listaRecovery);

					for (contadorPersonaje = 0;	contadorPersonaje < cantPersonajesRecovery;	contadorPersonaje++) {
							char personaje_id  = list_get(listaRecovery, contadorPersonaje);
							if (personaje_id == personajeMate) {
								list_remove(listaRecovery,contadorPersonaje);
								break;
							}
					}

				    pthread_mutex_unlock(&mutex_lista_recovery);
				    log_trace(logger,"[sacaBloqDeDeadLock] CERRANDO EL MUTEX RECOVERY");

				    //lo saco de la lista de bloqueados
					list_remove(lista_personajes, index);
					log_info(logger, "[sacaBloqDeDeadLock] Enviando TeMate a [%c]...", personajeMate);
					if (sendBytes(&personaje_ready->socket, data, sizeof(char), logger) == -1) {
					    log_error(logger, "[sacaBloqDeDeadLock] No envio TeMate a [%c] deadLock...", personajeMate);
					}

					free(data);
					free(personaje_ready);
					break;
				}

			}


		}

	}

	imprime_lista_bloqueados(contNiveles, logger);

	log_info(logger, "Saliendo [sacaBloqDeDeadLock]...");
	return 1;
}


void imprimirListaReady(t_list * lista_personajes_ready,t_log* logger, int  idNivel){
	log_trace(logger,"Procesando [imprimirListaReady]");
	int cantidad_readys = lista_personajes_ready->elements_count;
	if(cantidad_readys > 0){
		int i;
		log_trace(logger,"cantidad de personajes en ready=%d",cantidad_readys);
		char readys[cantidad_readys * 2];//por los guiones y el barra 0
		for(i = 0; i < cantidad_readys ; i++){
			t_personaje_ready* pjReady = list_get(lista_personajes_ready,i);
			log_trace(logger,"PONGO %c en [%d]",pjReady->id_personaje,i*2);
			readys[i * 2] = pjReady->id_personaje;

			if(i != 0){//en los pares menos el cero poner el guion
				log_trace(logger,"PONGO - en [%d]",(i*2)-1);
				readys[(i * 2) - 1] = '-';
			}
		}
		log_trace(logger,"PONGO BARRA CERO en [%d]",cantidad_readys * 2);
		readys[(cantidad_readys *2)-1] = '\0';
		log_info(logger,"Personajes del nivel %d en Ready=[%s]",idNivel,readys);
	}
	else{
		log_info(logger,"No hay personajes en Ready del nivel %d",idNivel);
	}
}
