/*
 * personaje.c
 *
 *  Created on: 25/04/2013
 *      Author: utnso
 */
#include "personaje.h"
#include <comunes/config.h>
#include <comunes/string.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

t_log* logger;
t_personaje* personaje ;
bool haceSigterm;

/********************CONEXIONES**************************/

void conectarse_a_orquestador(int* socket_personaje){
	log_info(logger,"{%c} Ejecutando [conectarse_a_orquestador]...",personaje->id);
	int connectResult = socket_connect_with_retry(socket_personaje, personaje->ip_orquestador, personaje->puerto_orquestador, logger);
	if(connectResult < 0){
		log_error(logger,"{%c} [conectarse_a_orquestador] No me pude reconectar al orquestador por problemas de red, Cerrandome...",personaje->id);
		exit(EXIT_FAILURE);
	}
	log_info(logger, "{%c} [conectarse_a_orquestador]Personaje se conecto al orquestador",personaje->id);
}



t_info_nivel* recibir_info_orquestador(t_plan_niveles* nivel_actual, int* socket){
	personajes serializador;
	serializador.id_personaje = personaje->id;
	serializador.nivel_actual = parsear_nivel(nivel_actual->nivel);

	log_info(logger, "[recibir_info_orquestador] Serializando datos y pedidos para el orquestador...");
	t_stream* stream = serializador_personaje(&serializador);

	log_info(logger, "[recibir_info_orquestador] Sending pedido al orquestador...");
	sendBytes(socket, stream->data, stream->size, logger);
	log_info(logger, "[recibir_info_orquestador] Send realizado...");

	t_header* header = receiveBytes(socket, sizeof(int) * 2, logger);
	t_info_nivel* info_nivel;
	if (header->tipo == INFO_NIVEL) {
		char* data = receiveBytes(socket, header->tamanio, logger);
		log_info(logger, "{%c}[recibir_info_orquestador] Se recibe informacion de nivel y planificador del orquestador...",personaje->id);
		info_nivel = info_nivel_deserialize(data);
		log_info(logger, "{%c}[recibir_info_orquestador] Se deserializa informacion de nivel y planificador del orquestador...",personaje->id);
	}

	free(stream);
	free(header);

	return info_nivel;
}

void desconectarme_orquestador(int* socket_personaje){
	socket_close(socket_personaje, logger);
}

void conectarme_nivel(char* ip_nivel,int puerto_nivel){
	int connectResult = socket_connect_with_retry(personaje->sk_nivel, ip_nivel, puerto_nivel, logger);
	if (connectResult < 0) {
		log_error(logger, "{%c} [conectarme_nivel] Fallo al conectarse al %s IP=[%s] Puerto=[%d]...",personaje->id,personaje->nivel_actual->nivel, ip_nivel, puerto_nivel);
		exit(EXIT_FAILURE);
	}

	log_info(logger, "{%c} [conectarme_nivel] Personaje se conecto al %s...",personaje->id,personaje->nivel_actual->nivel);
	log_info(logger, "{%c} [conectarme_nivel] Sending id de personaje a Nivel...", personaje->id);
	int sendResult = sendBytes(personaje->sk_nivel, &(personaje->id), sizeof(char), logger);
	if (sendResult < 0)
	{
		log_error(logger, "{%c} [conectarme_nivel] Fallo envio al %s...",personaje->id,personaje->nivel_actual->nivel);
		exit(EXIT_FAILURE);
	}

	log_info(logger, "{%c} [conectarme_nivel] Send realizado al %s...",personaje->id,personaje->nivel_actual->nivel);
}

void conectarme_planificador(char* ip_orquestador,int puerto_planificador){
	if ((socket_connect_with_retry(personaje->sk_planificador, ip_orquestador, puerto_planificador, logger)) < 0) {
		log_error(logger, "{%c} [conectarme_planificador] Fallo al conectarse al planificador IP=[%s] Puerto=[%d]",personaje->id, ip_orquestador, puerto_planificador);
		exit(EXIT_FAILURE);
	}

	log_info(logger, "{%c} [conectarme_planificador] Personaje se conecto al planificador",personaje->id);

	char nuevo_personaje[2];
	nuevo_personaje[0] = NUEVO_PERSONAJE;
	nuevo_personaje[1] = personaje->id;

	int sendResult = sendBytes(personaje->sk_planificador, &nuevo_personaje, sizeof(char)*2, logger);
	if (sendResult < 0) {
		log_error(logger, "{%c} [conectarme_planificador] Fallo envio",personaje->id);
		socket_close(personaje->sk_nivel, logger);
		exit(EXIT_FAILURE);
	}
	log_info(logger, "{%c} [conectarme_planificador] Send realizado al planificador",personaje->id);
}

