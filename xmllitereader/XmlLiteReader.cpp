//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
//
// Copyright (C) Microsoft Corporation.  All rights reserved.
//
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
//
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#include <ole2.h>
#include <xmllite.h>
#include <stdio.h>
#include <shlwapi.h>

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>

#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/group.h>
#include <vector>

/***************************************************************/
/*                     Local Definitions                       */
/***************************************************************/

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); printf("HR line %d\n", __LINE__);goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define PARSE_FAIL(ret)                do { printf("PARSE_FAIL line %d\n", __LINE__); return (ret); } while(0)

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif


#define MM2INCH(x) ((x)/25.4)
#define INCH2MM(x) ((x)*25.4)

#ifndef SU_CALL
#define SU_CALL(func) if ((func) != SU_ERROR_NONE) throw std::exception()
#endif

#define DISTANCE_X 50 //mm
#define DISTANCE_Y 50 //mm
#define DISTANCE_Z 3 //*thickness

/***************************************************************/
/*                       Local Types                           */
/***************************************************************/

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
    SIDE_UNDEFINED = 0,
    SIDE_FRONT,
    SIDE_LEFT,
    SIDE_TOP,
    SIDE_RIGHT,
    SIDE_BOTTOM,
    SIDE_BACK
} SIDES_T;

typedef struct {
    char *name;
    char *description;
    int material_id;
    double width;
    double height;
    double thickness;
    int multiplicity;
    int grain;
    size_t amount;
} DETAIL_DEF_T;

typedef enum {
    DETAIL_ATTR,
    DETAIL_EDGES,
    DETAIL_OPERATIONS
} DETAIL_STATE_T;

typedef enum {
    TYPE_UNDEFINED = 0,
    TYPE_SHEET,
    TYPE_BAND
} MATERIAL_TYPE_T;

typedef struct {
    MATERIAL_TYPE_T type;
    double thickness;
    int markingColor;
} MATERIAL_DEF_T;

typedef HRESULT (*attribute_cb)(const WCHAR* elementName,
                                const WCHAR* LocalName,
                                const WCHAR* Value,
                                void *data);

/***************************************************************/
/*                     Local Variables                         */
/***************************************************************/

static VIYAR_STATE_T _state = STATE_ROOT;
static MODEL_STATE_T _model_state = MODEL_NONE;
static DETAIL_STATE_T _detail_state = DETAIL_ATTR;
static double _last_detail_position_X = 0;
static double _last_detail_position_Y = 0;
static double _max_detail_position_X = 0;
static double _max_detail_position_Y = 0;
static int _detail_position_direction = 0;

static int _details_cnt = 0;
static DETAIL_DEF_T details[100];

static int _materials_cnt = 0;
static MATERIAL_DEF_T materials[10];

/***************************************************************/
/*                     Local Functions                         */
/***************************************************************/

void _dump_detail(DETAIL_DEF_T *d)
{
    if (!d)
    {
        return;
    }

    printf("name:        %s\n", d->name);
    printf("description: %s\n", d->description);
    printf("size:        %.1f/%.1f/%.1f\n", d->width, d->height, d->thickness);
    printf("amount:      %zd\n", d->amount);
}


static HRESULT _model_open_create()
{
    wprintf(L"TODO: create/read Model\n");
    return S_OK;
}

static HRESULT _model_save_close()
{
    wprintf(L"TODO: save/close Model\n");
    return S_OK;
}

