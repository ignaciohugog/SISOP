#include "serializadores.h"




t_header* deserializar_header(char* stream){
	int offset=0;
	t_header* header=malloc(sizeof(t_header));

	memcpy(&header->tipo, stream + offset,
			sizeof(int));
	offset += sizeof(int);
	memcpy(&header->tamanio, stream + offset,
			sizeof(int));



return header;
}



t_stream* serializador_nivel(t_nivel_serializado* nivel_a_serializar) {

	int offset = 0;

	int size=sizeof(int)+sizeof(int)+//del tipo y tamanio (header)
			 + sizeof(nivel_a_serializar->id_nivel)
							+ (strlen(nivel_a_serializar->ip) + 1)
							+ sizeof(nivel_a_serializar->puerto);
	t_stream* stream = stream_create(size);

	int *nivel=malloc(sizeof(int));

	*nivel=NIVEL;

	//tipo
	memcpy(stream->data + offset, nivel,
			sizeof(int));
	offset += sizeof(int);
	//tamanio
	int *tamanio=malloc(sizeof(int));

		*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

	memcpy(stream->data + offset, tamanio,
			sizeof(int));
	offset += sizeof(int);

	memcpy(stream->data + offset, &nivel_a_serializar->id_nivel,
			sizeof(nivel_a_serializar->id_nivel));
	offset += sizeof(nivel_a_serializar->id_nivel);


	memcpy(stream->data + offset, nivel_a_serializar->ip,
			strlen(nivel_a_serializar->ip) + 1);
	offset += strlen(nivel_a_serializar->ip) + 1;

	memcpy(stream->data + offset, &nivel_a_serializar->puerto,
			sizeof(nivel_a_serializar->puerto));
	offset += sizeof(nivel_a_serializar->puerto);
free(tamanio);
free(nivel);
	return stream;
}

t_nivel_deserializado* nivel_deserialize(char* stream) {
	t_nivel_deserializado* deserealizado = malloc(
			sizeof(t_nivel_deserializado));
	int offset = 0; //porque el primer byte era flag para saber que desearilizar, nivel o pj

	memcpy(&deserealizado->id_nivel, stream + offset,
			sizeof(deserealizado->id_nivel));
	offset += sizeof(deserealizado->id_nivel);

	char* ip = strdup(stream + offset);
	deserealizado->ip = ip;
	offset += strlen(ip) + 1;

	memcpy(&deserealizado->puerto, stream + offset,
			sizeof(deserealizado->puerto));
	offset += sizeof(deserealizado->puerto);

	return deserealizado;
}

//t_stream* serializar_pedido_buscar_recurso(char id) {
//	t_stream* stream = stream_create(sizeof(char) + sizeof(char));
//
//	stream->data[0] = BUSCAR_RECURSO;
//	stream->data[1] = id;
//	return stream;
//
//}
//t_stream* serializar_pedido_personaje_movete() {
//	t_stream* stream = stream_create(sizeof(char));
//	stream->data[0] = PERSONAJE_MOVETE;
//	return stream;
//}
//t_stream* serializar_pedido_posicion_recurso(int x, int y){
//	t_stream* stream = stream_create(sizeof(int) + sizeof(int));
//	memcpy(stream->data, &x,sizeof(int));
//	memcpy(stream->data+sizeof(int), &y,sizeof(int));
//	return stream;
//}

t_stream* serializador_personaje(personajes * personaje_a_serializar) {

	int offset = 0;

	int size= sizeof(int)+sizeof(int)+sizeof(personaje_a_serializar->id_personaje)

								+ sizeof(personaje_a_serializar->nivel_actual);

		t_stream* stream = stream_create(size);

		int*tipo=malloc(sizeof(int));
		*tipo=PERSONAJE;

		memcpy(stream->data+offset,tipo,sizeof(int));
		offset+=sizeof(int);

		int *tamanio=malloc(sizeof(int));

		*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

		memcpy(stream->data+offset,tamanio,sizeof(int));
		offset+=sizeof(int);

		memcpy(stream->data + offset, &personaje_a_serializar->id_personaje,
				sizeof(personaje_a_serializar->id_personaje));
		offset += sizeof(personaje_a_serializar->id_personaje);

		memcpy(stream->data + offset, &personaje_a_serializar->nivel_actual,
				sizeof(personaje_a_serializar->nivel_actual));
		offset += sizeof(personaje_a_serializar->nivel_actual);



		return stream;
}

