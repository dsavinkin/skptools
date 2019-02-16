#pragma once

#include "common.h"

typedef enum {
    TYPE_OP_UNDEFINED,
    TYPE_DRILLING,
    TYPE_RABBETING,
    TYPE_SHAPEBYPATTERN,
    TYPE_GROOVING,
    TYPE_CORNEROPERATION,
} OPERATION_TYPE_T;

typedef struct {
    OPERATION_TYPE_T type;
    int side;
    int corner;
    double x;   //for drilling we really need x,y,d and depth
    double y;
    double xo;
    double yo;
    double d;
    double depth;
    double millD;
    double r;
    int mill;
    int ext;
    int edgeMaterial;
    int edgeCovering;
    wchar_t *xl;
    wchar_t *yl;
    //WCHAR *subtype;
    int subtype;
} OPERATION_T;

typedef struct {
    wchar_t *name;
    char *description;
    int material_id;
    double width;
    double height;
    double thickness;
    int multiplicity;
    int grain;
    size_t amount;
    int m_bands[6];
    size_t operations_cnt;
    OPERATION_T *operations; //dynamic array
} DETAIL_DEF_T;

typedef enum {
    TYPE_M_UNDEFINED = 0,
    TYPE_SHEET,
    TYPE_BAND
} MATERIAL_TYPE_T;

typedef struct {
    MATERIAL_TYPE_T type;
    double thickness;
} MATERIAL_DEF_T;

typedef struct {
    MATERIAL_DEF_T *materials; //dynamic array
    DETAIL_DEF_T *details; //dynamic array
    int materials_cnt;
    int details_cnt;
} VIYAR_PROJECT_T;

/***************************************************************/
/*                  Function definitions                       */
/***************************************************************/

VIYAR_PROJECT_T project_init();

void project_destroy(VIYAR_PROJECT_T *project);

int parse_xml(const wchar_t* xmlfilename, VIYAR_PROJECT_T *project /* out */);

