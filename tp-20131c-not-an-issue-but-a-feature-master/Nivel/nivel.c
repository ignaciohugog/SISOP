#define PATH "NIVEL.cfg"
#include "nivel.h"
#include "tad_items.h"
#include <sockets.h>
#include <unistd.h>
#include <pthread.h>

t_log* logger;
int cantidad_key;
t_recursos_cantidad* disponibleAux;

/****************OTRAS*********************/
t_log* get_logger() {
    return logger;
}

void create_log(){
	logger = log_create(PATH_LOG, "Nivel", false, LOG_LEVEL_INFO);
}

t_nivel* configuracion_inicializar(char* path) {

	t_nivel* nivel = malloc(sizeof(t_nivel));
	nivel->lista_items = NULL;
	t_config* configuracion_nivel = config_create(path);
//nombre
	nivel->nombre = string_duplicate(
			config_get_string_value(configuracion_nivel, "Nombre"));
//deadlock
	nivel->deadlock = config_get_int_value(configuracion_nivel,
			"TiempoChequeoDeadlock");
//recovery
	nivel->recovery = config_get_int_value(configuracion_nivel, "Recovery");
//ip y puerto
	char* ip_puerto = config_get_string_value(configuracion_nivel,
			"orquestador");
	char**ip = string_split(ip_puerto, ":");
	strcpy(nivel->ip, ip[0]);
	//puerto
	char* puerto = ip[1];
	nivel->puerto = atoi(puerto);
	free(ip[0]);
	free(ip[1]);
	free(ip);

	//ip y puerto del nivel todo get ip
		char* ip_puerto_nivel= config_get_string_value(configuracion_nivel,
				"ip_nivel");
		char**ip_nivel = string_split(ip_puerto_nivel, ":");
		strcpy(nivel->ip_nivel, ip_nivel[0]);
		//puerto
		char* puerto_nivel = ip_nivel[1];
		nivel->puerto_nivel = atoi(puerto_nivel);
		free(ip_nivel[0]);
		free(ip_nivel[1]);
		free(ip_nivel);

	//cantidad de key en el archivo, que son de cajas nada mas, resto 6 por ahora
	cantidad_key = config_keys_amount(configuracion_nivel) - 5;
	//los numeros de caja empiezan de 1 y van en orden(siempre)
	int count;
	for (count = 1; count <= cantidad_key; count++) {
		//armo las keys de las cajas
		char* key = string_from_format("Caja %d", count);

		if (config_has_property(configuracion_nivel, key)) {

			char* caja = (char*) config_get_string_value(configuracion_nivel,
					key);
			char** array_values = string_split(caja, ",");
//del array saco el char del string que me viene
			char id = array_values[1][1];

			CrearCaja(&(nivel->lista_items), id, atoi(array_values[3]),
					atoi(array_values[4]), atoi(array_values[2]));
		}
	}
	config_destroy(configuracion_nivel);
	nivel->personajes_deadlock = list_create();
	log_info(logger, "se ha inicializado un nivel");
	return nivel;
}

/****************ACCIONES DE NIVEL*********************/

void iniciar_gui(t_nivel* nivel){
	int rows, cols;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&rows, &cols);
	nivel_gui_dibujar(nivel->lista_items);
}

void destruir_nivel(t_nivel* nivelX) {
	list_destroy(nivelX->personajes_deadlock);
//borrar items
	ITEM_NIVEL* self = nivelX->lista_items;
	ITEM_NIVEL* element;
	while (self != NULL ) {
		element = self;
		self = self->next;
		free(element);
	}
	free(nivelX->lista_items);
	free(nivelX);
}

t_posicion buscar_recurso(char id, ITEM_NIVEL* listaItems) {
	ITEM_NIVEL * temp;
	temp = listaItems;
	t_posicion pos;
	while ((temp != NULL )&& (temp->id != id)){
		temp = temp->next;
	}
	if ((temp != NULL )&& (temp->id == id)){
		pos.x=temp->posx;
		pos.y=temp->posy;
	}

	return pos;
}

