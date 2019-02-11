#pragma once

#include "common.h"

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); printf("HR line %d\n", __LINE__);goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define PARSE_FAIL(ret)                do { printf("PARSE_FAIL line %d\n", __LINE__); return (ret); } while(0)

typedef HRESULT (*attribute_cb)(const WCHAR* elementName,
                                const WCHAR* LocalName,
                                const WCHAR* Value,
                                void *data);

typedef enum {
    STATE_ROOT = 0,
    STATE_MATERIALS,
    STATE_DETAILS,
    STATE_MAX
} VIYAR_STATE_T;

typedef enum {
    MODEL_NONE,
    MODEL_OPENED,
    MODEL_CLOSED,
} MODEL_STATE_T;

typedef enum {
    DETAIL_ATTR,
    DETAIL_EDGES,
    DETAIL_OPERATIONS
} DETAIL_STATE_T;

extern int _details_cnt;
extern DETAIL_DEF_T details[100];

extern int _materials_cnt;
extern MATERIAL_DEF_T model_materials[10];


int parse_xml(const WCHAR* xmlfilename);
