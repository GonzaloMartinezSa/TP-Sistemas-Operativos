#include "../include/paginacion.h"

bool inicializarSwap(char* filename,int cant_uints);
int  buscarDisponiblePrimerNivel();
int  buscarDisponibleSegundoNivel();
int  buscarIndiceMarcosLibre();
bool llevar_marco_a_swap(int numero_marco);

int inicializarProceso(int tamano, int pid) {
    int tamanoPagina = conf.tam_pagina;
    int cantPaginas = (int) ceil((double)tamano / (double)tamanoPagina);
    int cantTP2 = (int) ceil((double)cantPaginas / (double)conf.entradas_por_tabla);

    int cant_uint32 = tamano / sizeof(uint32_t); // <--------------------------------------------------------- REVISAR

    // Todo proceso tiene una tabla de primer nivel
    int indiceTP1 = buscarDisponiblePrimerNivel();

    if(indiceTP1 >= 0){
        // Tengo una posicion para reutilizar no tengo que reservar mas memoria.. solo limpiar
        log_info(logger, "Reutilizo la tabla de 1er nivel %d", indiceTP1);
    }
    else{
        // Tengo que sumar una nueva posicion..
        indiceTP1 = asignadosPrimerNivel;
        asignadosPrimerNivel++;

        if(asignadosPrimerNivel > MAX_TABLAS_P1){
            log_error(logger, "El grado de multiprogramacion es incorrecto... intente asignar mas tablas de primer nivel de las que puedo!");
            exit(0);
        }
        log_info(logger, "Tengo que crear una nueva tabla de 1er nivel.. (Asignada nº:%d)", indiceTP1);
    }

    listaTablasPrimerNivel[indiceTP1].enUso = true;
    listaTablasPrimerNivel[indiceTP1].entradas = malloc(sizeof(int) * cantTP2);
    listaTablasPrimerNivel[indiceTP1].cant_entradas = cantTP2;
    listaTablasPrimerNivel[indiceTP1].punteroClock = 0;

    // Creo el archivito SWAP del proceso
    sprintf(listaTablasPrimerNivel[indiceTP1].swap_filename, "%s/%d.swap", conf.path_swap, pid);
    inicializarSwap(listaTablasPrimerNivel[indiceTP1].swap_filename, cant_uint32);

    // Itero la cantidad de TP2 que necesito
    for(int i = 0; i < cantTP2; i ++){

        int indiceP2 = buscarDisponibleSegundoNivel();

        if(indiceP2 >= 0){
            // Puedo reutilizar una pagina de 2 nivel
            log_info(logger, "Reutilizo la tabla de 2do nivel %d", indiceP2);
        }
        else{
            // Debo crear una nueva pagina
            indiceP2 = asignadosSegundoNivel;
            asignadosSegundoNivel++;
            if(asignadosSegundoNivel > MAX_TABLAS_P2){
                log_error(logger, "El grado de multiprogramacion es incorrecto... intente asignar mas tablas de segundo nivel de las que puedo!");
                exit(0);
            }
            log_info(logger, "Debo crear una nueva tabla de 2do nivel (Asignada nº:%d)", asignadosSegundoNivel);
        }

        listaTablasSegundoNivel[indiceP2].enUso    = true;
        listaTablasSegundoNivel[indiceP2].entradas = malloc(sizeof(t_descpaginaSegundoNivel*) * conf.entradas_por_tabla);
        
        // Reservo e inicializo todas las entradas de mi TP2
        t_descpaginaSegundoNivel* paginaAux;
        for(int i = 0; i < conf.entradas_por_tabla; i++){
            paginaAux = malloc(sizeof(t_descpaginaSegundoNivel));
            paginaAux->M = false;
            paginaAux->P = false;
            paginaAux->U = false;
            paginaAux->marco = -1;
            listaTablasSegundoNivel[indiceP2].entradas[i] = paginaAux;
        }

        listaTablasPrimerNivel[indiceTP1].entradas[i] = indiceP2; // Guardo la numeracion ABSOLUTA de esta TP2
    }

    // Le asigno marcos al proceso
    int indice = buscarIndiceMarcosLibre();
    listaTablasPrimerNivel[indiceTP1].primer_marco = indice * conf.marcos_por_proceso;

    log_info(logger,"El proceso %d tiene como primer marco el nº:%d (Indice:%d)--> Numero de tabla primer nivel %d\n", pid, indice * conf.marcos_por_proceso, indice, indiceTP1);

    return indiceTP1;
}


