#include <Windows.h>
#include <MMDeviceAPI.h>
#include <EndpointVolume.h>

void SetInputDeviceVolume(bool mute) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        // 处理初始化失败的情况
        return;
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        // 处理创建设备枚举器失败的情况
        CoUninitialize();
        return;
    }

    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr)) {
        // 处理获取默认音频设备失败的情况
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    IAudioEndpointVolume* pVolume = NULL;
    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pVolume);
    if (FAILED(hr)) {
        // 处理激活音量接口失败的情况
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    float volume = mute ? 0.0f : 1.0f;  // 静音时音量为0，否则为1
    hr = pVolume->SetMasterVolumeLevelScalar(volume, NULL);
    if (FAILED(hr)) {
        // 处理设置音量失败的情况
    }

    // 释放资源
    pVolume->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();
}

int main() {
    while (true) {
        if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) {
            // F1键被按下，开启音频设备
            SetInputDeviceVolume(false);
        } else {
            // F1键被释放，静音音频设备
            SetInputDeviceVolume(true);
        }

        Sleep(100);  // 每100毫秒检查一次
    }

    return 0;
}