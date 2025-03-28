#include "viyar.h"
#include "xnc.h"
#include "calc.h"
#include <string>

#include <ole2.h>
#include <xmllite.h>
#include <stdio.h>
#include <shlwapi.h>

/***************************************************************/
/*                     Local Definitions                       */
/***************************************************************/

#pragma warning(disable : 4127)  // conditional expression is constant

//TODO: add "tool.dia" to this macro
#define CALCULATE_EXPR(n, str)                          \
    if ((str) != NULL) {                                \
        std::wstring ws(str);                           \
        size_t pos;                                     \
        while ((pos = ws.find(L"dx")) != -1) {          \
            ws = ws.replace(pos, 2, dx);                \
        }                                               \
        while ((pos = ws.find(L"dy")) != -1) {          \
            ws = ws.replace(pos, 2, dy);                \
        }                                               \
        while ((pos = ws.find(L"dz")) != -1) {          \
            ws = ws.replace(pos, 2, dz);                \
        }                                               \
        while ((pos = ws.find(L"tool.dia")) != -1) {    \
            ws = ws.replace(pos, 2, tool_dia);          \
        }                                               \
        n = calc(ws.c_str());                           \
    }

/***************************************************************/
/*                       Local Types                           */
/***************************************************************/
typedef HRESULT (*attribute_cb)(const WCHAR* elementName,
                                const WCHAR* LocalName,
                                const WCHAR* Value,
                                void *data);

typedef enum {
    STATE_ROOT = 0,
    STATE_PROGRAM,
    STATE_MAX
} XNC_STATE_T;

/***************************************************************/
/*                     Local Variables                         */
/***************************************************************/

static XNC_STATE_T _state = STATE_ROOT;

static PROGRAM_DEF_T *current_program = NULL;

static OPERATION_DEF_T *current_operation = NULL;

static BORE_DEF_T *current_bore = NULL;
static TOOL_DEF_T *current_tool = NULL;

static TOOL_DEF_T *_get_program_tool(PROGRAM_DEF_T *prg, wchar_t *name)
{
    if ((prg == NULL) || (prg->tools_cnt == 0) || (prg->tools == NULL))
    {
        return NULL;
    }

    if (name == NULL)
    {
        //return the lastest
        return &prg->tools[prg->tools_cnt-1];
    }

    for (int i = 0; i < prg->tools_cnt; i++)
    {
        if (wcscmp(prg->tools[i].name, name) == 0)
        {
            return &prg->tools[i];
        }
    }

    return NULL;
}

static PROGRAM_DEF_T *_add_program(void)
{
    if (current_operation == NULL)
    {
        return NULL;
    }

    OPERATION_DEF_T *o = current_operation;

    o->programs_cnt++;
    o->programs = (PROGRAM_DEF_T*)realloc(o->programs, sizeof(PROGRAM_DEF_T)*o->programs_cnt);
    if (o->programs == NULL)
    {
        return NULL;
    }

    PROGRAM_DEF_T *prg = &o->programs[o->programs_cnt-1];
    memset(prg, 0, sizeof(PROGRAM_DEF_T));

    return prg;
}

static TOOL_DEF_T *_add_program_tool(PROGRAM_DEF_T *prg)
{
    if (prg == NULL)
    {
        return NULL;
    }

    prg->tools_cnt++;
    prg->tools = (TOOL_DEF_T*)realloc(prg->tools, sizeof(TOOL_DEF_T)*prg->tools_cnt);
    if (prg->tools == NULL)
    {
        return NULL;
    }

    TOOL_DEF_T *t = &prg->tools[prg->tools_cnt-1];

    memset(t, 0, sizeof(TOOL_DEF_T));

    return t;
}

static BORE_DEF_T *_add_program_bore(PROGRAM_DEF_T *prg, int side)
{
    if (prg == NULL)
    {
        return NULL;
    }

    prg->bores_cnt++;
    prg->bores = (BORE_DEF_T*)realloc(prg->bores, sizeof(BORE_DEF_T)*prg->bores_cnt);
    if (prg->bores == NULL)
    {
        return NULL;
    }

    BORE_DEF_T *b = &prg->bores[prg->bores_cnt-1];

    memset(b, 0, sizeof(BORE_DEF_T));
    b->side = side;

    return b;
}

