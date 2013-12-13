/*
 * funciones.h
 *
 *  Created on: 14/07/2013
 *      Author: utnso
 */

#include <comunes/collections/list.h>
#include <comunes/log.h>
#ifndef FUNCIONES_H_
#define FUNCIONES_H_
#define TRUE 1;
#define FALSE 0;


#endif /* FUNCIONES_H_ */


int validaNivel(int idNivel, t_list * listaNivelesConectados);
int puertoNivel(int idNivel, t_list * listaNivelesConectados);
int todosLosPersonajesTerminaron(t_list* personajes_jugando);
int funcionInfoNivel(int idNivel, t_list * listaNivelesConectados, int *socketNuevaConexion, t_log* logger);
int marcaPersonajesDesbloqueados(int posNivel, int posRecurso, int cantidadLiberada, char** liberados, int*contador, t_log* logger);
void procesar_fin_plan_niveles(int* socketNuevaConexion, t_header* header,t_log* logger);
char  mataPersonaje (char* personajesEnDead, t_log* logger);
int  validaDeadLock  (char* personajesEnDead);
void array_inicializar(struct pollfd* personajes, int cantidad_personajes);
int aceptar_conexion(struct pollfd* personajes, int* server,int cantidad_personajes, t_log* logger);
void imprime_lista_bloqueados(int idNivel, t_log* logger) ;