personajes* personaje_deserialize(char* stream) {


	personajes* deserealizado = malloc(
			sizeof(personajes));

	int offset = 0;

	memcpy(&deserealizado->id_personaje, stream + offset,
			sizeof(deserealizado->id_personaje));
	offset += sizeof(deserealizado->id_personaje);




	memcpy(&deserealizado->nivel_actual, stream + offset,
				sizeof(deserealizado->nivel_actual));
		offset += sizeof(deserealizado->nivel_actual);

	return deserealizado;
}

t_stream* serializador_info_nivel(t_info_nivel* info_nivel_a_serializar){

	int offset = 0;

	int size = sizeof(int) + sizeof(int) + //del tipo y tamanio (header)
			 + (strlen (info_nivel_a_serializar->ip_nivel) +1 )
			 + sizeof(info_nivel_a_serializar->puerto_nivel)
			 + sizeof(info_nivel_a_serializar->puerto_planificador);
	t_stream* stream = stream_create(size);

	int *info_nivel = malloc (sizeof(int));
	*info_nivel = INFO_NIVEL;

	//tipo
	memcpy(stream->data + offset, info_nivel,sizeof(int));
	offset += sizeof(int);

	//tamanio
	int *tamanio=malloc(sizeof(int));
	*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

	memcpy(stream->data + offset, tamanio,sizeof(int));
	offset += sizeof(int);

	memcpy(stream->data + offset, info_nivel_a_serializar->ip_nivel,
			strlen(info_nivel_a_serializar->ip_nivel) +1 );
	offset += strlen(info_nivel_a_serializar->ip_nivel) +1;

	memcpy(stream->data + offset, &info_nivel_a_serializar->puerto_nivel,
			sizeof(info_nivel_a_serializar->puerto_nivel));
	offset += sizeof(info_nivel_a_serializar->puerto_nivel);

	memcpy(stream->data + offset, &info_nivel_a_serializar->puerto_planificador,
			sizeof(info_nivel_a_serializar->puerto_planificador));
	offset += sizeof(info_nivel_a_serializar->puerto_planificador);

	free(tamanio);
	free(info_nivel);

	return stream;
}

t_info_nivel* info_nivel_deserialize(char* stream){

	t_info_nivel* deserializado = malloc(sizeof(t_info_nivel));
	int offset = 0;

	char*ip=strdup(stream+offset);
	deserializado->ip_nivel=ip;
	offset+=strlen(ip)+1;



	memcpy(&deserializado->puerto_nivel, stream + offset,
			sizeof(deserializado->puerto_nivel));
	offset += sizeof(deserializado->puerto_nivel);

	memcpy(&deserializado->puerto_planificador, stream + offset,
			sizeof(deserializado->puerto_planificador));
	offset += sizeof(deserializado->puerto_planificador);

	return deserializado;
}

t_stream* serializar_pedido_nivel_move(char id_personaje,int x,int y){
	int offset = 0;

		int size=sizeof(int)+sizeof(int)//del tipo y tamanio (header)
				 + sizeof(int)*2 //x,y
				+ sizeof(char);//id
		t_stream* stream = stream_create(size);

		int *tipo=malloc(sizeof(int));

		*tipo=NIVEL_MOVE;

		//tipo
		memcpy(stream->data + offset, tipo,
				sizeof(int));
		offset += sizeof(int);
		//tamanio
		int *tamanio=malloc(sizeof(int));

			*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

		memcpy(stream->data + offset, tamanio,
				sizeof(int));
		offset += sizeof(int);
		memcpy(stream->data + offset, &id_personaje,sizeof(char));
				offset += sizeof(char);
		memcpy(stream->data + offset, &x,sizeof(int));
		offset += sizeof(int);
		memcpy(stream->data + offset, &y,sizeof(int));
				offset += sizeof(int);

	free(tamanio);
	free(tipo);
    return stream;
	}