static HRESULT _element_start(const WCHAR* ElementName, void *data)
{
    wprintf(L"S %d: Element start (%p) <%s ...\n", _state, data, ElementName);

    switch (_state)
    {
        case STATE_ROOT:
            if (wcscmp(ElementName, L"project") == 0)
            {
                if (_model_state == MODEL_NONE)
                {
                    _model_state = MODEL_OPENED;
                    _model_open_create();
                }
            }
            else if (wcscmp(ElementName, L"materials") == 0)
            {
                if (_model_state != MODEL_OPENED)
                {
                    PARSE_FAIL(E_ABORT);
                }
                _state = STATE_MATERIALS;
                _materials_cnt = 0;
                memset(materials, 0, sizeof(materials));
            }
            else if (wcscmp(ElementName, L"details") == 0)
            {
                if (_model_state != MODEL_OPENED)
                {
                    PARSE_FAIL(E_ABORT);
                }
                _state = STATE_DETAILS;
                _details_cnt = 0;
                memset(details, 0, sizeof(details));
            }
            else
            {
                // Ignore
            }
            break;

        case STATE_MATERIALS:
            if (wcscmp(ElementName, L"material") == 0)
            {
                _materials_cnt++;
                wprintf(L"TODO: (%d) start adding material\n", _materials_cnt);
            }
            else
            {
                wprintf(L"TODO: (%d:%s) continue updating material\n", _materials_cnt, ElementName);
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                _details_cnt++;
                _detail_state = DETAIL_ATTR;
                wprintf(L"_detail_state = DETAIL_ATTR\n");
//                wprintf(L"TODO: (%d) start adding detail\n", _details_cnt);
            }
            else
            {
                wprintf(L"TODO: (%d:%s) continue updating detail\n", _details_cnt, ElementName);

                if (wcscmp(ElementName, L"edges") == 0)
                {
                    _detail_state = DETAIL_EDGES;
                    wprintf(L"_detail_state = DETAIL_EDGES\n");
                }
                else if (wcscmp(ElementName, L"operations") == 0)
                {
                    _detail_state = DETAIL_OPERATIONS;
                    wprintf(L"_detail_state = DETAIL_OPERATIONS\n");
                }

            }
            break;

        default:
            PARSE_FAIL(E_ABORT);
    }

    return S_OK;
}

static HRESULT _element_end(const WCHAR* ElementName, void *data)
{
    wprintf(L"S %d: End element </%s> (%p)\n", _state, ElementName, data);

    switch (_state)
    {
        case STATE_ROOT:
            if (wcscmp(ElementName, L"project") == 0)
            {
                if (_model_state == MODEL_OPENED)
                {
                    _model_state = MODEL_CLOSED;
                    _model_save_close();
                }
            }
            break;

        case STATE_MATERIALS:

            if (wcscmp(ElementName, L"material") == 0)
            {
                wprintf(L"TODO: add material (%d) to Model\n", _materials_cnt);

            }
            else if (wcscmp(ElementName, L"materials") == 0)
            {
                _state = STATE_ROOT;
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                wprintf(L"TODO: add detail (%d) to Model\n", _details_cnt);
            }
            else if (wcscmp(ElementName, L"details") == 0)
            {
                _state = STATE_ROOT;
            }
            break;

        default:
            PARSE_FAIL(E_ABORT);
    }

    return S_OK;
}

static HRESULT _parse_declaration(const WCHAR* ElementName,
                                  const WCHAR* LocalName,
                                  const WCHAR* Value,
                                  void *data)
{
    wprintf(L"declaration %s=\"%s\"> (%p)\n", LocalName, Value, data);

    return S_OK;
}

static HRESULT _parse_material(const WCHAR* ElementName,
                               const WCHAR* LocalName,
                               const WCHAR* Value,
                               void *data)
{
    if (_materials_cnt < 1)
    {
        PARSE_FAIL(E_ABORT);
    }

    if (wcscmp(ElementName, L"material") != 0)
    {
        return S_FALSE;
    }

    //wprintf(L"material (%d) %s=\"%s\"\n", _materials_cnt, LocalName, Value);
    MATERIAL_DEF_T *m = &materials[_materials_cnt-1];

    if (wcscmp(LocalName, L"id") == 0)
    {
        if (_wtol(Value) != _materials_cnt)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"type") == 0)
    {
        if (wcscmp(Value, L"sheet") == 0)
        {
            m->type = TYPE_SHEET;
        }
        else if (wcscmp(Value, L"band") == 0)
        {
            m->type = TYPE_BAND;
        }
        else
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"thickness") == 0)
    {
        m->thickness = _wtof(Value);
        if (m->thickness == 0.0)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"markingColor") == 0)
    {
        int red, green, blue;
        if (swscanf_s(Value, L"rgb(%d,%d,%d)", &red, &green, &blue) != 3)
        {
            PARSE_FAIL(E_ABORT);
        }

        m->markingColor = red;
        m->markingColor = (m->markingColor << 8) + green;
        m->markingColor = (m->markingColor << 8) + blue;
    }
    else
    {
        //wprintf(L"Ignore attribute material (%d) %s=\"%s\"\n", _materials_cnt, LocalName, Value);
        return S_FALSE;
    }

    return S_OK;
}

