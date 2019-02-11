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
#include <SketchUpAPI/color.h>
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
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/arccurve.h>

#include <vector>
#include <iostream>
#include <codecvt>
#include <locale>

#include "drill.h"
#include "viyar.h"

/***************************************************************/
/*                     Local Definitions                       */
/***************************************************************/

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); printf("HR line %d\n", __LINE__);goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define PARSE_FAIL(ret)                do { printf("PARSE_FAIL line %d\n", __LINE__); return (ret); } while(0)

#define DEFAULT_COLOR_ALPHA_BAND 192
#define DEFAULT_COLOR_ALPHA_SHEET 128

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
    DETAIL_ATTR,
    DETAIL_EDGES,
    DETAIL_OPERATIONS
} DETAIL_STATE_T;

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
static MATERIAL_DEF_T model_materials[10];

/***************************************************************/
/*                     Local Functions                         */
/***************************************************************/

void _dump_detail(DETAIL_DEF_T *d)
{
    if (!d)
    {
        return;
    }

    wprintf(L"name:        %s\n", d->name);
    printf("description: %s\n", d->description);
    printf("size:        %.1f/%.1f/%.1f\n", d->width, d->height, d->thickness);
    printf("amount:      %zd\n", d->amount);
}


static HRESULT _model_open_create()
{
    //wprintf(L"TODO: create/read Model\n");
    return S_OK;
}

static HRESULT _model_save_close()
{
    //wprintf(L"TODO: save/close Model\n");
    return S_OK;
}