bool finalizarProceso(int indiceP1){

    t_tablaPaginaPrimerNivel tp1 = listaTablasPrimerNivel[indiceP1];
    int CTP2 = tp1.cant_entradas;

    // Libero las tablas de 2do Nivel
    for(int i = 0; i < CTP2; i ++){
        int NTP2 = tp1.entradas[i];

        listaTablasSegundoNivel[NTP2].enUso = false;

        // <----------------------------------------------------------------------------- ESTO ES NUEVO Y NO ESTA PROBADO
        for(int c = 0; c < conf.entradas_por_tabla; c++){
            free(listaTablasSegundoNivel[NTP2].entradas[c]);
        }

        free(listaTablasSegundoNivel[NTP2].entradas);
    }

    //Libero los marcos asignados
    if(tp1.primer_marco >= 0)
    {
        int indice = (int) (tp1.primer_marco / conf.marcos_por_proceso);
        log_info(logger, "Libero el primer marco %d (Indice:%d)", tp1.primer_marco, indice);
        indice_marco_uso[indice] = false;
        
        int ultimo_marco = tp1.primer_marco + conf.marcos_por_proceso;

        // Marco los marcos (JAJA) en deshuso
        for(int i = tp1.primer_marco; i < ultimo_marco; i ++){

            // Actualizo el BITMAP
            bitmap_marco[i].enUso = false;
            bitmap_marco[i].NTP1 = 0;
            bitmap_marco[i].ETP1 = 0;
            bitmap_marco[i].ETP2 = 0;

            // Pongo en cero la memoria de usuario
            for(int e = 0; e < CANT_UINTS_MARCO; e ++){
                ((t_marco*)espacio_usuario)[i].datos[e] = 0;
            }
        }
    }
    else
        log_info(logger, "Libero el proceso con numero de tabla de primer nivel %d (no tenia marcos asignados)", indiceP1);

    // Elimino el archivo de SWAP para que en la proxima ejecucion no me plote todo
    remove(tp1.swap_filename);

    // Libero la TP1
    tp1.cant_entradas = -1;
    tp1.primer_marco = -1;
    tp1.punteroClock = 0;
    tp1.enUso = false;

    // Libero la tabla de primer nivel
    free(tp1.entradas);
}

bool inicializarSwap(char* filename, int cant_uints){

    struct stat st = {0};

    if (stat(conf.path_swap, &st) == -1) {
		mkdir(conf.path_swap, 0700);
	}

    FILE *fp = fopen(filename, "wb+");
    
    if(!fp){
        log_error(logger, "Error creando archivo SWAP.. Me faltan permisos??");
        exit(0);
    }

    for(int i = 0; i < cant_uints; i ++){
        uint32_t val = 0;
        fwrite(&val, sizeof(uint32_t), 1, fp);
    }

    fclose(fp);
}

int buscarDisponiblePrimerNivel(){
    for(int i = 0; i < asignadosPrimerNivel; i++){
        if(listaTablasPrimerNivel[i].enUso == false){
            return i;
        }
    }
    return -1;
}

int buscarDisponibleSegundoNivel(){
    for(int i = 0; i < asignadosSegundoNivel; i++){
        if(listaTablasSegundoNivel[i].enUso == false){
            return i;
        }
    }
    return -1;
}

int buscarIndiceMarcosLibre(){
    for(int i = 0; i < TOTAL_MARCOS / conf.marcos_por_proceso; i ++){
        if(!indice_marco_uso[i]){
            indice_marco_uso[i] = true;
            return i;
        }
    }
    // No tengo marcos para darle al proceso... algo paso
    log_error(logger, "El grado de multiprogramacion no es congruente con la cantidad de marcos por proceso en asignacion fija :(");
    exit(0);
}

t_marco extraer_pagina_swap(int numeroTabla, int entrada, int NTP1){


    log_info(logger, "Trayendo de SWAP NTP2(R):%d Entrada:%d NTP1:%d de SWAP\n", numeroTabla, entrada, NTP1);
   
    int numeroPagina = numeroTabla  * conf.entradas_por_tabla + entrada;
    int numero_uint  = numeroPagina * CANT_UINTS_MARCO;

    FILE* fp = fopen(listaTablasPrimerNivel[NTP1].swap_filename, "rb");
    
    fseek(fp, 0, SEEK_SET);
    fseek(fp, numero_uint  * sizeof(uint32_t), SEEK_SET);

    // Preapro el puntero en donde voy a leer
    t_marco marco;
    marco.datos = malloc(conf.tam_pagina);
    
    for(int i = 0; i < CANT_UINTS_MARCO; i ++){
        uint32_t val = 0;
        fread(&val, sizeof(uint32_t), 1, fp);
        //printf("POS:%d --> VALOR: %d\n", i+numero_uint, val);
        marco.datos[i] = val;
    }

    fclose(fp);

    usleep(conf.retardo_swap * 1000);

    return marco;
}

