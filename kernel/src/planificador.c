#include "../include/planificador.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Planificador de largo plazo
// Manejo la transicion NEW -> READY, converso con memoria para inicializar el proceso
void planificador_largoplazo(void* void_args) {

    // Siempre voy a estar esperando y chequeando la cola de new
    while(true){
        
        // Chequeo si tengo algo en la cola de NEW
        sem_wait(&planificador.sem[ENEW]);
        
        // Chequeo si estoy dentro del estandar de multi_prog
        sem_wait(&planificador.multi_prog);

        // Chequeo si tengo algo en status ready.. en caso de ser asi.. resigno el lugar en multi_prog
        pthread_mutex_lock(&prioridad_sr);
        int sr = sr_count;
        pthread_mutex_unlock(&prioridad_sr);

        if(sr > 0){
            sem_post(&planificador.multi_prog);
            continue;
        }

        // Extraigo el proceso de la cola de new
        pthread_mutex_lock(&planificador.mutex[ENEW]);
        t_pcb* pcb = lista_extraer(planificador.list[ENEW]);
        pthread_mutex_unlock(&planificador.mutex[ENEW]);
        
        // Charlo con memoria para poder iniciar las estructuras necesarias y que me de el valor de la tabla de paginas

        pthread_mutex_lock(&comunicacion_memoria);
        int tp = notificar_memoria(INICIALIZAR, pcb->id , pcb->tamano, 245);
        pthread_mutex_unlock(&comunicacion_memoria);
        
        if(tp >= 0){
            pcb->tp = tp;
        }
        else{
            log_error(logger, "No consegui una tabla de paginas para el proceso %d", pcb->id);
        }

        log_warning(logger, "PID:%d en READY!", pcb->id);
        
        // Paso el pcb a la tabla de READY
        insertar_en_ready(pcb, false);

    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Reinsercion de un PCB en ready con prioridad luego de una suspension del mismo
// Inserto el PCB en ready con prioridad, converso con memoria para quitar de suspendido el proceso
void reinsert_sr_ready(void* void_args){
    t_nodo_block  * nodo     = (t_nodo_block*) void_args;

    // Incremento el contador de pcbs en statusReady
    pthread_mutex_lock(&prioridad_sr);
    sr_count ++;
    pthread_mutex_unlock(&prioridad_sr);
    
    log_info(logger, "PID:%d esta siendo re-insertado en READY", nodo->pcb->id);

    // Chequeo multiProgramacion
    sem_wait(&planificador.multi_prog);

    log_info(logger, "Tenia suficiente grado de multiprog");

    // Paso el pcb a la tabla de READY
    insertar_en_ready(nodo->pcb, false);

    log_warning(logger, "PID:%d en READY", nodo->pcb->id);

    //free(nodo);
	
    pthread_mutex_lock(&prioridad_sr);
    sr_count --;
    pthread_mutex_unlock(&prioridad_sr);
    
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Insercion de procesos en la cola de ready
// Inserto de forma segura un pcb en ready y envio interrupciones a la CPU si estoy en SRT
void insertar_en_ready(t_pcb* pcb, bool reinsert){
    int count;

    if(config.algoritmo == SRT && !reinsert){
        enviar_interrupt(DESALOJO);
        log_info(logger, "IR enviada!");
    }

    pthread_mutex_lock(&planificador.mutex[EREADY]);
    lista_insertar(logger, planificador.list[EREADY], pcb, config.algoritmo, reinsert);
    count = planificador.list[EREADY]->cantidad;
    pthread_mutex_unlock(&planificador.mutex[EREADY]);

    sem_post(&planificador.sem[EREADY]);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Hilo para manejar el status bloqueado -> suspendido
// Espero el tiempo maximo de bloqueo y analizo si el proceso sigue en bloqueado... si sigue lo suspendo y converso con memoria para suspenderlo 
void manejo_bloqueo(void* void_args){
    t_nodo_block  * nodo     = (t_nodo_block*)void_args;

    int pid = nodo->pcb->id;

    // Espero el tiempo maximo de bloqueo...
    usleep(config.tmax * 1000);

    nodo->status = SUSPENDIDO;

    log_warning(logger, "PID:%d en SUSPENDIDO!", nodo->pcb->id);

    // Le aviso a memoria que este proceso pasa a suspendido
    pthread_mutex_lock(&comunicacion_memoria);
    int status = notificar_memoria(SUSPENDERP, nodo->pcb->id , nodo->pcb->tp, 423);
    pthread_mutex_unlock(&comunicacion_memoria);
    

    if(status < 0){
        log_error(logger, "No consegui avisar a memoria de la suspension del PID:%d", nodo->pcb->id);
    }

    // Tengo que bajar el grado de multiprogramacion
    sem_post(&planificador.multi_prog);  
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Planificador de mediano plazo
// Maneja la ejecucion de las IO y la reinsercion en READY
void planificador_medianoplazo(void* void_args) {
    // Contador de elementos en suspended Ready
    pthread_mutex_lock(&prioridad_sr);
    sr_count = 0;
    pthread_mutex_unlock(&prioridad_sr);

    t_nodo_block* nodo;

    // Siempre voy a estar esperando y chequeando la cola de new
    while(true){
        // Espero a tener algo en blockeado
        sem_wait(&planificador.sem[EBLOCK]);

        pthread_mutex_lock(&planificador.mutex[EBLOCK]);
        nodo = list_get(lista_block, 0);
        pthread_mutex_unlock(&planificador.mutex[EBLOCK]);

        log_warning(logger, "PID:%d en IO! (%dms)", nodo->pcb->id, nodo->block_t);
        usleep(1000 * nodo->block_t);

        if(nodo->status == BLOQUEADO){
            //log_info(logger, "Cancelo el hilo de suspension del PID:%d", nodo->pcb->id);
            pthread_cancel(nodo->th);
        }

        pthread_mutex_lock(&planificador.mutex[EBLOCK]);
        nodo = list_remove(lista_block, 0);
        pthread_mutex_unlock(&planificador.mutex[EBLOCK]);

        log_warning(logger, "PID:%d finaliza IO!", nodo->pcb->id);

        if(nodo->status == BLOQUEADO){
            // Paso el pcb a la tabla de READY
            insertar_en_ready(nodo->pcb, false);
            nodo->status = LISTO;
            free(nodo);
        }
        else{ // Esta suspendido
            nodo->status = LISTO;
            pthread_t t_rsusp;
            pthread_create(&t_rsusp, NULL, (void*) reinsert_sr_ready, (void*) nodo);
            pthread_detach(t_rsusp);
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Planificador de largo plazo
// Esta funcion esta esperando (estrategia productor-consumidor) por un proceso finalizado.. libera memoria y le responde a la consola
void manejo_exit(void* void_args) {
    // Siempre voy a estar esperando y chequeando la cola de new
    t_pcb * pcb;
    
    while(true){
        
        // Espero a tener algo en blockeado
        sem_wait(&planificador.sem[EEXIT]);

        // Me traigo el pcb
        pthread_mutex_lock(&planificador.mutex[EEXIT]);
        pcb = lista_extraer(planificador.list[EEXIT]);
        pthread_mutex_unlock(&planificador.mutex[EEXIT]);

        //Le aviso a memoria que libere...

        pthread_mutex_lock(&comunicacion_memoria);
        int status = notificar_memoria(LIBERAR, pcb->id , pcb->tp, 285);
        pthread_mutex_unlock(&comunicacion_memoria);

        if(status < 0){
            log_error(logger, "No consegui liberar en memoria al PID:%d", pcb->id);
        }
        else{
            log_info(logger, "Memoria liberada!");
        }


        // Busco el socket y le respondo a la consola      
        pthread_mutex_lock(&planificador.mutex[ENEW]);
        t_socketID socketID = borrarSocketID(pcb->id, planificador.lista);
        pthread_mutex_unlock(&planificador.mutex[ENEW]);

        if(socketID.id == -1 ){
            log_error(logger, "Tremendo error en las listas que relacion el PID con el socket...");
        }
        else{

            long end = currentTimeMillis();
            long timeElapsed = timeDifMillis(socketID.inicio, end);

            t_rta_consola msj;
            msj.sc = 200; 
            msj.tiempoBloqueo = timeElapsed;

            send_rta_consola(socketID.socket, &msj);
        
            log_warning(logger, "PID:%d FINALIZADO (%ldms)", pcb->id, timeElapsed);

            free(pcb->programa);
            free(pcb);

            // Incremento el numero de multi_prog disponible
            sem_post(&planificador.multi_prog);
        }
    }
}
