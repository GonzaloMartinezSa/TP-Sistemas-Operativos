#include "../include/list.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Creo un PCB 
// Reservo memoria para el PCB, lo cargo con info y retorno el puntero
t_pcb *crear_pcb(uint16_t id, uint16_t tamano, uint8_t pc, int tp, double srt, t_instruccion* programa){
    t_pcb *ptr      = malloc(sizeof(t_pcb));
    ptr->id       = id;
    ptr->tamano   = tamano;
    ptr->pc       = pc;
    ptr->tp       = tp;
    ptr->srt      = srt;
    ptr->programa = programa;
    return ptr;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Creo una lista
// 
t_desc* lista_crear(char* nombre){
    t_desc *ptr   = malloc(sizeof(t_desc));
    strcpy(ptr->nombre, nombre);
    ptr->first    = NULL;
    ptr->last     = NULL;
    ptr->cantidad = 0;
    return ptr;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Elimino una lista
// 
void lista_eliminar(t_desc* prt){
    free(prt);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Chequeo si mi lista esta vacia
// 
bool lista_vacia(t_desc *desc){
    if(desc->first == NULL && desc->last == NULL)
        return true;
    else
        return false;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Creo un nodo 
// Reservo memoria dinamica para dicho nodo y lo cargo con info
t_nodo *nuevo_nodo(t_pcb *pcb){
    //Creo y lleno de info al nodo
    t_nodo *nodo = malloc(sizeof(t_nodo));
    nodo->ant  = NULL;
    nodo->sgte = NULL;
    nodo->pcb  = pcb;
    return nodo;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Inserto un nodo en una lista
// Inserto en una lista determinada un nodo, bajo un algoritmo indicado (FIFO, SRT) 
bool lista_insertar(t_log* logger, t_desc *desc, t_pcb* pcb, algoritmo modo, bool prioridad){
    // Creo el nuevo nodo y lo cargo con info
    t_nodo *nodo = nuevo_nodo(pcb);

    // Si la lista estaba vacia..
    if(lista_vacia(desc)){
        nodo->ant      = NULL;
        nodo->sgte     = NULL;
        desc->first    = nodo;
        desc->last     = nodo;
        desc->cantidad = 1;
    }
    // La lista no estaba vacia..
    else{
        if(modo == FIFO){
            nodo->ant        = desc->last;
            nodo->sgte       = NULL;
            desc->last->sgte = nodo;
            desc->last       = nodo;
        }
        else if(modo == SRT){
            // Me desplazo siempre que mi srt sea mayor que el del nodo
            t_nodo *pos =  desc->first;
            int     mov =  0;
            if(prioridad){
                while(pos != NULL && (double)pos->pcb->srt < (double)pcb->srt ){
                    pos = pos->sgte;
                    mov ++;
                }
            }
            else{
                while(pos != NULL && (double)pos->pcb->srt <= (double)pcb->srt ){
                    pos = pos->sgte;
                    mov ++;
                }
            }

            // Si cai primero...
            if(mov == 0){
                nodo->sgte       = desc->first;
                desc->first->ant = nodo;
                nodo->ant        = NULL;
                desc->first      = nodo;
            }
            // Si cae ultimo
            else if(pos == NULL){
                nodo->ant        = desc->last;
                nodo->sgte       = NULL;
                desc->last->sgte = nodo;
                desc->last       = nodo;
            }
            // Esta en el medio
            else{
                pos->ant->sgte = nodo;
                nodo->sgte     = pos;
                nodo->ant      = pos->ant;
                pos->ant       = nodo;
            }
        }
        else{
            log_error(logger, "Algortimo desconocido! No se inserta nodo!");
            return false;
        }
        desc->cantidad ++;
    }

    //imprimir_lista(desc);
    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Buscar un ID y extraer el nodo
// Busco un nodo, lo extraigo de la lista y devuelvo su info
t_pcb* lista_buscar_extraer(t_desc* desc, uint16_t id){
  
    bool found = false;

    if(desc->first == NULL){
        return NULL;
    }

    t_nodo *pos =  desc->first;

    while(pos != NULL){
        if(pos->pcb->id == id){
            found = true;
            break;
        }
        pos = pos->sgte;
    }
    // Si efectivamente lo encontre
    if(pos != NULL && found == true){
        t_pcb* rta = pos->pcb;
        // Tengo que eliminar el nodo..
        
        if(desc->cantidad == 1){
            desc->cantidad = 0;
            desc->first    = NULL;
            desc->last     = NULL;
            free(pos);
        }
        else{
            if(pos == desc->first){
                desc->first      = pos->sgte;
                desc->first->ant = NULL;
                free(pos);
            }
            else if(pos == desc->last){
                desc->last = pos->ant;
                desc->last->sgte = NULL;
                free(pos);
            }
            else{
                pos->ant->sgte = pos->sgte;
                pos->sgte->ant = pos->ant;
                free(pos);
            }
            desc->cantidad = desc->cantidad - 1;
        }
        return rta;
    }
    return NULL;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Extraer primer nodo
// 
t_pcb *lista_extraer(t_desc* desc){
    t_pcb * data;
    // Me guardo el pcb para retornarlo
    if(desc->cantidad == 0){
        return NULL;
    }
    else if(desc->cantidad == 1){
        data = desc->first->pcb;
        free(desc->first);
        desc->first = NULL;
        desc->last  = NULL;
        desc->cantidad = 0;
    }
    else{
        t_nodo *borrar   = desc->first;
        data             = borrar->pcb;

        // Arreglo la lista para que no se rompa
        desc->first      = desc->first->sgte;
        desc->first->ant = NULL;
        desc->cantidad --;
        free(borrar);
    }
    return data;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Condicion de comprobacion de ID
// 
bool comprobarID(int pcb_id, t_socketID* socket){
	return socket->id == pcb_id;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Buscar y extraer un nodo de las listas de la catedra
// 
t_socketID borrarSocketID(int pcb_id, t_list* lista){
	t_socketID socketError;
	socketError.id         = -1;
	socketError.socket     = 0;
	
	t_socketID *socketStruct;

	for(int i = 0; i < lista->elements_count; i++){
        socketStruct = list_get(lista, i);
        if( comprobarID(pcb_id, socketStruct) ){
            list_remove(lista, i);
            t_socketID rta = *socketStruct;
            free(socketStruct);
			return rta;
		}
	}
	return socketError;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Agregar un nodo a las listas de la catedra
// 
void agregarSocketID(t_list *lista, t_socketID *socketID){

    list_add(lista, socketID);

    return;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Funciones de prueba
//
void imprimir_lista(t_desc* desc){

    printf("\n------------------------------Lista %s--------------------------------\n\n", desc->nombre);
    
    if(lista_vacia(desc)){
        printf("La lista esta vacia!\n");
    }
    else{
        printf("La lista dispone de %d Nodos.\n", desc->cantidad);
        t_nodo *pos = desc->first;
        int mov = 0;
        while(pos != NULL){
            printf("\n\t-------------------------------------\n");
            printf("\t NODO %d ==> PCB ID: %d\n", mov, pos->pcb->id);
            printf("\t PROGRAM COUNTER:%d\n", pos->pcb->pc);
            printf("\t SRT:%f", pos->pcb->srt);
            printf("\n\t-------------------------------------\n");
            pos = pos->sgte;
            mov ++;
        }
    }
    printf("\n---------------------------------------------------------------------\n");
}
