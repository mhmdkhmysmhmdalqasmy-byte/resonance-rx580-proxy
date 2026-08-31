#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdio>
#include <mutex>

#pragma comment(lib, "dxgi.lib")

using PFN_D3D12CreateDevice =
    HRESULT(WINAPI*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);

static HMODULE g_realD3D12 = nullptr;
static PFN_D3D12CreateDevice g_realCreateDevice = nullptr;
static std::once_flag g_init;

static void Log(const char* fmt, ...)
{
    char path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, path, MAX_PATH);

    char* slash = strrchr(path, '\\');
    if (slash)
        *slash = '\0';

    char logPath[MAX_PATH]{};
    wsprintfA(logPath, "%s\\Resonance_RX580_CustomFix.log", path);

    FILE* f = nullptr;
    fopen_s(&f, logPath, "a");
    if (!f)
        return;

    SYSTEMTIME st{};
    GetLocalTime(&st);

    fprintf(
        f,
        "[%02u:%02u:%02u] ",
        st.wHour,
        st.wMinute,
        st.wSecond
    );

    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);

    fprintf(f, "\n");
    fclose(f);
}

static void InitRealD3D12()
{
    char systemDir[MAX_PATH]{};
    GetSystemDirectoryA(systemDir, MAX_PATH);

    char path[MAX_PATH]{};
    wsprintfA(
        path,
        "%s\\d3d12.dll",
        systemDir
    );

    g_realD3D12 = LoadLibraryA(path);

    if (!g_realD3D12)
    {
        Log("ERROR: cannot load system d3d12.dll");
        return;
    }

    g_realCreateDevice =
        reinterpret_cast<PFN_D3D12CreateDevice>(
            GetProcAddress(
                g_realD3D12,
                "D3D12CreateDevice"
            )
        );

    if (!g_realCreateDevice)
    {
        Log("ERROR: cannot find D3D12CreateDevice");
        return;
    }

    Log("System d3d12.dll loaded: %s", path);
}

static bool EnsureLoaded()
{
    std::call_once(g_init, InitRealD3D12);
    return g_realCreateDevice != nullptr;
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12CreateDevice(
    IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    REFIID riid,
    void** ppDevice)
{
    if (!EnsureLoaded())
        return E_FAIL;

    Log(
        "D3D12CreateDevice requested FeatureLevel=0x%X",
        static_cast<unsigned>(MinimumFeatureLevel)
    );

    // RX 580 actually supports 12_0.
    // Let the real driver create the device at 12_0.
    D3D_FEATURE_LEVEL actualLevel = MinimumFeatureLevel;

    if (actualLevel > D3D_FEATURE_LEVEL_12_0)
        actualLevel = D3D_FEATURE_LEVEL_12_0;

    HRESULT hr = g_realCreateDevice(
        pAdapter,
        actualLevel,
        riid,
        ppDevice
    );

    Log(
        "D3D12CreateDevice actual=0x%X HRESULT=0x%08X",
        static_cast<unsigned>(actualLevel),
        static_cast<unsigned>(hr)
    );

    if (SUCCEEDED(hr))
        Log("Device creation SUCCESS");

    return hr;
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12GetDebugInterface(
    REFIID riid,
    void** ppvDebug)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(REFIID, void**);

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12GetDebugInterface"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(riid, ppvDebug);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12GetInterface(
    REFCLSID rclsid,
    REFIID riid,
    void** ppv)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(REFCLSID, REFIID, void**);

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12GetInterface"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(rclsid, riid, ppv);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12CreateRootSignatureDeserializer(
    LPCVOID pSrcData,
    SIZE_T SrcDataSizeInBytes,
    REFIID pRootSignatureDeserializerInterface,
    void** ppvDeserializer)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(
        LPCVOID,
        SIZE_T,
        REFIID,
        void**
    );

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12CreateRootSignatureDeserializer"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(
        pSrcData,
        SrcDataSizeInBytes,
        pRootSignatureDeserializerInterface,
        ppvDeserializer
    );
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12CreateVersionedRootSignatureDeserializer(
    LPCVOID pSrcData,
    SIZE_T SrcDataSizeInBytes,
    REFIID pVersionedRootSignatureDeserializerInterface,
    void** ppvDeserializer)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(
        LPCVOID,
        SIZE_T,
        REFIID,
        void**
    );

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12CreateVersionedRootSignatureDeserializer"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(
        pSrcData,
        SrcDataSizeInBytes,
        pVersionedRootSignatureDeserializerInterface,
        ppvDeserializer
    );
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12SerializeRootSignature(
    const D3D12_ROOT_SIGNATURE_DESC* pRootSignature,
    D3D_ROOT_SIGNATURE_VERSION Version,
    ID3DBlob** ppBlob,
    ID3DBlob** ppErrorBlob)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(
        const D3D12_ROOT_SIGNATURE_DESC*,
        D3D_ROOT_SIGNATURE_VERSION,
        ID3DBlob**,
        ID3DBlob**
    );

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12SerializeRootSignature"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(
        pRootSignature,
        Version,
        ppBlob,
        ppErrorBlob
    );
}

extern "C" __declspec(dllexport)
HRESULT WINAPI D3D12SerializeVersionedRootSignature(
    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature,
    ID3DBlob** ppBlob,
    ID3DBlob** ppErrorBlob)
{
    if (!EnsureLoaded())
        return E_FAIL;

    using Fn = HRESULT(WINAPI*)(
        const D3D12_VERSIONED_ROOT_SIGNATURE_DESC*,
        ID3DBlob**,
        ID3DBlob**
    );

    auto fn =
        reinterpret_cast<Fn>(
            GetProcAddress(
                g_realD3D12,
                "D3D12SerializeVersionedRootSignature"
            )
        );

    if (!fn)
        return E_NOINTERFACE;

    return fn(
        pRootSignature,
        ppBlob,
        ppErrorBlob
    );
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        Log("=== Custom RX580 Proxy loaded ===");
    }

    return TRUE;
}