static HRESULT _parse_detail(const WCHAR* ElementName,
                             const WCHAR* LocalName,
                             const WCHAR* Value,
                             void *data)
{
    if (wcscmp(ElementName, L"details") == 0)
    {
        //Skip <details> attributes
        return S_FALSE;
    }

    if (_details_cnt < 1)
    {
        PARSE_FAIL(E_ABORT);
    }

    DETAIL_DEF_T *d = &details[_details_cnt-1];

    wprintf(L"detail %d:%d <%s: %s=\"%s\"> (%p)\n", _details_cnt, _detail_state, ElementName, LocalName, Value, data);

    switch (_detail_state)
    {
        case DETAIL_ATTR:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                if (wcscmp(LocalName, L"id") == 0)
                {
                    if (_wtol(Value) != _details_cnt)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"material") == 0)
                {
                    d->material_id = _wtol(Value);
                    if ((d->material_id <= 0) || (d->material_id > _materials_cnt))
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                    // TODO: update thickness after reading multiplier
                    MATERIAL_DEF_T *m = &materials[d->material_id-1];
                    d->thickness = m->thickness;
                }
                else if (wcscmp(LocalName, L"amount") == 0)
                {
                    d->amount = _wtol(Value);
                    if (d->amount <= 0)
                    {
                        wprintf(L"Warning: %s = %s\n", LocalName, Value);
                    }
                }
                else if (wcscmp(LocalName, L"width") == 0)
                {
                    d->width = _wtof(Value);
                    if (d->width <= 0.0)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"height") == 0)
                {
                    d->height = _wtof(Value);
                    if (d->height <= 0.0)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"multiplicity") == 0)
                {
                    d->multiplicity = _wtol(Value);
                    if (d->multiplicity <= 0)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"description") == 0)
                {
                    //TODO: d->description = wstrdup(Value);
                }
                else if (wcscmp(LocalName, L"grain") == 0)
                {
                    d->grain = _wtol(Value);
                }
                else
                {
                    wprintf(L"Ignore attribute %s (%d) %s=\"%s\"\n", ElementName, _details_cnt, LocalName, Value);
                    return S_FALSE;
                }
            }
            else
            {
                wprintf(L"Ignore detail element %s (%d) %s=\"%s\"\n", ElementName, _details_cnt, LocalName, Value);
                return S_FALSE;
            }
            break;

        case DETAIL_EDGES:
            if (wcscmp(ElementName, L"edges") == 0)
            {
                if (wcscmp(LocalName, L"joint") == 0)
                {
                    //Always "0" in my files
                    if (wcscmp(Value, L"0") != 0)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
            }
            else if ((wcscmp(ElementName, L"left") == 0) ||
                     (wcscmp(ElementName, L"top") == 0) ||
                     (wcscmp(ElementName, L"right") == 0) ||
                     (wcscmp(ElementName, L"bottom") == 0))
            {
                if (wcscmp(LocalName, L"type") == 0)
                {
                    //Limit kromka and empty only for now
                    if ((wcscmp(Value, L"kromka") != 0) &&
                            (wcscmp(Value, L"") != 0))
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"param") == 0)
                {
                    int material_id = _wtol(Value);
                    if (material_id < 0)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                    else
                    {
                        MATERIAL_DEF_T *m = &materials[material_id-1];
                        if ((wcscmp(ElementName, L"top") == 0) ||
                            (wcscmp(ElementName, L"bottom") == 0))
                        {
                            d->height += m->thickness;
                        }
                        else
                        {
                            d->width += m->thickness;
                        }
                    }
                }
            }
            else
            {
                wprintf(L"Ignore detail element %s (%d) %s=\"%s\"\n", ElementName, _details_cnt, LocalName, Value);
                return S_FALSE;
            }
            break;

        case DETAIL_OPERATIONS:
            if (wcscmp(ElementName, L"operations") == 0)
            {
                // No attributes
            }
            else if (wcscmp(ElementName, L"operation") == 0)
            {

                // TODO: add operations
            }
            else
            {
                wprintf(L"Ignore detail element %s (%d) %s=\"%s\"\n", ElementName, _details_cnt, LocalName, Value);
                return S_FALSE;
            }
            break;
    }

    return S_OK;
}


