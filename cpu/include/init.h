#ifndef UITLS_INIT_H_
#define UITLS_INIT_H_

#include "utils.h"
#include <commons/log.h>
#include <commons/config.h> 
#include "comunicaciones.h"

t_cpuConfig cpu_config(char* argv);
int init_cpu_coms(t_log* logger, t_cpuConfig config, t_pcb* pcb);
extern t_log* logger_cpu_ciclo;
#endif 
