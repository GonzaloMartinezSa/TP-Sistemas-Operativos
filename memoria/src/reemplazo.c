#include "../include/reemplazo.h"


int reemplazoClock(int numeroTablaPrimerNivel){

    t_tablaPaginaPrimerNivel tp1 = listaTablasPrimerNivel[numeroTablaPrimerNivel];

    int primerMarcoUsuario = tp1.primer_marco;
    int puntero = tp1.punteroClock;
    int marcosPorProceso = conf.marcos_por_proceso;

    bool uso = 0;

    while(1){

        int marco_accedido = puntero + primerMarcoUsuario;

        int dirNTP2 = tp1.entradas[bitmap_marco[marco_accedido].ETP1];
        t_tablaPaginaSegundoNivel tp2 = listaTablasSegundoNivel[dirNTP2];

        uso = tp2.entradas[bitmap_marco[marco_accedido].ETP2]->U;

        if(puntero < marcosPorProceso - 1){
            puntero ++;
        }
        else{
            puntero = 0;
        }

        if(uso){
            tp2.entradas[bitmap_marco[marco_accedido].ETP2]->U = false;
        }
        else{
            tp1.punteroClock = puntero;
            return marco_accedido;
        }
    }
}

int reemplazoClockMejorado(int numeroTablaPrimerNivel){

    t_tablaPaginaPrimerNivel tp1 = listaTablasPrimerNivel[numeroTablaPrimerNivel];

    int primerMarcoUsuario = tp1.primer_marco;
    int puntero = tp1.punteroClock;
    int marcosPorProceso = conf.marcos_por_proceso;

    bool modificado = 0;
    bool uso = 0;
    int marco_accedido = 0;
    int NTP2 = 0;
    
    while(1){

        // PRIMER LOOP
        for(int i = 0; i < marcosPorProceso; i++){

            marco_accedido = puntero + primerMarcoUsuario;

            int dirNTP2 = tp1.entradas[bitmap_marco[marco_accedido].ETP1];
            t_tablaPaginaSegundoNivel tp2 = listaTablasSegundoNivel[dirNTP2];

            uso = tp2.entradas[bitmap_marco[marco_accedido].ETP2]->U;
            modificado = tp2.entradas[bitmap_marco[marco_accedido].ETP2]->M;

            if(puntero < marcosPorProceso - 1)
                puntero ++;
            else
                puntero = 0;
            
            if(!uso && !modificado){
                tp1.punteroClock = puntero;
                return marco_accedido;
            }
        }

        //Segundo LOOP
        for(int i = 0; i < marcosPorProceso; i++){
            marco_accedido = puntero + primerMarcoUsuario;
            
            int dirNTP2 = tp1.entradas[bitmap_marco[marco_accedido].ETP1];
            t_tablaPaginaSegundoNivel tp2 = listaTablasSegundoNivel[dirNTP2];

            uso = tp2.entradas[bitmap_marco[marco_accedido].ETP2]->U;
            modificado = tp2.entradas[bitmap_marco[marco_accedido].ETP2]->M;

            if(puntero < marcosPorProceso - 1)
                puntero ++;
            else
                puntero = 0;

            if(!uso && modificado){
                tp1.punteroClock = puntero;
                return marco_accedido;
            }
            else{
                tp2.entradas[bitmap_marco[marco_accedido].ETP2]->U = 0;
            }

        }
    }
}