t_info_nivel* conectarme_y_pedir_a_orquestador(t_plan_niveles* nivel_actual){

	log_info(logger, "{%c} Procesando [conectarme_y_pedir_a_orquestador]...", personaje->id);
	int *socket_personaje = socket_create_sin_puerto(logger);
	conectarse_a_orquestador(socket_personaje);
	t_info_nivel* info_nivel = recibir_info_orquestador(nivel_actual,socket_personaje);
	desconectarme_orquestador(socket_personaje);
	log_info(logger, "{%c} Saliendo [conectarme_y_pedir_a_orquestador]...", personaje->id);
	return info_nivel;
}

//notifico desconexion al nivel y al planificador, y me desconecto de ambos
void personaje_desconexion_plataforma(int* socket){
	log_trace(logger,"{%c} Ejecutando [personaje_desconexion_plataforma]...",personaje->id);
	char envioAPlataforma[2];
	envioAPlataforma[0] = PERSONAJE_FIN_NIVEL;
	envioAPlataforma[1] = personaje->id;
	log_info(logger,"{%c}[personaje_desconexion_plataforma]: Sending PERSONAJE_FIN_NIVEL ...",personaje->id);
	sendBytes(socket,envioAPlataforma,sizeof(char)*2, logger);
	log_info(logger,"{%c}[personaje_desconexion_plataforma]: Send realizado PERSONAJE_FIN_NIVEL ...",personaje->id);
//	log_info(logger,"{%c}[personaje_desconexion_plataforma]: Closing socket planificador ...",personaje->id);
	//socket_close(socket, logger);
//	log_info(logger,"{%c}[personaje_desconexion_plataforma]: Closed socket planificador ...",personaje->id);
	log_trace(logger,"{%c} Saliendo [personaje_desconexion_plataforma]...",personaje->id);
}

//me conecto al info_nivel
void personaje_conecta(t_info_nivel* info_nivel) {
	log_info(logger, "{%c} Procesando [personaje_conecta]...", personaje->id);

//	if(personaje->sk_nivel != NULL){
//		log_info(logger, "{%c} [personaje_conecta] Cerrando socket NIVEL...", personaje->id);
//		socket_close (personaje->sk_nivel, logger);
//	}
//
//	if(personaje->sk_planificador != NULL) {
//		log_info(logger, "{%c} [personaje_conecta] Cerrando socket PLANIFICADOR...", personaje->id);
//		socket_close (personaje->sk_planificador, logger);
//	}

	personaje->sk_nivel = socket_create_sin_puerto(logger);
	personaje->sk_planificador = socket_create_sin_puerto(logger);

	log_info(logger, "{%c} [personaje_conecta] Conectando al %s...",personaje->id,personaje->nivel_actual->nivel);
	conectarme_nivel(info_nivel->ip_nivel,info_nivel->puerto_nivel);

	log_info(logger, "{%c} [personaje_conecta] Conectando al Planificador...",personaje->id);
	conectarme_planificador(personaje->ip_orquestador, info_nivel->puerto_planificador);

	free(info_nivel);
	log_info(logger, "{%c} Saliendo [personaje_conecta]...", personaje->id);
}



/********************ACCIONES**************************/

