#pragma once

#include "common.h"

extern int _details_cnt;
extern DETAIL_DEF_T details[100];

extern int _materials_cnt;
extern MATERIAL_DEF_T model_materials[10];

/***************************************************************/
/*                  Function definitions                       */
/***************************************************************/

int parse_xml(const WCHAR* xmlfilename);
