#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/protocolo.h"
#include "utils.h"
#include <math.h>

extern t_cpuConfig config_cpu;

// Publico
void iniciar_tlb(t_log* logger);
bool buscar_en_tlb(int dirLogica, int* dirFisica);
void cargar_en_tlb(int pagina, int marco);
void limpiar_tlb();

void tests_tlb(t_log* logger);

#endif