t_posicion* calcular_proximo_movimiento(t_posicion* actual,t_posicion* destino){

	bool flag=0;

	if ((actual->x > destino->x)&&(flag==0)) {
	    (actual->x)--;
	    flag=1;
	}
	if ((actual->x < destino->x)&&(flag==0)) {
		(actual->x)++;
		flag=1;
	}

	if ((actual->y > destino->y)&&(flag==0))  {
	    (actual->y)--;
	    flag=1;
	}
	if ((actual->y < destino->y)&&(flag==0)) {
		(actual->y)++;
		flag=1;
	}
	if (((actual->x == destino->x)&&(actual->y < destino->y))&&(flag==0))  {
	    (actual->y)++;
	    flag=1;
	}
	if (((actual->x == destino->x)&&(actual->y > destino->y))&&(flag==0) ) {
	    (actual->y)--;
	    flag=1;
	}
	if (((actual->y == destino->y)&&(actual->x < destino->x))&&(flag==0) ) {
	    (actual->x)++;
	    flag=1;
	}
	if (((actual->y == destino->y)&&(actual->x > destino->x)) &&(flag==0) ){
	    (actual->x)--;
	    flag=1;
	}
	return actual;
}

//calcula el proximo movimiento y le dice a nivel que lo dibuje
void hacer_movimiento(int* socket,t_link_element** nodo_recurso_actual){

	if( (personaje->destino.x == 0) && (personaje->destino.y == 0) ){

		char* recurso = (char*) (*nodo_recurso_actual)->data;


		//el nivel me responde con la posicion de dicho recurso y lo seteo
		t_posicion* pos=solicitar_posicion_recurso(socket,recurso[0]);
		personaje->destino.x = pos->x;
		personaje->destino.y = pos->y;

		log_info(logger,"{%c} Voy en busca del recurso %c cuya posicion es x:%d , y:%d ",personaje->id,recurso[0],personaje->destino.x,personaje->destino.y);
	}

	t_posicion* posi =calcular_proximo_movimiento(&(personaje->actual),&(personaje->destino));
	log_info(logger,"{%c} Posicion actual de %s es x:%d , y:%d ",personaje->id,personaje->nombre,personaje->actual.x,personaje->actual.y);
	t_stream*stream= serializar_pedido_nivel_move(personaje->id,posi->x,posi->y);
	sendBytes(socket,stream->data,stream->size, logger);
}

char* solicitar_recurso_a_nivel(){

	char* recurso = (char*) personaje->nodo_recurso_actual->data;
	log_info(logger,"{%c} %s va a solicitar a nivel el recurso:%c",personaje->id,personaje->nombre,recurso[0]);
	t_stream* stream=serializar_pedido_solicitud_recurso(personaje->actual.x, personaje->actual.y,recurso[0],personaje->id);
	sendBytes(personaje->sk_nivel,stream->data,stream->size, logger);
	char*data=receiveBytes(personaje->sk_nivel, 4, logger);

	return data;
}

//durante la ejecucion de personaje evalua esta funcion y de ser verdadera envia la solicitud de recurso
bool llega_a_destino(t_posicion actual,t_posicion destino){

	return ((actual.x==destino.x)&&(actual.y==destino.y));
}

t_posicion* solicitar_posicion_recurso(int* socket,char id_recurso){

	t_stream* stream=serializar_pedido_buscar_recurso(id_recurso);

	sendBytes(socket,stream->data,stream->size, logger);
	t_posicion* buffer =receiveBytes(socket,sizeof(t_posicion), logger);

	return buffer;
}

//puede ser el proximo recurso o nivel->me conecto con orquestador y planificador le seteo 1,1 como actual y cargo en destino el prox recurso
int avanzar_proximo_objetivo(bool* finaliza_nivel) {

	personaje->nodo_recurso_actual = personaje->nodo_recurso_actual->next;
	personaje->destino.x = 0;
	personaje->destino.y = 0;

	if (personaje->nodo_recurso_actual == NULL ) {
		*finaliza_nivel = true;
		personaje_desconexion_plataforma(personaje->sk_planificador);
		abandonar_nivel(personaje->id,personaje->sk_nivel);
		if(personaje->nivel_actual->siguiente!=NULL){
			personaje->nivel_actual = personaje->nivel_actual->siguiente;
		}
		else{
			personaje->nivel_actual = NULL;
			usleep(SLEEP_DISTINTOS_FINES);
			int *sockeet = socket_create_sin_puerto(logger);
			conectarse_a_orquestador(sockeet);
			log_info(logger, "{%c} [avanzar_proximo_objetivo] Envia a orquestador Fin Plan de Niveles",personaje->id);
			t_stream* stream=serializador_header_fin_nivel(personaje->id);
			sendBytes(sockeet,stream->data,stream->size, logger);
			desconectarme_orquestador(sockeet);
			return 1;
		}

		personaje->nodo_recurso_actual = personaje->nivel_actual->objetivos->head;
		//seteo donde va a empezar el proximo nivel
        personaje->actual.x=1;
		personaje->actual.y=1;
        t_info_nivel* info_nivel=conectarme_y_pedir_a_orquestador(personaje->nivel_actual);
        //me conecnto al nuevo nivel y su planificador asociado
		personaje_conecta(info_nivel);
	}
	return 0;
}

