/*
 * Created by roberto on 3/5/21.
 */
#include "search.h"

#include <stdio.h>
#define MAX_STRING 1000
void results_search(char* from, char* to, int* n_choices, char*** choices, int max_length, int max_rows)
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
        int i = 0;
        int t = 0;
        int count = 0;
        int z = 0;
        int j = 0;
        FILE* f = NULL;
        char stream[MAX_STRING];
        char dummy;
        /* Query Output*/
        char** query_result_set = NULL;

        /*Abrir fichero con el resultado de la consulta*/
        f = fopen("aux.log", "r");
        if (f == NULL) {
                return;
        }

        /*Contar cuantas lineas tiene el resultado de la consulta*/
        while (fgets(stream, MAX_STRING, f)) {
                count++;
        }
        fclose(f);

        /*Reservar memoria para guardar la solucion a imprimir*/
        query_result_set = (char**)malloc(sizeof(char*) * (count));
        if (query_result_set == NULL) {
                return;
        }

        for (i = 0; i < count; i++) {
                query_result_set[i] = (char*)malloc(max_length);

                if (query_result_set[i] == NULL) {
                        for (j = 0; j < i; j++) {
                                free(query_result_set[j]);
                        }
                        free(query_result_set);
                        return;
                }
        }

        /*Abrir de nuevo el fichero para leer linea a linea*/
        f = fopen("aux.log", "r");
        if (f == NULL) {
                return;
        }

        /*Ir metiendo linea a linea al string final*/
        for (i = 0; fgets(stream, MAX_STRING - 1, f); i++) {
                strcpy(query_result_set[i], stream);
        }

        *n_choices = MIN(count, max_rows);

        for (i = 0; i < *n_choices; i++) {
                t = strlen(query_result_set[i]) + 1;
                t = MIN(t, max_length);
                strncpy((*choices)[i], query_result_set[i], t);
                (*choices)[i][max_length - 1] = '\0';
        }

        fclose(f);
}
