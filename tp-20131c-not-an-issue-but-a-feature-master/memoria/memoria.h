#ifndef LIBMEMORIA_H_
#define LIBMEMORIA_H_
	#include <stdbool.h>
    #include <commons/collections/list.h>
	#include <commons/log.h>
	#include <stdio.h>

    typedef char* t_memoria;

    typedef struct {
        char id;
        int inicio;
        int tamanio;
        char* dato;
        bool libre;
    } t_particion;

    //crea el segmento de memoria a particionar
    t_memoria crear_memoria(int tamanio);

    /* Crea una particion dentro del segmento de memoria de tamaño,
	 * identificador y contenido especificado. Devuelve el valor numerioc -1
	 * en caso de error (ej: tamaño de la particion mayor que el tamaño total
	 * del segmento, id duplicado, etc.), 1 en caso de exito y 0 en caso de
	 * no encotrar una particion libre lo suficientemente grande para
	 * almacenar la solicitud
	 */
    int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido);

    /* Esta funcion elimina la particion dentro del segmento de memoria
	 * correspondiente al identificador enviado como parametro. Devuelve el
	 * valor numerico -1 en caso de exito y 0 en caso de no encontrar
	 * una particion con dicho identificador
	 */
    int eliminar_particion(t_memoria segmento, char id);

    //esta funcion libera los recursos tomados en crear_memoria
    void liberar_memoria(t_memoria segmento);

    /*Esta funcion devuelve una lista en el formato t_list de las
     * commons-library con la siguiente descripcion por cada particion */
    t_list* particiones(t_memoria segmento);

#endif /* LIBMEMORIA_H_ */
