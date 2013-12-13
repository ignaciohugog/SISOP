/*
 * inotify.c
 *
 *  Created on: 28/05/2013
 *      Author: utnso
 */
#include "_inotify.h"
#include "planificador.h"
#define PATH_CONFIG "PLANIFICADOR.cfg"
#define PATH_LOG "log_planificador"
#define BUF_LEN 256

int cambiar_quantum_configuracion(t_log* logger) {

	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor, PATH_CONFIG,
			IN_MODIFY | IN_CREATE | IN_DELETE);

	// El file descriptor creado por inotify, es el que recibe la información sobre los eventos ocurridos
	// para leer esta información el descriptor se lee como si fuera un archivo comun y corriente pero
	// la diferencia esta en que lo que leemos no es el contenido de un archivo sino la información
	// referente a los eventos ocurridos
	int length = read(file_descriptor, buffer, BUF_LEN);
	//int length = 2;
	if (length < 0) {
		perror("read");
	}

	int offset = 0;

	// Luego del read buffer es un array de n posiciones donde cada posición contiene
	// un eventos ( inotify_event ) junto con el nombre de este.
	while (offset < length) {

		// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
		// nombres pueden tener nombres mas cortos que 24 caracteres el tamaño va a ser menor
		// a sizeof( struct inotify_event ) + 24.
		struct inotify_event *event = (struct inotify_event *) &buffer[offset];

		// El campo "len" nos indica la longitud del tamaño del nombre
		//if (event->len) {
		// Dentro de "mask" tenemos el evento que ocurrio y sobre donde ocurrio
		// sea un archivo o un directorio
		if (event->mask & IN_MODIFY) {
			if (!(event->mask & IN_ISDIR)) {

				inotify_rm_watch(file_descriptor, watch_descriptor);
				close(file_descriptor);
				return 1;
			}
			//	}
		}
		offset += sizeof(struct inotify_event) + event->len;
	}

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	return 0;
}

int leerQuantum() {
	int se_modifico;
	while (1) {
		t_log* logger = log_create(PATH_LOG, "Planificador", false,
				LOG_LEVEL_INFO);
		se_modifico = cambiar_quantum_configuracion(logger);

		t_config *configuracion_planificador = config_create(PATH_CONFIG);

		if (se_modifico == 1) {
			//agrego pequeño sleep para retardar la lecutra de la nueva config
			sleep(2);
			char*c = config_get_string_value(configuracion_planificador,
					"Quantum");
			if (c != NULL )

				q = atoi(c);
			se_modifico = 0;
			//TODO pasar a log
			printf("La variable q es %d", q);
			fflush(stdout);

		}
		config_destroy(configuracion_planificador);
		log_destroy(logger);
	}
	return 1;
}

