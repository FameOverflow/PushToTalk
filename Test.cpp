#include <Windows.h>
#include <MMDeviceAPI.h>
#include <Endpointvolume.h>
#include <thread>
#include <cstdio>
#include <mmsystem.h>

// #pragma comment(lib, "lole32.lib")
// #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

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

NOTIFYICONDATA nid;
#define BUTTON_ID 1
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
            break;

        case WM_XBUTTONUP:
            // 鼠标侧键被释放，静音
            SetMicrophoneMute(true);
            break;
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

int windowWidth = 350;  // 窗口的宽度
int windowHeight = 150; // 窗口的高度

int windowX = (screenWidth - windowWidth) / 2;
int windowY = (screenHeight - windowHeight) / 2;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    hHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
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

    HWND hwnd = CreateWindow(
        CLASS_NAME,          // 窗口类名
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