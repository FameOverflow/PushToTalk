// PushToTalk.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "PushToTalk.h"
#include "Windows.h"
#include "MMDeviceAPI.h"
#include "endpointvolume.h"
#include "shellapi.h"
#include <stdio.h>
#include <stdarg.h>

using namespace std;
#define MAX_LOADSTRING 100


#define IS_USE_OUTPUT_DEBUG_PRINT   1

#if  IS_USE_OUTPUT_DEBUG_PRINT 

#define  OUTPUT_DEBUG_PRINTF(str)  OutputDebugPrintf(str)
void OutputDebugPrintf(const char* strOutputString, ...)
{
#define PUT_PUT_DEBUG_BUF_LEN   1024
    char strBuffer[PUT_PUT_DEBUG_BUF_LEN] = { 0 };
    va_list vlArgs;
    va_start(vlArgs, strOutputString);
    _vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);  //_vsnprintf_s  _vsnprintf
    //vsprintf(strBuffer,strOutputString,vlArgs);
    va_end(vlArgs);
    OutputDebugStringA(strBuffer);  //OutputDebugString    // OutputDebugStringW

}
#else 
#define  OUTPUT_DEBUG_PRINTF(str) 
#endif

NOTIFYICONDATA nid;           // 通知区域图标的数据结构
// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
#define ID_BUTTON_CHANGEKEY 130 // 更改绑定的按键按钮的标识符
#define ID_EDIT_KEY 131         // 更改绑定的按键编辑框的标识符
#define ID_BUTTON_CONFIRM 133   // 确认更改按钮的标识符

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int windowWidth = 350;  // 窗口的宽度
int windowHeight = 150; // 窗口的高度
int windowX = (screenWidth - windowWidth) / 2;
int windowY = (screenHeight - windowHeight) / 2;

WCHAR key[100] = L"XBUTTON1"; // 绑定的按键
bool g_isEditFocused = false; // 用于保存输入框是否有焦点的标志
HWND hWnd;
HWND hwndEdit;
HWND hwndChangeButton;
HWND hwndConfirmButton;
HHOOK hHook;
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void SetMicrophoneMute(bool mute)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        // 处理初始化失败的情况
        return;
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr))
    {
        // 处理创建设备枚举器失败的情况
        CoUninitialize();
        return;
    }

    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr))
    {
        // 处理获取默认音频设备失败的情况
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    IAudioEndpointVolume* pEndpointVolume = NULL;
    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    if (FAILED(hr))
    {
        // 处理获取音频端点音量控制失败的情况
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pEndpointVolume->SetMute(mute, NULL);
    if (FAILED(hr))
    {
        // 处理设置静音状态失败的情况
    }

    // 释放资源
    pEndpointVolume->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();
};
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        switch (wParam)
        {
        case WM_XBUTTONDOWN:
            // 鼠标侧键被按下，取消静音
            SetMicrophoneMute(false);
            OutputDebugPrintf("g_isEditFocused: %d\n", g_isEditFocused);
            OutputDebugPrintf("mouseData: %d\n", ((MOUSEHOOKSTRUCTEX*)lParam)->mouseData);
            OutputDebugPrintf("KEYSTATE_WPARAM: %d\n", GET_KEYSTATE_WPARAM(wParam));
            OutputDebugPrintf("XBUTTON_WPARAM: %d\n",GET_XBUTTON_WPARAM(wParam));

            SetWindowText(hwndEdit, L"鼠标侧键1");
            if (g_isEditFocused)
            {
                if (((MOUSEHOOKSTRUCTEX*)lParam)->mouseData == XBUTTON1)
                {
                    // 鼠标侧键1被按下，填充到输入框中
                    SetWindowText(hwndEdit, L"鼠标侧键1");
                }
                else if (((MOUSEHOOKSTRUCTEX*)lParam)->mouseData == XBUTTON2)
                {
                    // 鼠标侧键2被按下，填充到输入框中
                    SetWindowText(hwndEdit, L"鼠标侧键2");
                }
            }
            break;

        case WM_XBUTTONUP:
            // 鼠标侧键被释放，静音
            SetMicrophoneMute(true);
            break;
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        switch (wParam)
        {
        case WM_KEYDOWN:
            // 检查是否是用户输入的按键被按下
            if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == static_cast<DWORD>(VkKeyScan(*key)))
            {
                // 取消静音
                SetMicrophoneMute(false);
            }
            if (g_isEditFocused)
            {
                wchar_t keyText[2];
                keyText[0] = (wchar_t)wParam;
                keyText[1] = '\0';
                SetWindowText(hwndEdit, keyText);
                break;
            };
            break;
        case WM_KEYUP:
            // 检查是否是用户输入的按键被释放
            if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == static_cast<DWORD>(VkKeyScan(*key)))
            {
                // 静音
                SetMicrophoneMute(true);
            }
            break;
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PUSHTOTALK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    };
    if ((wcscmp(key, L"XBUTTON1") == 0) || (wcscmp(key, L"XBUTTON2") == 0))
    {
        hHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
    }
    else
    {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    };
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PUSHTOTALK));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    SetMicrophoneMute(false);
    UnhookWindowsHookEx(hHook);
    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PUSHTOTALK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
   // 在窗口创建时创建改绑按钮
   hwndChangeButton = CreateWindow( 
       L"BUTTON",                                              // 预定义的按钮类名
       L"改绑",                                                // 按钮的文本
       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // 按钮的样式
       10,                                                    // 按钮的初始x坐标
       10,                                                    // 按钮的初始y坐标
       100,                                                   // 按钮的宽度
       30,                                                    // 按钮的高度
       hWnd,                                                  // 父窗口的句柄
       (HMENU)ID_BUTTON_CHANGEKEY,                            // 按钮的标识符
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),     // 应用程序实例的句柄
       NULL                                                   // 创建参数
   );
   // 在窗口创建时创建隐藏的编辑框和确认按钮
   hwndEdit = CreateWindow(
       L"EDIT",
       L"",
       WS_CHILD | WS_BORDER | ES_AUTOHSCROLL| WS_VISIBLE, 
       10, 50, 100, 25,
       hWnd,
       (HMENU)ID_EDIT_KEY,
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);

   hwndConfirmButton = CreateWindow(
       L"BUTTON",
       L"确认",
       WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON| WS_VISIBLE, 
       120, 50, 50, 25,
       hWnd,
       (HMENU)ID_BUTTON_CONFIRM,
       (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
       NULL);
   ShowWindow(hwndEdit, SW_HIDE);
   ShowWindow(hwndConfirmButton, SW_HIDE);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        lstrcpy(nid.szTip, L"PushToTalk");
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_BUTTON_CHANGEKEY:
                g_isEditFocused = true;
				ShowWindow(hwndEdit, SW_SHOW);
				ShowWindow(hwndConfirmButton, SW_SHOW);
				break;  
            case ID_BUTTON_CONFIRM:
                GetWindowText(hwndEdit, key, 99);
                MessageBox(hWnd, key, L"按键", MB_OK);
                ShowWindow(hwndEdit, SW_HIDE);
                ShowWindow(hwndConfirmButton, SW_HIDE);
                g_isEditFocused = false;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hWnd, SW_HIDE);
        }
        break;
    case WM_USER + 1:
        switch (lParam)
        {
        case WM_LBUTTONUP:
            ShowWindow(hWnd, SW_RESTORE);
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            break;
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        Shell_NotifyIcon(NIM_DELETE, &nid);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
