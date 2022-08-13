#include "../include/utils.h"


t_config* leer_config(void)
{
	char*   valor;

	//ahora, vamos a abrir el archivo de configuracion "consola.config"
	t_config* c_config = config_create("consola.config");


    //conf.ip    = config_get_string_value(c_config, "IP");

    //conf.port  = config_get_string_value(c_config, "PUERTO");

    //conf.id    = htonl(getpid());

	//config_destroy(c_config);

    return c_config;

}

t_instruccion* leer_programa(t_log* logger, char* filename, int *cant){

    //Creo mi puntero a instrucciones
    t_instruccion* instrucciones = malloc(sizeof(t_instruccion));

    char buffer[80];
    char* rest;

    FILE *fp = fopen(filename, "r");

   	if ( fp == NULL)
	{
		log_error(logger, "No se pudo abrir archivo...  Terminando consola");
		//log_error(c_logger, strerror(1));
		exit (EXIT_FAILURE);
	}

    // Realizo la lectura del programa
	while (fgets(buffer, sizeof(buffer)-1, fp))
	{
		    char *com;
		    int arg1 = -1, arg2 = -1, coop = -1;

		    com=strtok_r(buffer, " ", &rest);

		    if ((strcmp("NO_OP",com) == 0) || (strcmp("NO_OP\n",com) == 0))
		    {
                coop = NOOP;
				arg1 = atoi(strtok_r(NULL, "\n", &rest));
				if (arg1 < 0 )
				    exit(EXIT_FAILURE);
			}
			else if ((strcmp("I/O",com) == 0) || (strcmp("I/O\n",com) == 0) || (strcmp("IO",com) == 0) || (strcmp("IO\n",com) == 0))
		    {
                coop = IO;
				arg1 = atoi(strtok_r(NULL, "\n", &rest));
				if(arg1 < 0)
					exit (EXIT_FAILURE);
			}
            else if ((strcmp("READ",com) == 0) || (strcmp("READ\n",com) == 0))
		    {
                coop = READ;
				arg1 = atoi(strtok_r(NULL, " ", &rest));
				if ( arg1 < 0)
					exit (EXIT_FAILURE);
			}
			else if ((strcmp("WRITE",com) == 0) || (strcmp("WRITE\n",com) == 0))
		    {
                coop = WRITE;
				arg1 = atoi(strtok_r(NULL, " ", &rest));
				arg2 = atoi(strtok_r(NULL, "\n", &rest));
				if ( arg1 < 0 || arg2 < 0)
					exit (EXIT_FAILURE);
			}
            else if ((strcmp("COPY",com) == 0) || (strcmp("COPY\n",com) == 0))
		    {
                coop = COPY;
				arg1 = atoi(strtok_r(NULL, " ", &rest));
				arg2 = atoi(strtok_r(NULL, "\n", &rest));
				if ( arg1 < 0 || arg2 < 0)
					exit (EXIT_FAILURE);
			}
            else if ((strcmp("EXIT\n",com) == 0) || (strcmp("EXIT",com) == 0))
		    {
                coop = EXIT;
			}
            else if(strcmp("\n", com) !=0)
			{
				log_error(logger, "operacion no soportada{%s}.. \nTerminando consola", com);
				exit (EXIT_FAILURE);
		    }

           
			if(coop == NOOP && arg1 > 1){
				for(int i = 0; i < arg1; i++){
					if(*cant > 0)
						instrucciones = realloc(instrucciones, sizeof(t_instruccion) * (*cant + 1));
			
					instrucciones[*cant].coop = coop;
					instrucciones[*cant].arg1 = 1;
					instrucciones[*cant].arg2 = -1;

					*cant = *cant + 1;
				}
			}
			else{
				if(*cant > 0)
					instrucciones = realloc(instrucciones, sizeof(t_instruccion) * (*cant + 1));
			
				instrucciones[*cant].coop = coop;
				instrucciones[*cant].arg1 = arg1;
				instrucciones[*cant].arg2 = arg2;

				*cant = *cant + 1;
			}
			
	}

	fclose(fp);
    
    return instrucciones;
}
