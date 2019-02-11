#include "viyar.h"

#include <ole2.h>
#include <xmllite.h>
#include <stdio.h>
#include <shlwapi.h>

static VIYAR_STATE_T _state = STATE_ROOT;
static MODEL_STATE_T _model_state = MODEL_NONE;
static DETAIL_STATE_T _detail_state = DETAIL_ATTR;

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
/* Ignore since incorrect values in markingColor
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
*/
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
