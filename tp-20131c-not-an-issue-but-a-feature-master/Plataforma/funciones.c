/*
 * funciones.c
 *
 *  Created on: 14/07/2013
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
#include "comunes_plataforma.h"

int validaNivel(int idNivel, t_list * listaNivelesConectados) {
	int cantElementos;
	int i;
	t_nivel_deserializado * nivelCola = NULL;

	//primero pregunto si la lista de nivels esta vacia
	if (list_is_empty(listaNivelesConectados)) {
		return 0;
	}

	//si no esta vacia recorro buscando si el ID_Nivel ya existe
	cantElementos = list_size(listaNivelesConectados);

	for (i = 0; i < cantElementos; i++) {
		nivelCola = list_get(listaNivelesConectados, i);
		if (nivelCola->id_nivel == idNivel) {
			//el nivel ya fue cargado
			return 1;
		}
	}

	if (i == cantElementos) {
		//recorri la lista sin encontrar el ID Nivel
		return 0;
	}

	return -1;
}

int puertoNivel(int idNivel, t_list * listaNivelesConectados) {

	int cantElementos;
	int i;
	t_nivel_deserializado * nivelCola = NULL;

	//primero pregunto si la lista de nivels esta vacia
	if (list_is_empty(listaNivelesConectados)) {
		return 0;
	}

	//si no esta vacia recorro buscando si el ID_Nivel ya existe
	cantElementos = list_size(listaNivelesConectados);
	for (i = 0; i < cantElementos; i++) {
		nivelCola = list_get(listaNivelesConectados, i);
		if (nivelCola->id_nivel == idNivel) {
			//el nivel ya fue cargado
			//devuelvo el puerto
			return nivelCola->puertoPlanificador;
		}

	}

	if (i == cantElementos) {
		//recorri la lista sin encontrar el ID Nivel
		return 0;
	}

	return -1;
}

int todosLosPersonajesTerminaron(t_list* personajes_jugando) {
	int i = 0;
	int terminaron = TRUE
	;
	t_personaje_jugando* personaje_jugando = NULL;
	while ((terminaron) && (i < personajes_jugando->elements_count)) {
		personaje_jugando = list_get(personajes_jugando, i);
		if (!personaje_jugando->finPlanNiveles) {
			terminaron = FALSE
			;
		}
		i++;
	}
	return terminaron;
}

int funcionInfoNivel(int idNivel, t_list * listaNivelesConectados, int *socketNuevaConexion, t_log* logger) {
	int cantElementos;
	int i;
	t_nivel_deserializado * nivelCola = NULL;
	t_info_nivel infoNivel;
	t_stream *dataSerializada = NULL;

	//primero pregunto si la lista de nivels esta vacia
	if (list_is_empty(listaNivelesConectados)) {
		return 0;
	}

	//si no esta vacia recorro buscando si el ID_Nivel ya existe
	cantElementos = list_size(listaNivelesConectados);

	for (i = 0; i < cantElementos; i++) {
		nivelCola = list_get(listaNivelesConectados, i);

		if (nivelCola->id_nivel == idNivel) {
			//el nivel ya fue cargado
			//devuelvo la estructura infoNivel
			infoNivel.ip_nivel = nivelCola->ip;
			infoNivel.puerto_nivel = nivelCola->puerto;
			infoNivel.puerto_planificador = nivelCola->puertoPlanificador;
			//infoNivel.puerto_planificador = 5000;

			//serializo la informacion de nivel
			dataSerializada = serializador_info_nivel(&infoNivel);

			//envio la data del Nivel al personaje
			sendBytes(socketNuevaConexion, dataSerializada->data, dataSerializada->size, logger);
			return 1;
		}
	}

	if (i == cantElementos) {
		//recorri la lista sin encontrar el ID Nivel
		return 0;
	}

	return 0;
}

int marcaPersonajesDesbloqueados(int posNivel,int posRecurso,int cantidadLiberada,char** liberados,int*contador,t_log* logger) {

	log_info(logger, "Ejecutando [marcaPersonajesDesbloqueados]...");
	/* Lo que hace esta funcion es poner la bandera de personaje bloqueado
	 * en 1. Asi los planificadores pueden retirar de la lista de bloqueados
	 * y pasar a la de ready
	 */
	int cantElementosLista;
	int i;
	int menor;
	personaje_bloqueado *personaje;

	//Chequeo que la lista no este vacia. Si la lista esta vacia significa que no tengo
	//personajes bloqueados en ese nivel. En tal caso retorno 0 y no hago nada
	log_info(logger,"[marcaPersonajesDesbloqueados] Verificando existencia de personajes bloqueados en posNivel=[%d]. posRecurso=[%d]...",
			posNivel, posRecurso);
	if (list_is_empty(personajesBloqueadosGlobal[posNivel][posRecurso])) {
		log_info(logger,
				"Saliendo [marcaPersonajesDesbloqueados] - no hay personajes bloqueados...");
		return 0; //TODO en este caso tengo que devolver al nivel la misma lista que me entrego, o si no le mando un mensaje que no libere nada.
	}

	// Si hay elementos Recorro la lista y voy marcando de desbloqueado. El criterio utilizado es FIFO
	cantElementosLista = list_size(personajesBloqueadosGlobal[posNivel][posRecurso]);
	/*tengo que elegir el menor entre cantidad de recurso a liberar y la cantidad de elementos bloqueados
	 * de la lista. Es decir si la lista tiene 2 personajes bloqueados y por el recurso y se me liberaron 3 instancias
	 * tengo que recorrer hasta dos. En caso contrario el que limita es la cantidad de recursos.
	 */

	log_info(logger, "[marcaPersonajesDesbloqueados] cant. de bloqueados=[%d] | cant. recursos liberados=[%d].", cantElementosLista, cantidadLiberada);
	if (cantElementosLista < cantidadLiberada) {
		menor = cantElementosLista;
	} else {
		menor = cantidadLiberada;
	}

	for (i = 0; i < menor; i++) {
		personaje = list_get(personajesBloqueadosGlobal[posNivel][posRecurso], i);
		personaje->bool_recurso_ok = 1;

		//agrego a la lista de personajes liberados para el nivel
		(*contador)++;
		if (*contador != 1) {
			log_trace(logger, "[marcaPersonajesDesbloqueados] liberados realloc (%d).", *contador);
			*liberados = realloc(*liberados, sizeof(char)*(*contador));
		}

		log_info(logger, "[marcaPersonajesDesbloqueados] Id personaje desbloqueado=[%c].", personaje->id_personaje);
		(*liberados)[*contador - 1] = personaje->id_personaje;
	}


	log_info(logger, "Saliendo [marcaPersonajesDesbloqueados]");
	return 1;
}

