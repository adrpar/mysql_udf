/*****************************************************************
 * *******                    UDF_STRRPOS                  *******
 *****************************************************************
 * (C) 2012 A. Partl, eScience Group AIP - Distributed under GPL
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
 * implementation of the PHP strrpos function
 * 
 *****************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

extern "C" {
    
    my_bool strrpos_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
    void strrpos_deinit( UDF_INIT* initid );
    long long strrpos( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error );
    
}

my_bool strrpos_init( UDF_INIT* initid, UDF_ARGS* args, char* message ) {
    //checking stuff to be correct
    if(args->arg_count != 2) {
	strcpy(message, "wrong number of arguments: strrpos() requires two parameters");
	return 1;
    }
    
    if(args->arg_type[0] != STRING_RESULT) {
	strcpy(message, "strrpos() requires a string as parameter one");
	return 1;
    }
    
    if(args->arg_type[1] != STRING_RESULT) {
	strcpy(message, "strrpos() requires a string as parameter two");
	return 1;
    }
    
    //no limits on number of decimals
    initid->decimals = 31;
    initid->maybe_null = 0;
    initid->max_length = 17 + 31;
    
    return 0;
}

void strrpos_deinit( UDF_INIT* initid ) {
        
}

long long strrpos( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error ) {
    char* haystack;
    char* needle;

    haystack = (char*) malloc((args->lengths[0] + 1) * sizeof (char));
    if (haystack == NULL)
	return -1;

    needle = (char*) malloc((args->lengths[1] + 1) * sizeof (char));
    if (needle == NULL)
	return -1;

    strcpy(haystack, (char*) args->args[0]);
    strcpy(needle, (char*) args->args[1]);

    char* oldPos = NULL;
    char* currPos;
    int needleLen = strlen(needle);
    currPos = strstr(haystack, needle);
    while (currPos != NULL) {
	if (oldPos == currPos)
	    break;

	oldPos = currPos;
	currPos = strstr(oldPos + needleLen, needle);
    }

    long long result = -1;

    if (oldPos != NULL)
	result = oldPos - haystack;

    free(haystack);
    free(needle);

    return result;
}