static HRESULT _element_start(const WCHAR* ElementName, void *data)
{
    //wprintf(L"S %d: Element start (%p) <%s ...\n", _state, data, ElementName);

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
                memset(model_materials, 0, sizeof(model_materials));
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
                //wprintf(L"TODO: (%d) start adding material\n", _materials_cnt);
            }
            else
            {
                //wprintf(L"TODO: (%d:%s) continue updating material\n", _materials_cnt, ElementName);
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                _details_cnt++;
                _detail_state = DETAIL_ATTR;

                DETAIL_DEF_T *d = &details[_details_cnt-1];
                for (size_t i = 0 ; i < 6; i++)
                {
                    //Set default material for all bands = material 1
                    d->m_bands[i] = 1;
                }
                //wprintf(L"_detail_state = DETAIL_ATTR\n");
//                wprintf(L"TODO: (%d) start adding detail\n", _details_cnt);
            }
            else
            {
                //wprintf(L"TODO: (%d:%s) continue updating detail\n", _details_cnt, ElementName);

                if (wcscmp(ElementName, L"edges") == 0)
                {
                    _detail_state = DETAIL_EDGES;
                    //wprintf(L"_detail_state = DETAIL_EDGES\n");
                }
                else if (wcscmp(ElementName, L"edge") == 0)
                {
                    if (_detail_state != DETAIL_EDGES)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(ElementName, L"operations") == 0)
                {
                    _detail_state = DETAIL_OPERATIONS;
                    //wprintf(L"_detail_state = DETAIL_OPERATIONS\n");
                }
                else if (wcscmp(ElementName, L"operation") == 0)
                {
                    if (_detail_state != DETAIL_OPERATIONS)
                    {
                        PARSE_FAIL(E_ABORT);
                    }

                    DETAIL_DEF_T *d = &details[_details_cnt-1];

                    d->operations_cnt++;
                    d->operations = (OPERATION_T*)realloc(d->operations, sizeof(OPERATION_T)*d->operations_cnt);
                    if (d->operations == NULL)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
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
    //wprintf(L"S %d: End element </%s> (%p)\n", _state, ElementName, data);

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
                //wprintf(L"TODO: add material (%d) to Model\n", _materials_cnt);
            }
            else if (wcscmp(ElementName, L"materials") == 0)
            {
                _state = STATE_ROOT;
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                //wprintf(L"TODO: add detail (%d) to Model\n", _details_cnt);
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
    MATERIAL_DEF_T *m = &model_materials[_materials_cnt-1];

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

        m->color.red = red;
        m->color.green = green;
        m->color.blue = blue;
        m->color.alpha = DEFAULT_COLOR_ALPHA_BAND;
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

    //wprintf(L"detail %d:%d <%s: %s=\"%s\"> (%p)\n", _details_cnt, _detail_state, ElementName, LocalName, Value, data);

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
                    MATERIAL_DEF_T *m = &model_materials[d->material_id-1];
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
                    //wprintf(L"description='%s'\n", Value);
                    if (wcslen(Value) > 0)
                    {
                        // set name for non-empty components only.
                        d->name = _wcsdup(Value);
                    }
                }
                else if (wcscmp(LocalName, L"grain") == 0)
                {
                    d->grain = _wtol(Value);
                }
                else
                {
                    //wprintf(L"Ignore attribute %s (%d) %s=\"%s\"\n", ElementName, _details_cnt, LocalName, Value);
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
                    else if (material_id > 0)
                    {
                        MATERIAL_DEF_T *m = &model_materials[material_id-1];

                        if (wcscmp(ElementName, L"top") == 0)
                        {
                            d->m_bands[SIDE_TOP] = material_id;
                            d->height += m->thickness;
                        }
                        else if (wcscmp(ElementName, L"bottom") == 0)
                        {
                            d->m_bands[SIDE_BOTTOM] = material_id;
                            d->height += m->thickness;
                        }
                        else if (wcscmp(ElementName, L"left") == 0)
                        {
                            d->m_bands[SIDE_LEFT] = material_id;
                            d->width += m->thickness;
                        }
                        else if (wcscmp(ElementName, L"right") == 0)
                        {
                            d->m_bands[SIDE_RIGHT] = material_id;
                            d->width += m->thickness;
                        }
                    }
                    else
                    {
                        //Do not update default value in d->m_bands
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

                if (d->operations_cnt == 0)
                {
                    PARSE_FAIL(E_ABORT);
                }

                OPERATION_T * op = &d->operations[d->operations_cnt-1];

                if (wcscmp(LocalName, L"id") == 0)
                {
                    if (_wtol(Value) != d->operations_cnt)
                    {
                        PARSE_FAIL(E_ABORT);
                    }
                }
                else if (wcscmp(LocalName, L"type") == 0)
                {
                    if (wcscmp(Value, L"drilling") == 0)
                    {
                        op->type = TYPE_DRILLING;
                    }
                    else if (wcscmp(Value, L"shapeByPattern") == 0)
                    {
                        op->type = TYPE_SHAPEBYPATTERN;
                    }
                    else if (wcscmp(Value, L"rabbeting") == 0)
                    {
                        op->type = TYPE_RABBETING;
                    }
                    else if (wcscmp(Value, L"grooving") == 0)
                    {
                        op->type = TYPE_GROOVING;
                    }
                    else if (wcscmp(Value, L"cornerOperation") == 0)
                    {
                        op->type = TYPE_CORNEROPERATION;
                    }
                    else
                    {
                        wprintf(L"Ignore operation (%d) %s=\"%s\"\n", _details_cnt, LocalName, Value);
                        return S_FALSE;
                    }
                }
                else if (wcscmp(LocalName, L"subtype") == 0)
                {
                    //op->subtype = _wcsdup(Value);
                    op->subtype = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"xl") == 0)
                {
                    op->xl = _wcsdup(Value);
                }
                else if (wcscmp(LocalName, L"yl") == 0)
                {
                    op->yl = _wcsdup(Value);
                }
                else if (wcscmp(LocalName, L"x") == 0)
                {
                    op->x = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"y") == 0)
                {
                    op->y = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"xo") == 0)
                {
                    op->xo = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"yo") == 0)
                {
                    op->yo = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"d") == 0)
                {
                    op->d = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"r") == 0)
                {
                    op->r = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"depth") == 0)
                {
                    op->depth = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"millD") == 0)
                {
                    op->millD = _wtof(Value);
                }
                else if (wcscmp(LocalName, L"side") == 0)
                {
                    op->side = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"corner") == 0)
                {
                    op->corner = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"mill") == 0)
                {
                    op->mill = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"ext") == 0)
                {
                    op->ext = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"edgeMaterial") == 0)
                {
                    op->edgeMaterial = _wtol(Value);
                }
                else if (wcscmp(LocalName, L"edgeCovering") == 0)
                {
                    op->edgeCovering = _wtol(Value);
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

static void _add_face(SUEntitiesRef entities, SUPoint3D *vertices, size_t num_vertices, SUMaterialRef material)
{
    SULoopInputRef outer_loop = SU_INVALID;
    SU_CALL(SULoopInputCreate(&outer_loop));
    for (size_t i = 0; i < num_vertices; ++i) {
        SULoopInputAddVertexIndex(outer_loop, i);
    }
    // Create the face
    SUFaceRef face = SU_INVALID;

    SU_CALL(SUFaceCreate(&face, vertices, &outer_loop));

    if (!SUIsInvalid(material))
    {
        SU_CALL(SUFaceSetFrontMaterial(face, material));
        SU_CALL(SUFaceSetBackMaterial(face, material));
    }

    // Add the face to the entities
    SU_CALL(SUEntitiesAddFaces(entities, 1, &face));
}

static void _add_drill(SUEntitiesRef entities, SUPoint3D corner, OPERATION_T *op, SUVector3D normal, double depth, size_t side)
{
    SUPoint3D center = {MM2INCH(corner.x), MM2INCH(corner.y), MM2INCH(corner.z)};

    double X = MM2INCH(op->x);
    double Y = MM2INCH(op->y);
    double D = MM2INCH(op->d);
    double DEPTH = MM2INCH(depth);

    if ((side == SIDE_TOP) || (side == SIDE_BOTTOM))
    {
        center.z -= Y;
    }
    else
    {
        center.y += Y;
    }

    if ((side == SIDE_LEFT) || (side == SIDE_RIGHT))
    {
        center.z -= X;
    }
    else
    {
        center.x += X;
    }

    SUPoint3D start_point = center;

    if ((side == SIDE_LEFT) || (side == SIDE_RIGHT))
    {
        start_point.z += D/2;
    }
    else
    {
        start_point.x += D/2;
    }

    SUArcCurveRef arccurve = SU_INVALID;
    SU_CALL(SUArcCurveCreate(&arccurve, &center, &start_point, &start_point, &normal, 16));

    // Add the ArcCyrves to the entities
    SU_CALL(SUEntitiesAddArcCurves(entities, 1, &arccurve));

    SUPoint3D center2 = {
        center.x + normal.x*DEPTH,
        center.y + normal.y*DEPTH,
        center.z + normal.z*DEPTH,
    };

    start_point = center2;
    if ((side == SIDE_LEFT) || (side == SIDE_RIGHT))
    {
        start_point.z += D/2;
    }
    else
    {
        start_point.x += D/2;
    }

    SUArcCurveRef arccurve2 = SU_INVALID;
    SU_CALL(SUArcCurveCreate(&arccurve2, &center2, &start_point, &start_point, &normal, 16));

    // Add the ArcCyrves to the entities
    SU_CALL(SUEntitiesAddArcCurves(entities, 1, &arccurve2));

    SUEdgeRef edge = SU_INVALID;
    SU_CALL(SUEdgeCreate(&edge, &center, &center2));

    // Add the Edge to the entities
    SU_CALL(SUEntitiesAddEdges(entities, 1, &edge));
}

/* (points [*num_points-1]) contains current corner point */
static int _corner_operation(SUPoint3D points[12], int *band_materials, size_t *num_points, size_t cn, OPERATION_T *cop)
{
    if (!cop)
    {
        return 0;
    }

    SUPoint3D original_point = points[(*num_points)-1];

    double X = cop->x;
    double Y = cop->y;
    int material_H = 1; //same as for sheet
    int material_V = 1;

    if (cop->edgeMaterial > 0)
    {
        MATERIAL_DEF_T *m = &model_materials[cop->edgeMaterial-1];

        if (cop->edgeCovering == EDGE_COVER_BOTH)
        {
            X -= m->thickness;
            Y -= m->thickness;
            material_H = cop->edgeMaterial;
            material_V = cop->edgeMaterial;
            //printf("Set both material_H, material_V=%d\n", material_H);
        }
        else if (cop->edgeCovering == EDGE_COVER_H)
        {
            material_H = cop->edgeMaterial;
            Y -= m->thickness;
            //printf("Set material_H=%d\n", material_H);
        }
        else if (cop->edgeCovering == EDGE_COVER_V)
        {
            material_V = cop->edgeMaterial;
            X -= m->thickness;
            //printf("Set material_V=%d\n", material_V);
        }
    }


    if (cn == CORNER_LOWER_LEFT)
    {
        points[(*num_points)-1].x += (X);
        band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        if (cop->subtype == 3)
        {
            points[(*num_points)++] = points[(*num_points)-1];
            points[(*num_points)-1].y += (Y);
            band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        }

        points[(*num_points)++] = original_point;
        points[(*num_points)-1].y += (Y);
        band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
    }
    else if (cn == CORNER_UPPER_LEFT)
    {
        points[(*num_points)-1].y -= (Y);
        band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        if (cop->subtype == 3)
        {
            points[(*num_points)++] = points[(*num_points)-1];
            points[(*num_points)-1].x += (X);
            band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        }

        points[(*num_points)++] = original_point;
        points[(*num_points)-1].x += X;
        band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);

        points[(*num_points)-1].x += X;
    }
    else if (cn == CORNER_UPPER_RIGHT)
    {
        points[(*num_points)-1].x -= (X);
        band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        if (cop->subtype == 3)
        {
            points[(*num_points)++] = points[(*num_points)-1];
            points[(*num_points)-1].y -= (Y);
            band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        }

        points[(*num_points)++] = original_point;
        points[(*num_points)-1].y -= (Y);
        band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
    }
    else if (cn == CORNER_LOWER_RIGHT)
    {
        points[(*num_points)-1].y += (Y);
        band_materials[(*num_points)-1] = material_V;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        if (cop->subtype == 3)
        {
            points[(*num_points)++] = points[(*num_points)-1];
            points[(*num_points)-1].x -= (X);
            band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
        }

        points[(*num_points)++] = original_point;
        points[(*num_points)-1].x -= (X);
        band_materials[(*num_points)-1] = material_H;
//        wprintf(L"Added new point %.3f %.3f for corner %zd\n",
//                points[(*num_points)-1].x, points[(*num_points)-1].y, cn);
    }

    return 0;
}

