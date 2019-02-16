#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/initialize.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/model.h>
#include <SketchUpAPI/model/entities.h>
#include <SketchUpAPI/model/face.h>
#include <SketchUpAPI/model/edge.h>
#include <SketchUpAPI/model/vertex.h>

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/model/component_instance.h>
#include <SketchUpAPI/model/component_definition.h>
#include <SketchUpAPI/model/group.h>
#include <SketchUpAPI/model/material.h>
#include <SketchUpAPI/model/texture.h>
#include <vector>

#define MM2INCH(x) ((x)/25.4)
#define INCH2MM(x) ((x)*25.4)

#define PRINT_COUNT(func, ...) do { \
    size_t count; \
    func(__VA_ARGS__, &count); \
    printf("%s: "#func" = %zd\n", prefix, count); \
} while (0)

static void _print_material(SUMaterialRef material, const char *prefix)
{
    if (!SUIsInvalid(material))
    {
        if (1)
        {
            SUStringRef name = SU_INVALID;
            SUStringCreate(&name);
            SUMaterialGetName(material, &name);
            size_t name_length = 0;
            SUStringGetUTF8Length(name, &name_length);
            char* name_utf8 = new char[name_length + 1];
            SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
            // Now we have the name in a form we can use
            SUStringRelease(&name);
            printf("%s: name='%s'\n", prefix, name_utf8);
            delete []name_utf8;
        }

        if (1)
        {
            SUColor color;
            SUMaterialGetColor(material, &color);
            printf("%s: color=rgb(%d,%d,%d,%d)\n", prefix,
                   color.red, color.green, color.blue, color.alpha);
        }

        if (1)
        {
            SUTextureRef texture;
            SUMaterialGetTexture(material, &texture);
        }

        if (1)
        {
            double alpha;
            SUMaterialGetOpacity(material, &alpha);
            printf("%s: Opacity=%f\n", prefix, alpha);
        }

        if (1)
        {
            bool use_opacity;
            SUMaterialGetUseOpacity(material, &use_opacity);
            printf("%s: UseOpacity=%d\n", prefix, use_opacity);
        }

        if (1)
        {
            enum SUMaterialType type;
            SUMaterialGetType(material, &type);
            printf("%s: type=%d\n", prefix, type);
        }

        if (1)
        {
            bool transparency;
            SUMaterialIsDrawnTransparent(material, &transparency);
            printf("%s: transparency=%d\n", prefix, transparency);
        }
    }
}

