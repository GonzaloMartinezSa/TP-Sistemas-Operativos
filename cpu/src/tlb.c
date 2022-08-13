#include "../include/tlb.h"

// Privado

t_log* logger_tlb;

struct {
    int* paginas;
    int* marcos;
}tlb;

int* cola_paginas_fifo;
t_list* lista_paginas_lru;

int cant_entradas;
algoritmo_reemplazo algoritmo;

void imprimir_tlb();
bool tlb_full(void);
int  tlb_find_pagina(int pagina);
int  tlb_find_marco(int marco);
void poner_en_tlb(int entrada, int pagina, int marco);
void reemplazar_marco_ya_existente(int pagina, int marco);
void reemplazar_normal(int pagina, int marco);
void reemplazar_por_algoritmo(int pagina, int marco);
void logica_fifo(int pagina, int marco);
void logica_lru(int pagina, int marco);
void poner_en_lista_lru(int pagina);
void func_limpiar_lru(int* puntero);


//                          0       1       2       ...
//              PAG     |      |       |       |
//     TLB      ----------------------------------------
//              MARCO   |      |       |       |
//          
//          (La cantidad de entradas es definida por config)
//          Por config default, es 4.

// TABLA DE TLB:
//------------------------
// struct {
//    int* paginas;
//    int* marcos;
// }tlb;
//
// pagina en entrada x = tlb.paginas[entrada_x]
// marco  en entrada x = tlb.marcos[entrada_x]


// COLA DE PAGINAS PARA FIFO:
//-------------------------
// int* cola_paginas_fifo;
//
// Ej: Cant. de entradas = 4
//
// [2,3,-1,-1] 
// viene 7, no hay hit
// [2,3,7,-1]
//
// [2,3,4,5]
// viene 8, no hay hit
// [3,4,5,8]


// LISTA ENLAZADA DE PAGINAS PARA LRU
// ----------------------------------
// t_list* lista_paginas_lru;
//
// Ej: Cant. de entradas = 4
//
// []
// viene 2, no importa si hay hit o no
// [2]
// viene 7, no importa si hay hit o no
// [2,7]
//
// Y hay un array aux q es una copia de las paginas en tlb, 
// para poder comparar con la lista esta y modificarse segun corresponda (NO es global esta)


void iniciar_tlb(t_log* logger) {

    log_info(logger, "Iniciando TLB!");
    cant_entradas = config_cpu.entradas_tlb;
    algoritmo = config_cpu.reemplazo_tlb;
    
    if(algoritmo == FIFO)
        log_info(logger, "El algoritmo de reemplazo de TLB es FIFO");
    else
        log_info(logger, "El algoritmo de reemplazo de TLB es LRU");
        
    logger_tlb = logger;

    cola_paginas_fifo = malloc(sizeof(int) * cant_entradas); // Para FIFO

    lista_paginas_lru = list_create(); // Para LRU

    tlb.paginas = (int*) malloc(sizeof(int) * cant_entradas);
    tlb.marcos  = (int*) malloc(sizeof(int) * cant_entradas);

    limpiar_tlb();
}

bool buscar_en_tlb(int dirLogica, int* dirFisica) {

    int tamPagina = c_memoria.tamano_pagina;
    int pagina = dirLogica / (tamPagina);
    int desplazamiento = dirLogica - pagina * (tamPagina);

    //log_info(logger_tlb, "\nDir Logica: %d\nNumero de pagina: %d\nDesplazamiento: %d", dirLogica, pagina, desplazamiento);
    
    // int pagina = dirLogica; para pruebas

    int entrada = tlb_find_pagina(pagina);
    if(entrada != -1) {
        int marco = tlb.marcos[entrada];
        *dirFisica = marco * 1000 + desplazamiento;
        // *dirFisica = marco; para pruebas
        poner_en_lista_lru(pagina);
        return true;
    }
    else
        return false;
}