static int _create_detail_component(SUEntitiesRef entities, DETAIL_DEF_T *d)
{

    //End coordinates of detail in INCHES
    double X = (d->width);
    double Y = (d->height);
    double Z = (d->thickness);

    OPERATION_T *corner[CORNER_MAX];
    size_t num_corner_operations = 0;
    memset(corner, 0, sizeof(corner));
    for (size_t j = 0; j < d->operations_cnt; j++)
    {
        OPERATION_T *op = &d->operations[j];
        if (op->type == TYPE_CORNEROPERATION)
        {
            wprintf(L"%zd: Corner operation: corner=%d, subtype=%d, x=%.1f, y=%.1f, r=%.f, mill=%d, "
                    "ext=%d, edgeMaterial=%d, edgeCovering=%d\n",
                    j, op->corner, op->subtype, op->x, op->y, op->r, op->mill, op->ext, op->edgeMaterial, op->edgeCovering);

            if ((op->corner <= 0) || (op->corner > CORNER_MAX))
            {
                PARSE_FAIL(-1);
            }

            if (op->subtype != 3)
            {
                wprintf(L"TODO: Corner operation subtype=%d not supported.\n", op->subtype);
                continue;
            }

            if (op->ext != 1)
            {
                wprintf(L"TODO: Corner operation subtype=%d ext=%d not supported.\n", op->subtype, op->ext);
                continue;
            }

            corner[op->corner-1] = op;
            num_corner_operations++;
        }
        else if (op->type != TYPE_DRILLING)
        {
            //wprintf(L"TODO: Operation type=%d not supported.\n", op->type);
        }
    }


    SUPoint3D sides[6][4] = {
        {   //SIDE_FRONT
            { 0, 0, Z },
            { 0, Y, Z },
            { X, Y, Z },
            { X, 0, Z },
        },
        {   //SIDE_LEFT
            { 0, 0, Z },
            { 0, Y, Z },
            { 0, Y, 0 },
            { 0, 0, 0 },
        },
        {   //SIDE_TOP
            { 0, Y, Z },
            { X, Y, Z },
            { X, Y, 0 },
            { 0, Y, 0 },
        },
        {   //SIDE_RIGHT
            { X, 0, Z },
            { X, 0, 0 },
            { X, Y, 0 },
            { X, Y, Z },
        },
        {   //SIDE_BOTTOM
            { 0, 0, Z },
            { 0, 0, 0 },
            { X, 0, 0 },
            { X, 0, Z },
        },
        {   //SIDE_BACK
            { 0, 0, 0 },
            { 0, Y, 0 },
            { X, Y, 0 },
            { X, 0, 0 },
        },
    };

    SUVector3D normals[6] = {
        { 0,  0, -1},  //SIDE_FRONT
        { 1,  0,  0},  //SIDE_LEFT
        { 0, -1,  0},  //SIDE_TOP
        {-1,  0,  0},  //SIDE_RIGHT
        { 0,  1,  0},  //SIDE_BOTTOM
        { 0,  0,  1},  //SIDE_BACK
    };

    //for now it can be maximum 3*4
    SUPoint3D sheet_points[12];
    size_t num_sheet_points = 0;

    int band_materials[12]; //material index corresponds to the starting point of sheet_points
    memset(band_materials, 0, sizeof(band_materials));

    SUMaterialRef material = SU_INVALID;
    if (d->m_bands[SIDE_FRONT])
    {
        int m_id = d->m_bands[SIDE_FRONT];
        MATERIAL_DEF_T *m = &model_materials[m_id-1];
        material = m->material;
    }

    for (size_t cn = 0; cn < CORNER_MAX; cn++)
    {
        sheet_points[num_sheet_points++] = sides[SIDE_FRONT][cn];
        _corner_operation(sheet_points, band_materials, &num_sheet_points, cn, corner[cn]);
        band_materials[num_sheet_points-1] = d->m_bands[cn+1];
    }

    for (size_t j = 0 ; j < num_sheet_points; j++)
    {
        sheet_points[j].x =  MM2INCH(sheet_points[j].x);
        sheet_points[j].y =  MM2INCH(sheet_points[j].y);
        sheet_points[j].z =  MM2INCH(sheet_points[j].z);
    }

    num_sheet_points  = num_sheet_points;
    _add_face(entities, sheet_points, num_sheet_points, material);

    for (size_t j = 0 ; j < num_sheet_points ; j++)
    {
        SUMaterialRef material = SU_INVALID;

        if (band_materials[j])
        {
            int m_id = band_materials[j];
            MATERIAL_DEF_T *m = &model_materials[m_id-1];
            material = m->material;
        }

        SUPoint3D points[4];

        points[0] = sheet_points[j];
        points[1] = sheet_points[(j+1) % num_sheet_points];
        points[2] = points[1];
        points[2].z = 0;
        points[3] = points[0];
        points[3].z = 0;

        //add material
        _add_face(entities, points, 4, material);
    }

    if (d->m_bands[SIDE_BACK])
    {
        int m_id = d->m_bands[SIDE_BACK];
        MATERIAL_DEF_T *m = &model_materials[m_id-1];
        material = m->material;
    }

    for (size_t j = 0 ; j < num_sheet_points; j++)
    {
        sheet_points[j].z = 0;
    }

    _add_face(entities, sheet_points, num_sheet_points, material);

    for (size_t i = 0; i < 6; ++i)
    {
        size_t drill_cnt = 0;
        for (size_t j = 0; j < d->operations_cnt; j++)
        {
            OPERATION_T *op = &d->operations[j];
            if ((op->type == TYPE_DRILLING) && (op->side == i+1))
            {
                drill_cnt++;
                double depth = op->depth;
                if (((i == SIDE_FRONT) || (i == SIDE_BACK))
                        && (depth > d->thickness))
                {
                    depth = d->thickness;
                }

                _add_drill(entities, sides[i][0], op, normals[i], depth, i);
            }
        }
    }
    return 0;
}

