#ifndef MEMUSR_H_
#define MEMUSR_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "utils.h"
#include "comunicaciones.h"
#include "reemplazo.h"

int acceso_memoria_usr(OPM modo, int marco, int desplazamiento, uint32_t *dato);
int acceso_espacio_de_usuario(OPM modo, int numero_de_marco, int desplazamiento, uint32_t *dato);


#endif /* INIT_H_ */