void abandonar_nivel(char id_personaje,int* socket){
	log_info(logger,"{%c}[abandonar_nivel]: Abandonando nivel ...",id_personaje);
	t_stream* stream=serializar_abandono_pj(id_personaje);
	sendBytes(socket,stream->data,stream->size, logger);
	log_trace(logger,"{%c}[abandonar_nivel]: Closing socket nivel ...",id_personaje);
	socket_close(socket,logger);
	log_trace(logger,"{%c}[abandonar_nivel]: Closed socket nivel ...",id_personaje);
	log_info(logger,"{%c}[abandonar_nivel]: Nivel abandonado exitosamente ...",id_personaje);
}





/********************OTRAS**************************/

void create_log(){
	logger = log_create(PATH_LOG, "Personaje", true, LOG_LEVEL_INFO);
}

t_personaje* configuracion_inicializar(char* path) {
	t_config* configuracion_personaje = config_create(path);
	 personaje = malloc(sizeof(t_personaje));
	personaje->planDeNiveles = NULL;
	//posicion de inicio
	personaje->actual.x=1;
	personaje->actual.y=1;

	//esto lo hacemos para que la primera vez que se usa el pj entre al if de hacer movimiento
	//y setee el destino posta
	personaje->destino.x = 0;
	personaje->destino.y = 0;

	personaje->nivel_actual=malloc(sizeof(t_plan_niveles));
	personaje->nodo_recurso_actual=malloc(sizeof(t_link_element));
//sockets
	personaje->sk_nivel=NULL;
	personaje->sk_planificador=NULL;


	//nombre
	personaje->nombre = string_duplicate(
			config_get_string_value(configuracion_personaje, "nombre"));

//id
	personaje->id =
			(config_get_string_value(configuracion_personaje, "simbolo")[0]);
//vidas
	personaje->vidas_iniciales = config_get_int_value(configuracion_personaje, "vidas");
	personaje->vidas_actuales=personaje->vidas_iniciales;
//ip y puerto
	char* ip_puerto = config_get_string_value(configuracion_personaje,
			"orquestador");
	char**ip = string_split(ip_puerto, ":");
	strcpy(personaje->ip_orquestador, ip[0]);
	//puerto
	char* puerto = ip[1];
	personaje->puerto_orquestador= atoi(puerto);
	free(ip[0]);
	free(ip[1]);
	free(ip);
//niveles y objetivos
	char** nivele = config_get_array_value(configuracion_personaje,
			"planDeNiveles");
	int i;
	for (i = 0; nivele[i] != NULL ; i++) {
		char*nombre = string_duplicate(nivele[i]);
		char* nivel = string_from_format("obj [%s]", nombre);
		char**objetivos = config_get_array_value(configuracion_personaje,
				nivel);
		t_list* lista = list_create();
		int y;
		for (y = 0; objetivos[y] != NULL ; y++) {
			char* obj = objetivos[y];
			list_add(lista, obj);
		}

		agregar_plan(&personaje->planDeNiveles, nombre, lista);
	}

	config_destroy(configuracion_personaje);

	log_info(logger, "se ha inicializado un personaje");

	personaje->haceSigterm = 0;
	personaje->bloqueado = 0;
	return personaje;
}

void agregar_plan(t_plan_niveles** lista, char*nombre, t_list* list) {

	t_plan_niveles* plan = malloc(sizeof(t_plan_niveles));
	t_plan_niveles* aux = *lista;
	plan->nivel = nombre;
	plan->objetivos = list;
	plan->siguiente = NULL;
	if (*lista == NULL ) {
		plan->siguiente = *lista;
		*lista = plan;
	} else
	{
		t_plan_niveles* auxAnt = aux;
		while (aux->siguiente != NULL )
		{
			aux = aux->siguiente;
		}
		aux->siguiente = plan;
		*lista = auxAnt;
	}
}

