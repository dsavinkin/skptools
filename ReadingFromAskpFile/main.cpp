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
#include <SketchUpAPI/model/group.h>
#include <vector>

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

    // Get the entity container of the model.
    SUEntitiesRef entities = SU_INVALID;
    SUModelGetEntities(model, &entities);

    if (!SUIsInvalid(entities))
    {
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
                    SUEntityRef comp_ent = SUComponentInstanceToEntity(instance);
                    if (!SUIsInvalid(comp_ent))
                    {
                        printf("Valid component instance %zd\n", i);
                    }
                    else
                    {
                        printf("Not recognized instance %zd\n", i);
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
    }

    // Get all the faces from the entities object
    size_t faceCount = 0;
    SUEntitiesGetNumFaces(entities, &faceCount);
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

    // Must release the model or there will be memory leaks
    SUModelRelease(&model);

    // Always terminate the API when done using it
    SUTerminate();
    return 0;
}