static void _list_entities(SUEntitiesRef entities, const char *prefix)
{
    if (SUIsInvalid(entities))
    {
        printf("%s: Invalid entities.\n", prefix);
        return;
    }

    PRINT_COUNT(SUEntitiesGetNumFaces, entities);
    PRINT_COUNT(SUEntitiesGetNumCurves, entities);
    PRINT_COUNT(SUEntitiesGetNumArcCurves, entities);
    PRINT_COUNT(SUEntitiesGetNumGuidePoints, entities);
    PRINT_COUNT(SUEntitiesGetNumGuideLines, entities);
    PRINT_COUNT(SUEntitiesGetNumEdges, entities, false);
    PRINT_COUNT(SUEntitiesGetNumPolyline3ds, entities);
    PRINT_COUNT(SUEntitiesGetNumImages, entities);
    PRINT_COUNT(SUEntitiesGetNumGroups, entities);
    PRINT_COUNT(SUEntitiesGetNumImages, entities);
    PRINT_COUNT(SUEntitiesGetNumInstances, entities);
    PRINT_COUNT(SUEntitiesGetNumSectionPlanes, entities);
    PRINT_COUNT(SUEntitiesGetNumTexts, entities);
    PRINT_COUNT(SUEntitiesGetNumDimensions, entities);

    size_t num_instances = 0;
    SUEntitiesGetNumInstances(entities, &num_instances);
    printf("%s: SUEntitiesGetNumInstances=%zd\n", prefix, num_instances);

    if (num_instances > 0)
    {
        std::vector<SUComponentInstanceRef> instances(num_instances);
        SUEntitiesGetInstances(entities, num_instances,
                               &instances[0], &num_instances);

        for (size_t i = 0; i < num_instances; i++) {
            SUComponentInstanceRef instance = instances[i];
            if (!SUIsInvalid(instance))
            {
                if (1)
                {
                    SUStringRef name = SU_INVALID;
                    SUStringCreate(&name);
                    SUComponentInstanceGetName(instance, &name);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(name, &name_length);
                    char* name_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&name);
                    printf("%s: %zd: SUComponentInstanceGetName='%s'\n", prefix, i, name_utf8);
                    delete []name_utf8;
                }

                if (0)
                {
                    SUStringRef guid = SU_INVALID;
                    SUStringCreate(&guid);
                    SUComponentInstanceGetGuid(instance, &guid);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(guid, &name_length);
                    char* guid_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(guid, name_length + 1, guid_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&guid);
                    printf("%s: %zd: SUComponentInstanceGetGuid='%s'\n", prefix, i, guid_utf8);
                    delete []guid_utf8;
                }

                if (1)
                {
                    struct SUTransformation transform;
                    SUComponentInstanceGetTransform(instance, &transform);
                    printf("%s: %zd: SUComponentInstanceGetTransform=\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n",
                           prefix, i,
                           transform.values[0],transform.values[1],transform.values[2],transform.values[3],
                           transform.values[4],transform.values[5],transform.values[6],transform.values[7],
                           transform.values[8],transform.values[9],transform.values[10],transform.values[11],
                           transform.values[12]*25.4,transform.values[13]*25.4,transform.values[14]*25.4, transform.values[15]);
                }
            }
            else
            {
                printf("invalid instance %zd\n", i);
            }
        }
    }

    size_t num_groups;
    SUEntitiesGetNumGroups(entities, &num_groups);
    printf("%s: num_groups=%zd\n", prefix, num_groups);

    if (num_groups > 0) {
        std::vector<SUGroupRef> groups(num_groups);
        SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups);

        for (size_t i = 0; i < num_groups; i++) {
            SUGroupRef group = groups[i];
            if (!SUIsInvalid(group)) {

                if (1)
                {
                    struct SUTransformation transform;
                    SUGroupGetTransform(group, &transform);
                    printf("%s: %zd: SUGroupGetTransform=\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n",
                           prefix, i,
                           transform.values[0],transform.values[1],transform.values[2],transform.values[3],
                           transform.values[4],transform.values[5],transform.values[6],transform.values[7],
                           transform.values[8],transform.values[9],transform.values[10],transform.values[11],
                           transform.values[12]*25.4,transform.values[13]*25.4,transform.values[14]*25.4, transform.values[15]);
                }

                // Get the component part of the group
                //printf("Valid group %zd\n", i);
                SUEntitiesRef group_entities = SU_INVALID;
                SUGroupGetEntities(group, &group_entities);

                _list_entities(group_entities, "group");

            }
            else
            {
                printf("Invalid group %zd\n", i);
            }
        }
    }
    // Get all the faces from the entities object
    size_t faceCount = 0;
    SUEntitiesGetNumFaces(entities, &faceCount);
    printf("faceCount=%zd\n", faceCount);
    if (faceCount > 0) {
        std::vector<SUFaceRef> faces(faceCount);
        SUEntitiesGetFaces(entities, faceCount, &faces[0], &faceCount);

        // Get all the edges in this face
        for (size_t i = 0; i < faceCount; i++) {
            SUFaceRef face = faces[i];
            size_t edgeCount = 0;
            SUFaceGetNumEdges(face, &edgeCount);
            if (edgeCount > 0) {
                std::vector<SUEdgeRef> edges(edgeCount);
                SUFaceGetEdges(face, edgeCount, &edges[0], &edgeCount);

                // Get the vertex positions for each edge
                for (size_t j = 0; j < edgeCount; j++) {
                    SUEdgeRef edge = edges[j];
                    SUVertexRef startVertex = SU_INVALID;
                    SUVertexRef endVertex = SU_INVALID;
                    SUEdgeGetStartVertex(edge, &startVertex);
                    SUEdgeGetEndVertex(edge, &endVertex);
                    SUPoint3D start;
                    SUPoint3D end;
                    SUVertexGetPosition(startVertex, &start);
                    SUVertexGetPosition(endVertex, &end);
                    // Now do something with the point data

                    printf("face %zd: edge : (%.1f-%.1f-%.1f to %.1f-%.1f-%.1f)\n", i,
                           INCH2MM(start.x), INCH2MM(start.y), INCH2MM(start.z),
                           INCH2MM(end.x), INCH2MM(end.y), INCH2MM(end.z));
                }

                if (1)
                {
                    SUMaterialRef material = SU_INVALID;
                    if (SUFaceGetFrontMaterial(face, &material) == SU_ERROR_NONE)
                    {
                        _print_material(material, "face front");
                    }
                    else
                    {
                        printf("face %zd has no front material\n", i);
                    }

                    material = SU_INVALID;
                    if (SUFaceGetBackMaterial(face, &material) == SU_ERROR_NONE)
                    {
                        _print_material(material, "face back");
                    }
                    else
                    {
                        printf("face %zd has no back material\n", i);
                    }
                }
            }
        }
    }

    size_t edgeCount = 0;
    SUEntitiesGetNumEdges(entities, false, &edgeCount);
    if (edgeCount > 0)
    {
        std::vector<SUEdgeRef> edges(edgeCount);
        SUEntitiesGetEdges(entities, false, edgeCount, &edges[0], &edgeCount);

        // Get the vertex positions for each edge
        for (size_t j = 0; j < edgeCount; j++) {
            SUEdgeRef edge = edges[j];
            SUVertexRef startVertex = SU_INVALID;
            SUVertexRef endVertex = SU_INVALID;
            SUEdgeGetStartVertex(edge, &startVertex);
            SUEdgeGetEndVertex(edge, &endVertex);
            SUPoint3D start;
            SUPoint3D end;
            SUVertexGetPosition(startVertex, &start);
            SUVertexGetPosition(endVertex, &end);
            // Now do something with the point data

            printf("signle edge : (%.1f-%.1f-%.1f to %.1f-%.1f-%.1f)\n",
                   INCH2MM(start.x), INCH2MM(start.y), INCH2MM(start.z),
                   INCH2MM(end.x), INCH2MM(end.y), INCH2MM(end.z));
        }

#if 0
        std::vector<SUEntityRef> elements(edgeCount);
        for (size_t i = 0; i < edgeCount; i++)
        {
            elements[i] = SUEdgeToEntity(edges[i]);
        }

        // Erase all faces from component
        //SU_CALL(SUEntitiesErase(instance_entities, edgeCount, &elements[0]));

        //SU_CALL(SUEntitiesGetNumEdges(instance_entities, false, &edgeCount));
#endif
    }
}


