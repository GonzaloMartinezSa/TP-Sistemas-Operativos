#include "../include/init.h"

t_cpuConfig cpu_config(char* argv){
	
	// Leer de config
	t_config* config = config_create(argv);


	t_cpuConfig conf;

	conf.entradas_tlb             = config_get_int_value(config, "ENTRADAS_TLB");


	char aux[10];
	memset(aux, 0 , 10);
    memcpy(aux, config_get_string_value(config, "REEMPLAZO_TLB"), strlen(config_get_string_value(config, "REEMPLAZO_TLB")));

	if(!strcmp(aux, "FIFO"))
		conf.reemplazo_tlb  = FIFO;
	else if(!strcmp(aux, "LRU"))
		conf.reemplazo_tlb  = LRU;
	else{
		log_error(logger_cpu_ciclo, "NO SE RECONOCE EL ALGORITMO DE REEMPLAZO! '(%s)",aux );
		exit(1);
	}

	conf.retardo_noop       = config_get_int_value(config, "RETARDO_NOOP");

	memset(conf.ip_memoria, 0 , 80);
    memcpy(conf.ip_memoria, config_get_string_value(config, "IP_MEMORIA"), strlen(config_get_string_value(config, "IP_MEMORIA")));

	memset(conf.puerto_memoria, 0 , 15);
    memcpy(conf.puerto_memoria, config_get_string_value(config, "PUERTO_MEMORIA"), strlen(config_get_string_value(config, "PUERTO_MEMORIA")));

	memset(conf.puerto_escucha_dispatch, 0 , 15);
    memcpy(conf.puerto_escucha_dispatch, config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"), strlen(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH")));

	memset(conf.puerto_escucha_interrupt, 0 , 15);
    memcpy(conf.puerto_escucha_interrupt, config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"), strlen(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT")));

    pthread_mutex_init(&(conf.inst_mutex), NULL);
	pthread_mutex_lock(&(conf.inst_mutex));

	config_destroy(config);

	return conf;
}

int init_cpu_coms(t_log* logger, t_cpuConfig config, t_pcb* pcb){
    // t_pcbs_args* args = malloc(sizeof(t_pcbs_args));
    // args->log        = logger;
    // args->config     = config;
	// args->pcb        = pcb;

	pthread_t atender_kernel_;
    pthread_create(&atender_kernel_, NULL, (void*) atender_ir, logger);
    pthread_detach(atender_kernel_);
	

	return 1;
}