static void _add_update_detail_components(SUModelRef model, DETAIL_DEF_T *detail_def)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8;
    SUEntitiesRef entities = SU_INVALID;
    SUComponentDefinitionRef component = SU_INVALID;
    SUComponentInstanceRef instance = SU_INVALID;
    size_t componentNumInstancesCount = 0;
    bool ComponentFound = false;

    //faces locations inside the component_def
    struct SUTransformation transform = {
        {
            1.0,    0.0,    0.0,    0.0,
            0.0,    1.0,    0.0,    0.0,
            0.0,    0.0,    1.0,    0.0,
            0.0,    0.0,    0.0,    1,
        } };

    if (detail_def->amount == 0)
    {
        wprintf(L"detail_def->amount = 0 - skip adding component.");
        return;
    }

    SU_CALL(SUModelGetEntities(model, &entities));

    if (detail_def->name != NULL)
    {
        utf8 = converter.to_bytes(detail_def->name);

        size_t num_component_def = 0;
        SUModelGetNumComponentDefinitions(model, &num_component_def);

        if (num_component_def > 0)
        {
            std::vector<SUComponentDefinitionRef> components(num_component_def);
            SUModelGetComponentDefinitions(model, num_component_def,
                                           &components[0], &num_component_def);

            for (size_t i = 0; (i < num_component_def) && !ComponentFound; i++)
            {
                component = components[i];
                if (!SUIsInvalid(component))
                {
                    SUStringRef name = SU_INVALID;
                    SU_CALL(SUStringCreate(&name));
                    SU_CALL(SUComponentDefinitionGetName(component, &name));
                    size_t name_length = 0;
                    SU_CALL(SUStringGetUTF8Length(name, &name_length));
                    char* name_utf8 = new char[name_length + 1];
                    SU_CALL(SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length));
                    // Now we have the name in a form we can use
                    SU_CALL(SUStringRelease(&name));

                    ComponentFound = (strcmp(utf8.c_str(), name_utf8) == 0);
                    delete []name_utf8;

                    //SU_CALL(SUComponentDefinitionGetNumInstances(component, &componentNumInstancesCount));
                    SU_CALL(SUComponentDefinitionGetNumUsedInstances(component, &componentNumInstancesCount));
                }
            }
        }
    }

    if (ComponentFound)
    {
        printf("Found component with name '%s', instances =%zd (required %zd) - update it.\n",
                utf8.c_str(), componentNumInstancesCount, detail_def->amount);
    }
    else
    {
        componentNumInstancesCount = 0;
        component = SU_INVALID;
        SU_CALL(SUComponentDefinitionCreate(&component));
        if (detail_def->name != NULL)
        {
            printf("Set component name '%s'\n", utf8.c_str());
            SU_CALL(SUComponentDefinitionSetName(component, utf8.c_str()));
        }

        SU_CALL(SUModelAddComponentDefinitions(model, 1, &component));
    }

    if (componentNumInstancesCount > 0)
    {
        size_t instance_count;
        SU_CALL(SUComponentDefinitionGetInstances(component, 1, &instance, &instance_count));
    }
    else
    {
        // Add instance for this definition
        SU_CALL(SUComponentDefinitionCreateInstance(component, &instance));

        // Set default transformation for new component
        SU_CALL(SUComponentInstanceSetTransform(instance, &transform));
    }

    // Populate the entities of the definition using recursion
    SUEntitiesRef instance_entities = SU_INVALID;
    SU_CALL(SUComponentDefinitionGetEntities(component, &instance_entities));

    if (ComponentFound)
    {
        size_t faceCount = 0;
        SU_CALL(SUEntitiesGetNumFaces(instance_entities, &faceCount));
        if (faceCount > 0)
        {
            std::vector<SUFaceRef> faces(faceCount);
            SU_CALL(SUEntitiesGetFaces(instance_entities, faceCount, &faces[0], &faceCount));
            std::vector<SUEntityRef> elements(faceCount);
            for (size_t i = 0; i < faceCount; i++)
            {
                elements[i] = SUFaceToEntity(faces[i]);
            }

            // Erase all faces from component
            SU_CALL(SUEntitiesErase(instance_entities, faceCount, &elements[0]));

            SU_CALL(SUEntitiesGetNumFaces(instance_entities, &faceCount));
        }

        size_t edgeCount = 0;
        SU_CALL(SUEntitiesGetNumEdges(instance_entities, false, &edgeCount));
        if (edgeCount > 0)
        {
            std::vector<SUEdgeRef> edges(edgeCount);
            SU_CALL(SUEntitiesGetEdges(instance_entities, false, edgeCount, &edges[0], &edgeCount));
            std::vector<SUEntityRef> elements(edgeCount);
            for (size_t i = 0; i < edgeCount; i++)
            {
                elements[i] = SUEdgeToEntity(edges[i]);
            }

            // Erase all faces from component
            SU_CALL(SUEntitiesErase(instance_entities, edgeCount, &elements[0]));

            //SU_CALL(SUEntitiesGetNumEdges(instance_entities, false, &edgeCount));
        }

    }

    // Create detail component
    _create_detail_component(instance_entities, detail_def);

