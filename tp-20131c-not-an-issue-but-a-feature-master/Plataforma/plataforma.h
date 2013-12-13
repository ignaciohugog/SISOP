/*
 * plataforma.h
 *
 *  Created on: 21/04/2013
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define VAR 50
#ifndef PLATAFORMA_H_
#define PLATAFORMA_H_
//todas las estructuras ya estan definidas en serializadores.h

//personaje cola de listos
typedef struct{
	char id_personaje;
	int socket;
}__attribute__((packed)) t_personaje_ready;

//personaje cola de bloqueados
typedef struct{
	char id_personaje;
	char bool_recurso_ok;
	int socket;
}__attribute__((packed)) personaje_bloqueado;

//recurso cola bloqueados
typedef struct{
	char id_recurso;
	t_list *lista_personajes_bloqueados;
	//recurso_bloqueados* recBloq;
}__attribute__((packed)) recurso_bloqueados;

//nivel cola bloqueados
typedef struct{
	int id_nivel;
	recurso_bloqueados * recBloq;
}__attribute__((packed)) nivel_bloqueado;


typedef struct{
	char personaje;
}__attribute__((packed)) estructuraRecovery;

typedef struct{
	char personaje;
	int finPlanNiveles;
}t_personaje_jugando;

//extern t_list *lista_personajes;
extern t_list *lista_niveles_bloqueados;
extern t_list *lista_recursos_bloqueados;
extern t_list *lista_personajes_bloqueados;

extern t_list *personajesBloqueadosGlobal[VAR][VAR];

extern t_list* recursosGlobales;

extern t_list* nivelesGlobales;

extern t_list* listaRecovery;
extern t_list* personajes_jugando;


extern pthread_mutex_t mutex_lista_bloqueados;
extern pthread_mutex_t mutex_orquestador_bloqueados;
extern pthread_mutex_t mutex_log_planificador;
extern pthread_mutex_t mutex_personajes_jugando;
extern pthread_mutex_t mutex_lista_recovery;
extern char* koopa_config;
extern int flagCorriendoKoopa;

#endif /* PLATAFORMA_H_ */
