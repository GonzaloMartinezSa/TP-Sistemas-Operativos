#include "../include/comunicacion.h"

char decode [][7] = {"NO_OP", "I/O", "READ", "COPY", "WRITE", "EXIT"};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Procesar conexion proveniente de una consola
// Recibe el programa , crea el PCB y lo coloca en la cola de NEW
static void procesar_conexion(void* void_args) {

    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    int cliente_socket             = args->fd;
    char* server_name              = args->server_name;

    //Datos importantes que voy a completar cuando llegue el header
    int     memoria       = 0;
    int     id            = 0;

    //Datos para sostener la comunicacion
    op cop                  = 0;
    int  cant_instrucciones = 1;
    bool header             = 0;

    //Puntero al vector de instrucciones
    t_instruccion* l_instrucciones;

    log_info(logger, "Nuevo hilo creado para recepcion de consola");

    if (recv(cliente_socket, &cop, sizeof(op), MSG_WAITALL) != sizeof(op)) {
        log_info(logger, "DISCONNECT!");
        return;
    }
    
    op co_op;
    int arg1;
    int arg2;
    

    switch (cop) {

        case PROGRAMA:
        {
            if (!recv_header(cliente_socket, &cant_instrucciones, &memoria)) {
                log_error(logger, "Fallo recibiendo el header");
                break;
            }

            /*Numero como corresponde las consolas paertiendo de 0*/
            pthread_mutex_lock(&planificador.pid_mutex);
            planificador.pid++;  // Importante no usar el ++ aca
            id = planificador.pid;
            pthread_mutex_unlock(&planificador.pid_mutex);

            log_info(logger, "Me llego un header numerado como pid '%d' con %d instrucciones, solicita %d bytes en memoria", id, cant_instrucciones, memoria);

            //Armo el vector que voy a usar para guardar las instrucciones
            l_instrucciones = malloc(sizeof(t_instruccion) * cant_instrucciones);
            memset(l_instrucciones, 0, sizeof(t_instruccion) * cant_instrucciones);

            for(int l_pos = 0; l_pos < cant_instrucciones; l_pos++){

                        if (!recv_instruccion(cliente_socket, &co_op, &arg1, &arg2)) {
                            log_error(logger, "Fallo recibiendo instruccion");
                            break;
                        }
                        //Sumo la instruccion a mi lista de instrucciones
                        l_instrucciones[l_pos].coop = co_op;
                        l_instrucciones[l_pos].arg1 = arg1;
                        l_instrucciones[l_pos].arg2 = arg2;

                        //log_info(logger, "%s %d %d", decode[co_op], arg1, arg2);

                    }
            //Aviso que ya recibi el header..
            header = 1;

            break;
        }
        
        // Errores
        case -1:
            log_error(logger, "Cliente desconectado de %s...", server_name);
            return;
        default:
            log_error(logger, "Algo anduvo mal en el server de %s", server_name);
            log_info(logger, "Cop: %d", cop);
            return;
    }
        
    t_pcb * pcb = crear_pcb(id, memoria, 0, -1, config.estimacion_inicial, l_instrucciones);

    // Paso el PCB a la cola de NEW.. Guardo la relacion entre el pid y el socket para responder..
    t_socketID *socketID = malloc(sizeof(t_socketID));
    socketID->id         = id;
    socketID->socket     = cliente_socket;
    socketID->inicio     = currentTimeMillis();

    pthread_mutex_lock(&planificador.mutex[ENEW]);

    agregarSocketID(planificador.lista,socketID);

    lista_insertar(logger, planificador.list[ENEW] , pcb, config.algoritmo, 0);

    pthread_mutex_unlock(&planificador.mutex[ENEW]);

    sem_post(&planificador.sem[ENEW]);
    
    free(args);
    return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Escucha al servidor y por cada cliente crea un nuevo hilo de procesar_conexion
// 
int server_escuchar(char* server_name, int server_socket) {

    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->fd          = cliente_socket;
        args->server_name = server_name;

        //log_info(logger, "Creo nuevo hilo de ejecucion ya que recibi una nueva connsola...");
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        
        return 1;
    }
    return 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Calculo del SRT
// 
double calcularSRT (double estimacionAnterior, double realAnterior, double alfa){
    return alfa * estimacionAnterior + (1 - alfa) * realAnterior; 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Prepara el PCB para ser enviado a la CPU
// 
void* preparar_PCB_KTC(t_pcb* pcb, void* stream, uint8_t inst){
    int offset = sizeof(op) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double);
    
    serializar_header_PCB_KTC(pcb, stream , &inst);
    
    for(int i = 0; i < inst; i ++){
        memcpy(stream + offset, &(pcb->programa[i].coop), sizeof(op) );
        offset += sizeof(op);
        memcpy(stream + offset, &(pcb->programa[i].arg1), sizeof(int) );
        offset += sizeof(int);
        memcpy(stream + offset, &(pcb->programa[i].arg2), sizeof(int) );
        offset += sizeof(int);
    }
    
    return stream;
}
long tiempo_acumulado(int pid){
    
    t_tiempo_ac* nodo;

    // Chequeo si ya tengo el nodo
    for(int i = 0; i < lista_te->elements_count; i++){
        nodo = list_get(lista_te, i);
        if( nodo->pid == pid ){
            long rta = nodo->elapsed;
            list_remove(lista_te, i);
            free(nodo);
            return rta;
        }
	}

    return 0;
}

void manejo_nodo_elapsed(int pid, long elapsed){
    t_tiempo_ac* nodo;

    // Chequeo si ya tengo el nodo
    for(int i = 0; i < lista_te->elements_count; i++){
        nodo = list_get(lista_te, i);
        if( nodo->pid == pid ){
            nodo->elapsed += elapsed;
            return;
        }
	}

    nodo = malloc(sizeof(t_tiempo_ac));
    nodo->pid     = pid;
    nodo->elapsed = elapsed;

    list_add(lista_te, nodo);

    return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Envio del PCB a CPU y recepcion de la rta
// 
sc_cpu enviar_PCB(t_pcb* pcb, uint32_t * tiempoBloqueo_){
    
    //printf("Llegue a entrar aca\n");
    
    char decode_ [][20] = {"FINALIZADO", "BLOCK", "IR", "ERROR"};

    long timeElapsed = 0;
    long tac         = 0;
    long start       = currentTimeMillis();
    long end;
    
    // Me conecto a la CPU y mando el header + programa
    //int conexion = crear_conexion(logger, "CPU", config.ip_cpu, config.puerto_cpu_dispatch);

    size_t size = sizeof(op) + sizeof(int) + sizeof(uint16_t) * 2 + sizeof(uint8_t) * 2 + sizeof(double);

    uint8_t inst = (uint8_t)tamano_intrucciones_pcb(pcb);

    void* stream = malloc(size + sizeof(t_instruccion) * inst);
    stream = preparar_PCB_KTC(pcb, stream, inst);

    size_t instSize = sizeof(op) * inst + sizeof(int) * inst * 2;

    if(conexionCPU_dispatch > 0){ 
        send(conexionCPU_dispatch, stream, size + instSize, 0);
    }
    else
        log_warning(logger, "No me pude conectar al CPU --> IP:%s:%s (%d)", config.ip_cpu, config.puerto_cpu_dispatch, conexionCPU_dispatch);

    sc_val sc;
    recv_validacion(conexionCPU_dispatch , &sc);

    /*
    if(sc == VALIDADO){
        log_info(logger, "Envio de PCB validado!");
    }
    else{
        log_error(logger, "No se valido el envio del PCB!");
        exit(0);
    }
    */

    free(stream);
    

    t_instruccion* instrucciones = pcb->programa;

    sc_cpu   statuscode;
    uint8_t  cantInst   = 1;
    op cop              = 0;

    if(conexionCPU_dispatch != -1){

        if (recv(conexionCPU_dispatch, &cop, sizeof(op), MSG_WAITALL) != sizeof(op)) {
            log_info(logger, "CPU DISCONNECT!");
            exit(EXIT_FAILURE);
        }

        switch (cop) {

            case CPUPCB:
            {
                if (!recv_header_PCB_CTK(conexionCPU_dispatch, pcb, &cantInst, &statuscode, tiempoBloqueo_)) {
                    log_error(logger, "Fallo recibiendo el header de la CPU --> RTA PCB");
                    exit(0);
                }

                // Aca deberia de responder que esta todo ok..
                send_validacion(conexionCPU_dispatch);

                pcb->programa = instrucciones;

                break;
            }
            
            // Errores
            case -1:
                log_error(logger, "CPU desconectada :(");
                return 0;
            default:
                log_error(logger, "CO-COM NO RECONOCIDO! RECEPCION KERNEL -> CPU! :(");
                exit(0);
        }

    }

    end         = currentTimeMillis();
    timeElapsed = timeDifMillis(start, end);

    if(statuscode != IR){
        //Me fijo si tengo un nodo en TE con mi PID
        tac = tiempo_acumulado(pcb->id) + timeElapsed;
        if(config.algoritmo == SRT)
            pcb->srt = calcularSRT(pcb->srt, (double)tac, config.alfa); 
    }
    else{
        //Creo el nodo en la lista
        manejo_nodo_elapsed(pcb->id, timeElapsed);
    }

    if(config.algoritmo == SRT)
        log_info(logger, "Respuesta CPU PCB:%d en %ldms (%ldms) --> StatusCode:%s | Tiempo de bloqueo:%d | SRT:%.1lf", pcb->id, timeElapsed, tac, decode_[statuscode], (int) *tiempoBloqueo_, pcb->srt);
    else
        log_info(logger, "Respuesta CPU PCB:%d en %ldms (%ldms) --> StatusCode:%s | Tiempo de bloqueo:%d", pcb->id, timeElapsed, tac, decode_[statuscode], (int) *tiempoBloqueo_);
    
    return statuscode;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Envia una interrupt a CPU
// 
void enviar_interrupt(IR_code ir){

    //int conexion  = crear_conexion(logger, "CPU_INTERRUPT", config.ip_cpu, config.puerto_cpu_interrupt);

    int size      = sizeof(op) + sizeof(IR_code);
    void* stream  = malloc( size );
    int offset    = 0;
    op cop        = HEADER;

    memcpy(stream, &cop, sizeof(op));
    offset += sizeof(op);
    memcpy(stream + offset, &ir , sizeof(IR_code) );

    if (send(conexionCPU_interrupt, stream, size, 0) != size) {
        free(stream);
        log_error(logger, "Error enviando interrupcion");
        return;
    }

    free(stream);
    return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// notifica a memoria de un suceso
// 
int notificar_memoria(sc_rmem sc, int arg1 , int arg2, int arg3){

	if(conexionMemoria > 0){
        send_protocoloMemoria(conexionMemoria, sc, arg1, arg2, arg3);
    }
    else{
        log_error(logger, "Se murio la conexion con la memoria");
        exit(0);
        return -1;
    }
	
	sc_rmem cop;

	if (recv(conexionMemoria, &cop, sizeof(sc_rmem), MSG_WAITALL) == sizeof(sc_rmem))
    {
        if(cop == SOK){
            recv_protocoloMemoria(conexionMemoria, &arg1, &arg2, &arg3);
            return arg1;
        }
    }
    
    log_error(logger, "notificar_memoria fallo");
    return -1;
}