/*
    size_t edgeCount = 0;
    SU_CALL(SUEntitiesGetNumEdges(instance_entities, false, &edgeCount));
    printf("and now edgeCount=%zd\n", edgeCount);
*/
    if (detail_def->amount > componentNumInstancesCount)
    {
        // Need to add some component instances to the model
        _max_detail_position_X = MAX(_max_detail_position_X, _last_detail_position_X);
        _max_detail_position_Y = MAX(_max_detail_position_Y, _last_detail_position_Y);

        // Do position determination (depending on target detail size)
        if (_detail_position_direction == 0)
        {
            if ((_max_detail_position_X > 0) && (_last_detail_position_X + detail_def->width > _max_detail_position_X))
            {
                _last_detail_position_X = _max_detail_position_X + DISTANCE_X;
                _last_detail_position_Y = 0;
                _max_detail_position_X = MAX(_max_detail_position_X, _last_detail_position_X + detail_def->width);
                _detail_position_direction = 1;
            }
            else
            {
                //Update _max_detail_position_Y to size of detail
                _max_detail_position_Y = MAX(_max_detail_position_Y, _last_detail_position_Y + detail_def->height);
            }
        }
        else
        {
            if ((_max_detail_position_Y > 0) && (_last_detail_position_Y + detail_def->height > _max_detail_position_Y))
            {
                _last_detail_position_Y = _max_detail_position_Y + DISTANCE_Y;
                _last_detail_position_X = 0;
                _max_detail_position_Y = MAX(_max_detail_position_Y, _last_detail_position_Y + detail_def->height);
                _detail_position_direction = 0;
            }
            else
            {
                //Update _max_detail_position_Y to size of detail
                _max_detail_position_X = MAX(_max_detail_position_X, _last_detail_position_X + detail_def->width);
            }
        }

        //component instance location
        transform.values[12] = MM2INCH(_last_detail_position_X);
        transform.values[13] = MM2INCH(_last_detail_position_Y);

        // Update _last_detail_position
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

        for (size_t i = componentNumInstancesCount+1; i < detail_def->amount; i++)
        {
            transform.values[14] = MM2INCH(i*detail_def->thickness * DISTANCE_Z);

            SUComponentInstanceRef instance2 = SU_INVALID;
            SU_CALL(SUComponentDefinitionCreateInstance(component, &instance2));

            // Set the transformation
            SU_CALL(SUComponentInstanceSetTransform(instance2, &transform));
            SU_CALL(SUEntitiesAddInstance(entities, instance2, NULL));
        }
    }
    else if (detail_def->amount < componentNumInstancesCount)
    {
        wprintf(L"TODO: need to remove some components instances (required %zd but %zd present)\n",
                detail_def->amount, componentNumInstancesCount);
    }

}

