#include <Windows.h>
#include <MMDeviceAPI.h>
#include <Endpointvolume.h>

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

int main()
{
    while (true)
    {
        if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
        {
            // 鼠标侧键1被按下，取消静音
            SetMicrophoneMute(false);
        }
        else
        {
            // 鼠标侧键1被释放，静音
            SetMicrophoneMute(true);
        }

        Sleep(100); // 每100毫秒检查一次
    }

    return 0;
}