int buscar_marco_libre(int NTP1){

    int primerMarco = listaTablasPrimerNivel[NTP1].primer_marco;
    int ultimoMarco = listaTablasPrimerNivel[NTP1].primer_marco + conf.marcos_por_proceso;

    for(int i = primerMarco; i < ultimoMarco; i++){
        if(!bitmap_marco[i].enUso){

            bitmap_marco[i].enUso = true;
            return i;
        }
    }

    return -1;
}

bool llevar_marco_a_swap(int numero_marco){

    if(bitmap_marco[numero_marco].enUso){

        int numeroPagina = bitmap_marco[numero_marco].ETP1 * conf.entradas_por_tabla + bitmap_marco[numero_marco].ETP2;
        int numero_uint = numeroPagina * CANT_UINTS_MARCO;

        //log_info(logger, "Me llevo marco nº%d a swap (%s)", numero_marco, listaTablasPrimerNivel[bitmap_marco[numero_marco].NTP1].swap_filename);
        
        FILE* fp = fopen(listaTablasPrimerNivel[bitmap_marco[numero_marco].NTP1].swap_filename, "rb+");
        
        fseek(fp, numero_uint  * sizeof(uint32_t), SEEK_SET);

        for(int i = 0; i < CANT_UINTS_MARCO; i ++){
            uint32_t val = ((t_marco*)espacio_usuario)[numero_marco].datos[i];
            fwrite(&val, sizeof(uint32_t), 1, fp);
        }

        fclose(fp);

        usleep(conf.retardo_swap * 1000);
        
        return true;
    }
    else{
        log_error(logger,"Estoy intentando swapear un marco que no esta en uso!");
        exit(0);
    }
}


bool suspenderProceso(int NTP1){

    t_tablaPaginaPrimerNivel  t1 = listaTablasPrimerNivel[NTP1];
    int primer_marco = t1.primer_marco;
    int ultimo_marco = t1.primer_marco + conf.marcos_por_proceso;

    if(primer_marco < 0)
    {
        log_info(logger,"Suspendiendo proceso NTP1:%d el mismo no tenia marcos asociados!", NTP1);
        return 1;
    }
    
    for(int i = primer_marco; i < ultimo_marco; i ++){
        
        int dir_NTP2 = t1.entradas[bitmap_marco[i].ETP1];
        t_tablaPaginaSegundoNivel t2 = listaTablasSegundoNivel[dir_NTP2];
        
        if(bitmap_marco[i].enUso){
            if(t2.entradas[bitmap_marco[i].ETP2]->M){
                // Lo tengo que mandar a SWAP
                llevar_marco_a_swap(i);
                memset(((t_marco*)espacio_usuario)[i].datos, 0, sizeof(conf.tam_pagina));
            }
        }

        bitmap_marco[i].enUso = false;
        bitmap_marco[i].NTP1 = 0;
        bitmap_marco[i].ETP1 = 0;
        bitmap_marco[i].ETP2 = 0;
    }   
    // Ponho los bits de control para la suspension de todas las entradas de tablas por las dudas..
    for(int i = 0 ; i < t1.cant_entradas; i ++){

        for(int e = 0; e < conf.entradas_por_tabla; e ++ ){

            listaTablasSegundoNivel[t1.entradas[i]].entradas[e]->P = false;
            listaTablasSegundoNivel[t1.entradas[i]].entradas[e]->M = false;
            listaTablasSegundoNivel[t1.entradas[i]].entradas[e]->U = false;
            listaTablasSegundoNivel[t1.entradas[i]].entradas[e]->marco = -1;
        }
    }

    int indice_liberado = (int) (t1.primer_marco / conf.marcos_por_proceso); 
    t1.primer_marco = -1;
    indice_marco_uso[indice_liberado] = false;

    log_info(logger, "Libere el indice de marcos:%d", indice_liberado);
}

int dir_relativa_tp2(t_tablaPaginaPrimerNivel tp1, int absoluta){
    int cant_entradas = tp1.cant_entradas;

    for(int i = 0; i < cant_entradas; i ++){
        if(tp1.entradas[i] == absoluta){
            return i;
        }
    }

    log_error(logger, "No encontre la direccion relativa para el NTP2:%d", absoluta);
    exit(0);
}