ITEM_NIVEL* buscar_item(char id,ITEM_NIVEL* listaItems){
	ITEM_NIVEL * temp;
	temp = listaItems;

	while ((temp != NULL) && (temp->id != id)) {
			temp = temp->next;
	}

	if ((temp != NULL) && (temp->id == id)) {
		return temp;
	}
	else return NULL;
}

void mover_personaje(char id_personaje, ITEM_NIVEL* listaItems, int x, int y) {
//TODO validar los margenes

	MoverPersonaje(listaItems, id_personaje,x, y);
}

/****************COMUNICACION CON ORQUESTADOR*********************/

void conectar_orquestador_y_envio(t_nivel* nivel){
    log_trace(logger, "[conectar_orquestador_y_envio] Iniciando ...");

    log_trace(logger, "[conectar_orquestador_y_envio] Creando socket sin puerto ...");
    int* socket = socket_create_sin_puerto(logger);

    log_trace(logger, "[conectar_orquestador_y_envio] Conectando al orquestador ...");
    if(socket_connect_with_retry(socket, nivel->ip, nivel->puerto, logger)) {
        log_error(logger, "{%s} [conectar_orquestador_y_envio] No pudo conectar",nivel->nombre);
        exit(EXIT_FAILURE);
    }
    log_trace(logger, "[conectar_orquestador_y_envio] Conexion al orquestador exitosa! ...");

//creo estructura para serializar
	t_nivel_serializado serializado;
	serializado.id_nivel = parsear_nivel(nivel->nombre);
	serializado.ip = nivel->ip_nivel;
	serializado.puerto =nivel->puerto_nivel;
	t_stream* stream = serializador_nivel(&serializado);
	log_trace(logger, "[conectar_orquestador_y_envio] Serializacion realizada para envio ...");
//envio al orquestador
	log_trace(logger, "[conectar_orquestador_y_envio] Sending al orquestador con la data del nivel ...");
	if (sendBytes(socket, stream->data, stream->size, logger) < 0) {
		log_error(logger, "{%s} [conectar_orquestador_y_envio] Fallo con el envio al orquestador de la data de nivel..." ,nivel->nombre);
		/*log_error(logger, "{%s} [conectar_orquestador_y_envio] Fallo con el envio al orquestador de la data de nivel, CERRANDOME...",nivel->nombre);
		exit(EXIT_FAILURE);*/
	} else{

		log_info(logger, "[conectar_orquestador_y_envio] Sending con la data del nivel realizada ...");
	}

	log_trace(logger, "[conectar_orquestador_y_envio] Stream destroy ...");
	stream_destroy(stream);

	socket_close(socket, logger);
	log_trace(logger, "[conectar_orquestador_y_envio] Saliendo ...");
}

int conectarse_a_orquestador(int* socket_nivel,t_nivel* nivel){
    log_trace(logger, "Procesando [conectarse_a_orquestador]...");
    if(socket_connect_with_retry(socket_nivel, nivel->ip, nivel->puerto, logger)) {
           log_error(logger, "[conectar_orquestador_y_envio] No pudo conectar");
           exit(EXIT_FAILURE);
       }
    log_trace(logger, "Saliendo [conectarse_a_orquestador]...");


	return 0;
}

/****************COMUNICACION CON EL PJ*********************/

void solicitar_recurso(t_solicitud* solicitud,ITEM_NIVEL* listaItems,int*socket,t_list** personajes_deadlock){
	ITEM_NIVEL* recurso = buscar_item(solicitud->id_recurso, listaItems);

	int* data = malloc(sizeof(int));

	if((recurso->posx == solicitud->x)	&& (recurso->posy == solicitud->y)){
		t_personaje_deadlock* pj_dl = getPersonajeDeadLock(*personajes_deadlock,solicitud->id_pj);

		int i = getIndexRecurso(solicitud->id_recurso,pj_dl->recursosAsignados);

		if (recurso->quantity > 0) {
			//todo meter las dos fx en una sola
			restarRecurso(listaItems, solicitud->id_recurso);
			pj_dl->recursosAsignados[i].cantidad++;
			data[0]=RECURSO_OTORGADO;
			sendBytes(socket,data,sizeof(int), logger);
		}else{

			pj_dl->id_peticion = recurso->id;

			data[0]=NO_HAY_RECURSOS;
			sendBytes(socket,data,sizeof(int), logger);
		}
	}else{
		//no estaba en la posicion indicada
	}

}