t_movimiento* deserealizar_pedido_nivel_move(char* stream){
	t_movimiento* deserealizado = malloc(
				sizeof(t_movimiento));
		int offset = 0;

		memcpy(&deserealizado->id_personaje, stream + offset,sizeof(deserealizado->id_personaje));
		offset += sizeof(deserealizado->id_personaje);

		memcpy(&deserealizado->x, stream + offset,sizeof(deserealizado->x));
		offset += sizeof(deserealizado->x);
		memcpy(&deserealizado->y, stream + offset,sizeof(deserealizado->y));
		offset += sizeof(deserealizado->y);
		return deserealizado;
          }

t_stream* serializar_pedido_buscar_recurso(char id_recurso){
	int offset = 0;

			int size=sizeof(int)+sizeof(int)//del tipo y tamanio (header)
					+ sizeof(char);//id
			t_stream* stream = stream_create(size);

			int *tipo=malloc(sizeof(int));

			*tipo=BUSCAR_RECURSO;

			//tipo
			memcpy(stream->data + offset, tipo,
					sizeof(int));
			offset += sizeof(int);
			//tamanio
			int *tamanio=malloc(sizeof(int));

				*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

			memcpy(stream->data + offset, tamanio,
					sizeof(int));
			offset += sizeof(int);
			memcpy(stream->data + offset, &id_recurso,sizeof(char));
					offset += sizeof(char);

					free(tamanio);
					free(tipo);
				    return stream;
}

t_stream* serializar_pedido_solicitud_recurso(int x, int y, char id_recurso, char id_pj) {
	int offset = 0;

	int size = sizeof(int) + sizeof(int) + //del tipo y tamanio (header)
			+sizeof(char) + sizeof(int) + sizeof(int) + sizeof(char) ;

	t_stream* stream = stream_create(size);

	int *tipo = malloc(sizeof(int));
	*tipo = SOLICITAR_RECURSO;
	//tipo
	memcpy(stream->data + offset, tipo, sizeof(int));
	offset += sizeof(int);
	//tamanio
	int *tamanio = malloc(sizeof(int));
	*tamanio = ((stream->size) - (sizeof(int) + sizeof(int)));

	memcpy(stream->data + offset, tamanio, sizeof(int));
	offset += sizeof(int);

	memcpy(stream->data + offset, &x, sizeof(int));
	offset += sizeof(int);
	memcpy(stream->data + offset, &y, sizeof(int));
	offset += sizeof(int);
	memcpy(stream->data + offset, &id_recurso, sizeof(char));
	offset += sizeof(char);
	memcpy(stream->data + offset, &id_pj, sizeof(char));
	offset += sizeof(char);

	free(tamanio);
	free(tipo);
	return stream;
}

t_solicitud* deserealizar_pedido_solicitud_recurso(char* stream){
	t_solicitud* deserealizado = malloc(sizeof(t_solicitud));
	int offset = 0;

	memcpy(&deserealizado->x, stream + offset, sizeof(deserealizado->x));
	offset += sizeof(deserealizado->x);
	memcpy(&deserealizado->y, stream + offset, sizeof(deserealizado->y));
	offset += sizeof(deserealizado->y);

	memcpy(&deserealizado->id_recurso, stream + offset,
			sizeof(deserealizado->id_recurso));
	offset += sizeof(deserealizado->id_recurso);

	memcpy(&deserealizado->id_pj, stream + offset,
			sizeof(deserealizado->id_pj));
	offset += sizeof(deserealizado->id_pj);

	return deserealizado;
}

