#pragma once

#include "common.h"

/***************************************************************/
/*                     Global Definitions                      */
/***************************************************************/

#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) { printf("HR line %s:%d\n", __FILE__, __LINE__); goto CleanUp; }} while(0)
#define HR(stmt)                do { hr = (stmt); printf("HR line %s:%d\n", __FILE__, __LINE__);goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

/***************************************************************/
/*                       Global Types                          */
/***************************************************************/

typedef enum {
    TYPE_OP_UNDEFINED,
    TYPE_CUTTING,
    TYPE_EDGING,
    TYPE_XNC,
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
    int id;
} PART_DEF_T;

typedef struct {
    wchar_t *name;
    double d;
} TOOL_DEF_T;

typedef struct {
    int ver;
    int side;
    wchar_t *name;
    wchar_t *str_x;
    wchar_t *str_y;
    wchar_t *str_dp;
    wchar_t *str_as;

    double x;
    double y;
    double dp;
    double as;
    double dia;
    int ac;
    bool av;
    bool m;
} BORE_DEF_T;

typedef struct {
    //<ms x="0" y="2" dp="10" in="0" out="0" sxy="tool.dia/2" fwd="true" c="2" name="mill8"/>
    wchar_t *str_x;
    wchar_t *str_y;
    wchar_t *str_dp;
    wchar_t *str_sxy;
    double x;
    double y;
    double dp;
    double sxy;
    double dia;
    bool fwd;
    int c;
    wchar_t *name;

    //TODO: add array of trajectories
    //<ml x="dx" y="3" dp="10"/>
} MILL_DEF_T;

typedef struct {
    double dx;
    double dy;
    double dz;
    TOOL_DEF_T *tools;
    BORE_DEF_T *bores;
    MILL_DEF_T *mills;
    int tools_cnt;
    int bores_cnt;
    int mills_cnt;
} PROGRAM_DEF_T;

typedef struct {
    OPERATION_TYPE_T type;
    int id;
    int material_id;
    int parts_cnt;
    bool side;
    bool mirHor;
    bool mirVert;
    int turn;
    PART_DEF_T *parts;
    wchar_t *program;
    PROGRAM_DEF_T *programs;
    int programs_cnt;
} OPERATION_DEF_T;

typedef struct {
    int id;
    wchar_t *name;
    //int material_id;
    double width;
    double height;
    double thickness;
    bool txt;
    int m_el[6];
    //int multiplicity;
    //int grain;
    size_t amount;
    int m_bands[6]; //material indexes in material array
#if 0
    size_t operations_cnt;
    OPERATION_T *operations; //dynamic array
#endif
} DETAIL_DEF_T;

typedef enum {
    TYPE_M_UNDEFINED = 0,
    TYPE_SHEET,
    TYPE_BAND
} MATERIAL_TYPE_T;

typedef struct {
    MATERIAL_TYPE_T type;
    int id;
    double thickness;
} MATERIAL_DEF_T;

typedef struct {
    MATERIAL_DEF_T *materials; //dynamic array
    DETAIL_DEF_T *details; //dynamic array
    OPERATION_DEF_T *operations; //dynamic array
    int details_cnt;
    int materials_cnt;
    int operations_cnt;
} VIYAR_PROJECT_T;

/***************************************************************/
/*                  Function declarations                      */
/***************************************************************/

VIYAR_PROJECT_T project_init();

void project_destroy(VIYAR_PROJECT_T *project);

int parse_xml(const wchar_t* xmlfilename, VIYAR_PROJECT_T *project /* out */);
int project_process(VIYAR_PROJECT_T *project /* in_out */);

void _dump_detail(DETAIL_DEF_T *d);