int main(int argc, char **argv)
{
    const char *skp_filename = (argc == 1) ? "model.skp" : argv[1];
    const char *prefix = "model";
    printf("reading '%s'...\n", skp_filename);

    // Always initialize the API before using it
    SUInitialize();

    // Load the model from a file
    SUModelRef model = SU_INVALID;
    SUResult res = SUModelCreateFromFile(&model, skp_filename);

    // It's best to always check the return code from each SU function call.
    // Only showing this check once to keep this example short.
    if (res != SU_ERROR_NONE)
        return 1;

    // Get model name
    SUStringRef name = SU_INVALID;
    SUStringCreate(&name);
    SUModelGetName(model, &name);
    size_t name_length = 0;
    SUStringGetUTF8Length(name, &name_length);
    char* name_utf8 = new char[name_length + 1];
    SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
    // Now we have the name in a form we can use
    SUStringRelease(&name);
    printf("%s: name='%s'\n", prefix, name_utf8);
    delete []name_utf8;

    enum SUModelUnits units;
    SUModelGetUnits(model, &units);
    printf("%s: uinits=%d\n", prefix, units);

    PRINT_COUNT(SUModelGetNumMaterials, model);
    PRINT_COUNT(SUModelGetNumComponentDefinitions, model);
    PRINT_COUNT(SUModelGetNumGroupDefinitions, model);
    PRINT_COUNT(SUModelGetNumScenes, model);
    PRINT_COUNT(SUModelGetNumLayers, model);
    PRINT_COUNT(SUModelGetNumAttributeDictionaries, model);
    PRINT_COUNT(SUModelGetNumFonts, model);

    // Get the entity container of the model.
    SUEntitiesRef entities = SU_INVALID;
    SUModelGetEntities(model, &entities);

    if (!SUIsInvalid(entities))
    {
        _list_entities(entities, prefix);
    }

    size_t num_component_def = 0;
    SUModelGetNumComponentDefinitions(model, &num_component_def);
    printf("%s: num_component_def=%zd\n", prefix, num_component_def);

    if (num_component_def > 0)
    {
        std::vector<SUComponentDefinitionRef> components(num_component_def);
        SUModelGetComponentDefinitions(model, num_component_def,
                               &components[0], &num_component_def);

        for (size_t i = 0; i < num_component_def; i++) {
            SUComponentDefinitionRef component = components[i];
            if (!SUIsInvalid(component))
            {

                if (1)
                {
                    SUStringRef name = SU_INVALID;
                    SUStringCreate(&name);
                    SUComponentDefinitionGetName(component, &name);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(name, &name_length);
                    char* name_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(name, name_length + 1, name_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&name);
                    printf("%s: name='%s'\n", prefix, name_utf8);
                    delete []name_utf8;
                }

                if (0)
                {
                    SUStringRef guid = SU_INVALID;
                    SUStringCreate(&guid);
                    SUComponentDefinitionGetGuid(component, &guid);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(guid, &name_length);
                    char* guid_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(guid, name_length + 1, guid_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&guid);
                    printf("%s: guid='%s'\n", prefix, guid_utf8);
                    delete []guid_utf8;
                }

                if (1)
                {
                    SUStringRef desc = SU_INVALID;
                    SUStringCreate(&desc);
                    SUComponentDefinitionGetDescription(component, &desc);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(desc, &name_length);
                    char* guid_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(desc, name_length + 1, guid_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&desc);
                    printf("%s: desc='%s'\n", prefix, guid_utf8);
                    delete []guid_utf8;
                }

                if (1)
                {
                    SUStringRef path = SU_INVALID;
                    SUStringCreate(&path);
                    SUComponentDefinitionGetPath(component, &path);
                    size_t name_length = 0;
                    SUStringGetUTF8Length(path, &name_length);
                    char* guid_utf8 = new char[name_length + 1];
                    SUStringGetUTF8(path, name_length + 1, guid_utf8, &name_length);
                    // Now we have the name in a form we can use
                    SUStringRelease(&path);
                    printf("%s: path='%s'\n", prefix, guid_utf8);
                    delete []guid_utf8;
                }

                SUPoint3D insertPoint;
                SUComponentDefinitionGetInsertPoint(component, &insertPoint);
                printf("%s: insert_point: %f-%f-%f\n", prefix, insertPoint.x, insertPoint.y, insertPoint.z);

                enum SUComponentType type;
                SUComponentDefinitionGetType(component, &type);
                printf("%s: type=%d\n", prefix, type);

                PRINT_COUNT(SUComponentDefinitionGetNumUsedInstances, component);
                PRINT_COUNT(SUComponentDefinitionGetNumInstances, component);
                PRINT_COUNT(SUComponentDefinitionGetNumOpenings, component);

                SUEntitiesRef entities;
                SUComponentDefinitionGetEntities(component, &entities);
                _list_entities(entities, "component");
            }
            else
            {
                printf("invalid instance %zd\n", i);
            }
        }
    }


    size_t num_materials = 0;
    SUModelGetNumMaterials(model, &num_materials);
    printf("%s: num_materials=%zd\n", prefix, num_materials);

    if (num_materials > 0)
    {
        std::vector<SUMaterialRef> materials(num_materials);
        SUModelGetMaterials(model, num_materials,
                            &materials[0], &num_materials);

        for (size_t i = 0; i < num_materials; i++) {
            SUMaterialRef material = materials[i];
            _print_material(material, prefix);
        }
    }

    // Must release the model or there will be memory leaks
    SUModelRelease(&model);

    // Always terminate the API when done using it
    SUTerminate();
    return 0;
}
