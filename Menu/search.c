/*
 * Created by roberto on 3/5/21.
 */
#include "search.h"

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include <ctype.h>

#define MAX_STRING 1000

void trim_whitespace(char *str) {
    char *end;

    if (str == NULL) return;
        
    /* Trim espacios al inicio*/
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return; /* Solo espacios*/
    
    /* Trim espacios al final */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    /* Escribe el nuevo null terminator*/
    end[1] = '\0';
}


void results_search(char* from, char* to, char *date, int* n_choices, char*** choices, int max_length, int max_rows)
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
    int i=0, j=0;
    int t=0;
    int size, len;
    char **query_result_set = malloc(max_rows * sizeof(char*));
    char *select = NULL;

    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;

    SQLCHAR sqlstate[6];
    SQLCHAR message[1024];
    SQLSMALLINT length;
    SQLINTEGER native;
    char buffer[512];

    SQLCHAR flight_chain[256], original_departure[16], final_arrival[16];
    SQLCHAR original_departure_time[64], final_arrival_time[64];
    SQLCHAR total_duration[64], connection_count[16], min_free_seats[16];

    SQLLEN flight_chain_ind, original_departure_ind, final_arrival_ind;
    SQLLEN original_departure_time_ind, final_arrival_time_ind;
    SQLLEN total_duration_ind, connection_count_ind, min_free_seats_ind;

    FILE *log = NULL;
    FILE *file = NULL;
    FILE *result_log = NULL;
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
  /* Verificar parámetros de entrada*/
    if (!from || !to || !date) {
        if (log) {
            fprintf(log, "ERROR: Parámetros from o to son NULL\n");
            fclose(log);
        }
        free(query_result_set);
        return;
    }

    trim_whitespace(from);
    trim_whitespace(to);
    trim_whitespace(date);

    if (log) {
        fprintf(log, "Parámetros después de trim:\n");
        fprintf(log, "from: '%s'\n", from);
        fprintf(log, "to: '%s'\n", to);
        fprintf(log, "date: '%s'\n", date);        
        fflush(log);
    }

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
        fprintf(stderr, "SEARCH: leyendo el archivo sql\n");
        *n_choices = 0;
        return;
    }

    select[size] = '\0';
    fclose(file);

    if (log) {
        fprintf(log, "Consulta SQL leída:\n%s\n", select);
        fflush(log);
    }

    /* Conexión */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        fprintf(stderr, "SEARCH: conexión fallida.\n");
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

    /*Preparar statement*/
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
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(date), 0, date, 0, NULL);
    


    if (log) {
        fprintf(log, "Parámetros bindeados: from='%s', to='%s'\n", from, to);
        fflush(log);
    }

    /* Ejecutar */
    ret = SQLExecute(stmt);
    if (!SQL_SUCCEEDED(ret)) {
        if (log) {
            fprintf(log, "ERROR: SQLExecute falló, ret=%d\n", ret);
            odbc_extract_error("SQLExecute", stmt, SQL_HANDLE_STMT);           
            SQLError(SQL_NULL_HENV, SQL_NULL_HDBC, stmt, sqlstate, &native, message, sizeof(message), &length);
            fprintf(log, "SQLSTATE: %s\n", sqlstate);
            fprintf(log, "Mensaje error: %s\n", message);



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

    /* Enlazar columnas de salida 
    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_id, sizeof(flight_id), &flight_id_ind);
    SQLBindCol(stmt, 2, SQL_C_CHAR, dep_time, sizeof(dep_time), &dep_time_ind);
    SQLBindCol(stmt, 3, SQL_C_CHAR, arrival_time, sizeof(arrival_time), &arrival_time_ind);*/


    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_chain, sizeof(flight_chain), &flight_chain_ind);
    SQLBindCol(stmt, 2, SQL_C_CHAR, original_departure, sizeof(original_departure), &original_departure_ind);
    SQLBindCol(stmt, 3, SQL_C_CHAR, final_arrival, sizeof(final_arrival), &final_arrival_ind);
    SQLBindCol(stmt, 4, SQL_C_CHAR, original_departure_time, sizeof(original_departure_time), &original_departure_time_ind);
    SQLBindCol(stmt, 5, SQL_C_CHAR, final_arrival_time, sizeof(final_arrival_time), &final_arrival_time_ind);
    SQLBindCol(stmt, 6, SQL_C_CHAR, total_duration, sizeof(total_duration), &total_duration_ind);
    SQLBindCol(stmt, 7, SQL_C_CHAR, connection_count, sizeof(connection_count), &connection_count_ind);
    SQLBindCol(stmt, 8, SQL_C_CHAR, min_free_seats, sizeof(min_free_seats), &min_free_seats_ind);


    /* Leer resultados y construir texto para out_window
    while(SQLFetch(stmt) == SQL_SUCCESS) {
        sprintf(buffer, "%s %s %s", flight_id, dep_time, arrival_time);
        print_to_out_window(buffer);
    }*/

    /*  Leer resultados y guardarlos en query_result_set */
i = 0;
while (i < max_rows && SQL_SUCCEEDED(SQLFetch(stmt))) {
    /* Verificar si hay datos NULL para las NUEVAS variables */
    if (flight_chain_ind == SQL_NULL_DATA) strcpy((char*)flight_chain, "NULL");
    if (original_departure_ind == SQL_NULL_DATA) strcpy((char*)original_departure, "NULL");
    if (final_arrival_ind == SQL_NULL_DATA) strcpy((char*)final_arrival, "NULL");
    if (original_departure_time_ind == SQL_NULL_DATA) strcpy((char*)original_departure_time, "NULL");
    if (final_arrival_time_ind == SQL_NULL_DATA) strcpy((char*)final_arrival_time, "NULL");
    if (total_duration_ind == SQL_NULL_DATA) strcpy((char*)total_duration, "NULL");
    if (connection_count_ind == SQL_NULL_DATA) strcpy((char*)connection_count, "NULL");

    /* Formatear la salida con las NUEVAS variables*/
    sprintf(buffer, "%-12s | %-3s -> %-3s | %-16s | %-16s | %-12s | %-3s | %-3s", 
            (char*)flight_chain,
            (char*)original_departure, 
            (char*)final_arrival,
            (char*)original_departure_time,
            (char*)final_arrival_time,
            (char*)total_duration,
            (char*)connection_count, 
            (char*)min_free_seats);

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

    /* Escribir resultados en returnSQL.log*/
    result_log = fopen("returnSQL.log", "w");
    if (result_log) {
        fprintf(result_log, "%-15s | %-4s -> %-4s | %-20s | %-20s | %-15s | %s\n", "Flight Chain", "From", "To", "First Departure", "Last Arrival", "Total Duration", "Connections");
        fprintf(result_log, "%-15s-+-%-4s-+-%-4s-+-%-20s-+-%-20s-+-%-15s-+-%s\n", "---------------", "----", "----", "--------------------", "--------------------", "---------------", "-----------");
        for (j = 0; j < *n_choices; j++) {
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

      
    /*for (i = 0; i < max_rows; i++) {
        fprintf(log, "%s\n", query_result_set[i]);
    }*/
    
    for (i = 0 ; i < max_rows ; i++) {
        t = strlen(query_result_set[i]) + 1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], query_result_set[i], t);
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