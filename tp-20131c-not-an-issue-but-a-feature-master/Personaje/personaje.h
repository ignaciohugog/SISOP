/*
 * personaje.h
 *
 *  Created on: 25/04/2013
 *      Author: utnso
 */

#ifndef PERSONAJE_H_
#define PERSONAJE_H_

#include <comunes/collections/list.h>
#include <comunes/log.h>
#include <sockets.h>
#include <Serializadores/serializadores.h>
#include <signal.h>

#define PATH_LOG "log_personaje"
#define TRUE 1
#define FALSE 0
#define CANTIDAD_INTENTOS_RECONEXIONES 100
#define SLEEP_DISTINTOS_FINES 500000
#define RECONEXION_LUEGO_DE_MUERTE 50000


 struct plan_nivel{
	char* nivel;
	t_list* objetivos;
	struct plan_nivel *siguiente;
};
typedef struct plan_nivel t_plan_niveles;

typedef struct {
	int x;
	int y;
}t_posicion;


typedef struct {

	char* nombre;
	char id;
    int vidas_iniciales;
    int vidas_actuales;
	t_plan_niveles* planDeNiveles;
	char ip_orquestador[30];
	int puerto_orquestador;
	t_posicion actual;
	t_posicion destino;
	int* sk_planificador;
	int* sk_nivel;
	t_plan_niveles* nivel_actual;
	t_link_element* nodo_recurso_actual;
	int haceSigterm;
	int bloqueado;

}t_personaje;


/********************CONEXIONES**************************/
void conectarse_a_orquestador(int* socket_personaje);

t_info_nivel* recibir_info_orquestador(t_plan_niveles* nivel_actual, int* socket);

void desconectarme_orquestador(int* socket_personaje);

void conectarme_nivel(char* ip_nivel,int puerto_nivel);

void conectarme_planificador(char* ip_orquestador,int puerto_planificador);

t_info_nivel* conectarme_y_pedir_a_orquestador(t_plan_niveles* nivel_actual);

void personaje_desconexion_plataforma(int* socket);

void personaje_conecta(t_info_nivel* info_nivel);

/********************ACCIONES**************************/

t_posicion* calcular_proximo_movimiento(t_posicion* actual,t_posicion* destino);

void hacer_movimiento(int* socket,t_link_element** nodo_recurso_actual);

char* solicitar_recurso_a_nivel();

t_posicion* solicitar_posicion_recurso(int* socket,char id_recurso);

int avanzar_proximo_objetivo(bool* finaliza_nivel);

bool llega_a_destino(t_posicion actual,t_posicion destino);

void abandonar_nivel(char id_personaje,int* socket);

void personaje_morir();

/********************OTRAS**************************/
t_personaje* configuracion_inicializar(char* path);

void agregar_plan(t_plan_niveles** lista,char* nombre,t_list* list);

void create_log();

void destruir_log();

void destruir_personaje();

void handler (int n);

void loguear_muerte();

t_log* recibir_logger();

int funcionDelSigterm();

#endif /* PERSONAJE_H_ */
