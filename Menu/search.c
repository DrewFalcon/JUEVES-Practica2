/*
* Created by roberto on 3/5/21.
*/
#include <stdio.h>
#include "search.h"
#define MAX_CHAR 1000
void    results_search(char * from, char *to,
                       int * n_choices, char *** choices,
                       int max_length,
                       int max_rows)
   /**here you need to do your query and fill the choices array of strings
 *
 * @param from form field from
 * @param to form field to
 * @param n_choices fill this with the number of results
 * @param choices fill this with the actual results
 * @param max_length output win maximum width
 * @param max_rows output win maximum number of rows
  */
{
    int i=0;
    int t=0;
    int count = 0;
    int z = 0;
    FILE* f = NULL;
    char stream[MAX_CHAR];
    char dummy;
    /* Query Output*/
    char *query_result_set = NULL;

    /*Abrir fichero con el resultado de la consulta*/
    f = fopen_choicesn("aux.log", "r");
    if (f == NULL) {
      return;
    }

    /*Contar cuantos caracteres tiene el resultado de la consulta*/
    while (fscanf(f, "%c", &dummy) == 1) {
      count++;
    }
    fclose(f);

    /*Reservar memoria para guardar la solucion a imprimir*/
    query_result_set = (char*)malloc(sizeof(char) * (count+10));
    if (query_result_set == NULL) {
      return;
    }

    /*Abrir de nuevo el fichero para leer linea a linea*/
    f = fopen("aux.log", "r");
    if (f == NULL) {
      return;
    }

    /*Ir metiendo linea a linea al string final*/
    while(fgets(stream, MAX_CHAR -1, f)) {
      fprintf(stdout, "Linea leida: %s", stream);
      strcat(query_result_set, stream);
    }

    *n_choices = sizeof(query_result_set) / sizeof(query_result_set[0]);

    max_rows = MIN(*n_choices, max_rows);
    for (i = 0 ; i < max_rows ; i++) {
        t = strlen(query_result_set[i])+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], query_result_set[i], t);
    }

    fclose(f);
}

