#include "../include/init.h"
#include <pthread.h>

char edecode [][10] = {"NEW", "READY", "EXEC", "BLOCK", "SB", "SR", "EXIT"};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Inicio el planificador..
// Inicializo listas, semaforos y mutex
t_planificador iniciar_planificador(algoritmo modo){

    t_planificador plan;

    plan.modo           = modo;
    plan.lista          = list_create();
    
    // Inicio las colas con sus mutex
    for(int i =0; i < 7; i ++){
        plan.list[i]   = lista_crear(edecode[i]);
        int status     = pthread_mutex_init(&plan.mutex[i], NULL);
        if(status != 0){
            printf("Error al crear el mutex (%d)!\n", status);
        }

        if(i != EEXEC)
            sem_init(&plan.sem[i], 0, 0);
        else
            sem_init(&plan.sem[i], 0, 1);
    }

    // Inicializo el acumulador tras IR
    lista_te = list_create();

    // Inicio el sistema de manejo de multiProgramacio
    sem_init(&plan.multi_prog, 0, config.multi_prog);

    // Inicio el sistema de numeracion de los procesos
    pthread_mutex_init(&plan.pid_mutex, NULL);
    plan.pid        = 0;

    // Inicializo la herramienta para establecer prioridad en suspended ready
    pthread_mutex_init(&prioridad_sr, NULL);
    
    // Mutex de comunicacion con memoria
    pthread_mutex_init(&comunicacion_memoria, NULL);
    
    // Preparo los elementos del sincronizacion y listas
    lista_block = list_create();

    char ealg [][10] = {"FIFO", "SRT", "ERR"};
    log_info(logger, "Inicializando Kernel con algoritmo '%s'", ealg[plan.modo]);

    return plan;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Lectura de la configuracion
// 
t_kernelConfig leer_config(char* argv){
	//ahora, vamos a abrir el archivo de configuracion "consola.config"
	t_config* c_config = config_create(argv);

    t_kernelConfig conf;

    memset(conf.ip_memoria, 0 , 50);
    memcpy(conf.ip_memoria, config_get_string_value(c_config, "IP_MEMORIA"), strlen(config_get_string_value(c_config, "IP_MEMORIA")));

    memset(conf.ip_cpu, 0 , 50);
    memcpy(conf.ip_cpu, config_get_string_value(c_config, "IP_CPU"), strlen(config_get_string_value(c_config, "IP_CPU")));

    memset(conf.puerto_memoria, 0 , 10);
    memcpy(conf.puerto_memoria, config_get_string_value(c_config, "PUERTO_MEMORIA"), strlen(config_get_string_value(c_config, "PUERTO_MEMORIA")));

    memset(conf.puerto_cpu_dispatch, 0 , 10);
    memcpy(conf.puerto_cpu_dispatch, config_get_string_value(c_config, "PUERTO_CPU_DISPATCH"), strlen(config_get_string_value(c_config, "PUERTO_CPU_DISPATCH")));

    memset(conf.puerto_cpu_interrupt, 0 , 10);
    memcpy(conf.puerto_cpu_interrupt, config_get_string_value(c_config, "PUERTO_CPU_INTERRUPT"), strlen(config_get_string_value(c_config, "PUERTO_CPU_INTERRUPT")));

    memset(conf.puerto, 0 , 10);
    memcpy(conf.puerto, config_get_string_value(c_config, "PUERTO_ESCUCHA"), strlen(config_get_string_value(c_config, "PUERTO_ESCUCHA")));

    char* algoritmo_aux  = malloc(5);
    memset(algoritmo_aux, 0, 5);
    memcpy(algoritmo_aux, config_get_string_value(c_config, "ALGORITMO_PLANIFICACION"), strlen(config_get_string_value(c_config, "ALGORITMO_PLANIFICACION")));
    
    if(!strcmp(algoritmo_aux, "FIFO")){
        conf.algoritmo = FIFO;
    }
    else if(!strcmp(algoritmo_aux, "SRT")){
        conf.algoritmo = SRT;
    }
    else{
        log_error(logger, "No reconozco el campo 'ALGORITMO_PLANIFICACION'!\n");
        exit(0);
    }

    free(algoritmo_aux);

    conf.estimacion_inicial   = atoi(config_get_string_value(c_config, "ESTIMACION_INICIAL"));
    conf.alfa                 = config_get_double_value(c_config, "ALFA");
    conf.multi_prog           = config_get_int_value(c_config, "GRADO_MULTIPROGRAMACION");
    conf.tmax                 = config_get_int_value(c_config, "TIEMPO_MAXIMO_BLOQUEADO");

    conexionCPU_dispatch = crear_conexion(logger, "Conexion CPU", conf.ip_cpu, conf.puerto_cpu_dispatch);
    conexionCPU_interrupt = crear_conexion(logger, "Conexion CPU - Interrupt", conf.ip_cpu, conf.puerto_cpu_interrupt);
    conexionMemoria = crear_conexion(logger, "Conexion Memoria", conf.ip_memoria, conf.puerto_memoria);

    config_destroy(c_config);

    return conf;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Hilo de atencion de las consolas
// Cada nueva comunicacion crea un nuevo hilo para atenderla y crear el consecuente PCB 
static void atender_consolas(void* void_args) {

    int servidor = iniciar_servidor(logger, "kernel", "0.0.0.0", config.puerto);

    while (server_escuchar("kernel", servidor));

    log_error(logger, "Error grave de comunicaciones.. no estoy escuchando concolas..");
    exit(1);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Inicializacion del kernel con todos sus hilos
// 
bool iniciar_kernel(){

    pthread_t t_consolas;
    pthread_create(&t_consolas, NULL, (void*) atender_consolas, NULL);
    
    pthread_t t_pl;
    pthread_create(&t_pl, NULL, (void*) planificador_largoplazo, NULL);
    
    pthread_t t_exit;
    pthread_create(&t_exit, NULL, (void*) manejo_exit, NULL);

    pthread_t t_pm;
    pthread_create(&t_pm, NULL, (void*) planificador_medianoplazo, NULL);

    //pthread_t t_count;
    //pthread_create(&t_count, NULL, (void*) manejo_suspension, NULL);

    pthread_detach(t_pl);
    pthread_detach(t_consolas);
    pthread_detach(t_exit);
    pthread_detach(t_pm); 
    //pthread_detach(t_count);

    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Funciones de prueba de la libreria de listas..
// 
/*
void imprimir_listas_estados(t_planificador* planificador){
    printf("\e[1;1H\e[2J");
    for(int i =0; i < 7; i ++){
        t_desc* lista = planificador->list[i];
        printf("\n------------------------------%s--------------------------------\n", lista->nombre);
        if(lista->cantidad == 0)
            printf("Lista vacia!");
        else{
            t_nodo* pos = lista->first;
            while(pos != NULL){
                printf("PID:%d  ", pos->pcb->id);
                pos = pos->sgte;
            }
        }
    } 
}
*/

void test_list(t_log* logger, t_planificador* planificador){

    // Me armo pcbs de prueba
    t_pcb * pcb1 = crear_pcb(1,10, 0, 1825, 6, NULL);
    t_pcb * pcb2 = crear_pcb(2,20, 0, 1616, 20000, NULL);
    t_pcb * pcb3 = crear_pcb(3,20, 0, 2458, 20000, NULL);
    t_pcb * pcb4 = crear_pcb(4,20, 0, 2458, 2.5, NULL);
    t_pcb * pcb5 = crear_pcb(5,20, 0, 2455, 20000, NULL);


    lista_insertar(logger, planificador->list[EEXIT], pcb1, planificador->modo, 0);
    lista_insertar(logger, planificador->list[EEXIT], pcb2, planificador->modo, 0);
    lista_insertar(logger, planificador->list[EEXIT], pcb3, planificador->modo, 0);
    lista_insertar(logger, planificador->list[EEXIT], pcb4, planificador->modo, 0);
    lista_insertar(logger, planificador->list[EEXIT], pcb5, planificador->modo, 0);

    imprimir_lista(planificador->list[EEXIT]);

}