static HRESULT _parse_element(const WCHAR* ElementName,
                              const WCHAR* LocalName,
                              const WCHAR* Value,
                              void *data)
{
    //wprintf(L"<%s %s=\"%s\"> (%p)\n", ElementName, LocalName, Value, data);

    if (_state == STATE_MATERIALS)
    {
        return _parse_material(ElementName, LocalName, Value, data);
    }
    else if (_state == STATE_DETAILS)
    {
        return _parse_detail(ElementName, LocalName, Value, data);
    }
    else if (_state == STATE_ROOT)
    {
        //return S_FALSE in ROOT state
        return S_FALSE;
    }

    PARSE_FAIL(E_ABORT);
}

HRESULT WriteAttributes(IXmlReader* pReader, const WCHAR* ElementName, attribute_cb cb, void *data)
{
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;

    HRESULT hr = _element_start(ElementName, data);

    if (S_OK != hr)
    {
        wprintf(L"Callback returned error (%d)\n", hr);
        return hr;
    }

    hr = pReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx", hr);
        return hr;
    }
    else
    {
        do
        {
            if (!pReader->IsDefault())
            {
                UINT cwchPrefix;
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    return hr;
                }
/*
                if (cwchPrefix > 0)
                    wprintf(L"%s Attr: %s:%s=\"%s\" \n", ElementName, pwszPrefix, pwszLocalName, pwszValue);
                else
                    wprintf(L"%s Attr: %s=\"%s\" \n", ElementName, pwszLocalName, pwszValue);
*/
                if (cb)
                {
                    if (FAILED(hr = cb(ElementName, pwszLocalName, pwszValue, data)))
                    {
                        wprintf(L"Callback returned error (%d)\n", hr);
                        return hr;
                    }
                }
            }
        } while (pReader->MoveToNextAttribute() == S_OK);
    }
    return hr;
}

static void _add_face(SUEntitiesRef entities, SUPoint3D vertices[4])
{
    SULoopInputRef outer_loop = SU_INVALID;
    SULoopInputCreate(&outer_loop);
    for (size_t i = 0; i < 4; ++i) {
        SULoopInputAddVertexIndex(outer_loop, i);
    }
    // Create the face
    SUFaceRef faces[2] = {SU_INVALID, SU_INVALID};
    SUFaceRef face = SU_INVALID;

    SUFaceCreate(&face, vertices, &outer_loop);
    // Add the face to the entities
    SUEntitiesAddFaces(entities, 1, &face);
}

static void _create_detail(//SUComponentInstanceRef instance,
                               SUEntitiesRef entities,
                               double width, double height, double thickness)
{
    SUPoint3D sides[6][4] = {
        {
            { 0,                0,                  0 },
            { 0,                MM2INCH(height),    0 },
            { MM2INCH(width),   MM2INCH(height),    0 },
            { MM2INCH(width),   0,                  0 },
        },
        {
            { 0,                0,                  MM2INCH(thickness) },
            { MM2INCH(width),   0,                  MM2INCH(thickness) },
            { MM2INCH(width),   MM2INCH(height),    MM2INCH(thickness) },
            { 0,                MM2INCH(height),    MM2INCH(thickness) },
        },
        {
            { 0,                0,                  0 },
            { MM2INCH(width),   0,                  0 },
            { MM2INCH(width),   0,                  MM2INCH(thickness) },
            { 0,                0,                  MM2INCH(thickness) }
        },
        {
            { 0,                MM2INCH(height),    0 },
            { 0,                MM2INCH(height),    MM2INCH(thickness) },
            { MM2INCH(width),   MM2INCH(height),    MM2INCH(thickness) },
            { MM2INCH(width),   MM2INCH(height),    0 },
        },
        {
            { 0,                0,                  0 },
            { 0,                0,                  MM2INCH(thickness) },
            { 0,                MM2INCH(height),    MM2INCH(thickness) },
            { 0,                MM2INCH(height),    0 },
        },
        {
            { MM2INCH(width),   0,                  0 },
            { MM2INCH(width),   MM2INCH(height),    0 },
            { MM2INCH(width),   MM2INCH(height),    MM2INCH(thickness) },
            { MM2INCH(width),   0,                  MM2INCH(thickness) },
        },
    };

    for (size_t i = 0; i < 6; ++i)
    {
        _add_face(entities, sides[i]);
    }
}