static void _add_update_material(SUModelRef model, SUMaterialRef *m_ptr, const char *m_name, SUColor *color)
{
    bool material_found = false;
    size_t num_materials = 0;
    SUMaterialRef material = SU_INVALID;
    SU_CALL(SUModelGetNumMaterials(model, &num_materials));

    // Find same materials in the model
    if (num_materials > 0)
    {
        std::vector<SUMaterialRef> materials(num_materials);
        SU_CALL(SUModelGetMaterials(model, num_materials,
                                    &materials[0], &num_materials));

        for (size_t i = 0; (i < num_materials) && !material_found; i++)
        {
            material = materials[i];

            SUStringRef name = SU_INVALID;
            SU_CALL(SUStringCreate(&name));
            SU_CALL(SUMaterialGetName(material, &name));
            size_t name_length = 0;
            SU_CALL(SUStringGetUTF8Length(name, &name_length));
            char* name_utf8 = new char[name_length + 1];
            SU_CALL(SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length));
            // Now we have the name in a form we can use
            SU_CALL(SUStringRelease(&name));

            material_found = (strcmp(m_name, name_utf8) == 0);
            delete []name_utf8;
        }
    }

    if (!material_found)
    {
        material = SU_INVALID;
        SU_CALL(SUMaterialCreate(&material));
        if (m_name != NULL)
        {
            SU_CALL(SUMaterialSetName(material, m_name));
        }
        SU_CALL(SUModelAddMaterials(model, 1, &material));
    }

    SU_CALL(SUMaterialSetColor(material, color));
    SU_CALL(SUMaterialSetType(material, SUMaterialType_Colored));

    *m_ptr = material;
}

