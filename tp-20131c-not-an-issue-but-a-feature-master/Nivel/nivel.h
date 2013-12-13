/*
 * nivel.h
 *
 *  Created on: 22/04/2013
 *      Author: utnso
 */

#ifndef NIVEL_H_
#define NIVEL_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nivel.h> //nivel-gui
#include <Serializadores/serializadores.h>
#include <comunes/string.h>
#include <comunes/config.h>
#include <comunes/collections/list.h>
#include <comunes/collections/queue.h>
#include <comunes/log.h>
#include <sys/poll.h>

#define PATH_LOG "log_nivel"
#define FALSE 0
#define TRUE 1
#define BACKLOG 50
#define TIMEOUT 100

typedef struct {
	char* nombre;
	char ip[30];
	int puerto;
	t_list* personajes_deadlock;
	int deadlock;
	bool recovery;
	ITEM_NIVEL * lista_items; //cajas o personajes
	char ip_nivel[30];
	int puerto_nivel;
}   t_nivel;

typedef struct {
	int x;
	int y;
}t_posicion;

typedef struct{
	char id_pj;
	bool existe_dl;
	char id_peticion;
	t_recursos_cantidad* recursosAsignados;
}t_personaje_deadlock;

extern pthread_mutex_t mutex ;

/****otras****/
void create_log();
t_nivel* configuracion_inicializar(char* path);

/****acciones****/
void iniciar_gui(t_nivel* nivel);
void destruir_nivel(t_nivel* nivel);
t_posicion buscar_recurso(char id,ITEM_NIVEL* listaItems);
ITEM_NIVEL* buscar_item(char id,ITEM_NIVEL* listaItems);
void mover_personaje(char id_personaje, ITEM_NIVEL* listaItems, int x, int y);

/****comunicacion con orquestador****/
void conectar_orquestador_y_envio(t_nivel* nivel);//envio de la data del nivel al orquestador
int conectarse_a_orquestador(int* socket_nivel,t_nivel* nivel);

/****comunicacion con pj****/
void solicitar_recurso(t_solicitud* solicitud,ITEM_NIVEL* listaItems,int*socket,t_list** personajes_deadlock);

/****baja de personaje****/
void baja_de_personaje(char id_pj,t_nivel* nivel,ITEM_NIVEL** listaItems);
void cargarRecursosAOrquestador(t_list* recursos_a_orquestador,char id_pj,t_list* personajes_deadlock);
void sumar_recursos(ITEM_NIVEL* ListaItems, char id, int quantity);
t_recursos_cantidad* getRecursoAOrquesador(t_list* recursos_a_orquestador,char id_recurso);
void agregarDisponibles(ITEM_NIVEL** lista_items,t_list* recursos_a_orquestador);
void actualizarPersonajesYActualizarDisponibles(char* personajes,t_list* personajes_deadlock,t_list* recursos_a_orquestador,ITEM_NIVEL** listaItems);

/****poll****/
int aceptar_conexion(struct pollfd* personajes, int* server,int cantidad_personajes);
void array_inicializar(struct pollfd* personajes, int cantidad_personajes);

/****manejo de deadlock****/
void cargarPersonajeAEstructuraDeadLock(char id_pj,t_list* personajes_deadlock,ITEM_NIVEL* lista_items);
int getIndexPersonajeDeadlock(t_list* personajes_deadlock,char id_pj);
t_personaje_deadlock* getPersonajeDeadLock(t_list* personajes_deadlock, char id_pj);
int getIndexRecurso(char id_recurso, t_recursos_cantidad* recursosAsignados);
void* algoritmoDeadLock(void* nivelVoid);
bool checkAndDoDeadLock(t_list* personajes_deadlock,char** interbloqueados);
bool asignacionEsCero(t_recursos_cantidad* recursosAsignados);
bool entraEnDisponible(char id_peticion, t_recursos_cantidad* disponibleAux);
void resetPersonajesToDefault(t_list* personajes_deadlock);
void liberarPersonaje(t_list* personajes_deadlock,char id_pj);
void agregarAInterbloqueados(char** interbloqueados,char id_pj,int* cantidad);

t_log* get_logger();

#endif /* NIVEL_H_ */