void procesar_fin_plan_niveles(int* socketNuevaConexion, t_header* header,
		t_log* logger) {

	int contPersonaje = 0;
	int personaje_en_lista = FALSE;

	log_info(logger, "Procesando Fin Plan Niveles...");
	log_trace(logger, "Leyendo datos del socket. Tamanio=[%d]", header->tamanio);

	char* data = NULL;
	data = receiveBytes(socketNuevaConexion, header->tamanio, logger);

	log_trace(logger, "Deserializando personaje...");
	personajes* personaje = personaje_deserialize(data);

	pthread_mutex_lock(&mutex_personajes_jugando);
	int elementos = ((t_list*) personajes_jugando)->elements_count;

	log_info(logger, "Buscando al personaje [%c] en lista de personajes jugando...", personaje->id_personaje);
	t_personaje_jugando* pj_jugando = NULL;
	while ((!personaje_en_lista) && (contPersonaje < elementos)) {
		log_trace(logger, "Looping personajes [%d/%d]...", contPersonaje + 1, elementos);
		pj_jugando = list_get(personajes_jugando, contPersonaje);
		if (personaje->id_personaje == pj_jugando->personaje) {
			personaje_en_lista = TRUE
			;
			log_info(logger, "Personaje [%c] encontrado",
					pj_jugando->personaje);
		}
		contPersonaje++;

	}

	if (personaje_en_lista) {
		log_info(logger,
				"Seteando en TRUE el Fin Plan de Niveles del jugador...");
		pj_jugando->finPlanNiveles = TRUE
		;
//        list_remove(personajes_jugando, contPersonaje - 1);
	}

	pthread_mutex_unlock(&mutex_personajes_jugando);

	//Entonces pregunta si todos los personajes terminaron el plan de niveles y lanza koopa
	if (todosLosPersonajesTerminaron(personajes_jugando)) {
		char * arg2[] = { "koopa", koopa_config, NULL };
		char * arg3[] = { "TERM=xterm", NULL };
		log_info(logger, "Invocando koopa...");
		int resultado = execve("koopa", arg2, arg3);
		log_info(logger, "Koopa termino de procesar con resultado=[%d]", resultado);
	} else {
		log_info(logger, "Todavia quedan personajes jugando");
	}
}

char mataPersonaje(char* personajesEnDead, t_log*logger) {
	log_info(logger, "Ejecutando [mataPersonaje] - personajesEnDead=[%s]...", personajesEnDead);
	int i;
	int j;
	estructuraRecovery * personajeListRec = malloc(sizeof(estructuraRecovery));
	char flagEncontre;
	char personajeEncontrado;

	flagEncontre = 0;
	log_trace(logger, "[mataPersonaje] lock mutex...");
	pthread_mutex_lock(&mutex_lista_recovery);

	for (i = 0; i < listaRecovery->elements_count; i++) {
		log_trace(logger, "[mataPersonaje] looping listaRecovery %d...", i);
		personajeListRec->personaje = list_get(listaRecovery, i);
		personajeEncontrado = 0;
		int len = strlen(personajesEnDead);
		for (j = 0; j < len; j++) {
			log_trace(logger, "[mataPersonaje] looping personajesEnDead indice=[%d]/[%d]. ", j, len);
			if (personajeListRec->personaje == personajesEnDead[j]) {
				personajeEncontrado = personajeListRec->personaje;
				flagEncontre = 1;
			}

			if (flagEncontre == 1) {
				log_info(logger, "[mataPersonaje] Personaje encontrado [%c]!...", personajeEncontrado);
				break;
			}
		}

		if (flagEncontre == 1) {
			break;
		}
	}

	if(flagEncontre == -1){
		log_error(logger, "[mataPersonaje] Personaje no encontrado!");
	}

	log_trace(logger, "[mataPersonaje] free personajeListRec...");
	free(personajeListRec);
	log_trace(logger, "[mataPersonaje] unlock mutex...");
	pthread_mutex_unlock(&mutex_lista_recovery);
	log_info(logger, "Saliendo [mataPersonaje]...");
	return personajeEncontrado;

}

