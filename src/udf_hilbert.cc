/*****************************************************************
 * *******                   UDF_HILBERT                   *******
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
 * exposing libhilbert as UDFs
 * 
 *****************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

extern "C" {
	#include "libhilbert/hilbertKey.h"

    my_bool hilbertKey_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
    void hilbertKey_deinit(UDF_INIT* initid);
    long long hilbertKey(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error);

    my_bool coordFromHilbertKey_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
    void coordFromHilbertKey_deinit(UDF_INIT* initid);
    double coordFromHilbertKey(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error);

}

my_bool hilbertKey_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
    //checking stuff to be correct
    if(args->arg_count < 4) {
		strcpy(message, "wrong number of arguments: hilbertKey() requires at least three parameters");
		return 1;
    }
    
    if(args->arg_type[0] != INT_RESULT) {
		strcpy(message, "hilbertKey() requires an integer as hilbert order");
		return 1;
    }

    if(args->args[0] != NULL && *(long long*)args->args[0] < 0) {
        strcpy(message, "hilbertKey() requires the hilbert order to be non negative");
        return 1;
    }

    if(args->arg_type[1] != REAL_RESULT && args->arg_type[1] != DECIMAL_RESULT) {
		strcpy(message, "hilbertKey() requires an real number as box size");
		return 1;
    }

    if(args->arg_type[2] != INT_RESULT) {
		strcpy(message, "hilbertKey() requires an integer as dimension");
		return 1;
    }

    if(args->args[2] != NULL && *(long long*)args->args[2] < 0) {
        strcpy(message, "hilbertKey() requires the dimension to be non negative");
        return 1;
    }

    //loop through all the coordinates and check those
    if(args->args[2] != NULL && args->arg_count != 3 + *(long long*)args->args[2]) {
		strcpy(message, "wrong number of arguments: hilbertKey() did not reveive enough coordinates to match the dimension");
		return 1;
    }

    int i=0;
    if(args->args[2] != NULL) {
        for(i=0; i<*(long long*)args->args[2]; i++) {
            if(args->arg_type[3+i] != REAL_RESULT && args->arg_type[3+i] != DECIMAL_RESULT) {
                strcpy(message, "hilbertKey() coordinates need to be real numbers");
                return 1;
            }

            if(args->args[3+i] != NULL && *(double*)args->args[3+i] < 0.0) {
                strcpy(message, "hilbertKey() coordinates need to be positive numbers");
                return 1;
            }
        }
    }

    //no limits on number of decimals
    initid->decimals = 31;
    initid->maybe_null = 0;
    initid->max_length = 17 + 31;
    
    return 0;
}

void hilbertKey_deinit(UDF_INIT* initid) {

}

long long hilbertKey(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error) {
	uint64_t result;

	int32_t m = (int32_t)*(long long*)args->args[0];
	double boxSize;

    if(args->arg_type[1] == REAL_RESULT) {
        boxSize = *(double*)args->args[1];
    } else {
        boxSize = atof(args->args[1]);
    }

    int32_t dim = (int32_t)*(long long*) args->args[2];
	double * point = NULL;
	int err;

	point = (double*)malloc(dim * sizeof(double));
	if(point == NULL) {
		*is_error = 1;
		return -1;
	}

	int i=0;
	for(i=0; i<dim; i++) {
        if(args->arg_type[3+i] == REAL_RESULT) {
            point[i] = *(double*)args->args[3+i];
        } else {
            point[i] = atof(args->args[3+i]);
        }
	}

	result = getHKeyFromCoord( m, boxSize, dim, point, &err );

	free(point);

	return (long long)result;
}

my_bool coordFromHilbertKey_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
    //checking stuff to be correct
    if(args->arg_count != 5) {
		strcpy(message, "wrong number of arguments: coordFromHilbertKey() requires five parameters");
		return 1;
    }
    
    if(args->arg_type[0] != INT_RESULT) {
		strcpy(message, "coordFromHilbertKey() requires an integer as hilbert order");
		return 1;
    }

    if(args->args[0] != NULL && *(long long*)args->args[0] < 0) {
        strcpy(message, "coordFromHilbertKey() requires the hilbert order to be non negative");
        return 1;
    }

    if(args->arg_type[1] != REAL_RESULT && args->arg_type[1] != DECIMAL_RESULT) {
		strcpy(message, "coordFromHilbertKey() requires an real number as box size");
		return 1;
    }

    if(args->arg_type[2] != INT_RESULT) {
		strcpy(message, "coordFromHilbertKey() requires an integer as dimension");
		return 1;
    }

    if(args->args[2] != NULL && *(long long*)args->args[2] < 0) {
        strcpy(message, "coordFromHilbertKey() requires the dimension to be non negative");
        return 1;
    }

    if(args->arg_type[3] != INT_RESULT) {
		strcpy(message, "coordFromHilbertKey() requires an integer as hilbert key");
		return 1;
    }

    if(args->args[3] != NULL && *(long long*)args->args[3] < 0) {
        strcpy(message, "coordFromHilbertKey() requires the hilbert key to be non negative");
        return 1;
    }

    if(args->arg_type[4] != INT_RESULT) {
		strcpy(message, "coordFromHilbertKey() requires an integer as current dimension");
		return 1;
    }

    if(args->args[4] != NULL && *(long long*)args->args[4] < 0) {
        strcpy(message, "coordFromHilbertKey() requires the current dimension to be non negative");
        return 1;
    }

    if(args->args[4] != NULL && args->args[2] != NULL && *(long long*)args->args[4] >= *(long long*)args->args[2]) {
        strcpy(message, "coordFromHilbertKey() requires the current dimension to start at 0 and not exceed the dimension - 1");
        return 1;
    }

    //no limits on number of decimals
    initid->decimals = 31;
    initid->maybe_null = 0;
    initid->max_length = 17 + 31;
    
    return 0;
}

void coordFromHilbertKey_deinit(UDF_INIT* initid) {

}

double coordFromHilbertKey(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error){
	int32_t m = (int32_t)*(long long*)args->args[0];
    double boxSize;

    if(args->arg_type[1] == REAL_RESULT) {
        boxSize = *(double*)args->args[1];
    } else {
        boxSize = atof(args->args[1]);
    }

	int32_t dim = (int32_t)*(long long*)args->args[2];
	int64_t key = *(long long*)args->args[3];
	int32_t currDim = (int32_t)*(long long*)args->args[4];
	int err;
	double result;

	double * point = NULL;
	point = (double*)malloc(dim * sizeof(double));
	if(point == NULL) {
		*is_error = 1;
		return -1.0;
	}

	getCoordFromHKey( point, m, boxSize, dim, key, &err );

	result = point[currDim];

	free(point);

	return result;
}