static HRESULT _element_start(const WCHAR* ElementName, void *data)
{
    wprintf(L"S %d %d: Element start (%p) <%s ...\n", _state, _state, data, ElementName);

    switch (_state)
    {
        case STATE_ROOT:
            if (wcscmp(ElementName, L"program") == 0)
            {
                _state = STATE_PROGRAM;
                current_program = _add_program();
                if (current_program == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            break;

        case STATE_PROGRAM:
        {
            PROGRAM_DEF_T *prg = current_program;

            if (wcscmp(ElementName, L"tool") == 0)
            {
                current_tool = _add_program_tool(prg);
                if (current_tool == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"bf") == 0)
            {
                current_bore = _add_program_bore(prg, SIDE_FRONT);
                if (current_bore == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"br") == 0)
            {
                current_bore = _add_program_bore(prg, SIDE_RIGHT);
                if (current_bore == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"bl") == 0)
            {
                current_bore = _add_program_bore(prg, SIDE_LEFT);
                if (current_bore == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"bt") == 0)
            {
                current_bore = _add_program_bore(prg, SIDE_TOP);
                if (current_bore == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"bb") == 0)
            {
                current_bore = _add_program_bore(prg, SIDE_BACK);
                if (current_bore == NULL)
                {
                    PARSE_FAIL(E_ABORT);
                }
            }
            else if (wcscmp(ElementName, L"ms") == 0)
            {
                //_add_mill();
            }

            break;
        }

        default:
            PARSE_FAIL(E_ABORT);
    }

    return S_OK;
}

static HRESULT _element_end(const WCHAR* ElementName, void *data)
{
    wprintf(L"E %d %d: End element </%s> (%p)\n", _state, _state, ElementName, data);

    switch (_state)
    {
        case STATE_ROOT:
            break;

        case STATE_PROGRAM:
            if (wcscmp(ElementName, L"program") == 0)
            {
                _state = STATE_ROOT;
                current_program = NULL;
            }
            else if (wcscmp(ElementName, L"tool") == 0)
            {
                current_tool = NULL;
            }
            else if ((wcscmp(ElementName, L"bf") == 0) ||
                     (wcscmp(ElementName, L"br") == 0) ||
                     (wcscmp(ElementName, L"bl") == 0) ||
                     (wcscmp(ElementName, L"bt") == 0) ||
                     (wcscmp(ElementName, L"bb") == 0))
            {
                current_bore = NULL;
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
    wprintf(L"_parse_declaration: declaration %s=\"%s\"> (%p)\n", LocalName, Value, data);

    return S_OK;
}

static HRESULT _parse_project(const WCHAR* ElementName,
                              const WCHAR* LocalName,
                              const WCHAR* Value,
                              void *data)
{
    return S_OK;
}

static HRESULT _parse_boolean(bool *ret, const WCHAR* Value)
{
    if (wcscmp(Value, L"true") == 0)
    {
        *ret = true;
    }
    else if (wcscmp(Value, L"false") == 0)
    {
        *ret = false;
    }
    else
    {
        PARSE_FAIL(E_ABORT);
    }

    return S_OK;
}

static HRESULT _parse_bore(BORE_DEF_T *b, const WCHAR* LocalName, const WCHAR* Value)
{
    if (wcscmp(LocalName, L"name") == 0)
    {
        b->name = _wcsdup(Value);
    }
    else if (wcscmp(LocalName, L"x") == 0)
    {
        b->str_x = _wcsdup(Value);
    }
    else if (wcscmp(LocalName, L"y") == 0)
    {
        b->str_y = _wcsdup(Value);
    }
    else if (wcscmp(LocalName, L"dp") == 0)
    {
        b->str_dp = _wcsdup(Value);
    }
    else if (wcscmp(LocalName, L"as") == 0)
    {
        b->str_as = _wcsdup(Value);
    }
    else if (wcscmp(LocalName, L"ver") == 0)
    {
        b->ver = _wtol(Value);
        if (errno)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"ac") == 0)
    {
        b->ac = _wtol(Value);
        if (errno)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"av") == 0)
    {
        if (_parse_boolean(&b->av, Value) != S_OK)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(LocalName, L"m") == 0)
    {
        if (_parse_boolean(&b->m, Value) != S_OK)
        {
            PARSE_FAIL(E_ABORT);
        }
    }

    return S_OK;
}

static HRESULT _parse_program(const WCHAR* ElementName,
                              const WCHAR* LocalName,
                              const WCHAR* Value,
                              void *data)
{
    PROGRAM_DEF_T *prg = current_program;

    if (prg == NULL)
    {
        PARSE_FAIL(E_ABORT);
    }

    if (wcscmp(ElementName, L"program") == 0)
    {
        if (wcscmp(LocalName, L"dx") == 0)
        {
            prg->dx = _wtof(Value);
            if (errno)
            {
                PARSE_FAIL(E_ABORT);
            }
        }
        else if (wcscmp(LocalName, L"dy") == 0)
        {
            prg->dy = _wtof(Value);
            if (errno)
            {
                PARSE_FAIL(E_ABORT);
            }
        }
        else if (wcscmp(LocalName, L"dz") == 0)
        {
            prg->dz = _wtof(Value);
            if (errno)
            {
                PARSE_FAIL(E_ABORT);
            }
        }
    }
    else if (wcscmp(ElementName, L"com") == 0)
    {
        //<com comment="CREATE_IN_EDITOR"/>
    }
    else if (wcscmp(ElementName, L"tool") == 0)
    {
        TOOL_DEF_T *t = current_tool;
        if (t == NULL)
        {
            PARSE_FAIL(E_ABORT);
        }

        if (wcscmp(LocalName, L"d") == 0)
        {
            t->d = _wtof(Value);
            if (errno)
            {
                PARSE_FAIL(E_ABORT);
            }
        }
        else if (wcscmp(LocalName, L"name") == 0)
        {
            t->name = _wcsdup(Value);
        }
    }
    else if ((wcscmp(ElementName, L"bf") == 0) ||
             (wcscmp(ElementName, L"br") == 0) ||
             (wcscmp(ElementName, L"bl") == 0) ||
             (wcscmp(ElementName, L"bt") == 0) ||
             (wcscmp(ElementName, L"bb") == 0))
    {
        if (_parse_bore(current_bore, LocalName, Value) != S_OK)
        {
            PARSE_FAIL(E_ABORT);
        }
    }
    else if (wcscmp(ElementName, L"ms") == 0)
    {
        //<ms x="0" y="2" dp="10" in="0" out="0" sxy="tool.dia/2" fwd="true" c="2" name="mill8"/>
    }
    else if (wcscmp(ElementName, L"ml") == 0)
    {
        //<ml x="dx" y="3" dp="10"/>
    }
    else
    {
        wprintf(L"Unknown Element '%s'\n", ElementName);
        PARSE_FAIL(E_ABORT);
    }

    return S_OK;
}

static HRESULT _parse_element(const WCHAR* ElementName,
                              const WCHAR* LocalName,
                              const WCHAR* Value,
                              void *data)
{
    wprintf(L"P %d %d Element parse: <%s %s=\"%s\"> (%p)\n", _state, _state, ElementName, LocalName, Value, data);

    switch (_state)
    {
        case STATE_ROOT:
            return _parse_project(ElementName, LocalName, Value, data);

        case STATE_PROGRAM:
            return _parse_program(ElementName, LocalName, Value, data);

        default:
            break;
    }

    PARSE_FAIL(E_ABORT);
}

static HRESULT WriteAttributes(IXmlReader* pReader, const WCHAR* ElementName, attribute_cb cb, void *data)
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
        wprintf(L"Error moving to first attribute, error is %08.8lx\n", hr);
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
                    wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx\n", hr);
                    return hr;
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
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

int parse_xml_program(const wchar_t* xmlstr, OPERATION_DEF_T *operation/* in_out */)
{
    HRESULT hr = S_OK;
    IStream *pStream = NULL;
    IStream *pFileStream = NULL;
    IXmlReader *pReader = NULL;
    IXmlReaderInput *xmlReaderInput = NULL;
    XmlNodeType nodeType;
    const wchar_t* pwszPrefix;
    const wchar_t* pwszLocalName;
    const wchar_t* pwszValue;
    UINT cwchPrefix;
    STATSTG ssStreamData = {0};
    ULONG dwWritten = 0;

    current_operation = operation;

    if (FAILED(hr = CreateStreamOnHGlobal(NULL, FALSE, &pStream)))
    {
        wprintf(L"Error creating Stream, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pStream->Write(xmlstr, (ULONG)(wcslen(xmlstr) * sizeof(WCHAR)), &dwWritten)))
    {
        wprintf(L"Error writing Stream, error is %08.8lx\n", hr);
        HR(hr);
    }

    LARGE_INTEGER pos;
    pos.QuadPart = 0;
    if (FAILED(hr = pStream->Seek(pos, STREAM_SEEK_SET, NULL)))
    {
        wprintf(L"Seek Stream, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = CreateXmlReaderInputWithEncodingName(pStream, nullptr, L"UTF-16", FALSE,
                    L"c:\\temp", &xmlReaderInput)))
    {
        wprintf(L"Error creating xml reader with encoding code page, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx\n", hr);
        HR(hr);
    }

    if (FAILED(hr = pReader->SetInput(xmlReaderInput)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx\n", hr);
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
                    wprintf(L"Error writing attributes, error is %08.8lx\n", hr);
                    HR(hr);
                }
                break;
            case XmlNodeType_Element:
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx\n", hr);
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
                    wprintf(L"Error writing attributes, error is %08.8lx\n", hr);
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
                    wprintf(L"Error getting prefix, error is %08.8lx\n", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx\n", hr);
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
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    HR(hr);
                }
                //wprintf(L"Text: >%s<\n", pwszValue);
                break;
            case XmlNodeType_CDATA:
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    HR(hr);
                }
                wprintf(L"CDATA: %s\n", pwszValue);
                break;
            case XmlNodeType_ProcessingInstruction:
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error getting name, error is %08.8lx\n", hr);
                    HR(hr);
                }
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    HR(hr);
                }
                wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue);
                break;
            case XmlNodeType_Comment:
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
                {
                    wprintf(L"Error getting value, error is %08.8lx\n", hr);
                    HR(hr);
                }
                wprintf(L"Comment: %s\n", pwszValue);
                break;
            case XmlNodeType_DocumentType:
                wprintf(L"DOCTYPE is not printed\n");
                break;
        }
    }

    PROGRAM_DEF_T *prg = &current_operation->programs[current_operation->programs_cnt-1];
    printf("prg dx=%f, dy=%f, dz=%f\n", prg->dx, prg->dy, prg->dz);

    wchar_t dx[32];
    wchar_t dy[32];
    wchar_t dz[32];
    wchar_t tool_dia[32];
    swprintf(dx, sizeof(dx), L"%f", prg->dx);
    swprintf(dy, sizeof(dy), L"%f", prg->dy);
    swprintf(dz, sizeof(dz), L"%f", prg->dz);

    for (int i = 0; i < prg->tools_cnt; i++)
    {
        TOOL_DEF_T *t = &prg->tools[i];
        wprintf(L" - tool %d: name=%s, d=%f\n", i, t->name, t->d);
    }

    for (int i = 0; i < prg->bores_cnt; i++)
    {
        BORE_DEF_T *b = &prg->bores[i];
        TOOL_DEF_T *t = _get_program_tool(prg, b->name);
        if (t != NULL)
        {
            b->dia = t->d;
            swprintf(tool_dia, sizeof(tool_dia), L"%f", t->d);
        }
        else
        {
            memset(tool_dia, 0, sizeof(tool_dia));
        }

        CALCULATE_EXPR(b->x, b->str_x);
        CALCULATE_EXPR(b->y, b->str_y);
        CALCULATE_EXPR(b->dp, b->str_dp);
        CALCULATE_EXPR(b->as, b->str_as);

        wprintf(L" - bore %d: side=%d, name=%s, dia=%f, x=%f, y=%f, dp=%f, as=%f, ac=%d\n",
                i, b->side, b->name, b->dia, b->x, b->y, b->dp, b->as, b->ac);
    }

    for (int i = 0; i < prg->mills_cnt; i++)
    {
        MILL_DEF_T *m = &prg->mills[i];
        TOOL_DEF_T *t = _get_program_tool(prg, m->name);
        if (t != NULL)
        {
            m->dia = t->d;
            swprintf(tool_dia, sizeof(tool_dia), L"%f", t->d);
        }
        else
        {
            memset(tool_dia, 0, sizeof(tool_dia));
        }

        CALCULATE_EXPR(m->x, m->str_x);
        CALCULATE_EXPR(m->y, m->str_y);
        CALCULATE_EXPR(m->dp, m->str_dp);
        CALCULATE_EXPR(m->sxy, m->str_sxy);

        wprintf(L" - mill %d: name=%s, dia=%f, x=%f, y=%f, dp=%f, sxy=%f\n", i, m->name, m->dia, m->x, m->y, m->dp, m->sxy);
    }

    hr = S_OK;

    CHKHR(_state == STATE_ROOT ? S_OK : E_ABORT);

CleanUp:
    SAFE_RELEASE(pStream);
    SAFE_RELEASE(pReader);
    return hr;

}
