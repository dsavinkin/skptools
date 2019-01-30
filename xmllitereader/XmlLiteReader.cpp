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
#define HR(stmt)                do { hr = (stmt); goto CleanUp; } while(0)
#define SAFE_RELEASE(I)         do { if (I){ I->Release(); } I = NULL; } while(0)


HRESULT WriteAttributes(IXmlReader* pReader)
{
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    HRESULT hr = pReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx", hr);
        return hr;
    }
    else
    {
        while (TRUE)
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
                if (cwchPrefix > 0)
                    wprintf(L"Attr: %s:%s=\"%s\" \n", pwszPrefix, pwszLocalName, pwszValue);
                else
                    wprintf(L"Attr: %s=\"%s\" \n", pwszLocalName, pwszValue);
            }

            if (S_OK != pReader->MoveToNextAttribute())
                break;
        }
    }
    return hr;
}

int write_new_model()
{
    // Always initialize the API before using it
    SUInitialize();
    // Create an empty model
    SUModelRef model = SU_INVALID;
    SUResult res = SUModelCreate(&model);
    // It's best to always check the return code from each SU function call.
    // Only showing this check once to keep this example short.
    if (res != SU_ERROR_NONE)
        return 1;
    // Get the entity container of the model
    SUEntitiesRef entities = SU_INVALID;
    SUModelGetEntities(model, &entities);
    // Create a loop input describing the vertex ordering for a face's outer loop
    SULoopInputRef outer_loop = SU_INVALID;
    SULoopInputCreate(&outer_loop);
    for (size_t i = 0; i < 4; ++i) {
        SULoopInputAddVertexIndex(outer_loop, i);
    }
    // Create the face
    SUFaceRef face = SU_INVALID;
    SUPoint3D vertices[4] = { { 0,   0,   0 },
        { 100, 100, 0 },
        { 100, 100, 100 },
        { 0,   0,   100 } };
    SUFaceCreate(&face, vertices, &outer_loop);
    // Add the face to the entities
    SUEntitiesAddFaces(entities, 1, &face);
    // Save the in-memory model to a file
    SUModelSaveToFile(model, "new_model.skp");
    // Must release the model or there will be memory leaks
    SUModelRelease(&model);
    // Always terminate the API when done using it
    SUTerminate();
    return 0;
}

int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    HRESULT hr = S_OK;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    UINT cwchPrefix;

    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteReader.exe name-of-input-file\n");
        return 0;
    }

    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(argv[1], STGM_READ, &pFileStream)))
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
            if (FAILED(hr = WriteAttributes(pReader)))
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
            if (cwchPrefix > 0)
                wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"Element: %s\n", pwszLocalName);

            if (FAILED(hr = WriteAttributes(pReader)))
            {
                wprintf(L"Error writing attributes, error is %08.8lx", hr);
                HR(hr);
            }

            if (pReader->IsEmptyElement() )
                wprintf(L" (empty)");
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
            if (cwchPrefix > 0)
                wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
            else
                wprintf(L"End Element: %s\n", pwszLocalName);
            break;
        case XmlNodeType_Text:
        case XmlNodeType_Whitespace:
            if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
            {
                wprintf(L"Error getting value, error is %08.8lx", hr);
                HR(hr);
            }
            wprintf(L"Text: >%s<\n", pwszValue);
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

CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);
    return hr;
}