/****************BAJA DE PERSONAJE*********************/

void baja_de_personaje(char id_pj,t_nivel* nivel,ITEM_NIVEL** listaItems){
    log_info(logger, "Ejecutando [baja_de_personaje]...");
	t_list* recursos_a_orquestador = list_create();
    log_info(logger, "[baja_de_personaje] Cargando recursos a orquestador...");
	cargarRecursosAOrquestador(recursos_a_orquestador,id_pj,nivel->personajes_deadlock);

	//int* socket = socket_create_sin_puerto();
	int* socket = socket_create(15000, logger);
	log_trace(logger, "Procesando [conectarse_a_orquestador]...");

	 if (socket_connect_with_retry(socket, nivel->ip, nivel->puerto, logger) < 0) {
		log_error(logger, "[baja_de_personaje] No pudo conectar");
		exit(EXIT_FAILURE);
	}
	log_trace(logger, "Saliendo [baja_de_personaje]...");


	int id_nivel = parsear_nivel(nivel->nombre);
	t_stream* stream=lista_liberados_serialize(recursos_a_orquestador,id_nivel);
	log_info(logger, "[baja_de_personaje] Sending bytes...");
	sendBytes(socket,stream->data,stream->size, logger);

	log_info(logger, "[baja_de_personaje] Borrando item para id_pj=[%c]...", id_pj);
	BorrarItem(&nivel->lista_items,id_pj);

	liberarPersonaje(nivel->personajes_deadlock,id_pj);
	log_info(logger, "[baja_de_personaje] recibiendo del orquestador posibles personajes desbloqueados...");
	char* data=receiveBytes(socket,sizeof(int)*2, logger);
	if(data!=NULL){
	    log_info(logger, "[baja_de_personaje] Deserializando data...");
		t_header* header=(t_header*)deserializar_header(data);
		switch (header->tipo){
			case NO_LIBERE:
			{
			    log_info(logger, "[baja_de_personaje] Agregando a disponibles...");
				agregarDisponibles(listaItems,recursos_a_orquestador);
				break;
			}
			case LIBERADOS:
			{
			    log_info(logger, "[baja_de_personaje] Liberados -> Recibiendo del orquestador Personajes Desbloqueados...");
				data = receiveBytes(socket,header->tamanio, logger);
				char* personajes = deserializar_liberados(data);

				actualizarPersonajesYActualizarDisponibles(personajes,nivel->personajes_deadlock,recursos_a_orquestador,listaItems);
				break;
			}
		}
	}

	log_info(logger, "[baja_de_personaje] cerrando socket=[%d]...", *socket);
	socket_close(socket, logger);
	log_info(logger, "Saliendo [baja_de_personaje]...");
}

void cargarRecursosAOrquestador(t_list* recursos_a_orquestador,char id_pj,t_list* personajes_deadlock){
	t_personaje_deadlock* pj_dl = getPersonajeDeadLock(personajes_deadlock,id_pj);
	int i;
	for(i = 0 ; i < cantidad_key ; i++){
		if(pj_dl->recursosAsignados[i].cantidad != 0){
			t_recursos_cantidad* rec_cant = malloc(sizeof(t_recursos_cantidad));
			rec_cant->id_recurso = pj_dl->recursosAsignados[i].id_recurso;
			rec_cant->cantidad = pj_dl->recursosAsignados[i].cantidad;
			list_add(recursos_a_orquestador,rec_cant);
		}
	}
}

t_recursos_cantidad* getRecursoAOrquesador(t_list* recursos_a_orquestador,char id_recurso){
	int index = 0;
	t_recursos_cantidad* recurso_cantidad = list_get(recursos_a_orquestador,index);
	while(id_recurso != recurso_cantidad->id_recurso){
		index++;
		recurso_cantidad = list_get(recursos_a_orquestador,index);
	}
	return recurso_cantidad;
}