int validaDeadLock(char* personajesEnDead) {

	int x;
	int y;
	int j;
	int cantPerBloq = 0;
	int k;
	t_list * lista_personajes;
	personaje_bloqueado *personaje;

	for (j = 0; j < strlen(personajesEnDead); j++) {

		for (x = 0; x < 50; x++) {

			for (y = 0; y < 50; y++) {
				lista_personajes = personajesBloqueadosGlobal[x][y];

				for (k = 0; lista_personajes->elements_count; k++) {
					personaje = list_get(lista_personajes, k);

					if (personajesEnDead[j] == personaje->id_personaje) {
						cantPerBloq++;

					}
				}
			}
		}
	}

	if (strlen(personajesEnDead) == cantPerBloq)
		return 1;
	else
		return 0;

}

void array_inicializar(struct pollfd* personajes, int cantidad_personajes) {
	int i;
	for (i = 0; i < cantidad_personajes; i++) {
		personajes[i].fd = 0;
		personajes[i].events = POLLIN;
		personajes[i].revents = 0;
	}
}

int aceptar_conexion(struct pollfd* personajes, int* server, int cantidad_personajes, t_log* logger) {
	int i = 0, indice = cantidad_personajes;
	while (i <= indice) {
		if (personajes[i].fd == 0) {
			indice = i;
			personajes[indice].fd = *socket_server_accept(server, logger);
		}
		i++;
	}
	return indice;
}

void imprime_lista_bloqueados(int idNivel, t_log* logger) {
	/*log_trace(logger,"Procesando [imprime_lista_bloqueados]");
	int cantRecursosSistema = recursosGlobales->elements_count;
	int indexNivel = posNivel(idNivel);

	if(!list_is_empty(nivelesGlobales)){
		int i;

		int tamanioBloqueadosTotales = 0;
		char* bloqueados = malloc(sizeof(char));
		bloqueados[0] = '\0';
		int ponerGuion = 0;
		for(i = 0 ; i < cantRecursosSistema ; i++){//Recorro columnas de la matriz
			if(strlen(bloqueados) > 0){
				ponerGuion = 1;
			}
			if(!list_is_empty(personajesBloqueadosGlobal[indexNivel][i])){

				int cantidadBloqueados = personajesBloqueadosGlobal[indexNivel][i]->elements_count;

				char bloqueadosAux[cantidadBloqueados * 2]; //por los guiones y el barra 0
				int j;
				for(j = 0 ; j < cantidadBloqueados ; j++){//Dentro de la celda (Nivel/Recurso) recorro la lista
					personaje_bloqueado* pjBloqueado = list_get(personajesBloqueadosGlobal[indexNivel][i],j);
					log_trace(logger,"PONGO %c en [%d], bucleColumna [%i]",pjBloqueado->id_personaje,j*2,i);
					bloqueadosAux[j * 2] = pjBloqueado->id_personaje;

					if(j != 0){//en los pares menos el cero poner el guion
						log_trace(logger,"PONGO - en [%d]",(j*2)-1);
						bloqueadosAux[(j * 2) - 1] = '-';
					}
				}
				log_trace(logger,"PONGO BARRA CERO en [%d]",cantidadBloqueados * 2);
				bloqueadosAux[(cantidadBloqueados *2)-1] = '\0';

				if(ponerGuion == 1){
					char guion[2];
					guion[0] = '-';
					guion[1] = '\0';
					tamanioBloqueadosTotales = strlen(bloqueados) + strlen(bloqueadosAux) + 2;// por el barra 0 y el guion;
					bloqueados = realloc(bloqueados,sizeof(char) * tamanioBloqueadosTotales);
					bloqueados = strcat(bloqueados,guion);
				}
				else{
					tamanioBloqueadosTotales = strlen(bloqueados) + strlen(bloqueadosAux) + 1;// por el barra 0;
					bloqueados = realloc(bloqueados,sizeof(char) * tamanioBloqueadosTotales);
				}
				bloqueados = strcat(bloqueados,bloqueadosAux);
			}
		}


		log_info(logger,"Personajes del nivel %d Bloqueados =[%s]",idNivel,bloqueados);
		free(bloqueados);

	}
	else{
		log_info(logger,"No hay personajes Bloqueados en el nivel %d",idNivel);

	}
	*/
}
