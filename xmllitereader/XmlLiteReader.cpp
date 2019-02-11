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

#define DEFAULT_COLOR_ALPHA_BAND 192
#define DEFAULT_COLOR_ALPHA_SHEET 128


/***************************************************************/
/*                       Local Types                           */
/***************************************************************/





/***************************************************************/
/*                     Local Variables                         */
/***************************************************************/

static double _last_detail_position_X = 0;
static double _last_detail_position_Y = 0;
static double _max_detail_position_X = 0;
static double _max_detail_position_Y = 0;
static int _detail_position_direction = 0;

int _details_cnt = 0;
DETAIL_DEF_T details[100];

int _materials_cnt = 0;
MATERIAL_DEF_T model_materials[10];

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
