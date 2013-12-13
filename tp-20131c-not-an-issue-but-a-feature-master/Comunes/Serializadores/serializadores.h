/*
 * serializadores.h
 *
 *  Created on: 05/05/2013
 *      Author: utnso
 */

#ifndef SERIALIZADORES_H_
#define SERIALIZADORES_H_



#endif /* SERIALIZADORES_H_ */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Stream.h"
#include "../comunes/collections/list.h"



//guarda con esta estructura porque me tengo que guardar ademas el socket
//perteneciente al personaje, ya que como orquestador mata al personaje
//no tiene forma de conectar con este salvo por este socket, que lo va a
//tener planificador en "bloqueados"
//http://www.campusvirtual.frba.utn.edu.ar/especialidad/mod/forum/discuss.php?d=18933
typedef struct {
	char id_personaje;
    int nivel_actual;//Aca me pasa el nivel actual. Tomo este para buscar en la lista de niveles?
} __attribute__((packed)) personajes;

//nivel
typedef struct {
	int id_nivel;
	char* ip;
	int puerto;
}__attribute__((packed)) t_nivel_serializado;

//orquestador
typedef struct {
	int id_nivel;
	char* ip;
	int puerto;
	int puertoPlanificador;
}__attribute__((packed)) t_nivel_deserializado;



typedef struct {
	char id_personaje;
	int x;
	int y;
}__attribute__((packed)) t_movimiento;

typedef struct {
	int x;
	int y;
	char id_recurso;
	char id_pj;
}__attribute__((packed)) t_solicitud;



typedef struct{
	int tipo;
	int tamanio;
}t_header;

typedef struct {
	char* ip_nivel;
	int puerto_nivel;
	int puerto_planificador;
}__attribute__((packed)) t_info_nivel;

typedef struct{
	char id_recurso;
	int cantidad;
}t_recursos_cantidad;


typedef struct{
	 int id_nivel;
	t_list* recurso_cantidad;
}t_recursos;

typedef struct{
	int socket;
	t_header * header;
} t_parametrosKoopa;


//no estoy seguro de personaje_bloqueate
enum{PERSONAJE,NIVEL,BUSCAR_RECURSO,NIVEL_MOVE,SOLICITAR_RECURSO,INFO_NIVEL,RECURSO_OTORGADO,
	NO_HAY_RECURSOS,ABANDONO_PJ,PERSONAJE_MOVIO,PERSONAJE_BLOQUEADO,PERSONAJE_DESBLOQUEATE,
	PERSONAJE_FIN_NIVEL,PERSONAJE_MOVETE,NUEVO_PERSONAJE,RECURSOS_LIBERADOS,
NO_LIBERE,LIBERADOS,FIN_PLAN_NIVELES,INTERBLOQUEADOS,CADAVER,TE_MATE,PERSONAJE_BLOQ,IM_READY};//tipos






//no hace falta hacer deserializador, porque son dos char, uno para saber el tipo y otro el id de recurso

t_stream* serializar_pedido_posicion_recurso(int x, int y);//no se usa

t_stream* serializar_pedido_personaje_movete();//no se usa

t_header* deserializar_header(char* stream);

t_stream* serializar_pedido_buscar_recurso(char id_recurso);//no se deserealiza
t_stream* serializar_pedido_nivel_move(char id_personaje,int x, int y);
t_movimiento* deserealizar_pedido_nivel_move(char* stream);
t_stream* serializar_pedido_solicitud_recurso(int x,int y,char id_recurso,char id_pj);
t_solicitud* deserealizar_pedido_solicitud_recurso(char* data);

t_stream* serializar_abandono_pj(char id_personaje);



t_stream* serializar_cadaver(char id_personaje);


t_stream* recurso_serializar(t_recursos_cantidad* recurso);
t_stream* lista_liberados_serialize(t_list* recursos,int id_nivel);
t_recursos* lista_liberados_deserialize(char* stream, int* size);
t_recursos_cantidad* recurso_deserialize(char* stream, int* size);

t_stream* serializar_liberados(char* liberados);
char* deserializar_liberados(char* liberados);

t_stream* serializar_interbloqueados(char* interbloqueados);
char* deserializar_interbloqueados(char* interbloqueados);

t_stream* serializador_header_no_libere();
t_stream* serializador_header_fin_nivel(char id_personaje);


t_stream* serializador_nivel(t_nivel_serializado* nivel_a_serializar );
t_nivel_deserializado* nivel_deserialize(char* stream) ;


//Nacho estas funciones
//Serian para cuando el personaje se conecta con el orquestador

t_stream* serializador_personaje(personajes* personaje_a_serializar );
personajes* personaje_deserialize(char* stream) ;

t_stream* serializador_info_nivel(t_info_nivel* info_nivel_a_serializar);
t_info_nivel* info_nivel_deserialize(char* stream);