int acceso_tabla(int arg1, int arg2, int arg3){

    if(arg3 < 0){
        // NTP1  = arg1
        // ETP1  = arg2
        // -1    = arg3  
        if(arg1 > asignadosPrimerNivel){
            log_warning(logger, "Error en direccionamiento a Tabla de 1er Nivel!");
            exit(0);
        }
        
        int rta = listaTablasPrimerNivel[arg1].entradas[arg2];

        if(arg2 > listaTablasPrimerNivel[arg1].cant_entradas){
            log_error(logger, "ERROR de direccionamiento.. Se intento exceder el numero de entrada de la TP1 Nº:%d (Cant Entradas:%d)!", arg1, listaTablasPrimerNivel[arg1].cant_entradas);
            exit(0);
        }

        return rta;
    }
    else{
        // NTP2  = arg1 --> Absoluto
        // ETP2  = arg2
        // NTP1  = arg3  
        if(arg1 > asignadosSegundoNivel){
            log_warning(logger, "Error en direccionamiento a Tabla de 2do Nivel!");
            exit(0);
        }

        t_tablaPaginaPrimerNivel tp1 = listaTablasPrimerNivel[arg3];

        // Calculo direccion absoluta y relativa de mi TP2
        int dirAbsoluta = arg1;
        int dirRelativa = dir_relativa_tp2(tp1, dirAbsoluta);

        //log_info(logger, "Direccion Absoluta:%d, Direccion relativa:%d",dirAbsoluta, dirRelativa);

        t_tablaPaginaSegundoNivel tp2 = listaTablasSegundoNivel[dirAbsoluta];

        // Chequeo si vengo de una suspension
        if(tp1.primer_marco < 0){
            int indice = buscarIndiceMarcosLibre();
            tp1.primer_marco = indice * conf.marcos_por_proceso;
            log_info(logger, "Des-suspenden a la tabla de paginas de primer nivel nº%d, le asigno el indice de marcos %d (Primer marco:%d)", arg3, indice, tp1.primer_marco);
        } 

        // Chequeo presencia
        if(tp2.entradas[arg2]->P){
            int rta = tp2.entradas[arg2]->marco;
            log_info(logger, "PAGE HIT (NTP2 Abs:%d Rel:%d) (E:%d) (#P:%d) --> MARCO Nº: %d", dirAbsoluta, dirRelativa, arg2, dirRelativa * conf.entradas_por_tabla + arg2, rta);
            return rta;
        }
        else{

            log_info(logger, "PAGE FAULT!");
            
            t_marco marco = extraer_pagina_swap(dirRelativa, arg2, arg3);

            int marco_a_usar = buscar_marco_libre(arg3);

            if(marco_a_usar < 0){
                
                if(conf.algoritmo_reemplazo == CLOCK){
                    log_info(logger, "SWAPEO CON CLOCK");
                    marco_a_usar = reemplazoClock(arg3);
                }
                if(conf.algoritmo_reemplazo == CLOCKM){
                    log_info(logger, "SWAPEO CON CLOCK MEJORADO");
                    marco_a_usar = reemplazoClockMejorado(arg3);
                }

                log_info(logger, "Desalojando el marco Nº%d --> el puntero queda en %d", marco_a_usar, tp1.punteroClock);
                
                llevar_marco_a_swap(marco_a_usar);

                int dirNTP2_desalojar = listaTablasPrimerNivel[arg3].entradas[bitmap_marco[marco_a_usar].ETP1];
                t_tablaPaginaSegundoNivel tp2_desalojar = listaTablasSegundoNivel[dirNTP2_desalojar];
                
                if(tp2_desalojar.enUso){
                    tp2_desalojar.entradas[bitmap_marco[marco_a_usar].ETP2]->P = false;
                    tp2_desalojar.entradas[bitmap_marco[marco_a_usar].ETP2]->M = false;
                    tp2_desalojar.entradas[bitmap_marco[marco_a_usar].ETP2]->U = false;
                }
                else{
                    log_error(logger, "Estoy reemplazando un marco que no estaba en uso!");
                    exit(0);
                }
            }

            // Muevo a memoria de usuario el nuevo marco

            //memcpy(&espacio_usuario[marco_a_usar], &marco, sizeof(t_marco));

            for(int i = 0; i < CANT_UINTS_MARCO; i ++){
                ((t_marco*)espacio_usuario)[marco_a_usar].datos[i] = marco.datos[i];
            }

            free(marco.datos);

            // Actualizo el bitmap
            bitmap_marco[marco_a_usar].enUso = true;
            bitmap_marco[marco_a_usar].NTP1 = arg3;
            bitmap_marco[marco_a_usar].ETP1 = dirRelativa;
            bitmap_marco[marco_a_usar].ETP2 = arg2;

            // Actualizo la pagina de segundo nivel del nuevo marco
            tp2.entradas[arg2]->P = true;
            tp2.entradas[arg2]->marco = marco_a_usar;
            tp2.entradas[arg2]->U = true;
            tp2.entradas[arg2]->M = false;

            //log_info(logger, "COLOQUE (NTP2 Abs:%d Rel:%d) (E:%d) (#P:%d) --> MARCO Nº: %d", dirAbsoluta, dirRelativa, arg2, dirRelativa * conf.entradas_por_tabla + arg2, marco_a_usar);

            return marco_a_usar;
        }
        
    }

}