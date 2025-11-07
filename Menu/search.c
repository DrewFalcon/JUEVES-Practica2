/*
 * Created by roberto on 3/5/21.
 */
#include "search.h"

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"

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
    int i=0;
    int t=0;
    int size, len;
    /* 10 commandments from King Jorge Bible 
    char *query_result_set[]={
            "1. Thou shalt have no other gods before me.",
            "2. Thou shalt not make unto thee any graven image,"
            " or any likeness of any thing that is in heaven above,"
            " or that is in the earth beneath, or that is in the water "
            "under the earth.",
            "3. Remember the sabbath day, to keep it holy.",
            "4. Thou shalt not take the name of the Lord thy God in vain.",
            "5. Honour thy father and thy mother.",
            "6. Thou shalt not kill.",
            "7. Thou shalt not commit adultery.",
            "8. Thou shalt not steal.",
            "9. Thou shalt not bear false witness against thy neighbor.",
            "10. Thou shalt not covet thy neighbour's house, thou shalt not"
            " covet thy neighbour's wife, nor his manservant, "
            "nor his maidservant, nor his ox, nor his ass, "
            "nor any thing that is thy neighbour's."
    };*/
    char **query_result_set = malloc(max_rows * sizeof(char*));
    char *select = NULL;

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;

    SQLCHAR flight_id[16], dep_time[64], arrival_time[64];
    char buffer[256];


    FILE *file = fopen("search.sql", "r");
    if (file == NULL) {
      *n_choices = 0;  
      return;
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    select = (char *)malloc(size + 1);
    if (!select) {
        fclose(file);
        fprintf(stderr, "SEARCH: malloc\n");
        *n_choices = 0;
        return;
    }
    if (fread(select, 1, size, file) != (size_t)size) {
        fprintf(stderr, "SEARCH: leyendo el archivo sql\n");
        free(select);
        fclose(file);
        *n_choices = 0;
        return;
    }

    select[size] = '\0';
    fclose(file);


    /* Conexión */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        fprintf(stderr, "SEARCH: conexión fallida.\n");
        free(select);
        *n_choices = 0;
        return;
    }
    
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    SQLPrepare(stmt, (SQLCHAR *)select, SQL_NTS);
    free(select);



    /* Meter parámetros */
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, from, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, to, 0, NULL);
    /*SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, date, 0, NULL);*/

    /* Ejecutar */
    SQLExecute(stmt);

    /* Enlazar columnas de salida */
    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_id, sizeof(flight_id), NULL);
    SQLBindCol(stmt, 2, SQL_C_CHAR, dep_time, sizeof(dep_time), NULL);
    SQLBindCol(stmt, 3, SQL_C_CHAR, arrival_time, sizeof(arrival_time), NULL);


    /* Leer resultados y construir texto para out_window
    while(SQLFetch(stmt) == SQL_SUCCESS) {
        sprintf(buffer, "%s %s %s", flight_id, dep_time, arrival_time);
        print_to_out_window(buffer);
    }*/

    /*  Leer resultados y guardarlos en query_result_set */
    i = 0;
    while (i < max_rows && SQL_SUCCEEDED(SQLFetch(stmt))) {
        snprintf(buffer, sizeof(buffer), "%s | %s -> %s", (char*)flight_id, (char*)dep_time, (char*)arrival_time);

        len = strlen(buffer);
        query_result_set[i] = malloc(len + 1);
        if (query_result_set[i] != NULL) {
            memcpy(query_result_set[i], buffer, len + 1);
        }
        
        if (!query_result_set[i]) break;
        i++;
    }

    *n_choices = i;


    /* Liberar recursos */
    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    odbc_disconnect(env, dbc);




    
    /*n_choices = sizeof(query_result_set) / sizeof(query_result_set[0]);*/

    max_rows = MIN(*n_choices, max_rows);
    for (i = 0 ; i < max_rows ; i++) {
        t = strlen(query_result_set[i])+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], query_result_set[i], t);
    }


    /* liberar query_result_set */
    for (i = 0; i < *n_choices; i++) {
        free(query_result_set[i]);
    }
    free(query_result_set);


}