void poner_en_tlb(int entrada, int pagina, int marco) {
    // Privado, para abstraer el poner los datos directamente en la tlb

    if(algoritmo == FIFO) {
        // Actualizo cola FIFO
        if(!tlb_full()) {
            // Encolo
            for(int i=0; i<cant_entradas; i++) {
                if(cola_paginas_fifo[i] == -1) {
                    cola_paginas_fifo[i] = pagina;
                    break;
                }
            }
        } else {
            // Desplazo a la izq. y pongo al final
            for(int i=1; i<cant_entradas; i++) {
                cola_paginas_fifo[i-1] = cola_paginas_fifo[i];
            }
            cola_paginas_fifo[cant_entradas - 1] = pagina;
        }
    } else if(algoritmo == LRU) {
        // Actualizo lista LRU
        poner_en_lista_lru(pagina);
    }
    
    tlb.paginas[entrada] = pagina;
    tlb.marcos[entrada]  = marco;

    // Ver la cola
    /*
    log_info(logger_tlb, "Cola:");
    for(int j=0; j<cant_entradas; j++) {
        log_info(logger_tlb, "%d", cola_paginas_fifo[j]);
    }
    */
}

void reemplazar_marco_ya_existente(int pagina, int marco) {
    log_warning(logger_tlb, "Se sobreescribe un marco.");
    int entrada = tlb_find_marco(marco);
    poner_en_tlb(entrada, pagina, marco);
    imprimir_tlb();
}

void cargar_en_tlb(int pagina, int marco) {
    
    if(tlb_find_pagina(pagina) != -1) {
        log_error(logger_tlb, "Trataste de cargar una pagina que ya estaba cargada");
        return;
    }

    if(tlb_find_marco(marco) != -1) {
        reemplazar_marco_ya_existente(pagina, marco);
        return;
    }

    if(!tlb_full()) {
        // Meter al final de la cola
        reemplazar_normal(pagina, marco);
        imprimir_tlb();
    } else {
        // Usar algoritmo de sustitucion
        reemplazar_por_algoritmo(pagina, marco);
        imprimir_tlb();
    }

}

bool tlb_full() {

    int entrada = tlb_find_pagina(-1);

    if(entrada == -1)
        return true;
    else
        return false;
}

void reemplazar_normal(int pagina, int marco) {

    log_info(logger_tlb, "Reemplazo normal");

    int entrada = tlb_find_pagina(-1);
    poner_en_tlb(entrada, pagina, marco);
}

void reemplazar_por_algoritmo(int pagina, int marco) {

    log_info(logger_tlb, "Reemplazo por algoritmo");

    if(algoritmo == FIFO) {

        logica_fifo(pagina, marco);

    } else if(algoritmo == LRU) {

        logica_lru(pagina, marco);

    }

}

void logica_fifo(int pagina, int marco) {

    int pag_a_reemplazar = cola_paginas_fifo[0]; // First In
    int entrada_a_reemplazar = tlb_find_pagina(pag_a_reemplazar);
    poner_en_tlb(entrada_a_reemplazar, pagina, marco);
}

void logica_lru(int pagina, int marco) {

    // Pos. (indicie) de la lista LRU donde voy a empezar a mirar
    int pos_inicio = list_size(lista_paginas_lru);
    // Valor que hay en la posicion de la lista LRU en un determinado momento
    int valor_aux;
    // Array copia del array de paginas de la TLB. Va a ser modificado
    int* array_aux = malloc(sizeof(int) * cant_entradas);
    for(int i=0; i<cant_entradas; i++) {
        array_aux[i] = tlb.paginas[i];
    }
    // Empiezo a ver en la posicion inicial, y voy yendo para atras de uno en uno
    for(int i=pos_inicio-1; i>=0; i--) {
        // Agarro el valor de la posicion actual de la lista LRU
        valor_aux = *((int*)list_get(lista_paginas_lru, i));
        // Busco donde esta ese valor en el array auxiliar y lo borro
        for(int j=0; j<cant_entradas; j++) {
            if(array_aux[j] == valor_aux) {
                array_aux[j] = -1;
                break;
            }
        }
        // Me fijo si queda uno, que seria el ultimo que se uso (Last Recently Used)
        // Si queda uno, pongo la pag. y el marco donde estaba ese que fue el ultimo que se uso
        int indice_ultimo;
        int cant = 0;
        for(int j=0; j<cant_entradas; j++) {
            if(array_aux[j] != -1) {
                indice_ultimo = j;
                cant++;
            }
        }
        if(cant == 1) {
            poner_en_tlb(indice_ultimo, pagina, marco);
            free(array_aux);
            break;
        }
    }
}

