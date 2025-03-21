#include "calc.h"

#include <iomanip>

static double number();
static double factor();
static double expr();
static double brackets();

static wchar_t cin_get(void);
static void cin_putback(wchar_t c);

static const wchar_t *input_str = NULL;

static wchar_t cin_get(void)
{
    wchar_t c = *input_str;
    input_str++;

    return c;
}

static void cin_putback(wchar_t c)
{
#if 0
    *input_str = c;
#endif
    input_str--;
}

double calc(const wchar_t *str)
{
    if (str == NULL)
    {
        return 0.0;
    }

    input_str = str;

    return expr();
}

double number()
{
    double result = 0.0;
    double k = 10.0;
    int sign = 1;
    wchar_t c;

    c = cin_get();

    while (c == ' ')
        c = cin_get();

    if (c == '-')
        sign = -1;
    else
        cin_putback(c);

    while (true)
    {
        c = cin_get();

        while (c == ' ')
            c = cin_get();

        if (c >= '0' && c <= '9')
            result = result * 10.0 + (c - '0');
        else
        {
            cin_putback(c);
            break;
        }
    }

    c = cin_get();

    if (c == '.')
    {
        while (true)
        {
            c = cin_get();

            if (c >= '0' && c <= '9')
            {
                result += (c - '0') / k;
                k *= 10.0;
            }
            else
            {
                cin_putback(c);
                break;
            }
        }
    }
    else
        cin_putback(c);

    return sign * result;
}

double factor()
{
    double result = brackets();
    double temp;
    wchar_t c;

    while (true)
    {
        c = cin_get();

        while (c == ' ')
            c = cin_get();

        switch (c)
        {
            case '*':
                result *= brackets();
                break;
            case '/':
                temp = brackets();

                if (temp == 0.0)
                {
                    printf("Divide by zero\n");
                    exit(-1);
                }

                result /= temp;
                break;
            default:
                cin_putback(c);
                return result;
        }
    }
}

double expr()
{
    double result = factor();
    wchar_t c;

    while (true)
    {
        c = cin_get();

        while (c == ' ')
            c = cin_get();

        switch (c)
        {
            case '+':
                result += factor();
                break;
            case '-':
                result -= factor();
                break;
            default:
                cin_putback(c);
                return result;
        }
    }
}

double brackets()
{
    double result;
    int sign = 1;
    wchar_t c;

    c = cin_get();

    while (c == ' ')
        c = cin_get();

    if (c == '-')
    {
        sign = -1;
        c = cin_get();
    }

    while (c == ' ')
        c = cin_get();

    if (c == '(')
    {
        result = sign * expr();

        c = cin_get();

        if (c != ')')
        {
            printf("Brackets error\n");
            exit(-1);
        }

        return result;
    }
    else
    {
        cin_putback(c);

        return sign * number();
    }
}