t_stream* serializar_abandono_pj(char id_personaje){

int offset = 0;

			int size=sizeof(int)+sizeof(int)//del tipo y tamanio (header)
					+ sizeof(char);//id
			t_stream* stream = stream_create(size);

			int *tipo=malloc(sizeof(int));

			*tipo=ABANDONO_PJ;

			//tipo
			memcpy(stream->data + offset, tipo,
					sizeof(int));
			offset += sizeof(int);
			//tamanio
			int *tamanio=malloc(sizeof(int));

				*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

			memcpy(stream->data + offset, tamanio,
					sizeof(int));
			offset += sizeof(int);
			memcpy(stream->data + offset, &id_personaje,sizeof(char));
					offset += sizeof(char);

					free(tamanio);
					free(tipo);
				    return stream;
}



t_stream* recurso_serializar(t_recursos_cantidad* recurso){

	t_stream *stream = stream_create(sizeof(t_recursos_cantidad));
	memcpy(stream->data, recurso, sizeof(t_recursos_cantidad));
	return stream;

}


t_stream* lista_liberados_serialize(t_list* recursos,int id_nivel) {
    int offset = 0;

    int size = sizeof(int) + sizeof(int) + //del tipo y tamanio (header)
            +sizeof(int);
    t_stream* stream = stream_create(size);

    int *tipo = malloc(sizeof(int));

    *tipo = RECURSOS_LIBERADOS;

    //tipo
    memcpy(stream->data + offset, tipo,
            sizeof(int));
    offset += sizeof(int);
    //tamanio
    int *tamanio = malloc(sizeof(int));

    *tamanio = ((stream->size) - (sizeof(int) + sizeof(int)) +
            (sizeof(t_recursos_cantidad)) * (recursos->elements_count) + sizeof(int)); //2

    memcpy(stream->data + offset, tamanio, sizeof(int));
    offset += sizeof(int);

    memcpy(stream->data + offset, &id_nivel, sizeof(int));
    offset += sizeof(id_nivel);

    void serialize_element_recurso(void* element) {
        t_recursos_cantidad* recurso = element;

        t_stream* stream_recurso = recurso_serializar(recurso);
        stream->data = realloc(stream->data, stream->size + stream_recurso->size);
        memcpy(stream->data + offset, stream_recurso->data, stream_recurso->size);
        stream->size += stream_recurso->size;
        offset += stream_recurso->size;
        stream_destroy(stream_recurso);
    }

    int count_recursos = list_size(recursos);
    stream->data = realloc(stream->data, stream->size + sizeof(int));
    memcpy(stream->data + offset, &count_recursos, sizeof(int));
    stream->size += sizeof(int);
    offset += sizeof(int);

    list_iterate(recursos, serialize_element_recurso);

    return stream;
}


t_recursos* lista_liberados_deserialize(char* stream, int* size) {
	t_recursos* deserealizado= malloc(sizeof(t_recursos));
	int offset = 0, tmp_size = 0, i = 0;
	memcpy(&deserealizado->id_nivel, stream + offset,tmp_size=
				sizeof(deserealizado->id_nivel));
		offset += tmp_size;


	int elements_recurso = 0;
	memcpy(&elements_recurso, stream + offset, sizeof(elements_recurso));
	offset += sizeof(elements_recurso);

	t_list* recursos = list_create();
	for (i = 0; i < elements_recurso; ++i) {
		t_recursos_cantidad* recurso = recurso_deserialize(stream + offset, &tmp_size);
		offset += tmp_size;
		list_add(recursos, recurso);
	}
	deserealizado->recurso_cantidad = recursos;

	*size = offset;

	return deserealizado;
}

t_recursos_cantidad* recurso_deserialize(char* stream, int* size) {
	t_recursos_cantidad *recurso = malloc(sizeof(t_recursos_cantidad));
	memcpy(recurso, stream, sizeof(t_recursos_cantidad));

	*size = sizeof(t_recursos_cantidad);

	return recurso;
}


t_stream* serializar_liberados(char* liberados){

	int offset = 0;
	int sizeLiberados = 0;
	sizeLiberados = strlen(liberados) + 1;

	int size=sizeof(int)+sizeof(int)+//del tipo y tamanio (header)
							+ (sizeLiberados);
	t_stream* stream = stream_create(size);

	int tipo=LIBERADOS;

	//tipo
	memcpy(stream->data + offset, &tipo,sizeof(int));
	offset += sizeof(int);
	//tamanio
	int tamanio = 0;
	tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

	memcpy(stream->data + offset,&tamanio,sizeof(int));
	offset += sizeof(int);

	memcpy(stream->data + offset, liberados,sizeLiberados);
	offset += sizeLiberados;

	return stream;
}

