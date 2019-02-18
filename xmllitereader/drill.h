#pragma once

#include "common.h"

/***************************************************************/
/*                     Global Definitions                      */
/***************************************************************/

/***************************************************************/
/*                       Global Types                          */
/***************************************************************/

extern "C"
{

typedef struct {
    double d;
    double x;
    double y;
    double depth;
    double tdepth; //through
    int side;
} DRILL_T;

typedef struct {
    char *name;
    double dmin;
    double dmax;
    double depthmin;
    double depthmax;
    bool through;
} DRILL_CLASS_T;

typedef struct {
    char *name;
    DRILL_CLASS_T *dc1;
    DRILL_CLASS_T *dc2;
    bool side;
    double distmin;
    double distmax;
} JOINT_T;

/***************************************************************/
/*                  Function definitions                       */
/***************************************************************/

void drill_init(void);
void drill_append(const DRILL_T *dr, size_t amount);
void drill_print_stat(void);
void drill_deinit(void);

} //extern "C"
