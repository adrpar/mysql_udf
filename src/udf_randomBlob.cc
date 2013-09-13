/*****************************************************************
 ********                   UDF_RANDOMBLOB                 *******
 *****************************************************************
 * (C) 2013 A. Partl, eScience Group AIP - Distributed under GPL
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************
 * UDF generating random BLOBs of given size (using a crappy random
 * number generator)
 *****************************************************************
 */

#define MYSQL_SERVER 1

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

extern "C" {
    
    my_bool randomBlob_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
    void randomBlob_deinit( UDF_INIT* initid );
    char* randomBlob( UDF_INIT* initid, UDF_ARGS* args, 
                        char* result, unsigned long *res_length,
                        char* is_null, char* error );
    
}

my_bool randomBlob_init( UDF_INIT* initid, UDF_ARGS* args, char* message ) {
    //checking stuff to be correct
    if(args->arg_count != 1) {
    	strcpy(message, "wrong number of arguments: randomBlob() requires one parameters");
    	return 1;
    }
    
    if(args->arg_type[0] != INT_RESULT) {
    	strcpy(message, "randomBlob() requires an integer as parameter one (number of bytes)");
    	return 1;
    }

    if(*(long long*)args->args[0] < 0) {
        strcpy(message, "randomBlob() requires parameter one to be non negative");
        return 1;
    }
    
    //no limits on number of decimals
    initid->maybe_null = 0;
    initid->ptr = (char*)malloc(*(long long*)args->args[0]);
    initid->max_length = *(long long*)args->args[0];
    
    return 0;
}

void randomBlob_deinit( UDF_INIT* initid ) {
    free(initid->ptr);
}

char* randomBlob( UDF_INIT* initid, UDF_ARGS* args, char* result, unsigned long *res_length,
                    char* is_null, char* error ) {
    long long i = 0;
    char* blob = initid->ptr;

    *res_length = initid->max_length;

    srand(time(NULL));

    //fill blob with random stuff
    for(i = 0; i < *res_length; i++) {
        blob[i] = (char)(rand() % 256);
    }

    return initid->ptr;
}