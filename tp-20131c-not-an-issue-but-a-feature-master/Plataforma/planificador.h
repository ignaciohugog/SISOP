/*
 * planificador.h
 *
 *  Created on: 21/04/2013
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_



#endif /* PLANIFICADOR_H_ */
#include<pthread.h>
#include<poll.h>
extern int q; //quantum
extern int tiempo_espera;//tiempo espera del movimiento del personaje
void* plaificador_main ();
