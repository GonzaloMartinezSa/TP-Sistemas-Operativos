#include "include/main.h"

#define LOG_KERNEL "kernel.log"

void cerrar_programa() {
	log_destroy(logger);
    list_destroy(planificador.lista);

    for(int i = 0; i < 7; i ++){
        lista_eliminar(planificador.list[i]);
    }
}

int main(int argc, char** argv) {

    fclose(fopen(LOG_KERNEL, "w"));

    logger        = log_create(LOG_KERNEL, "kernel", true, LOG_LEVEL_DEBUG);
    
    if(argc < 2) {
		log_error(logger, "Falta parametro de configuracion!");
        return EXIT_FAILURE;
    }
    
    config        = leer_config(argv[1]);
    
    planificador  = iniciar_planificador(config.algoritmo);

    bool status   = iniciar_kernel();

    log_info(logger, "Kernel status %d", status);

    uint32_t tblock    = 0;
    t_nodo_block* nodo = NULL;

    /*
    PLANIFICADOR DE CORTO PLAZO
    */
    while(true){
        // Espero a tener algo en ready..
        sem_wait(&planificador.sem[EREADY]);

        // Extraigo un proceso de la cola de ready
        pthread_mutex_lock(&planificador.mutex[EREADY]);
        t_pcb* pcb = lista_extraer(planificador.list[EREADY]);
        pthread_mutex_unlock(&planificador.mutex[EREADY]);
        
        log_warning(logger, "PID:%d en EJECUCION", pcb->id);

        // Envio el pcb a ejecucion
        int rta_cpu = enviar_PCB(pcb, &tblock);

        pthread_t t_block_;

        // Manejo el PCB en base a la RTA
        switch(rta_cpu){

            case IR:
                log_warning(logger, "PID:%d DESALOJADO", pcb->id);
                insertar_en_ready(pcb, 1);
                break;
                
            case FINALIZADO:

                pthread_mutex_lock(&planificador.mutex[EEXIT]);
                lista_insertar(logger, planificador.list[EEXIT] , pcb, config.algoritmo, 0);
                pthread_mutex_unlock(&planificador.mutex[EEXIT]);

                // Le aviso a memoria que libere recursos? 
                log_warning(logger, "PID:%d en EXIT", pcb->id);

                // Incremento el semaforo para que las consolas empiecen a chequear que onda..
                sem_post(&planificador.sem[EEXIT]);

                // Libero en uno la multi_prog... esto deberia de ir en EXIT...
                sem_post(&planificador.multi_prog);

                break;

            case BLOCK:
                nodo          = malloc(sizeof(t_nodo_block));
                nodo->pcb     = pcb;
                nodo->block_t = tblock;    
                nodo->status  = BLOQUEADO;

                // t_suspension_args* args = malloc(sizeof(t_suspension_args));
                // args->nodo    = nodo;  

                pthread_create(&t_block_, NULL, (void*) manejo_bloqueo, (void*) nodo);
                nodo->th = t_block_;
                pthread_detach(t_block_);

                pthread_mutex_lock(&planificador.mutex[EBLOCK]);
                list_add(lista_block, nodo); // Agrego al final 
                pthread_mutex_unlock(&planificador.mutex[EBLOCK]);

                sem_post(&planificador.sem[EBLOCK]);

                log_warning(logger, "PID:%d en BLOQUEADO (%dms)", pcb->id, tblock);
                break;

            case ERROR:
                pthread_mutex_lock(&planificador.mutex[EEXIT]);
                lista_insertar(logger, planificador.list[EEXIT] , pcb, config.algoritmo, 0);
                pthread_mutex_unlock(&planificador.mutex[EEXIT]);
                
                // Incremento el semaforo para que las consolas empiecen a chequear que onda..
                sem_post(&planificador.sem[EEXIT]);

                log_info(logger, "El proceso %d tuvo un error de ejecucion.. lo paso a EXIT", pcb->id);
                break;

            default:
                // Habria que ver que onda..
                log_error(logger, "Respuesta inesperada de la CPU... Se desconoce el estado del proceso %d!", pcb->id);
                break;
        } 
    }

    cerrar_programa();

    return 0;
}