t_stream* serializar_interbloqueados(char* interbloqueados){

	int offset = 0;

	int size=sizeof(int)+sizeof(int)+//del tipo y tamanio (header)
							+ (strlen(interbloqueados) + 1);
	t_stream* stream = stream_create(size);

	int *tipo=malloc(sizeof(int));

	*tipo=INTERBLOQUEADOS;

	//tipo
	memcpy(stream->data + offset, tipo,
			sizeof(int));
	offset += sizeof(int);
	//tamanio
	int *tamanio=malloc(sizeof(int));

		*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

	memcpy(stream->data + offset, tamanio,
			sizeof(int));
	offset += sizeof(int);



	memcpy(stream->data + offset, interbloqueados,
			strlen(interbloqueados) + 1);
	offset += strlen(interbloqueados) + 1;

free(tamanio);
free(tipo);
	return stream;
}

char* deserializar_interbloqueados(char* interbloqueados){
	int offset = 0;
	char* deserealizado;

	char* personajes = strdup(interbloqueados + offset);
	deserealizado = personajes;
	offset += strlen(personajes) + 1;
	//deserealizado[offset+1]='\0';
	return deserealizado;
}

char* deserializar_liberados(char* liberados) {
	int offset = 0;
	char* deserealizado;

	char* personajes = strdup(liberados + offset);
	deserealizado = personajes;
	offset += strlen(personajes) + 1;
//deserealizado[offset+1]='\0';
	return deserealizado;
}


t_stream* serializador_header_no_libere() {

	int offset = 0;

	int size=sizeof(int)+sizeof(int);//del tipo y tamanio (header)

	t_stream* stream = stream_create(size);

	int *tipo=malloc(sizeof(int));

	*tipo=NO_LIBERE;

	//tipo
	memcpy(stream->data + offset, tipo,
			sizeof(int));
	offset += sizeof(int);
	//tamanio
	int *tamanio=malloc(sizeof(int));

		*tamanio=((sizeof(int)+sizeof(int)));

	memcpy(stream->data + offset, tamanio,
			sizeof(int));
	offset += sizeof(int);

	return stream;

}

t_stream* serializador_header_fin_nivel(char id_personaje){
	int offset = 0;
	int size=sizeof(int)+sizeof(int)+sizeof(char);//del tipo y tamanio (header)
	t_stream* stream = stream_create(size);
	int *tipo=malloc(sizeof(int));
	*tipo=FIN_PLAN_NIVELES;
	//tipo
	memcpy(stream->data + offset, tipo, sizeof(int));
	offset += sizeof(int);
	//tamanio
	int *tamanio=malloc(sizeof(int));
	*tamanio=sizeof(char);
	memcpy(stream->data + offset, tamanio, sizeof(int));
	offset += sizeof(int);
	memcpy(stream->data + offset, &id_personaje, sizeof(char));
	offset += sizeof(char);

	free(tamanio);
	free(tipo);
	return stream;
}
t_stream* serializar_cadaver(char id_personaje){

int offset = 0;

			int size=sizeof(int)+sizeof(int)//del tipo y tamanio (header)
					+ sizeof(char);//id
			t_stream* stream = stream_create(size);

			int *tipo=malloc(sizeof(int));

			*tipo=ABANDONO_PJ;

			//tipo
			memcpy(stream->data + offset, tipo,
					sizeof(int));
			offset += sizeof(int);
			//tamanio
			int *tamanio=malloc(sizeof(int));

				*tamanio=((stream->size)-(sizeof(int)+sizeof(int)));

			memcpy(stream->data + offset, tamanio,
					sizeof(int));
			offset += sizeof(int);
			memcpy(stream->data + offset, &id_personaje,sizeof(char));
					offset += sizeof(char);

					free(tamanio);
					free(tipo);
				    return stream;
}
