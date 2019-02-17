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

/***************************************************************/
/*                  Function definitions                       */
/***************************************************************/

void drill_init(void);
void drill_append(const DRILL_T *dr, size_t amount);
void drill_print_stat(void);
void drill_deinit(void);

} //extern "C"
