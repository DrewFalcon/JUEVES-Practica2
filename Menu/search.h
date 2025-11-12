/*
 Created by roberto on 3/5/21.
*/

#ifndef NCOURSES_SEARCH_H
#define NCOURSES_SEARCH_H
#include "windows.h"
#include <string.h>
/*#include <unistd.h>*/

/*For the management of errors showing up in msg window, we return an integer*/
/*0 is for a succesful execution, -1 to -x are for their respective errors*/
int results_search(char * from, char *to, char *date, int * n_choices,
                    char *** choices, int max_length, int max_rows);

#endif /*NCOURSES_SEARCH_H*/