int write_new_model(const WCHAR *model_filename)
{
    // Always initialize the API before using it
    SUInitialize();
    // Create an empty model
    SUModelRef model = SU_INVALID;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string model_filename_utf8;
    model_filename_utf8 = converter.to_bytes(model_filename);
    printf("Model file is '%s'\n", model_filename_utf8.c_str());

    if (SUModelCreateFromFile(&model, model_filename_utf8.c_str()) != SU_ERROR_NONE)
    {
        wprintf(L"Unable to open model file '%s' - will create new one.\n", model_filename);
        SU_CALL(SUModelCreate(&model));
    }

    //Add materials to the model and save them as materials[].material
    for (size_t i = 0; i < _materials_cnt; i++)
    {
        MATERIAL_DEF_T *m = &model_materials[i];
        printf("material %zd: type=%d, thickness=%.1f\n", i+1,
               m->type, m->thickness);

        if (m->type == TYPE_BAND)
        {
            //Create custom colors based on thickness
            SUColor color;
            color.alpha = DEFAULT_COLOR_ALPHA_BAND;

            if (m->thickness <= 0.6)
            {
                color.red = 0;
                color.green = 153;
                color.blue = 0;
            }
            else if (m->thickness <= 1.0)
            {
                color.red = 101;
                color.green = 255;
                color.blue = 255;
            }
            else if (m->thickness < 2.0)
            {
                color.red = 0;
                color.green = 0;
                color.blue = 153;
            }
            else if (m->thickness == 2.0)
            {
                color.red = 102;
                color.green = 0;
                color.blue = 102;
            }

            char m_name[32];
            snprintf(m_name, sizeof(m_name), "kromka_%.1f", m->thickness);
            _add_update_material(model, &m->material, m_name, &color);
        }
        else if (m->type == TYPE_SHEET)
        {
            SUColor color;
            color.alpha = DEFAULT_COLOR_ALPHA_SHEET;
            color.red = 255;
            color.green = 255;
            color.blue = 255;

            _add_update_material(model, &m->material, "Sheet", &color);
        }
        else
        {
            m->material = SU_INVALID;
        }
    }

    for (size_t i = 0; i < _details_cnt; i++)
    {
#if 0
        printf("Detail %zd:\n", i);
        _dump_detail(&details[i]);
#endif
        _add_update_detail_components(model, &details[i]);
    }

    // Save the in-memory model to a file
    SU_CALL(SUModelSaveToFile(model, model_filename_utf8.c_str()));
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
    IXmlReaderInput *xmlReaderInput = NULL;
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

    if (FAILED(hr = CreateXmlReaderInputWithEncodingName(pFileStream, nullptr, L"windows-1251", FALSE,
                    L"c:\temp", &xmlReaderInput)))
    {
        wprintf(L"Error creating xml reader with encoding code page, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(xmlReaderInput)))
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
    hr = S_OK;

    CHKHR(_model_state == MODEL_CLOSED ? S_OK : E_ABORT);

CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;

}

int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    if (argc != 3)
    {
        wprintf(L"Usage: XmlLiteReader <viyar_project_file> <sketchup_model_file>\n");
        wprintf(L"       If sketchup_model_file not present program will create new one\n");
        return 0;
    }

    HRESULT hr = parse_xml(argv[1]);

    if (FAILED(hr))
    {
        return hr;
    }

    return write_new_model(argv[2]);
}