void sumar_recursos(ITEM_NIVEL* ListaItems, char id, int quantity) {
	ITEM_NIVEL * temp;
	temp = ListaItems;
	while ((temp != NULL) && (temp->id != id)) {
    	temp = temp->next;
	}

	if ((temp != NULL) && (temp->id == id)) {
    	if (temp->item_type) {
        	temp->quantity = temp->quantity + quantity;
    	}
	}
}

void agregarDisponibles(ITEM_NIVEL** lista_items,t_list* recursos_a_orquestador){
	int i;
	for(i = 0 ; i < recursos_a_orquestador->elements_count ; i++){
		t_recursos_cantidad* recurso_cantidad = list_get(recursos_a_orquestador,i);
		sumar_recursos(*lista_items,recurso_cantidad->id_recurso,recurso_cantidad->cantidad);
	}
}

void actualizarPersonajesYActualizarDisponibles(char* personajes,t_list* personajes_deadlock,t_list* recursos_a_orquestador,ITEM_NIVEL** listaItems){
    log_info(logger, "Ejecutando [actualizarPersonajesYActualizarDisponibles]...");
	int maximo= strlen(personajes);
	int i;
	for(i = 0 ; i < maximo ; i++){
		t_personaje_deadlock* pj_deadlock = getPersonajeDeadLock(personajes_deadlock,personajes[i]);
		t_recursos_cantidad* recurso_cantidad = getRecursoAOrquesador(recursos_a_orquestador,pj_deadlock->id_peticion);
		recurso_cantidad->cantidad--;
		int j = getIndexRecurso(pj_deadlock->id_peticion,pj_deadlock->recursosAsignados);
		pj_deadlock->recursosAsignados[j].cantidad++;
		pj_deadlock->id_peticion = '0';
	}
	agregarDisponibles(listaItems,recursos_a_orquestador);

	log_info(logger, "Saliendo [actualizarPersonajesYActualizarDisponibles]...");
}


/*************************POLL**************************/

int aceptar_conexion(struct pollfd* personajes, int* server,int cantidad_personajes) {
	int i = 0, indice = cantidad_personajes;
	while (i <= indice) {
		if (personajes[i].fd == 0) {
			indice = i;
			personajes[indice].fd = *socket_server_accept(server, logger);
		}
		i++;
	}
	return indice;
}

void array_inicializar(struct pollfd* personajes, int cantidad_personajes) {
	int i;
	for (i = 0; i < cantidad_personajes; i++) {
		personajes[i].fd = 0;
		personajes[i].events = POLLIN;
		personajes[i].revents = 0;
	}
}

/***************************MANEJO DE DEADLOCK**************************/

void cargarPersonajeAEstructuraDeadLock(char id_pj,t_list* personajes_deadlock,ITEM_NIVEL* lista_items){
	t_recursos_cantidad *recursosAsign = malloc(sizeof(t_recursos_cantidad)* cantidad_key);
	ITEM_NIVEL* items_aux = lista_items;
	int i = 0;
	while(i < cantidad_key){
		if(items_aux->item_type == 1){
			recursosAsign[i].id_recurso = items_aux->id;
			recursosAsign[i].cantidad = 0;
			i++;
		}
		items_aux = items_aux->next;
	}

	t_personaje_deadlock* pj_dl = malloc(sizeof(t_personaje_deadlock));
	pj_dl->id_pj = id_pj;
	pj_dl->existe_dl = TRUE;//LO NECESITO ASI PARA MI ALGORITMO
	pj_dl->id_peticion = '0';
	pj_dl->recursosAsignados = recursosAsign;

	list_add(personajes_deadlock,pj_dl);
}

int getIndexPersonajeDeadlock(t_list* personajes_deadlock,char id_pj){
	int i = 0;
	while(i < personajes_deadlock->elements_count){
	t_personaje_deadlock* aux = list_get(personajes_deadlock,i);
		if( aux->id_pj == id_pj ){//encontre mi pj_dl
			return i;
		}
		i++;
	}
	return -1;
}

