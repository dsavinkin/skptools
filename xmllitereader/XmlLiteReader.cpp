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

#pragma warning(disable : 4127)  // conditional expression is constant
#define CHKHR(stmt)             do { hr = (stmt); if (FAILED(hr)) goto CleanUp; } while(0)
#define HR(stmt)                do { hr = (stmt); printf("HR line %d\n", __LINE__);goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)

#define PARSE_FAIL(ret)                do { printf("HR line %d\n", __LINE__); return (ret); } while(0)


#define MM2INCH(x) ((x)/25.4)
#define INCH2MM(x) ((x)*25.4)

#ifndef SU_CALL
#define SU_CALL(func) if ((func) != SU_ERROR_NONE) throw std::exception()
#endif

#define DISTANCE_X 50 //mm
#define DISTANCE_Z 3 //*thickness

typedef enum {
    ELEMENT_PROJECT,
    ELEMENT_VIYAR,
    ELEMENT_USER,
    ELEMENT_ORDER,
    ELEMENT_CONSTRUCTOR,
    ELEMENT_MATERIALS,
    ELEMENT_MATERIAL,
    ELEMENT_PARTS,
    ELEMENT_DETAILS,
    ELEMENT_DETAIL,
    ELEMENT_EDGES,
    ELEMENT_LEFT,
    ELEMENT_RIGHT,
    ELEMENT_TOP,
    ELEMENT_BOTTOM,
    ELEMENT_OPERATIONS,
    ELEMENT_OPERATION,
    ELEMENT_PRODUCTS,
    ELEMENT_UNKNOWN,
    MAX_ELEMENTS
} ELEMENTS_T;

const WCHAR *elements_str[MAX_ELEMENTS] = {
    L"project",
    L"viyar",
    L"user",
    L"order",
    L"constructor",
    L"materials",
    L"material",
    L"parts",
    L"details",
    L"detail",
    L"edges",
    L"left",
    L"right",
    L"top",
    L"bottom",
    L"operations",
    L"operation",
    L"products",
    L"unknown"
};

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
    SIDE_FRONT,
    SIDE_LEFT,
    SIDE_TOP,
    SIDE_RIGHT,
    SIDE_BOTTOM,
    SIDE_BACK
} SIDES_T;

typedef struct {
    char* name;
    double width;
    double height;
    double thickness;
    size_t amount;
} DETAIL_DEF_T;

static VIYAR_STATE_T _state = STATE_ROOT;
static MODEL_STATE_T _model_state = MODEL_NONE;

typedef HRESULT (*attribute_cb)(const WCHAR* elementName,
                                const WCHAR* LocalName,
                                const WCHAR* Value,
                                void *data);

static double _last_detail_position_X = 0;


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
                    wprintf(L"TODO: create/read Model\n");
                }
            }
            else if (wcscmp(ElementName, L"materials") == 0)
            {
                if (_model_state != MODEL_OPENED)
                {
                    PARSE_FAIL(E_ABORT);
                }
                _state = STATE_MATERIALS;
            }
            else if (wcscmp(ElementName, L"details") == 0)
            {
                if (_model_state != MODEL_OPENED)
                {
                    PARSE_FAIL(E_ABORT);
                }
                _state = STATE_DETAILS;
            }
            else
            {
                // Ignore
            }
            break;

        case STATE_MATERIALS:
            if (wcscmp(ElementName, L"material") == 0)
            {
                wprintf(L"TODO: start adding materials\n");
            }
            else
            {
                wprintf(L"TODO: (%s) continue updating materials\n", ElementName);
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                wprintf(L"TODO: start adding details\n");
            }
            else
            {
                wprintf(L"TODO: (%s) continue updating details\n", ElementName);
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
                    wprintf(L"TODO: save/close Model\n");
                }
            }
            break;

        case STATE_MATERIALS:
            if (wcscmp(ElementName, L"materials") == 0)
            {
                _state = STATE_ROOT;
            }
            break;

        case STATE_DETAILS:
            if (wcscmp(ElementName, L"detail") == 0)
            {
                wprintf(L"TODO: add detail to Model\n");
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

static HRESULT _parse_element(const WCHAR* ElementName,
                              const WCHAR* LocalName,
                              const WCHAR* Value,
                              void *data)
{
    //wprintf(L"<%s %s=\"%s\"> (%p)\n", ElementName, LocalName, Value, data);

    return S_OK;
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
    SU_CALL(SUComponentDefinitionSetName(definition, detail_def->name));
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

    //component instance location
    transform.values[12] = _last_detail_position_X;

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

    _last_detail_position_X += MM2INCH(DISTANCE_X + detail_def->width);

}

int write_new_model()
{
    // Always initialize the API before using it
    SUInitialize();
    // Create an empty model
    SUModelRef model = SU_INVALID;
    SU_CALL(SUModelCreate(&model));

    DETAIL_DEF_T detail;
    detail.name = "detail 1";
    detail.width = 70;
    detail.height = 360;
    detail.thickness = 18;
    detail.amount = 5;

    _create_detail_components(model, &detail);

    detail.name = "detail 2";
    detail.width = 270;
    detail.height = 160;
    detail.thickness = 18;
    detail.amount = 3;

    _create_detail_components(model, &detail);


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
                if (FAILED(hr = WriteAttributes(pReader, pwszLocalName, _parse_element, NULL)))
                {
                    wprintf(L"Error writing attributes, error is %08.8lx", hr);
                    HR(hr);
                }

                if (pReader->IsEmptyElement() )
                    wprintf(L"Element %s (empty)\n", pwszLocalName);
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

    hr = write_new_model();

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