static void _create_detail_components(SUModelRef model,
                                      DETAIL_DEF_T *detail_def)
{

    SUEntitiesRef entities = SU_INVALID;
    SU_CALL(SUModelGetEntities(model, &entities));

    SUComponentDefinitionRef definition = SU_INVALID;
    SU_CALL(SUComponentDefinitionCreate(&definition));
    if (detail_def->name != NULL)
    {
        SU_CALL(SUComponentDefinitionSetName(definition, detail_def->name));
    }
    SU_CALL(SUModelAddComponentDefinitions(model, 1, &definition));

    // Add instance for this definition
    SUComponentInstanceRef instance = SU_INVALID;
    SU_CALL(SUComponentDefinitionCreateInstance(definition, &instance));

    //faces locations inside the component_def
    struct SUTransformation transform = {
            {
                1.0,    0.0,    0.0,    0.0,
                0.0,    1.0,    0.0,    0.0,
                0.0,    0.0,    1.0,    0.0,
                0.0,    0.0,    0.0,    1,
            } };

    // Set the transformation
    SU_CALL(SUComponentInstanceSetTransform(instance, &transform));

    // Populate the entities of the definition using recursion
    SUEntitiesRef instance_entities = SU_INVALID;
    SU_CALL(SUComponentDefinitionGetEntities(definition, &instance_entities));

    _create_detail(instance_entities, detail_def->width, detail_def->height, detail_def->thickness);

    _max_detail_position_X = MAX(_max_detail_position_X, _last_detail_position_X);
    _max_detail_position_Y = MAX(_max_detail_position_Y, _last_detail_position_Y);

    // Do position determination (depending on target detail size)
#if 1
    printf("%d X: %8.1f (%8.1f) / Y: %8.1f (%8.1f) (%.1f/%.1f)", _detail_position_direction,
            _last_detail_position_X, _max_detail_position_X,
            _last_detail_position_Y, _max_detail_position_Y,
            detail_def->width, detail_def->height);
#endif


    if (_detail_position_direction == 0)
    {
        printf("x-1 ");
        if ((_max_detail_position_X > 0) && (_last_detail_position_X + detail_def->width > _max_detail_position_X))
        {
            printf("x-2 ");
            _last_detail_position_X = _max_detail_position_X + DISTANCE_X;
            //_last_detail_position_X = 0;
            _last_detail_position_Y = 0;
            //_last_detail_position_Y = _max_detail_position_Y + DISTANCE_Y;
            _detail_position_direction = 1;
        }
        else
        {
            printf("x-3 ");
            //Update _max_detail_position_Y to size of detail
            _max_detail_position_Y = MAX(_max_detail_position_Y, _last_detail_position_Y + detail_def->height);
        }
    }
    else
    {
        printf("y-1 ");
        if ((_max_detail_position_Y > 0) && (_last_detail_position_Y + detail_def->height > _max_detail_position_Y))
        {
            printf("y-2 ");
            //_last_detail_position_X = _max_detail_position_X + DISTANCE_X;
            _last_detail_position_X = 0;
            //_last_detail_position_Y = 0;
            _last_detail_position_Y = _max_detail_position_Y + DISTANCE_Y;
            _detail_position_direction = 0;
        }
        else
        {
            printf("y-3 ");
            //Update _max_detail_position_Y to size of detail
            _max_detail_position_X = MAX(_max_detail_position_X, _last_detail_position_X + detail_def->width);
        }
    }

    printf("\n%d X: %8.1f (%8.1f) / Y: %8.1f (%8.1f)\n", _detail_position_direction,
    _last_detail_position_X, _max_detail_position_X,
    _last_detail_position_Y, _max_detail_position_Y);

    //component instance location
    transform.values[12] = MM2INCH(_last_detail_position_X);
    transform.values[13] = MM2INCH(_last_detail_position_Y);


    if (_detail_position_direction == 0)
    {
        _last_detail_position_X += DISTANCE_X + detail_def->width;
    }
    else
    {
        _last_detail_position_Y += DISTANCE_Y + detail_def->height;
    }

    SU_CALL(SUComponentInstanceSetTransform(instance, &transform));
    SU_CALL(SUEntitiesAddInstance(entities, instance, NULL));

    for (size_t i = 1; i < detail_def->amount; i++)
    {
        transform.values[14] = MM2INCH(i*detail_def->thickness * DISTANCE_Z);

        SUComponentInstanceRef instance2 = SU_INVALID;
        SU_CALL(SUComponentDefinitionCreateInstance(definition, &instance2));

        // Set the transformation
        SU_CALL(SUComponentInstanceSetTransform(instance2, &transform));
        SU_CALL(SUEntitiesAddInstance(entities, instance2, NULL));
    }

}

