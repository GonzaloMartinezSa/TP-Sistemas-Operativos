#ifndef CPU_COMS_H_
#define CPU_COMS_H_

#include "utils.h"

bool hay_interrupcion;
void atender_ir(t_log* logger);
int recibir_pcb(t_log* logger, t_pcb* pcb, t_cpuConfig  config);

t_conf_memoria obtener_config_memoria(t_log* logger, t_cpuConfig config);

extern int estoy_ejecutando;

extern int conexionKernel_dispatch;
extern int conexionKernel_interrupt;
extern int conexionMemoria;

#endif