t_personaje_deadlock* getPersonajeDeadLock(t_list* personajes_deadlock, char id_pj){
	int index = getIndexPersonajeDeadlock(personajes_deadlock,id_pj);
	t_personaje_deadlock* pj_dl = list_get(personajes_deadlock,index);

	return pj_dl;
}

int getIndexRecurso(char id_recurso, t_recursos_cantidad* recursosAsignados){
	int i = 0;
	while((id_recurso != recursosAsignados[i].id_recurso) && (i < cantidad_key)){
		i++;
	}
	return i;
}

void* algoritmoDeadLock(void* nivelVoid){
	char* data;
	t_nivel* nivel = (t_nivel*) nivelVoid;

	while(1){

		usleep(nivel->deadlock);
		pthread_mutex_lock(&mutex);

		log_info(logger,"se ha iniciado la deteccion de deadlock");

		disponibleAux = malloc(sizeof(t_recursos_cantidad)*cantidad_key);
		ITEM_NIVEL* items_aux = nivel->lista_items;
		int i = 0;
		while(i < cantidad_key){
			if(items_aux->item_type == 1){
				disponibleAux[i].id_recurso = items_aux->id;
				disponibleAux[i].cantidad = items_aux->quantity;
				i++;
			}
			items_aux = items_aux->next;
		}

		char* interbloqueados;
		interbloqueados = malloc(sizeof(char));

		if(checkAndDoDeadLock(nivel->personajes_deadlock,&interbloqueados)){

			//loguear personajes en deadlock
			for(i = 0; i < strlen(interbloqueados) ; i++){
				log_info(logger,"esta en deadlock:%c",interbloqueados[i]);
			}

			if(nivel->recovery){
				log_info(logger,"el recovery se encuentra activado");
				t_stream* stream = serializar_interbloqueados(interbloqueados);
				//todo ver esto del puerto
				int *socket_nivel = socket_create_sin_puerto(logger);
				conectarse_a_orquestador(socket_nivel,nivel);
				sendBytes(socket_nivel,stream->data,stream->size, logger);
				log_info(logger,"le envie al orquestador los personajes interbloqueados");
				data = receiveBytes(socket_nivel,sizeof(char), logger);

				if(data!=NULL){
					log_info(logger,"recibi del orquestador el personaje asesinado [%c]", data[0]);
					baja_de_personaje(data[0],nivel,&(nivel->lista_items));
				}

				log_info(logger,"[algoritmoDeadLock] llamando stream_destroy...");
				stream_destroy(stream);
				
				log_info(logger,"[algoritmoDeadLock] cerrando socket de nivel...");
				socket_close(socket_nivel, logger);

			}
		}
		else{
			log_info(logger,"no hay personajes en deadlock");
		}

		resetPersonajesToDefault(nivel->personajes_deadlock);

		free(disponibleAux);
		free(interbloqueados);

		pthread_mutex_unlock(&mutex);

	}
	return NULL;

}

