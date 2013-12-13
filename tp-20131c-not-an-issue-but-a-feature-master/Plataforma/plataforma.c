#include <stdio.h>
#include <comunes/collections/list.h>
#include <comunes/collections/node.h>
#include <comunes/config.h>
#include <pthread.h>
#include "planificador.h"
#include "plataforma.h"
#include "_inotify.h"
#include "Orquestador.h"
#include <comunes/log.h>
#define FALSE 0
#define TRUE 1
#define PATH_CONFIG "PLANIFICADOR.cfg"
#define PATH_LOG "log_plataforma"





//estructura Personajes Bloqueados

t_list*personajesBloqueadosGlobal[VAR][VAR];
t_list* nivelesGlobales;
t_list* recursosGlobales;
t_list* listaRecovery;
t_list* personajes_jugando;

pthread_mutex_t mutex_lista_bloqueados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_orquestador_bloqueados=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log_planificador=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_personajes_jugando =PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_recovery =PTHREAD_MUTEX_INITIALIZER;


int q; //quantum
int tiempo_espera;//tiempo de espera de movimiento del personaje
t_list* listaRecovery;
char* koopa_config = NULL;

int flagCorriendoKoopa=FALSE;

int main(void) {
	t_log* logger;

	pthread_t threadOrquestador;
	pthread_t threadInotfy;
	int i;
	int j;


	nivelesGlobales=list_create();
	recursosGlobales=list_create();
	listaRecovery=list_create();

	logger = log_create(PATH_LOG, "Plataforma", true, LOG_LEVEL_INFO);
	//Inicializo el tiempo  para movimiento del personaje
	read_config_file(logger, PATH_CONFIG);
	//inicializo la estructura global

	for (i = 0; i < VAR; i++) {
		for (j = 0; j < VAR; j++) {
			personajesBloqueadosGlobal[i][j] = list_create();
		}
	}

	//Generacion del hilo inotify

	//Generacion del hilo orquestador

	if (pthread_create(&threadOrquestador, NULL, orquestador_main, NULL )) {
		//fprintf(stderr, "Error creating thread\n");
		log_error(logger, "Error al crear el hilo Orquestador");
		return 1;

	}
	log_info(logger, "Se inició hilo Orquestador");

	if (pthread_create(&threadInotfy, NULL, leerQuantum, NULL )) {
		//fprintf(stderr, "Error creating thread\n");
		log_error(logger,"Error al crear el hilo que lee el cambio de archivo para el Quantum");
		return 1;

	}
	log_info(logger,"Se inició hilo escucha de cambios en archivo de configuración");

	if (pthread_join(threadOrquestador, NULL )) {
		//fprintf(stderr, "Error joining thread\n");
		log_error(logger,"Error al joinear el hilo Orquestador");
		return 2;
	}

	/*if (pthread_create(&threadPlanificador, NULL,
	 plaificador_main, NULL )) {
	 fprintf(stderr, "Error creating thread\n");
	 return 1;
	 }


	 if (pthread_join(threadPlanificador, NULL )) {

	 fprintf(stderr, "Error joining thread\n");
	 return 2;

	 }*/

	//pers=list_get(lista_personajes,0);
	printf("\n");

	//printf("%s\n",pers->ip);

	printf("fin");
	log_info(logger, "Thank you Mario. Your Quest is Over");

	log_destroy(logger);

	return 1;

}
