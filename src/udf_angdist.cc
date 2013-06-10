/*****************************************************************
 * *******                 UDF_ANGDIST                     *******
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
 * this function will calculate the great circle distance using the
 * Vincenty formula as given on Wikipedia:
 *
 * http://en.wikipedia.org/wiki/Great-circle_distance
 * 
 *****************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <math.h>

extern "C" {
    
    my_bool angdist_init( UDF_INIT* initid, UDF_ARGS* args, char* message );
    void angdist_deinit( UDF_INIT* initid );
    double angdist( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error );
    
}

my_bool angdist_init( UDF_INIT* initid, UDF_ARGS* args, char* message ) {
    //checking stuff to be correct
    if(args->arg_count != 4) {
        strcpy(message, "angdist() requires four parameters: ra dec of point a, ra dec of point b");
        return 1;
    }
    
    if(args->arg_type[0] != REAL_RESULT && args->arg_type[0] != DECIMAL_RESULT && args->arg_type[0] != INT_RESULT) {
        strcpy(message, "angdist() requires a double/float as parameter one");
        return 1;
    }
    
    if(args->arg_type[1] != REAL_RESULT && args->arg_type[1] != DECIMAL_RESULT && args->arg_type[1] != INT_RESULT) {
        strcpy(message, "angdist() requires a double/float as parameter two");
        return 1;
    }
    
    if(args->arg_type[2] != REAL_RESULT && args->arg_type[2] != DECIMAL_RESULT && args->arg_type[2] != INT_RESULT) {
        strcpy(message, "angdist() requires a double/float as parameter three");
        return 1;
    }
    
    if(args->arg_type[3] != REAL_RESULT && args->arg_type[3] != DECIMAL_RESULT && args->arg_type[3] != INT_RESULT) {
        strcpy(message, "angdist() requires a double/float as parameter four");
        return 1;
    }

    //dangerous cast
    if(args->arg_type[0] == INT_RESULT) {
        args->arg_type[0] = REAL_RESULT;
        *(double*)args->args[0] = (double)*(long long*)args->args[0];
    }
    
    if(args->arg_type[1] == INT_RESULT) {
        args->arg_type[1] = REAL_RESULT;
        *(double*)args->args[1] = (double)*(long long*)args->args[1];
    }

    if(args->arg_type[2] == INT_RESULT) {
        args->arg_type[2] = REAL_RESULT;
        *(double*)args->args[2] = (double)*(long long*)args->args[2];
    }

    if(args->arg_type[3] == INT_RESULT) {
        args->arg_type[3] = REAL_RESULT;
        *(double*)args->args[3] = (double)*(long long*)args->args[3];
    }

    if((double*)args->args[0] != NULL && (*(double*)args->args[0] < 0.0 || *(double*)args->args[0] > 360.0)) {
        strcpy(message, "angdist(): RA of point a needs to be between 0 and 360.");
        return 1;
    }
    
    if((double*)args->args[1] != NULL && (*(double*)args->args[1] < -90.0 || *(double*)args->args[1] > 90.0)) {
        strcpy(message, "angdist(): DEC of point A needs to be between -90 and 90.");
        return 1;
    }

    if((double*)args->args[2] != NULL && (*(double*)args->args[2] < 0.0 || *(double*)args->args[2] > 360.0)) {
        strcpy(message, "angdist(): RA of point B needs to be between 0 and 360.");
        return 1;
    }
    
    if((double*)args->args[3] != NULL && (*(double*)args->args[3] < -90.0 || *(double*)args->args[3] > 90.0)) {
        strcpy(message, "angdist(): DEC of point B needs to be between -90 and 90.");
        return 1;
    }

    //no limits on number of decimals
    initid->decimals = 31;
    initid->maybe_null = 0;
    initid->max_length = 17 + 31;
    
    return 0;
}

void angdist_deinit( UDF_INIT* initid ) {
        
}

double angdist( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* is_error ) {
    double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;

    //sanity check
    if((double*)args->args[0] == NULL || 
        (double*)args->args[1] == NULL || 
        (double*)args->args[2] == NULL || 
        (double*)args->args[3] == NULL) {
        
        *is_null = 1;
        return NULL;
    }

    //obtain input parameters
    double pointA_ra_deg;
    double pointA_dec_deg;
    double pointB_ra_deg;
    double pointB_dec_deg;

    if(args->arg_type[0] == REAL_RESULT) {
        pointA_ra_deg = *(double*)args->args[0];
    } else {
        pointA_ra_deg = atof(args->args[0]);
    }

    if(args->arg_type[1] == REAL_RESULT) {
        pointA_dec_deg = *(double*)args->args[1];
    } else {
        pointA_dec_deg = atof(args->args[1]);
    }

    if(args->arg_type[2] == REAL_RESULT) {
        pointB_ra_deg = *(double*)args->args[2];
    } else {
        pointB_ra_deg = atof(args->args[2]);
    }

    if(args->arg_type[3] == REAL_RESULT) {
        pointB_dec_deg = *(double*)args->args[3];
    } else {
        pointB_dec_deg = atof(args->args[3]);
    }

    //transform them into RAD
    double pointA_dec_rad = pointA_dec_deg * PI / 180.0;
    double pointA_ra_rad = ( ( pointA_ra_deg < 0.0 ) ? 360.0 + pointA_ra_deg : pointA_ra_deg) * PI / 180.0;
    double pointB_dec_rad = pointB_dec_deg * PI / 180.0;
    double pointB_ra_rad = ( ( pointB_ra_deg < 0.0 ) ? 360.0 + pointB_ra_deg : pointB_ra_deg) * PI / 180.0;

    //calculate delta
    double pointA_ra_delta = pointB_ra_rad - pointA_ra_rad;

    //calculate trigo
    double sin_ra_delta = sin( pointA_ra_delta );
    double cos_ra_delta = cos( pointA_ra_delta );

    double sin_pointA_dec = sin( pointA_dec_rad );
    double sin_pointB_dec = sin( pointB_dec_rad );
    double cos_pointA_dec = cos( pointA_dec_rad );
    double cos_pointB_dec = cos( pointB_dec_rad );

    double tmp1 = cos_pointB_dec * sin_ra_delta;
    double tmp2 = cos_pointA_dec * sin_pointB_dec - sin_pointA_dec * cos_pointB_dec * cos_ra_delta;
    double tmp3 = sin_pointA_dec * sin_pointB_dec + cos_pointA_dec * cos_pointB_dec * cos_ra_delta;

    double result = atan2( sqrt( tmp1 * tmp1 + tmp2 * tmp2 ), tmp3 );

    //and back to deg
    result = result * 180.0 / PI;

    return result;
}