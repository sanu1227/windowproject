// main.cpp
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "resource.h"

HINSTANCE g_hInst;
HWND g_hDlg;

bool g_isBinary = false;
std::wstring g_decimalBeforeBinary;

INT_PTR CALLBACK CalcProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

std::wstring GetEditText(HWND hDlg)
{
    wchar_t buf[512];
    GetDlgItemTextW(hDlg, IDC_EDIT, buf, 512);
    return buf;
}

void SetEditText(HWND hDlg, const std::wstring& text)
{
    SetDlgItemTextW(hDlg, IDC_EDIT, text.c_str());
}

bool IsOperator(wchar_t ch)
{
    return ch == L'+' || ch == L'-' || ch == L'*' || ch == L'/';
}

std::wstring FormatNumber(double value)
{
    if (std::fabs(value - std::round(value)) < 0.000000001)
    {
        long long v = static_cast<long long>(std::round(value));
        return std::to_wstring(v);
    }

    std::wstringstream ss;
    ss << std::fixed << std::setprecision(10) << value;

    std::wstring result = ss.str();

    while (!result.empty() && result.back() == L'0')
        result.pop_back();

    if (!result.empty() && result.back() == L'.')
        result.pop_back();

    return result;
}

std::wstring RemoveTrailingOperators(std::wstring s)
{
    while (!s.empty() && IsOperator(s.back()))
        s.pop_back();

    if (s.empty())
        return L"0";

    return s;
}

bool ParseNumber(const std::wstring& s, size_t& i, double& value)
{
    std::wstring num;

    if (s[i] == L'-')
    {
        num += L'-';
        i++;
    }

    bool hasDigit = false;

    while (i < s.size())
    {
        if ((s[i] >= L'0' && s[i] <= L'9') || s[i] == L'.')
        {
            if (s[i] >= L'0' && s[i] <= L'9')
                hasDigit = true;

            num += s[i];
            i++;
        }
        else
        {
            break;
        }
    }

    if (!hasDigit)
        return false;

    value = _wtof(num.c_str());
    return true;
}

bool EvaluateExpression(std::wstring expr, double& result)
{
    expr = RemoveTrailingOperators(expr);

    std::vector<double> nums;
    std::vector<wchar_t> ops;

    size_t i = 0;

    while (i < expr.size())
    {
        if (expr[i] == L' ')
        {
            i++;
            continue;
        }

        bool unaryMinus =
            expr[i] == L'-' &&
            (i == 0 || IsOperator(expr[i - 1]));

        if ((expr[i] >= L'0' && expr[i] <= L'9') || expr[i] == L'.' || unaryMinus)
        {
            double value = 0;

            if (!ParseNumber(expr, i, value))
                return false;

            nums.push_back(value);
        }
        else if (IsOperator(expr[i]))
        {
            ops.push_back(expr[i]);
            i++;
        }
        else
        {
            return false;
        }
    }

    if (nums.empty())
        return false;

    if (nums.size() != ops.size() + 1)
        return false;

    // 먼저 * / 계산
    for (size_t j = 0; j < ops.size();)
    {
        if (ops[j] == L'*' || ops[j] == L'/')
        {
            double a = nums[j];
            double b = nums[j + 1];
            double v = 0;

            if (ops[j] == L'*')
            {
                v = a * b;
            }
            else
            {
                if (b == 0)
                    return false;

                v = a / b;
            }

            nums[j] = v;
            nums.erase(nums.begin() + j + 1);
            ops.erase(ops.begin() + j);
        }
        else
        {
            j++;
        }
    }

    // 그 다음 + - 계산
    result = nums[0];

    for (size_t j = 0; j < ops.size(); j++)
    {
        if (ops[j] == L'+')
            result += nums[j + 1];
        else if (ops[j] == L'-')
            result -= nums[j + 1];
    }

    return true;
}

std::wstring ToBinary(long long value)
{
    if (value == 0)
        return L"0";

    bool negative = value < 0;

    if (negative)
        value = -value;

    std::wstring result;

    while (value > 0)
    {
        result = wchar_t(L'0' + value % 2) + result;
        value /= 2;
    }

    if (negative)
        result = L"-" + result;

    return result;
}

void AppendDigit(HWND hDlg, int digit)
{
    std::wstring text = GetEditText(hDlg);

    if (g_isBinary || text == L"0")
    {
        text = L"";
        g_isBinary = false;
    }

    text += wchar_t(L'0' + digit);
    SetEditText(hDlg, text);
}

void AppendOperator(HWND hDlg, wchar_t op)
{
    std::wstring text = GetEditText(hDlg);

    if (g_isBinary)
        g_isBinary = false;

    if (text.empty())
        text = L"0";

    if (IsOperator(text.back()))
    {
        text.back() = op;
    }
    else
    {
        text += op;
    }

    SetEditText(hDlg, text);
}

void CalculateEqual(HWND hDlg)
{
    std::wstring text = GetEditText(hDlg);
    double result = 0;

    if (EvaluateExpression(text, result))
    {
        SetEditText(hDlg, FormatNumber(result));
        g_isBinary = false;
    }
    else
    {
        SetEditText(hDlg, L"Error");
        g_isBinary = false;
    }
}