void destruir_personaje() {
	while (personaje->planDeNiveles!=NULL){
		t_plan_niveles *aux =personaje->planDeNiveles;
		list_destroy(personaje->planDeNiveles->objetivos);
		personaje->planDeNiveles=personaje->planDeNiveles->siguiente;
		free(aux);
	}
	free(personaje);
}

void destruir_log(){
	log_destroy(logger);
}

void personaje_morir(){
	log_trace(logger,"{%c} Ejecutando [personaje_morir]",personaje->id);
	log_info(logger,"{%c} Acabo de perder una vida :(",personaje->id);
	if(personaje->vidas_actuales == 0){
		//reiniciar plan de niveles y recargar vida
		personaje->nivel_actual=personaje->planDeNiveles;
		personaje->vidas_actuales=personaje->vidas_iniciales;
		log_info(logger,"{%c} Reinicio mis vidas,Vidas actuales = [%d]",personaje->id,personaje->vidas_actuales);
	}else{
		// reducir una vida, sigo en el mismo nivel
		personaje->vidas_actuales--;
		log_info(logger,"{%c} Mis Vidas actuales = [%d]",personaje->id,personaje->vidas_actuales);
	}
	personaje->actual.x=1;
	personaje->actual.y=1;
	personaje->destino.x=0;
	personaje->destino.y=0;

	personaje->nodo_recurso_actual=personaje->nivel_actual->objetivos->head;
	log_trace(logger,"{%c} Saliendo [personaje_morir]",personaje->id);
}

t_log* recibir_logger(){
	return logger;
}

int funcionDelSigterm(){
	log_info(logger,"{%c }muere el personaje %s por SIGTERM",personaje->id,personaje->nombre);
	personaje_morir();
	personaje_desconexion_plataforma(personaje->sk_planificador);
	abandonar_nivel(personaje->id,personaje->sk_nivel);
	//usleep(RECONEXION_LUEGO_DE_MUERTE);
	t_info_nivel* info_nivel = conectarme_y_pedir_a_orquestador(personaje->nivel_actual);
	personaje_conecta(info_nivel);
	return 0;
}

void handler (int n) {
	switch (n) {
		case SIGTERM:{
			if(personaje->bloqueado == 1){
				abandonar_nivel(personaje->id,personaje->sk_nivel);
				int* soocket = socket_create_sin_puerto(logger);
				conectarse_a_orquestador(soocket);
				log_trace(logger,"{%c} [SIGTERM_BLOQ] Serializando ABANDONO_PERSONAJE",personaje->id);
				t_stream* stream = serializar_abandono_pj(personaje->id);
				log_trace(logger,"{%c} [SIGTERM_BLOQ] Serializacion finalizada ABANDONO_PERSONAJE",personaje->id);

				log_trace(logger,"{%c} [SIGTERM_BLOQ] Sending ABANDONO_PERSONAJE",personaje->id);
				sendBytes(soocket,stream->data,sizeof(t_stream),logger);
				log_trace(logger,"{%c} [SIGTERM_BLOQ] Send realizado con ABANDONO_PERSONAJE",personaje->id);
				sendBytes(soocket,&(personaje->id),sizeof(char),logger);
				log_trace(logger,"{%c} [SIGTERM_BLOQ] Re-Send realizado con id",personaje->id);
				desconectarme_orquestador(soocket);
			}
			else{
				log_debug(logger,"[SIGTERM] Valor de haceSigterm = %d",personaje->haceSigterm);
				personaje->haceSigterm = 1;
				log_debug(logger,"[SIGTERM] Valor de haceSigterm = %d",personaje->haceSigterm);
			}
			break;
		}
		case SIGUSR1:{
			(*personaje).vidas_actuales++;
			log_info(logger,"{%c} Me agregaste una vida :),Vidas actuales: [%d]",personaje->id,personaje->vidas_actuales);
			break;
		}
		case SIGINT:{
			log_info(logger,"muere el personaje %s anormalmente, se actualizaran recursos en nivel y desconectara del planificador",personaje->nombre);
			personaje_desconexion_plataforma(personaje->sk_planificador);
			abandonar_nivel(personaje->id,personaje->sk_nivel);
			exit(n);
		}
	}

}

