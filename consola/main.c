#include "include/main.h"
#define LOG_CONSOLA "consola.log"

int main(int argc,  char *argv[]) {
    //Creo el log
    t_log* c_logger;
    fclose(fopen(LOG_CONSOLA, "w"));
    c_logger = log_create(LOG_CONSOLA, "Consola", true, LOG_LEVEL_DEBUG);
    
    log_info(c_logger, "Inicio consola con el PID '%d'\n", getpid());

    //Me traigo la config
    t_config* config = leer_config();

    //Leo los argumentos
    char filename[PATH_MAX]={0};

    if (argc != 3)
    {
        log_warning(c_logger, "Uso: ./consola {path} {tamano}\nTerminando consola...");
        return EXIT_FAILURE;
    }
    else{
        strncpy(filename, argv[1], PATH_MAX-1);
    }

    int memoria = atoi(argv[2]);

    if(memoria <= 0){
        log_warning(c_logger, "Se debe solitar memoria para ejecutar un programa!");
        return EXIT_FAILURE;
    }

    // Leo el programita
    int inst = 0;
    t_instruccion* instrucciones = leer_programa(c_logger, filename, &inst);
    log_info(c_logger, "Detecto %d instrucciones!\n", inst);

    // Me conecto al kernel y mando el header + programa
    int conexion = crear_conexion(c_logger, "kernel", config_get_string_value(config, "IP_KERNEL"), config_get_string_value(config, "PUERTO_KERNEL"));

    if(conexion > 0){
        // envio el programa
        enviarPrograma(inst, instrucciones, memoria, conexion);
    }
    else{
        log_warning(c_logger, "No me pude conectar al kernel --> IP:%s:%s (%d)", config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO"), conexion);
    }

    free(instrucciones);

    log_info(c_logger, "Esperando respuesta...\n");

    t_rta_consola msj;

    if(recv_rta_kernel (conexion, &msj)){
        if(msj.sc != 200){
            log_warning(c_logger, "El proceso finalizo con error 0x%d\n", msj.sc); 
        }
        else{
            log_info(c_logger, "El proceso finalizo con status code 0x%d en %ld ms\n",msj.sc, msj.tiempoBloqueo);
        }
    }

    close(conexion);

    config_destroy(config);
    log_destroy(c_logger);
    return EXIT_SUCCESS;
}

