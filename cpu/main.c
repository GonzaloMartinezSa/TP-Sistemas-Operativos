#include "include/main.h"

#define LOG_CPU "cpu.log"


void reset_interrupt(){
    hay_interrupcion = 0;
}

int main(int argc, char** argv) {
	fclose(fopen(LOG_CPU, "w"));
	logger_cpu_ciclo = log_create(LOG_CPU, "CPU", 1, LOG_LEVEL_DEBUG);

	if(argc < 2) {
		log_error(logger_cpu_ciclo, "Falta parametro de configuracion!");
        return EXIT_FAILURE;
    }

	config_cpu    = cpu_config(argv[1]);
	pcb           = malloc(sizeof(t_pcb));

	c_memoria = obtener_config_memoria(logger_cpu_ciclo, config_cpu);

	log_info(logger_cpu_ciclo,"Ya estoy esperando en Dispatch a Kernel\n");
	log_info(logger_cpu_ciclo,"Ya estoy esperando en Interrupt a Kernel\n");

	int server_com  = iniciar_servidor(logger_cpu_ciclo, "cpu_dispatch", "0.0.0.0", config_cpu.puerto_escucha_dispatch);
	int server_com2 = iniciar_servidor(logger_cpu_ciclo, "cpu_interrupts", "0.0.0.0", config_cpu.puerto_escucha_interrupt);
	
	conexionKernel_dispatch  = esperar_cliente(logger_cpu_ciclo, "cpu_dispatch", server_com);
    conexionKernel_interrupt = esperar_cliente(logger_cpu_ciclo, "cpu_interrupt", server_com2);
	
	int status       = init_cpu_coms(logger_cpu_ciclo, config_cpu, pcb);
	estoy_ejecutando = 0;

	iniciar_tlb(logger_cpu_ciclo);

	reset_interrupt();

	while(1){

		log_info(logger_cpu_ciclo, "Esperando un PCB");

		if(recibir_pcb(logger_cpu_ciclo, pcb, config_cpu)){

			limpiar_tlb();
			
			estoy_ejecutando = 1;

			log_info(logger_cpu_ciclo, "Procesando PCB: %d", pcb->id);

			ciclo_de_instruccion(pcb);

			estoy_ejecutando = 0;

			reset_interrupt();

		}
		else{

			log_error(logger_cpu_ciclo, "Fallo recibiendo pcb del kernel");
			return false;
		}
	}

	return 0;
}
