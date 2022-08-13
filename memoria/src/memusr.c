#include "../include/memusr.h"

void imprimir_espacio_usuario();

int acceso_memoria_usr(OPM modo, int marco, int desplazamiento, uint32_t *dato){

	if(marco > TOTAL_MARCOS || desplazamiento > conf.tam_pagina){
		log_error(logger, "SEGMENTATION FAULT!");
		exit(0);
	}

	desplazamiento = desplazamiento / sizeof(uint32_t);

    return acceso_espacio_de_usuario(modo, marco, desplazamiento, dato);
}

int  acceso_espacio_de_usuario(OPM modo, int numero_de_marco, int desplazamiento, uint32_t *dato)
{
	int tam_pagina = conf.tam_pagina;


	if(! bitmap_marco[numero_de_marco].enUso){
		log_error(logger, "SEGMENTATION FAULT! (READ) El marco %d no esta en uso!", numero_de_marco);
		exit(0);
	}

	t_tablaPaginaPrimerNivel t1 = listaTablasPrimerNivel[bitmap_marco[numero_de_marco].NTP1];
	t_tablaPaginaSegundoNivel tp2;

	if(t1.enUso)
	{
		if(t1.cant_entradas >= bitmap_marco[numero_de_marco].ETP1)
		{	
			int dir_NTP2 = t1.entradas[bitmap_marco[numero_de_marco].ETP1];
			tp2 = listaTablasSegundoNivel[dir_NTP2];
			
			if(tp2.enUso)
				tp2.entradas[bitmap_marco[numero_de_marco].ETP2]->U = true;
			else{
				log_error(logger, "SEGMENTATION FAULT! La NTP2 %d (Absoluta) no esta en uso! ", dir_NTP2);
				exit(0);
			}
		}
		else{
			log_error(logger,"Estas intentando acceder a una entrada de TP1 inexistente! Supera el maximo..");
		}	
	}
	else{
		log_error(logger, "HAY DISPARIDAD ENTRE EL BITMAP Y LAS TABLAS");
		exit(0);
	}

	if (modo == OPM_READ)
	{
		*(dato) = ((t_marco*)espacio_usuario)[numero_de_marco].datos[desplazamiento];
		//memcpy(dato,&espacio_usuario[numero_de_marco].datos + desplazamiento, sizeof(uint32_t));
	}
		
	if (modo == OPM_WRITE){
		tp2.entradas[bitmap_marco[numero_de_marco].ETP2]->M = true;
		((t_marco*)espacio_usuario)[numero_de_marco].datos[desplazamiento] = *(dato);
		//memcpy(&espacio_usuario[numero_de_marco].datos + desplazamiento, dato, sizeof(uint32_t));
	}

	//imprimir_espacio_usuario();

	return 1;
}


void imprimir_espacio_usuario(){
	printf("\n");
	for(int i = 0; i < TOTAL_MARCOS; i ++){

		for(int c = 0; c < CANT_UINTS_MARCO; c ++){
			int val = 0;

			val = (int)((t_marco*)espacio_usuario)[i].datos[c];

			printf("%3d ", val);
		}
		printf(" --> MarcoOcupado:%s --> TPN1:%02d --> #Pagina:%d\n", bitmap_marco[i].enUso ? "Si" : "No", bitmap_marco[i].NTP1, bitmap_marco[i].ETP1 * conf.entradas_por_tabla + bitmap_marco[i].ETP2);
	}
	printf("\n");
}