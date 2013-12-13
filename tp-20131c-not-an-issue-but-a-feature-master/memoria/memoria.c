//export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/tp-20131c-not-an-issue-but-a-feature/memoria/Debug
//export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/tp-20131c-not-an-issue-but-a-feature/so-commons-library/Debug

#include <stdlib.h>
#include <string.h>
#include "memoria.h"

#define PATH_LOG 		"log_koopa.log"
#define PROGRAM_NAME	"koopa"

// Creates a new partition
t_particion* create_partition(char id, int size, char* content, t_memoria segment_base, int partition_start);

// Searches and retrieves the next free space based on Next-Fit algorithm
t_particion* find_next_fit(int size, t_list* list);

// Searches and retrieves a partition by id
// If it does not exist, returns NULL
t_particion* find_partition_by_id(char id, t_list* list);

// Validates the content size and the declared size
bool content_size_mismatch(char* content, int size);

// Validates whether a partition with the given id already exists
bool id_exist(char id, t_list* list);

// Sorts the partitions list by the beginning of the partition
void sort_partitions(t_list* list);

// Clones the partition
t_particion* clone_partition(t_particion* partition);

// Creates the logger
void ensure_log();

t_log* logger;
int segment_size = 0;
t_list* partitions_list;

// used to avoid 'searching' the last element of the list
t_particion* last_created = NULL;

t_memoria crear_memoria(int tamanio) {
	ensure_log();
	log_trace(logger, "[crear_memoria]/tamanio=[%d].", tamanio);
	t_memoria data = NULL;
	if (!(data = malloc(sizeof(char) * tamanio))) {
		log_error(logger, "Error creating memory.");
	} else {
		segment_size = tamanio;
		memset(data, '\0', tamanio);
		// ensure list is instantiated
		if (!partitions_list) {
			partitions_list = list_create();
		}

		t_particion* mainPartition = create_partition('0', tamanio, data, data, 0);
		mainPartition->libre = true;
		list_add(partitions_list, mainPartition);
	}

	return data;
}

int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido) {
	ensure_log();
	log_trace(logger, "[almacenar_particion]/segmento=[%p] id=[%c] tamanio=[%d] contenido=[%s].", segmento, id, tamanio, contenido);
	if (segment_size < tamanio) {
		log_info(logger, "Not enough space into segment. Requested=[%d]. Available=[%d]", tamanio, segment_size);
		return -1;
	}

	if(content_size_mismatch(contenido, tamanio)) {
		log_info(logger, "Invalid request: Declared content size is different than effective content size.");
		return -1;
	}

	if(id_exist(id, partitions_list)) {
		log_info(logger, "Invalid request: Partition with id=[%c] already exists.", id);
		return -1;
	}

	// find partition start address
	t_particion* free_partition = find_next_fit(tamanio, partitions_list);
	if(!free_partition){
		log_info(logger, "There isn't enough free space to attend the request.");
		return 0;
	}

	if(free_partition->tamanio == tamanio){
		free_partition->id = id;
		free_partition->libre = false;
		memcpy(free_partition->dato, contenido, tamanio);
	}
	else {
		t_particion* partition = create_partition(id, tamanio, contenido, segmento, free_partition->inicio);
		free_partition->inicio += tamanio;
		free_partition->tamanio -= tamanio;
		free_partition->dato += tamanio;
		last_created = free_partition;
		list_add(partitions_list, partition);
		sort_partitions(partitions_list);
	}

	return 1;
}

int eliminar_particion(t_memoria segmento, char id) {
	ensure_log();
	log_trace(logger, "[eliminar_particion]/segmento=[%p] id=[%c].", segmento, id);
	t_particion* partition = NULL;
	if(!(partition = find_partition_by_id(id, partitions_list))) {
		log_info(logger, "Invalid request: Partition with id=[%c] does not exist.", id);
		return 0;
	}

	partition->id = '0';
	partition->libre = true;
	memset(partition->dato, '\0', partition->tamanio);
	return -1;
}

void liberar_memoria(t_memoria segmento) {
	ensure_log();
	log_trace(logger, "[liberar_memoria]");
	list_destroy(partitions_list);
	free(segmento);
}

t_list* particiones(t_memoria segmento) {
	ensure_log();
	log_trace(logger, "[liberar_memoria]");
	t_list* list = list_create();
	int count = partitions_list->elements_count;
	t_particion* item = NULL;
	t_particion* clon = NULL;
	int i = 0;
	for (i = 0; i < count; ++i) {
		item = list_get(partitions_list, i);
		clon = clone_partition(item);
		list_add(list, clon);
	}

	return list;
}

t_particion* create_partition(char id, int size, char* content, t_memoria segment_base, int partition_start) {
	t_particion* partition = malloc(sizeof(t_particion));
	partition->id = id;
	partition->inicio = partition_start;
	partition->tamanio = size;
	partition->libre = false;
	partition->dato = (char*) (segment_base + partition_start);
	memcpy(partition->dato, content, size);
	return partition;
}

t_particion* clone_partition(t_particion* partition) {
	if(!partition)
		return NULL;

	t_particion* clon = malloc(sizeof(t_particion));
	clon->id = partition->id;
	clon->inicio = partition->inicio;
	clon->tamanio = partition->tamanio;
	clon->libre = partition->libre;
	clon->dato = partition->dato;
	return clon;
}

t_particion* find_next_fit(int size, t_list* list) {
	t_particion* last_fit = last_created;
	bool is_next_free(t_particion* partition){
	 	return (partition->libre) & (partition->tamanio >= size) & (!last_fit || partition->inicio >= last_fit->inicio);
	}

	t_particion* result = list_find(list, (void*)is_next_free);
	if(!result) {
		// find from first position
		last_fit = list_get(list, 0);
		result = list_find(list, (void*)is_next_free);
	}

	return result;
}

t_particion* find_partition_by_id(char id, t_list* list) {
	bool equals(t_particion* partition) {
		return partition->id == id;
	}

	return list_find(list, (void*)equals);
}

bool content_size_mismatch(char* content, int size) {
	int content_len = strlen(content);
	return content_len != size;
}

bool id_exist(char id, t_list* list) {
	return find_partition_by_id(id, list);
}

void sort_partitions(t_list* list){
	bool is_previous_than(void* x, void* y) {
		return ((t_particion*)x)->inicio < ((t_particion*)y)->inicio;
	}

	list_sort(list, is_previous_than);
}

void ensure_log() {
	if(!logger)
		logger = log_create(PATH_LOG, PROGRAM_NAME, false, LOG_LEVEL_TRACE);
}