bool checkAndDoDeadLock(t_list* personajes_deadlock,char** interbloqueados){
    log_trace(logger, "Procesando [checkAndDoDeadLock]...");
	t_personaje_deadlock* pj_dl; //lo voy a usar muchas veces a lo largo del algoritmo

	/************* Paso 2. se marcan como  FALSE los personajes que tienen asignacion = 0 *************/
	//   recorro los personajes
	int i;
	for(i = 0 ; i < personajes_deadlock->elements_count ; i++){
		pj_dl = list_get(personajes_deadlock,i);

		//si no tienen ningun elemento asignado no existe deadlock y lo marco
		if(asignacionEsCero(pj_dl->recursosAsignados)){
			pj_dl->existe_dl = FALSE;
		}
	}
	/************* Paso 3. se buscan personajes sin marcar y que la solicitud <= disponible *************/
	//recorro los pjs sin marca, si marco alguno los vuelvo a recorrer, si no marque ninguno termine
	i = 0;
	while(i < personajes_deadlock->elements_count){
		pj_dl = list_get(personajes_deadlock,i);

		if((pj_dl->existe_dl) && (entraEnDisponible(pj_dl->id_peticion,disponibleAux))){
			/**********Paso 4. se marca y se incrementea el disp con asignacion*************/
			int j;
			for(j = 0 ; j < cantidad_key ; j++){
				pj_dl->existe_dl = FALSE;
				disponibleAux[j].cantidad += pj_dl->recursosAsignados[j].cantidad;
			}
			i = 0;//esto es para que recorra todos los sin marcar denuevo!
		}
		else i++;
	}
	/************* Paso 5. si quedaron elementos sin marcar hay DL y los agrego a interbloqueados *************/
	bool hay_dl = FALSE;
	int cantidad = 0;
	for(i = 0 ; i < personajes_deadlock->elements_count ; i++){
		pj_dl = list_get(personajes_deadlock,i);
		if(pj_dl->existe_dl == TRUE){
			hay_dl = TRUE;
			agregarAInterbloqueados(interbloqueados,pj_dl->id_pj,&cantidad);
		}
	}

	agregarAInterbloqueados(interbloqueados,'\0',&cantidad);
	log_trace(logger, "Saliendo [checkAndDoDeadLock]...");
	return hay_dl;
}

bool asignacionEsCero(t_recursos_cantidad* recursosAsignados){
    log_trace(logger, "Procesando [asignacionEsCero]...");
	int i;
	for( i = 0 ; i < cantidad_key ; i++){
		if(recursosAsignados[i].cantidad != 0){
		    log_trace(logger, "Saliendo [asignacionEsCero]...");
			return FALSE;
		}
	}

	log_trace(logger, "Saliendo [asignacionEsCero]...");
	return TRUE;
}

bool entraEnDisponible(char id_peticion, t_recursos_cantidad* disponibleAux){
    log_trace(logger, "Procesando [entraEnDisponible]...");
	if(id_peticion == '0'){
	    log_trace(logger, "Saliendo [entraEnDisponible]...");
		return TRUE;
	}

	int i = 0;
	while(id_peticion != disponibleAux[i].id_recurso){
		i++;
	}
	if(disponibleAux[i].cantidad <= 0){
	    log_trace(logger, "Saliendo [entraEnDisponible]...");
		return FALSE;
	}

	log_trace(logger, "Saliendo [entraEnDisponible]...");
	return TRUE;
}

void resetPersonajesToDefault(t_list* personajes_deadlock){
    log_trace(logger, "Procesando [resetPersonajesToDefault]...");
	int i;
	for(i = 0; i < personajes_deadlock->elements_count; i++){
		t_personaje_deadlock* pj_dl = list_get(personajes_deadlock,i);
		pj_dl->existe_dl = TRUE;
		log_debug(logger, "[resetPersonajesToDefault] Reseteando pj=[%c] peticion=[%c]", pj_dl->id_pj, pj_dl->id_peticion);
	}

	log_trace(logger, "Saliendo [resetPersonajesToDefault]...");
}

void liberarPersonaje(t_list* personajes_deadlock,char id_pj){
    log_trace(logger, "Ejecutando [liberarPersonaje]...");
	t_personaje_deadlock* pj_dl = getPersonajeDeadLock(personajes_deadlock,id_pj);
	free(pj_dl->recursosAsignados);

	int index = getIndexPersonajeDeadlock(personajes_deadlock,id_pj);

	list_remove(personajes_deadlock,index);
	log_trace(logger, "Saliendo [liberarPersonaje]...");
}

void agregarAInterbloqueados(char** interbloqueados,char id_pj,int* cantidad){
    log_trace(logger, "Ejecutando [agregarAInterbloqueados]...");
	(*cantidad)++;
	if(*cantidad != 1){
		*interbloqueados = realloc(*interbloqueados,*cantidad);
	}
	(*interbloqueados)[*cantidad-1] = id_pj;
	log_trace(logger, "Saliendo [agregarAInterbloqueados]...");
}

