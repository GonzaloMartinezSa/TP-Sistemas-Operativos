#include "include/main.h"

#define LOG_MEMORIA "memoria.log"

int main(int argc, char** argv) {

	fclose(fopen(LOG_MEMORIA, "w"));

	logger =  log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);

	if(argc < 2) {
		log_error(logger, "Falta parametro de configuracion!");
        return EXIT_FAILURE;
    }

	leerConfig(argv[1]);
	
	bool status = iniciar_estructuras_memoria();

	int server_atencion_kernel = iniciar_servidor(logger, "memoria", "0.0.0.0", conf.puerto_escucha);
	
    while(true) {
        pthread_t hilo;
        int socket_cliente = esperar_cliente(logger, "conexion_kernel", server_atencion_kernel);
        pthread_create(&hilo, NULL, (void*) procesar_conexion, &socket_cliente);
    }

	return 0;
}
