
/*
*  This source file is part of the Far Cry 5 ScriptHook by Force67
*  More information regarding licensing can be found in LICENSE.md
*/

#include <Hooking.h>
#include <Nomad/nomad_base_function.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <imgui_impl_dx11.h>
#include <imgui.h>
#include <Menu.h>

struct D3D_Class
{
    char padding[48];
    IDXGIFactory *factory;
};

static IDXGISwapChain **g_swapchain;

static ID3D11Device* d3dDevice = nullptr;
static ID3D11DeviceContext* d3dContext = nullptr;
static ID3D11RenderTargetView*  DX11RenderTargetView = nullptr;

static bool Initialize(IDXGISwapChain* _SwapChain)
{
    _SwapChain->GetDevice(__uuidof(d3dDevice), reinterpret_cast<void**>(&d3dDevice));
    d3dDevice->GetImmediateContext(&d3dContext);

    // and create a texture
    DXGI_SWAP_CHAIN_DESC sd;
    _SwapChain->GetDesc(&sd);

    // Create the render target
    ID3D11Texture2D* pBackBuffer;
    D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
    ZeroMemory(&render_target_view_desc, sizeof(render_target_view_desc));
    render_target_view_desc.Format = sd.BufferDesc.Format;
    render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    _SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    d3dDevice->CreateRenderTargetView(pBackBuffer, &render_target_view_desc, &DX11RenderTargetView);
    d3dContext->OMSetRenderTargets(1, &DX11RenderTargetView, nullptr);
    pBackBuffer->Release();

    return ImGui_ImplDX11_Init(sd.OutputWindow, d3dDevice, d3dContext);
}

static HRESULT(*D3D11Present_Wrap_Orig)(int64_t*, int64_t*, int64_t*);

static HRESULT D3D11Present_Wrap(int64_t* Device3D, int64_t* a2, int64_t* a3)
{
    ImGui_ImplDX11_NewFrame();

	if (g_MenuActive)
		g_Menu->Draw();

    // process render tasks
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return D3D11Present_Wrap_Orig(Device3D, a2, a3);
}

HRESULT (STDMETHODCALLTYPE *CreateSwapChain_Org)(IDXGIFactory *self,
    _In_  IUnknown *pDevice,
    _In_  DXGI_SWAP_CHAIN_DESC *pDesc,
    _COM_Outptr_  IDXGISwapChain **ppSwapChain);

HRESULT CreateSwapChain_Hook(IDXGIFactory *factory, ID3D11Device *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
{
    auto result = CreateSwapChain_Org(factory, pDevice, pDesc, ppSwapChain);

    if (FAILED(result))
    {
        MessageBoxW(nullptr, L"Not enough vram to run the game!", FXNAME_WIDE, MB_ICONERROR);
        exit(1);
    }

    // store this for later use
    g_swapchain = ppSwapChain;

    if (!Initialize(*ppSwapChain))
    {
        MessageBoxW(nullptr, L"Unable to create CryHook UI", FXNAME_WIDE, MB_ICONWARNING);
    }

    return result;
}

static IDXGIFactory *GetFactory(D3D_Class *dat_class)
{
    auto vtable = (DWORD_PTR*)((DWORD_PTR*)dat_class->factory)[0];

    // hook the swap chain creation in a bit of a ugly way
    DWORD op;
    VirtualProtect(&vtable[10], 8, PAGE_EXECUTE_READWRITE, &op);
    CreateSwapChain_Org = (decltype(CreateSwapChain_Org))(vtable)[10];
    ((uintptr_t**)vtable)[10] = (uintptr_t*)&CreateSwapChain_Hook;

    return dat_class->factory;
}

static nomad::base_function init([]()
{
	//fc5: "\mtl-zeta-whitefish\\Code\\platforms\\d3d11\\GraphicsToolbox\\Device3D\\SwapChainNative.cpp"
	//new dawn: "O:\mtl-bowmore-bighorn\Code\platforms\d3d11\GraphicsToolbox\Device3DNative.cpp"

    // verified
    char *loc = nio::pattern("4D 89 C4 49 89 D5 E8 ? ? ? ? 49 89 C7").first(6);
    nio::put_call(loc, GetFactory);

	// verified
    loc = nio::pattern("74 19 41 8B 57 04").first(9);
    nio::set_call(&D3D11Present_Wrap_Orig, loc);
    nio::put_call(loc, D3D11Present_Wrap);
});