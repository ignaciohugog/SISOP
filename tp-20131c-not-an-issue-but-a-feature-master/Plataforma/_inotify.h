/*
 * inotify.h
 *
 *  Created on: 28/05/2013
 *      Author: utnso
 */


#ifndef INOTIFY_H_
#define INOTIFY_H_

#include <comunes/log.h>
#include <Serializadores/serializadores.h>
#include "planificador.h"
//#include "plataforma.h"
#include "comunes_plataforma.h"
#include <unistd.h>
#include <sys/inotify.h>
#include <comunes/config.h>

//
//
int cambiar_quantum_configuracion(t_log* logger) ;

int leerQuantum() ;


#endif /* INOTIFY_H_ */