void ReverseNumbers(HWND hDlg)
{
    std::wstring text = GetEditText(hDlg);
    std::wstring result;

    size_t i = 0;

    while (i < text.size())
    {
        if (text[i] >= L'0' && text[i] <= L'9')
        {
            std::wstring number;

            while (i < text.size() && text[i] >= L'0' && text[i] <= L'9')
            {
                number += text[i];
                i++;
            }

            for (int j = static_cast<int>(number.size()) - 1; j >= 0; j--)
                result += number[j];
        }
        else
        {
            result += text[i];
            i++;
        }
    }

    SetEditText(hDlg, result);
    g_isBinary = false;
}

void ClearEntry(HWND hDlg)
{
    std::wstring text = GetEditText(hDlg);

    if (text.empty() || text == L"0")
    {
        SetEditText(hDlg, L"0");
        return;
    }

    if (IsOperator(text.back()))
    {
        text.pop_back();
    }
    else
    {
        while (!text.empty() && !IsOperator(text.back()))
            text.pop_back();
    }

    if (text.empty())
        text = L"0";

    SetEditText(hDlg, text);
    g_isBinary = false;
}

void Backspace(HWND hDlg)
{
    std::wstring text = GetEditText(hDlg);

    if (text.empty() || text == L"0")
    {
        SetEditText(hDlg, L"0");
        return;
    }

    text.pop_back();

    if (text.empty())
        text = L"0";

    SetEditText(hDlg, text);
    g_isBinary = false;
}

void ConvertBinary(HWND hDlg)
{
    if (!g_isBinary)
    {
        std::wstring text = GetEditText(hDlg);
        double value = 0;

        if (!EvaluateExpression(text, value))
        {
            SetEditText(hDlg, L"Error");
            return;
        }

        g_decimalBeforeBinary = FormatNumber(value);

        long long intValue = static_cast<long long>(value);
        SetEditText(hDlg, ToBinary(intValue));
        g_isBinary = true;
    }
    else
    {
        SetEditText(hDlg, g_decimalBeforeBinary);
        g_isBinary = false;
    }
}

void HalfValue(HWND hDlg)
{
    double value = 0;

    if (EvaluateExpression(GetEditText(hDlg), value))
    {
        value /= 2.0;
        SetEditText(hDlg, FormatNumber(value));
    }
    else
    {
        SetEditText(hDlg, L"Error");
    }

    g_isBinary = false;
}

void Multiply10(HWND hDlg)
{
    double value = 0;

    if (EvaluateExpression(GetEditText(hDlg), value))
    {
        value *= 10.0;
        SetEditText(hDlg, FormatNumber(value));
    }
    else
    {
        SetEditText(hDlg, L"Error");
    }

    g_isBinary = false;
}

void SquareValue(HWND hDlg)
{
    double value = 0;

    if (EvaluateExpression(GetEditText(hDlg), value))
    {
        value = value * value;
        SetEditText(hDlg, FormatNumber(value));
    }
    else
    {
        SetEditText(hDlg, L"Error");
    }

    g_isBinary = false;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    g_hInst = hInstance;

    g_hDlg = CreateDialogW(
        hInstance,
        MAKEINTRESOURCEW(IDD_CALC),
        NULL,
        CalcProc
    );

    ShowWindow(g_hDlg, SW_SHOW);
    UpdateWindow(g_hDlg);

    MSG msg;

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        if (!IsDialogMessageW(g_hDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return 0;
}

INT_PTR CALLBACK CalcProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetEditText(hDlg, L"0");
        return TRUE;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        if (id >= IDC_BTN_0 && id <= IDC_BTN_9)
        {
            AppendDigit(hDlg, id - IDC_BTN_0);
            return TRUE;
        }

        switch (id)
        {
        case IDC_BTN_PLUS:
            AppendOperator(hDlg, L'+');
            break;

        case IDC_BTN_MINUS:
            AppendOperator(hDlg, L'-');
            break;

        case IDC_BTN_MUL:
            AppendOperator(hDlg, L'*');
            break;

        case IDC_BTN_DIV:
            AppendOperator(hDlg, L'/');
            break;

        case IDC_BTN_EQUAL:
            CalculateEqual(hDlg);
            break;

        case IDC_BTN_R:
            ReverseNumbers(hDlg);
            break;

        case IDC_BTN_CE:
            ClearEntry(hDlg);
            break;

        case IDC_BTN_C:
            SetEditText(hDlg, L"0");
            g_isBinary = false;
            break;

        case IDC_BTN_BIN:
            ConvertBinary(hDlg);
            break;

        case IDC_BTN_HALF:
            HalfValue(hDlg);
            break;

        case IDC_BTN_MUL10:
            Multiply10(hDlg);
            break;

        case IDC_BTN_BACK:
            Backspace(hDlg);
            break;

        case IDC_BTN_SQUARE:
            SquareValue(hDlg);
            break;

        case IDC_BTN_EXIT:
            DestroyWindow(hDlg);
            PostQuitMessage(0);
            break;
        }

        return TRUE;
    }

    case WM_CLOSE:
        DestroyWindow(hDlg);
        PostQuitMessage(0);
        return TRUE;
    }

    return FALSE;
}