int  tlb_find_pagina(int pagina) {

    for(int i=0; i<cant_entradas; i++) {
        if(tlb.paginas[i] == pagina) {
            return i;
        }
    }
    //log_error(logger_tlb, "Se busco la entrada de una pag. en TLB, pero no estaba en primer lugar");
    return -1;
}

int  tlb_find_marco(int marco) {

    for(int i=0; i<cant_entradas; i++) {
        if(tlb.marcos[i] == marco) {
            return i;
        }
    }
    //log_error(logger_tlb, "Se busco la entrada de una pag. en TLB, pero no estaba en primer lugar");
    return -1;
}

void poner_en_lista_lru(int pagina) {

    int* pag = malloc(sizeof(int));
    *pag = pagina;
    list_add(lista_paginas_lru, (void*)pag);
}

void imprimir_tlb() {

    for(int j=0; j<cant_entradas; j++) {
        log_info(logger_tlb, "TLB Entrada: %d\t Pagina: %d\t Marco: %d", j, tlb.paginas[j], tlb.marcos[j]);
    }
}

void limpiar_tlb() {

    for(int j=0; j<cant_entradas; j++) {
        // -1 = No hay nada
        tlb.paginas[j] = -1;
        tlb.marcos[j] = -1; 
    }
    for(int i=0; i<cant_entradas; i++) {
        cola_paginas_fifo[i] = -1;
    }
    if(list_size(lista_paginas_lru) > 0) {
        list_clean_and_destroy_elements(lista_paginas_lru, (void*)func_limpiar_lru);
    }
    /*
    log_info(logger_tlb, "\n chequeo q se limpio bien:");
    imprimir_tlb();
    for(int i=0; i<cant_entradas; i++) {
        log_info(logger_tlb, "Cola FIFO: %d", cola_paginas_fifo[i]);
    }
    log_info(logger_tlb, "La cant. de elementos en la Lista LRU es: %d", list_size(lista_paginas_lru));
    */
}

void func_limpiar_lru(int* puntero) {
    free(puntero);
}


// TESTS

void tests_tlb(t_log* logger) {

    // Con lo sig. el test no pasa tal cual:
    //
    //int tamPagina = c_memoria.tamano_pagina;
    //int pagina = floor(dirLogica / tamPagina);
    //int desplazamiento = dirLogica - pagina * tamPagina;

    iniciar_tlb(logger);
	int dirLogica = 2;
	int dirFisica = 50;
	bool ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 7;
	dirFisica = 100;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 8;
	dirFisica = 110;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 6;
	dirFisica = 70;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 7;
	dirFisica = 15;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 2;
	dirFisica = 23;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 5;
	dirFisica = 25;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 8;
	dirFisica = 90;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}
	dirLogica = 3;
	dirFisica = 67;
	ok = buscar_en_tlb(dirLogica, &dirFisica);
	if(ok) {printf("Hit en TLB! La direccion fisica es :%d\n", dirFisica);} else 
	{printf("No hit :(\n"); cargar_en_tlb(dirLogica, dirFisica);}

	// VALORES ESPERADOS:
	//----------------------------
	// FIFO:
	// [-1,-1,-1,-1]
	// [2,-1,-1,-1]   <--- vino 2, no hit
	// [2,7,-1,-1]    <--- vino 7, no hit
	// [2,7,8,-1]     <--- vino 8, no hit
	// [2,7,8,6]      <--- vino 6, no hit
	// [2,7,8,6]      <--- vino 7, hit
	// [2,7,8,6]      <--- vino 2, hit
	// [5,7,8,6]      <--- vino 5, no hit
	// [5,7,8,6]      <--- vino 8, hit
	// [5,3,8,6]      <--- vino 3, no hit
	//
	// LRU:
	// [-1,-1,-1,-1]
	// [2,-1,-1,-1]   <--- vino 2, no hit
	// [2,7,-1,-1]    <--- vino 7, no hit
	// [2,7,8,-1]     <--- vino 8, no hit
	// [2,7,8,6]      <--- vino 6, no hit
	// [2,7,8,6]      <--- vino 7, hit
	// [2,7,8,6]      <--- vino 2, hit
	// [2,7,5,6]      <--- vino 5, no hit
	// [2,7,5,8]      <--- vino 8, no hit
	// [2,3,5,8]      <--- vino 3, no hit
}