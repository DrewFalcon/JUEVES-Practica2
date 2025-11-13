/*
 * Created by roberto on 3/5/21.
 */
#include "lbpass.h"
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include <ctype.h>

#define MAX_STRING 1000

/* Funciones privadas para facilitar el uso de results bpass: */
void trim_whitespace_bpass(char *str) {
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

void replace_newlines_bpass(char *str) {
    int i = 0;
    if (str == NULL) return;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n' || str[i] == '\r') {
            str[i] = ' '; 
        }
    }
}

int results_bpass(char* bookID, 
                    int * n_choices, char *** choices, 
                    int max_length, 
                    int max_rows)
 /**here you need to do your query and fill the choices array of strings
*
* @param bookID  form field bookId
* @param n_choices fill this with the number of results
* @param choices fill this with the actual results
* @param max_length output win maximum width
* @param max_rows output win maximum number of rows
*/
{
    int i = 0, j = 0;
    int t = 0;
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

    SQLCHAR ticket_no[20], passenger_name[100], flight_id[10];
    SQLCHAR scheduled_departure[20], aircraft_code[10];

    SQLLEN ticket_no_ind, passenger_name_ind, flight_id_ind;
    SQLLEN scheduled_departure_ind, aircraft_code_ind;

    FILE *log = NULL;
    FILE *file = NULL;
    FILE *result_log = NULL;
    log = fopen("debug_bpass.log", "w");
    if (!log) {
        fprintf(stderr, "ERROR: No se pudo crear debug_bpass.log\n");
    } else {
        fprintf(log, "=== INICIO DEBUG BPASS ===\n");
        fprintf(log, "Parámetros recibidos:\n");
        fprintf(log, "bookID: '%s'\n", bookID ? bookID : "NULL");
        fprintf(log, "max_rows: %d\n", max_rows);
        fflush(log);
    }

    /* Verificar parámetros de entrada*/
    if (!bookID) {
        if (log) {
            fprintf(log, "ERROR: Parámetro bookID es NULL\n");
            fclose(log);
        }
        free(query_result_set);
        *n_choices = 0;
        return -1;
    }

    trim_whitespace_bpass(bookID);

    if (log) {
        fprintf(log, "Parámetros después de trim:\n");
        fprintf(log, "bookID: '%s'\n", bookID);
        fflush(log);
    }

    file = fopen("bpass.sql", "r");
    if (file == NULL) {
         if (log) {
            fprintf(log, "ERROR: No se pudo abrir bpass.sql\n");
            fclose(log);
        }
        free(query_result_set);
        *n_choices = 0;  
        return -2;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (log) {
        fprintf(log, "Tamaño de bpass.sql: %d bytes\n", size);
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
        return 0;
    }

    if (fread(select, 1, size, file) != (size_t)size) {
        if (log) {
            fprintf(log, "ERROR: Error leyendo bpass.sql\n");
            fclose(log);
        }
        free(select);
        fclose(file);
        free(query_result_set);
        *n_choices = 0;
        return -3;
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
        fprintf(stderr, "BPASS: conexión fallida.\n");
        if (log) {
            fprintf(log, "ERROR: Conexión ODBC fallida\n");
            fclose(log);
        }
        free(select);
        free(query_result_set);
        *n_choices = 0;
        return -4;
    }

    if (log) {
        fprintf(log, "Conexión ODBC exitosa\n");
        fflush(log);
    }

    /*SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);*/

    /* Preparar statement */
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
        return 0;
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
        return 0;
    }  
    
    free(select);

    /* Meter parámetros */
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(bookID), 0, bookID, 0, NULL);

    if (log) {
        fprintf(log, "Parámetro bindeado: bookID='%s'\n", bookID);
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
        /*SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_ROLLBACK);*/
        odbc_disconnect(env, dbc);
        free(query_result_set);

        replace_newlines_bpass((char*)message);
        *n_choices = 0;
        strncpy((*choices)[0], (char*)message, max_length);
        return -5;
    }

    if (log) {
        fprintf(log, "Consulta ejecutada exitosamente\n");
        fflush(log);
    }

    /*SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT);*/

    /* Enlazar columnas de salida */
    SQLBindCol(stmt, 1, SQL_C_CHAR, ticket_no, sizeof(ticket_no), &ticket_no_ind);
    SQLBindCol(stmt, 2, SQL_C_CHAR, passenger_name, sizeof(passenger_name), &passenger_name_ind);
    SQLBindCol(stmt, 3, SQL_C_CHAR, flight_id, sizeof(flight_id), &flight_id_ind);
    SQLBindCol(stmt, 4, SQL_C_CHAR, scheduled_departure, sizeof(scheduled_departure), &scheduled_departure_ind);
    SQLBindCol(stmt, 5, SQL_C_CHAR, aircraft_code, sizeof(aircraft_code), &aircraft_code_ind);

    /* Leer resultados y guardarlos en query_result_set */
    i = 0;
    while (i < max_rows && SQL_SUCCEEDED(SQLFetch(stmt))) {
        /* Verificar si hay datos NULL */
        if (ticket_no_ind == SQL_NULL_DATA) strcpy((char*)ticket_no, "NULL");
        if (passenger_name_ind == SQL_NULL_DATA) strcpy((char*)passenger_name, "NULL");
        if (flight_id_ind == SQL_NULL_DATA) strcpy((char*)flight_id, "NULL");
        if (scheduled_departure_ind == SQL_NULL_DATA) strcpy((char*)scheduled_departure, "NULL");
        if (aircraft_code_ind == SQL_NULL_DATA) strcpy((char*)aircraft_code, "NULL");

        /* Formatear la salida */
        sprintf(buffer, "%-10s | %-20s | %-8s | %-19s | %-4s", 
                (char*)ticket_no,
                (char*)passenger_name, 
                (char*)flight_id,
                (char*)scheduled_departure,
                (char*)aircraft_code);

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

    /* Escribir resultados en returnSQL_bpass.log */
    result_log = fopen("returnSQL_bpass.log", "w");
    if (result_log) {
        fprintf(result_log, "%-10s | %-20s | %-8s | %-19s | %-4s\n", "Ticket", "Passenger Name", "Flight", "Scheduled Departure", "Aircraft");
        fprintf(result_log, "%-10s-+-%-20s-+-%-8s-+-%-19s-+-%-4s\n", "----------", "--------------------", "--------", "-------------------", "------");
        for (j = 0; j < *n_choices; j++) {
            fprintf(result_log, "%s\n", query_result_set[j]);
        }
        fclose(result_log);
    }

    /* Liberar recursos */
    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    odbc_disconnect(env, dbc);

    max_rows = MIN(*n_choices, max_rows);

    for (i = 0 ; i < max_rows ; i++) {
        t = (int)strlen(query_result_set[i])+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], query_result_set[i], (size_t)t);
    }

    /* liberar query_result_set */
    for (i = 0; i < *n_choices; i++) {
        free(query_result_set[i]);
    }
    free(query_result_set);

    if (log) {
        fprintf(log, "=== FIN DEBUG BPASS ===\n");
        fclose(log);
    }

    return 0;
}