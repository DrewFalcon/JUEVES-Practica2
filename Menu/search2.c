#include "search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"

#define MAX_STRING 1000

void results_search(char* from, char* to, int* n_choices, char*** choices, int max_length, int max_rows)
{
    int i=0;
    int t=0;
    int size, len;
    char **query_result_set = malloc(max_rows * sizeof(char*));
    char *select = NULL;

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;

    SQLCHAR flight_id[16], dep_time[64], arrival_time[64];
    SQLLEN flight_id_ind, dep_time_ind, arrival_time_ind;
    char buffer[256];

    FILE *log = NULL;
    FILE *file = NULL;
    FILE *result_log = NULL;

    // INICIO: Diagnóstico
    log = fopen("debug_search.log", "w");
    if (!log) {
        fprintf(stderr, "ERROR: No se pudo crear debug_search.log\n");
    } else {
        fprintf(log, "=== INICIO DEBUG ===\n");
        fprintf(log, "Parámetros recibidos:\n");
        fprintf(log, "from: '%s'\n", from ? from : "NULL");
        fprintf(log, "to: '%s'\n", to ? to : "NULL");
        fprintf(log, "max_rows: %d\n", max_rows);
        fflush(log);
    }
    // FIN Diagnóstico

    // Inicializar n_choices
    *n_choices = 0;

    // Verificar parámetros de entrada
    if (!from || !to) {
        if (log) {
            fprintf(log, "ERROR: Parámetros from o to son NULL\n");
            fclose(log);
        }
        free(query_result_set);
        return;
    }

    // Leer archivo SQL
    file = fopen("search.sql", "r");
    if (file == NULL) {
        if (log) {
            fprintf(log, "ERROR: No se pudo abrir search.sql\n");
            fclose(log);
        }
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (log) {
        fprintf(log, "Tamaño de search.sql: %d bytes\n", size);
        fflush(log);
    }

    select = (char *)malloc(size + 1);
    if (!select) {
        if (log) {
            fprintf(log, "ERROR: malloc falló para select\n");
            fclose(log);
        }
        fclose(file);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    if (fread(select, 1, size, file) != (size_t)size) {
        if (log) {
            fprintf(log, "ERROR: Error leyendo search.sql\n");
            fclose(log);
        }
        free(select);
        fclose(file);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    select[size] = '\0';
    fclose(file);

    if (log) {
        fprintf(log, "Consulta SQL leída:\n%s\n", select);
        fflush(log);
    }

    // Conexión a BD
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        if (log) {
            fprintf(log, "ERROR: Conexión ODBC fallida\n");
            fclose(log);
        }
        free(select);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    if (log) {
        fprintf(log, "Conexión ODBC exitosa\n");
        fflush(log);
    }

    // Preparar statement
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        if (log) {
            fprintf(log, "ERROR: No se pudo allocar statement\n");
            fclose(log);
        }
        odbc_disconnect(env, dbc);
        free(select);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    // Preparar consulta
    ret = SQLPrepare(stmt, (SQLCHAR *)select, SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        if (log) {
            fprintf(log, "ERROR: SQLPrepare falló\n");
            fclose(log);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        odbc_disconnect(env, dbc);
        free(select);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    free(select);



    /* Meter parámetros */
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(from), 0, from, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(to), 0, to, 0, NULL);
    /*SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, date, 0, NULL);*/


    if (log) {
        fprintf(log, "Parámetros bindeados: from='%s', to='%s'\n", from, to);
        fflush(log);
    }

    /* Ejecutar */
    ret = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(ret)) {
        if (log) {
            fprintf(log, "ERROR: SQLExecute falló, ret=%d\n", ret);
            fclose(log);
        }
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        odbc_disconnect(env, dbc);
        free(query_result_set);
        *n_choices = 0;
        return;
    }

    if (log) {
        fprintf(log, "Consulta ejecutada exitosamente\n");
        fflush(log);
    }

    /* Enlazar columnas de salida */
    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_id, sizeof(flight_id), &flight_id_ind);
    SQLBindCol(stmt, 2, SQL_C_CHAR, dep_time, sizeof(dep_time), &dep_time_ind);
    SQLBindCol(stmt, 3, SQL_C_CHAR, arrival_time, sizeof(arrival_time), &arrival_time_ind);


    /* Leer resultados y construir texto para out_window
    while(SQLFetch(stmt) == SQL_SUCCESS) {
        sprintf(buffer, "%s %s %s", flight_id, dep_time, arrival_time);
        print_to_out_window(buffer);
    }*/

    /*  Leer resultados y guardarlos en query_result_set */
    // Procesar resultados
    i = 0;
    while (i < max_rows && SQL_SUCCEEDED(SQLFetch(stmt))) {
        /* Verificar si hay datos NULL*/
        if (flight_id_ind == SQL_NULL_DATA) strcpy((char*)flight_id, "NULL");
        if (dep_time_ind == SQL_NULL_DATA) strcpy((char*)dep_time, "NULL");
        if (arrival_time_ind == SQL_NULL_DATA) strcpy((char*)arrival_time, "NULL");

        // Usar sprintf temporalmente para evitar problemas con snprintf
        sprintf(buffer, "%s | %s -> %s", (char*)flight_id, (char*)dep_time, (char*)arrival_time);

        len = strlen(buffer);
        query_result_set[i] = malloc(len + 1);
        if (query_result_set[i] != NULL) {
            strcpy(query_result_set[i], buffer);
            
            if (log) {
                fprintf(log, "Fila %d: %s\n", i, query_result_set[i]);
                fflush(log);
            }
        } else {
            break;
        }
        i++;
    }

    *n_choices = i;

    if (log) {
        fprintf(log, "Total de filas obtenidas: %d\n", *n_choices);
        fflush(log);
    }

    // Escribir resultados en returnSQL.log
    result_log = fopen("returnSQL.log", "w");
    if (result_log) {
        for (int j = 0; j < *n_choices; j++) {
            fprintf(result_log, "%s\n", query_result_set[j]);
        }
        fclose(result_log);
    }

    /* Liberar recursos */
    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    odbc_disconnect(env, dbc);


    /*n_choices = sizeof(query_result_set) / sizeof(query_result_set[0]);*/

    max_rows = MIN(*n_choices, max_rows);


    for (i = 0; i < *n_choices && i < max_rows; i++) {
        t = strlen(query_result_set[i]) + 1;
        t = (t < max_length) ? t : max_length;
        strncpy((*choices)[i], query_result_set[i], t);
        (*choices)[i][t-1] = '\0'; // Asegurar terminación nula
    }

    
    /* liberar query_result_set */
    for (i = 0; i < *n_choices; i++) {
        free(query_result_set[i]);
    }
    free(query_result_set);

    if (log) {
        fprintf(log, "=== FIN DEBUG ===\n");
        fclose(log);
    }
}