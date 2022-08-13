#include "../include/comunicaciones.h"


int recibir_pcb(t_log* logger, t_pcb* pcb, t_cpuConfig  config) {

    //*cliente_socket = esperar_cliente(logger, server_name, server_socket);

    // Aca proceso la solicitud del kernel
    op cop         = 0;
    t_instruccion* l_instrucciones;
    int l_pos      = 0;
    uint8_t  c_ins = 1;
    bool header    = 0;
        
    if (recv(conexionKernel_dispatch, &cop, sizeof(op), MSG_WAITALL) != sizeof(op)) {
        log_info(logger, "KERNEL DISCONNECT!");
        exit(0);
    }

    int memoria;
    uint8_t instrucciones;
    pid_t id;

    op co_op;
    int arg1;
    int arg2;

    switch (cop) {

        case KERNELPCB:
        {
            
            if (!recv_header_PCB_KTC(conexionKernel_dispatch, pcb, &c_ins)) {
                log_error(logger, "Fallo recibiendo el header del kernel! :(");
                exit(0);
            }
            //Armo el vector que voy a usar para guardar las instrucciones
            l_instrucciones = malloc(sizeof(t_instruccion) * c_ins);
            memset(l_instrucciones, 0, sizeof(t_instruccion) * c_ins);

                
            for(int l_pos = 0; l_pos < c_ins; l_pos++){

                if (!recv_instruccion(conexionKernel_dispatch, &co_op, &arg1, &arg2)) {
                    log_error(logger, "Fallo recibiendo instruccion! :(");
                    exit(0);
                }

                //log_info(c_logger, "lpos:(%d) coop:%s %d %d\n",l_pos, decode[co_op], arg1, arg2);

                //Sumo la instruccion a mi lista de instrucciones
                
                l_instrucciones[l_pos].coop = co_op;
                l_instrucciones[l_pos].arg1 = arg1;
                l_instrucciones[l_pos].arg2 = arg2;

                //Rastreo del protocolo... podria sumar a l_pos
                pcb->programa = l_instrucciones;
              
            }

            send_validacion(conexionKernel_dispatch);

            pthread_mutex_unlock(&(config.inst_mutex));

            return 1;
            
        }
        
        // Errores
        case -1:
            log_error(logger, "Kernel desconectado :(");
            return 0;
        default:
            log_error(logger, "CO-COM DESCONOCIDO! Recepcion PCB!");
            exit(0);
    
    }
    
    return 1;

}


int server_escuchar_kernel(t_log* logger, char* server_name){
    
    //log_warning(logger, "llego algo a interrupt");
        
    op cop      = 0;
    IR_code ir  = 1;
    bool header = 0;
        
    if (recv(conexionKernel_interrupt, &cop, sizeof(op), MSG_WAITALL) != sizeof(op)) {
        log_info(logger, "INTERRUPT DISCONNECT!");
        return 0;
    }

    if(recv(conexionKernel_interrupt, &ir, sizeof(IR_code), MSG_WAITALL) != sizeof(IR_code)) {
        log_info(logger, "INTERRUPT DISCONNECT!");
        return 0;
    }

    switch (ir) {

        case DESALOJO:
        {
            if(estoy_ejecutando){
                hay_interrupcion = 1; 
                log_warning(logger, "INTERRUPT!");
            }       
            break;
        }
        
        // Errores
        case -1:
            log_error(logger, "ir_code incorrecto..., desconectado de %s", server_name);
            return 0;
        default:
            log_error(logger, "Algo anduvo mal en el server de %s", server_name);
            log_info(logger, "Cop: %d", cop);
            return 0;
    
    }

    return 1;
}

void atender_ir(t_log* logger) {

    // t_pcbs_args* args = (t_pcbs_args*) void_args;
    // t_log* logger                   = args->log;
    // t_cpuConfig  config             = args->config;
    // t_pcb* pcb                      = args->pcb;
    // int*   kernel_com               = args->kernel_com;

    while (server_escuchar_kernel(logger, "cpu_interrupts"));
}


t_conf_memoria obtener_config_memoria(t_log* logger, t_cpuConfig config){

    t_conf_memoria conf;
    conexionMemoria = crear_conexion(logger, "memoria", config.ip_memoria , config.puerto_memoria);
    
	if(conexionMemoria > 0){
        send_protocoloMemoria(conexionMemoria, MEMINFO, 1, 2, 3);
    }
    else{
        log_error(logger, "No se pudo conectar con la memoria");
        exit(1);
    }
	
	sc_rmem cop;
	int arg1 = -1, arg2,arg3;
    
	if (recv(conexionMemoria, &cop, sizeof(sc_rmem), MSG_WAITALL) == sizeof(sc_rmem)) {
    	if(recv_protocoloMemoria(conexionMemoria, &arg1, &arg2, &arg3)){
			if(arg3 == 123)
        		log_info(logger, "Configuracion de memoria recibida --> TamañoPagina:%d, CantidadEntradasXPagina:%d", arg1, arg2);
			else{
				log_error(logger, "Detecto ruido recibiendo info de la memoria");
                arg1 = -1;
            }
    	}
        else
            arg1 = -1;
    }
	
    conf.cant_entradas_x_pagina = arg2;
    conf.tamano_pagina          = arg1;

    //close(memoria);

    return conf;
}