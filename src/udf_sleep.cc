/*****************************************************************
 ********                      UDF_IDLE                    *******
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
 * implementation of the C sleep function to just let the server 
 * idle...
 * 
 *****************************************************************
 */

#define MYSQL_SERVER 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mysql_version.h>
#include <mysql.h>
#include <sql_class.h>

extern "C" {
    
    my_bool idle_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
    void idle_deinit( UDF_INIT* initid );
    long long idle( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error );
    
}

my_bool idle_init( UDF_INIT* initid, UDF_ARGS* args, char* message ) {
    //checking stuff to be correct
    if(args->arg_count != 1) {
	strcpy(message, "wrong number of arguments: idle() requires one parameters");
	return 1;
    }
    
    if(args->arg_type[0] != INT_RESULT) {
	strcpy(message, "idle() requires an integer as parameter one");
	return 1;
    }

    if(args->args[0] != NULL && *(long long*)args->args[0] < 0) {
        strcpy(message, "idle() requires parameter one to be non negative");
        return 1;
    }
    
    //no limits on number of decimals
    initid->decimals = 31;
    initid->maybe_null = 0;
    initid->max_length = 17 + 31;
    
    return 0;
}

void idle_deinit( UDF_INIT* initid ) {
        
}

long long idle( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error ) {
    unsigned int sleepValue = 0;
    int i = 0;

    THD * thd = current_thd;

    sleepValue = (unsigned int)(*(long long*)args->args[0]);

    for(i=0; i < sleepValue; i++) {

#if defined(MARIADB_BASE_VERSION) && MYSQL_VERSION_ID >= 50500
        if(thd->killed != NOT_KILLED)
#else
        if(thd->killed != THD::NOT_KILLED)
#endif
            break;

        sleep(1);
    }

    return sleepValue - i;
}