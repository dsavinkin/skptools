#pragma once

#include <SketchUpAPI/model/material.h>

/***************************************************************/
/*                     Global Definitions                      */
/***************************************************************/

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#define MM2INCH(x) ((x)/25.4)
#define INCH2MM(x) ((x)*25.4)

#ifndef SU_CALL
#define SU_CALL(func) if ((func) != SU_ERROR_NONE) { printf("Error on Line %d\n", __LINE__); throw std::exception(); }
#endif

#define DISTANCE_X 50 //mm
#define DISTANCE_Y 50 //mm
#define DISTANCE_Z 3 //*thickness

#define SIDE_FRONT  0
#define SIDE_LEFT   1
#define SIDE_TOP    2
#define SIDE_RIGHT  3
#define SIDE_BOTTOM 4
#define SIDE_BACK   5

#define CORNER_LOWER_LEFT   0
#define CORNER_UPPER_LEFT   1
#define CORNER_UPPER_RIGHT  2
#define CORNER_LOWER_RIGHT  3
#define CORNER_MAX          4

#define EDGE_COVER_BOTH     0
#define EDGE_COVER_H        1
#define EDGE_COVER_V        2


#define DEFAULT_COLOR_ALPHA_BAND 192
#define DEFAULT_COLOR_ALPHA_SHEET 128

/***************************************************************/
/*                       Global Types                          */
/***************************************************************/

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
    //void *material;
    SUMaterialRef material;
} MATERIAL_DEF_T;
