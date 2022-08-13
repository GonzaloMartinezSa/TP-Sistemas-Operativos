#include "../include/init.h"

bool iniciar_estructuras_memoria(){

	// Inicializa las tablas de primer nivel
	for(int i = 0; i < MAX_TABLAS_P1; i ++){
		listaTablasPrimerNivel[i].enUso = false;
		listaTablasPrimerNivel[i].cant_entradas = -1;
    	listaTablasPrimerNivel[i].primer_marco = -1; 
    	listaTablasPrimerNivel[i].punteroClock = 0; // <----------------------------------------------------------------- REVISAR
	}

	// Inicializa las tablas de segundo nivel
	for(int i = 0; i < MAX_TABLAS_P2; i ++){
		listaTablasSegundoNivel[i].enUso = false;
	}

	// Calculo la memoria maxima que puede solicitar un proceso
	MEM_MAX_X_PROCESO = conf.tam_pagina * conf.entradas_por_tabla * conf.entradas_por_tabla;

	// Calculo la cantidad total de marcos
	TOTAL_MARCOS = conf.tam_memoria / conf.tam_pagina;

	// Calculo la cantidad de uints que me entran en un marco
	CANT_UINTS_MARCO = conf.tam_pagina / sizeof(uint32_t);

	// Inicializa estructuras de manejo de tablas libres
	asignadosPrimerNivel  = 0;
	asignadosSegundoNivel = 0;

	// Inicializa estructuras de manejo de marcos libres
	int cantidad_indices = TOTAL_MARCOS / conf.marcos_por_proceso;
	indice_marco_uso = malloc(sizeof(bool) * cantidad_indices);

	for(int i = 0; i < cantidad_indices; i ++){
		indice_marco_uso[i] = false;
	}

	// Mapeo/reservo la memoria de usuario
	espacio_usuario =  mmap(NULL, sizeof(t_marco) * TOTAL_MARCOS, PROT_READ| PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	// Mapeo/reservo el bitmap
	bitmap_marco    = (t_bitmap_marco*) malloc(sizeof(t_bitmap_marco) * TOTAL_MARCOS);

	for(int i = 0; i < TOTAL_MARCOS; i ++){

		// Inicializo todas las posiciones del la memusr en cero
		((t_marco*)espacio_usuario)[i].datos = mmap(NULL, conf.tam_pagina, PROT_READ| PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		
		memset(((t_marco*)espacio_usuario)[i].datos, 0, sizeof(conf.tam_pagina));

		// Inicializo todas las posiciones del bitmap
		bitmap_marco[i].enUso = false;
		bitmap_marco[i].NTP1 = 0;
		bitmap_marco[i].ETP1 = 0;
		bitmap_marco[i].ETP2 = 0;
	}

	// Inicializo el semaforo que evita solapamiento de operaciones
	sem_init(&sem, 0, 1);

	return 1;
}

void leerConfig(char* argv){
	
	t_config* config =  config_create(argv);

    memset(conf.puerto_escucha, 0 , 10);
    memcpy(conf.puerto_escucha, config_get_string_value(config, "PUERTO_ESCUCHA"), strlen(config_get_string_value(config, "PUERTO_ESCUCHA")));

	conf.tam_memoria         = config_get_int_value(config, "TAM_MEMORIA");
	conf.tam_pagina          = config_get_int_value(config, "TAM_PAGINA");
	conf.entradas_por_tabla  = config_get_int_value(config, "ENTRADAS_POR_TABLA");
	conf.retardo_por_memoria = config_get_int_value(config, "RETARDO_MEMORIA");

	char aux[10];
	memset(aux, 0 , 10);
    memcpy(aux, config_get_string_value(config, "ALGORITMO_REEMPLAZO"), strlen(config_get_string_value(config, "ALGORITMO_REEMPLAZO")));

	if(!strcmp(aux, "CLOCK"))
		conf.algoritmo_reemplazo = CLOCK;
	else if(!strcmp(aux, "CLOCK-M"))
		conf.algoritmo_reemplazo = CLOCKM;
	else{
		log_error(logger, "NO ME PUSIERON UN ALGORITMO DE REEMPLAZO VALIDO!");
		exit(1);
	}

	conf.marcos_por_proceso   = config_get_int_value(config, "MARCOS_POR_PROCESO");
	conf.retardo_swap         = config_get_int_value(config, "RETARDO_SWAP");

	memset(conf.path_swap, 0 , 50);
    memcpy(conf.path_swap, config_get_string_value(config, "PATH_SWAP"), strlen(config_get_string_value(config, "PATH_SWAP")));

	config_destroy(config);
}