int write_new_model()
{
    // Always initialize the API before using it
    SUInitialize();
    // Create an empty model
    SUModelRef model = SU_INVALID;
    SU_CALL(SUModelCreate(&model));


    for (size_t i = 0; i < _details_cnt; i++)
    {
//        printf("Detail %zd:\n", i);
//      _dump_detail(&details[i]);
        _create_detail_components(model, &details[i]);
    }

    // Save the in-memory model to a file
    SU_CALL(SUModelSaveToFile(model, "new_model.skp"));
    SU_CALL(SUModelSaveToFileWithVersion(model, "new_model_SU2017.skp", SUModelVersion_SU2017));
    SU_CALL(SUModelSaveToFileWithVersion(model, "new_model_SU2016.skp", SUModelVersion_SU2016));
    SU_CALL(SUModelSaveToFileWithVersion(model, "new_model_SU3.skp", SUModelVersion_SU3)); //oldest supported version

    // Must release the model or there will be memory leaks
    SU_CALL(SUModelRelease(&model));
    // Always terminate the API when done using it
    SUTerminate();
    return 0;
}

int parse_xml(const WCHAR* xmlfilename)
{
    HRESULT hr = S_OK;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    UINT cwchPrefix;

    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(xmlfilename, STGM_READ, &pFileStream)))
    {
        wprintf(L"Error creating file reader, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx", hr);
        HR(hr);
    }

    BOOL is_empty = FALSE;

    //read until there are no more nodes
    while (S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
            case XmlNodeType_XmlDeclaration:
                wprintf(L"XmlDeclaration\n");
                if (FAILED(hr = WriteAttributes(pReader, L"Declaration", _parse_declaration, NULL)))
                {
                    wprintf(L"Error writing attributes, error is %08.8lx", hr);
                    HR(hr);
                }
                break;
            case XmlNodeType_Element:
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx", hr);
                    HR(hr);
                }
                /*
                            if (cwchPrefix > 0)
                                wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
                            else
                                wprintf(L"Element: %s\n", pwszLocalName);
                */

                // for empty elements call _element_end after parsing attributes
                is_empty = pReader->IsEmptyElement();

                if (FAILED(hr = WriteAttributes(pReader, pwszLocalName, _parse_element, NULL)))
                {
                    wprintf(L"Error writing attributes, error is %08.8lx", hr);
                    HR(hr);
                }

                if (is_empty)
                {
                    hr = _element_end(pwszLocalName, NULL);
                    CHKHR(hr);
                }

                break;
            case XmlNodeType_EndElement:
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx", hr);
                    HR(hr);
                }
                /*
                            if (cwchPrefix > 0)
                                wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
                            else
                                wprintf(L"End Element: %s\n", pwszLocalName);
                */
                hr = _element_end(pwszLocalName, NULL);
                CHKHR(hr);
                break;
            case XmlNodeType_Text:
            case XmlNodeType_Whitespace:
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    HR(hr);
                }
                //wprintf(L"Text: >%s<\n", pwszValue);
                break;
            case XmlNodeType_CDATA:
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    HR(hr);
                }
                wprintf(L"CDATA: %s\n", pwszValue);
                break;
            case XmlNodeType_ProcessingInstruction:
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting name, error is %08.8lx", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    HR(hr);
                }
                wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue);
                break;
            case XmlNodeType_Comment:
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    HR(hr);
                }
                wprintf(L"Comment: %s\n", pwszValue);
                break;
            case XmlNodeType_DocumentType:
                wprintf(L"DOCTYPE is not printed\n");
                break;
        }
    }

#if 0
    for (size_t i = 0; i < _details_cnt; i++)
    {
        printf("Detail %zd:\n", i);
        _dump_detail(&details[i]);
    }
#endif
    // TODO: remove this call
    hr = write_new_model();

    CHKHR(_model_state == MODEL_CLOSED ? S_OK : E_ABORT);

CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;

}

int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteReader.exe name-of-input-file\n");
        return 0;
    }

    return parse_xml(argv[1]);

}
