#include <Windows.h>
#include <MMDeviceAPI.h>
#include <Endpointvolume.h>
#include <thread>
#include <cstdio>
#include <mmsystem.h>

// #pragma comment(lib, "lole32.lib")
// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#define ID_BUTTON_CHANGEKEY 1 // 更改绑定的按键按钮的标识符
#define ID_EDIT_KEY 2         // 更改绑定的按键编辑框的标识符
#define ID_BUTTON_CONFIRM 3   // 确认更改按钮的标识符
NOTIFYICONDATA nid;           // 通知区域图标的数据结构
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
int windowWidth = 350;  // 窗口的宽度
int windowHeight = 150; // 窗口的高度
int windowX = (screenWidth - windowWidth) / 2;
int windowY = (screenHeight - windowHeight) / 2;
char key[100] = "XBUTTON1"; // 绑定的按键

HWND hwndEdit;
HWND hwndChangeButton;
HWND hwndConfirmButton;
bool g_isEditFocused = false; // 用于保存输入框是否有焦点的标志
void SetMicrophoneMute(bool mute)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        // 处理初始化失败的情况
        return;
    }

    IMMDeviceEnumerator *pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
    if (FAILED(hr))
    {
        // 处理创建设备枚举器失败的情况
        CoUninitialize();
        return;
    }

    IMMDevice *pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr))
    {
        // 处理获取默认音频设备失败的情况
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    IAudioEndpointVolume *pEndpointVolume = NULL;
    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)&pEndpointVolume);
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
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = (HICON)LoadImage(NULL, "mic.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        lstrcpy(nid.szTip, "PushToTalk");
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hwnd, SW_HIDE);
        }
        break;
    case WM_USER + 1:
        switch (lParam)
        {
        case WM_LBUTTONUP:
            ShowWindow(hwnd, SW_RESTORE);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            break;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_CHANGEKEY)
        {
            ShowWindow(hwndEdit, SW_SHOW);
            ShowWindow(hwndConfirmButton, SW_SHOW);
        }
        else if (LOWORD(wParam) == ID_BUTTON_CONFIRM)
        {
            GetWindowText(hwndEdit, key, sizeof(key));
            MessageBox(hwnd, key, "按键", MB_OK);
            ShowWindow(hwndEdit, SW_HIDE);
            ShowWindow(hwndConfirmButton, SW_HIDE);
        }
        break;
    case WM_SETFOCUS:
        // 输入框获取焦点，设置标志
        if ((HWND)wParam == hwndEdit)
        {
            g_isEditFocused = true;
        }
        break;
    case WM_KILLFOCUS:
        // 输入框失去焦点，清除标志
        if ((HWND)wParam == hwndEdit)
        {
            g_isEditFocused = false;
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HHOOK hHook;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        switch (wParam)
        {
        case WM_XBUTTONDOWN:
            // 鼠标侧键被按下，取消静音
            SetMicrophoneMute(false);
            if (g_isEditFocused)
            {
                if (((MOUSEHOOKSTRUCTEX *)lParam)->mouseData == XBUTTON1)
                {
                    // 鼠标侧键1被按下，填充到输入框中
                    SetWindowText(hwndEdit, "鼠标侧键1");
                }
                else if (((MOUSEHOOKSTRUCTEX *)lParam)->mouseData == XBUTTON2)
                {
                    // 鼠标侧键2被按下，填充到输入框中
                    SetWindowText(hwndEdit, "鼠标侧键2");
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
            if (((KBDLLHOOKSTRUCT *)lParam)->vkCode == static_cast<DWORD>(VkKeyScan(*key)))
            {
                // 取消静音
                SetMicrophoneMute(false);
            }
            break;

        case WM_KEYUP:
            // 检查是否是用户输入的按键被释放
            if (((KBDLLHOOKSTRUCT *)lParam)->vkCode == static_cast<DWORD>(VkKeyScan(*key)))
            {
                // 静音
                SetMicrophoneMute(true);
            }
            break;
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if ((strcmp(key, "XBUTTON1") == 0) || (strcmp(key, "XBUTTON2") == 0))
    {
        hHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
    }
    else
    {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    }
    HWND hwnd = CreateWindow(
        "Window",            // 窗口类名
        "PushToTalk",        // 窗口标题
        WS_OVERLAPPEDWINDOW, // 窗口样式
        windowX,             // 窗口初始X坐标
        windowY,             // 窗口初始Y坐标
        windowWidth,         // 窗口初始宽度
        windowHeight,        // 窗口初始高度
        NULL,                // 父窗口句柄
        NULL,                // 菜单句柄
        hInstance,           // 应用程序实例句柄
        NULL                 // 附加参数
    );
    // 在窗口创建时创建改绑按钮
    HWND hwndChangeButton = CreateWindow(
        "BUTTON",                                              // 预定义的按钮类名
        "更改绑定的按键",                                      // 按钮的文本
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // 按钮的样式
        10,                                                    // 按钮的初始x坐标
        10,                                                    // 按钮的初始y坐标
        100,                                                   // 按钮的宽度
        30,                                                    // 按钮的高度
        hwnd,                                                  // 父窗口的句柄
        (HMENU)ID_BUTTON_CHANGEKEY,                            // 按钮的标识符
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),     // 应用程序实例的句柄
        NULL                                                   // 创建参数
    );
    // 在窗口创建时创建隐藏的编辑框和按钮
    HWND hwndEdit = CreateWindow(
        "EDIT",
        "",
        WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, // 不包含WS_VISIBLE样式
        10, 50, 200, 25,
        hwnd,
        (HMENU)ID_EDIT_KEY,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    HWND hwndConfirmButton = CreateWindow(
        "BUTTON",
        "确认",
        WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON, // 不包含WS_VISIBLE样式
        220, 50, 50, 25,
        hwnd,
        (HMENU)ID_BUTTON_CONFIRM,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    SetMicrophoneMute(false);
    UnhookWindowsHookEx(hHook);
    return 0;
}