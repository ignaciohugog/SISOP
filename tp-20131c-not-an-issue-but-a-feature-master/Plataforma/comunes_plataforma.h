/*
 * comunes_plataforma.h
 *
 *  Created on: 25/05/2013
 *      Author: utnso
 */


#include <comunes/collections/list.h>
#ifndef COMUNES_PLATAFORMA_H_
#define COMUNES_PLATAFORMA_H_




#endif /* COMUNES_PLATAFORMA_H_ */
int  posNivel(int Nivel) ;
char posRecurso(char recurso) ;
int posicion_recurso(char id_recurso);
//int  meteBloqEnColaRedy(int idNivel, t_list**colaReady, int idNivel, t_log* logger);

void read_config_file(t_log* logger, char* path);
int sacaBloqDeDeadLock(char personajeMate, t_log* logger);
void imprimirListaReady(t_list * lista_personajes_ready,t_log* logger, int  idNivel);
int meteBloqEnColaRedy(int posNivel, t_list**colaReady, int idNivel, t_log* logger);
