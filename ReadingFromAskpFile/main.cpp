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

#define PRINT_COUNT(func, ...) do { \
    size_t count; \
    func(__VA_ARGS__, &count); \
    printf(#func" = %zd\n", count); \
} while (0)

int main() {
	const char *skp_filename = "model.skp";
	printf("reading '%s'\n", skp_filename);

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
    printf("name=%s\n", name_utf8);
    delete []name_utf8;

    enum SUModelUnits units;
    SUModelGetUnits(model, &units);
    printf("uinits=%d\n", units);

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
        printf("num_instances=%zd\n", num_instances);

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
                        printf("name=%s\n", name_utf8);
                        delete []name_utf8;
                    }

                    if (1)
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
                        printf("guid=%s\n", guid_utf8);
                        delete []guid_utf8;
                    }

                    if (1)
                    {
                        struct SUTransformation transform;
                        SUComponentInstanceGetTransform(instance, &transform);
                        printf("transform=\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n%f\t%f\t%f\t%f\n",
                              transform.values[0],transform.values[1],transform.values[2],transform.values[3],
                              transform.values[4],transform.values[5],transform.values[6],transform.values[7],
                              transform.values[8],transform.values[9],transform.values[10],transform.values[11],
                              transform.values[12],transform.values[13],transform.values[14],transform.values[15]);
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
        printf("num_groups=%zd\n", num_groups);

        if (num_groups > 0) {
            std::vector<SUGroupRef> groups(num_groups);
            SUEntitiesGetGroups(entities, num_groups, &groups[0], &num_groups);

            for (size_t i = 0; i < num_groups; i++) {
                SUGroupRef group = groups[i];
                if (!SUIsInvalid(group)) {
                    // Get the component part of the group
                    printf("Valid group %zd\n", i);
                    SUEntitiesRef group_entities = SU_INVALID;
                    SUGroupGetEntities(group, &group_entities);

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
                size_t edgeCount = 0;
                SUFaceGetNumEdges(faces[i], &edgeCount);
                if (edgeCount > 0) {
                    std::vector<SUEdgeRef> edges(edgeCount);
                    SUFaceGetEdges(faces[i], edgeCount, &edges[0], &edgeCount);

                    // Get the vertex positions for each edge
                    for (size_t j = 0; j < edgeCount; j++) {
                        SUVertexRef startVertex = SU_INVALID;
                        SUVertexRef endVertex = SU_INVALID;
                        SUEdgeGetStartVertex(edges[j], &startVertex);
                        SUEdgeGetEndVertex(edges[j], &endVertex);
                        SUPoint3D start;
                        SUPoint3D end;
                        SUVertexGetPosition(startVertex, &start);
                        SUVertexGetPosition(endVertex, &end);
                        // Now do something with the point data

                        printf("face %zd: edge %zd : (%.0f-%.0f-%.0f to %.0f-%.0f-%.0f)\n", i, j,
                               start.x, start.y, start.z,
                               end.x, end.y, end.z);
                    }
                }
            }
        }
    }

    size_t num_component_def = 0;
    SUModelGetNumComponentDefinitions(model, &num_component_def);
    printf("num_component_def=%zd\n", num_component_def);

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
                    printf("name=%s\n", name_utf8);
                    delete []name_utf8;
                }

                if (1)
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
                    printf("guid=%s\n", guid_utf8);
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
                    printf("desc=%s\n", guid_utf8);
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
                    printf("path=%s\n", guid_utf8);
                    delete []guid_utf8;
                }

                SUPoint3D insertPoint;
                SUComponentDefinitionGetInsertPoint(component, &insertPoint);
                printf("insert_point: %f-%f-%f\n", insertPoint.x, insertPoint.y, insertPoint.z);

                enum SUComponentType type;
                SUComponentDefinitionGetType(component, &type);
                printf("type=%d\n", type);

                PRINT_COUNT(SUComponentDefinitionGetNumUsedInstances, component);
                PRINT_COUNT(SUComponentDefinitionGetNumInstances, component);
                PRINT_COUNT(SUComponentDefinitionGetNumOpenings, component);
            }
            else
            {
                printf("invalid instance %zd\n", i);
            }
        }
    }

    // Must release the model or there will be memory leaks
    SUModelRelease(&model);

    // Always terminate the API when done using it
    SUTerminate();
    return 0;
}
