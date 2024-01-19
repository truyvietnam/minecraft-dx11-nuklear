#include "hook.h"
#include "../utils.h"
#include <nuklear_d3d11.h>

typedef HRESULT(__thiscall* present_t)(IDXGISwapChain*, UINT, UINT);
present_t original_present;

typedef void(__thiscall* Mouse)(__int64 a1, char mouseButton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8);
Mouse _Mouse;

// store the game's new D3D11 device here
ID3D11Device* device;
ID3D11RenderTargetView* mainRenderTargetView = nullptr;
ID3D11DeviceContext* ppContext = nullptr;
ID3D11Texture2D* pBackBuffer = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;

struct nk_context* ctx;
struct nk_colorf bg;

void mouseClickCallback(__int64 a1, char mouseButton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8) {
    static int x;
    static int y;
    if (x != mouseX || y != mouseY)
    {
        x = mouseX;
        y = mouseY;
        nk_input_motion(ctx, x, y);
    }
	switch (mouseButton) {
		case 1: //left click
            if (isDown)
            {
                nk_input_begin(ctx);
                nk_input_button(ctx, NK_BUTTON_LEFT, mouseX, mouseY, 1);
            }
            else
            {
                nk_input_begin(ctx);
                nk_input_button(ctx, NK_BUTTON_LEFT, mouseX, mouseY, 0);
                nk_input_end(ctx);
            }
            printf("mouse click\n");
			break;
		case 2: //right click
			
			break;
		case 3: // middle click
			
			break;
		case 4: //scroll
			
			break;
		default:
			break;
		}
	_Mouse(a1, mouseButton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
    
}

HRESULT present_callback(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags) {
    static auto window = (HWND)FindWindowA(nullptr, (LPCSTR)"Minecraft");
    static bool once = false;
    static bool initNk = false;
    static float volume = 20;

    if (!once) {
        // the game will fall back to D3D11 if the D3D12 device is dropped
        // useful for D2D but is kind of unstable
        ID3D12Device* bad_device;
        if (SUCCEEDED(swap_chain->GetDevice(IID_PPV_ARGS(&bad_device))))
        {
            dynamic_cast<ID3D12Device5*>(bad_device)->RemoveDevice();
            return original_present(swap_chain, sync_interval, flags);
        }
        
        
        once = true;
    }
    
    // wait until we can get a D3D11 device
    if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&device))))
        return original_present(swap_chain, sync_interval, flags);
    
    // the game is now using D3D11

    if (!initNk){
        ctx = nk_d3d11_init(device, 1920, 1000, MAX_VERTEX_BUFFER, MAX_INDEX_BUFFER);

		nk_font_atlas* pNkAtlas;
		nk_d3d11_font_stash_begin(&pNkAtlas);
		nk_d3d11_font_stash_end();

        initNk = true;
    }

    device->GetImmediateContext(&ppContext);
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);

    ppContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);

    if (device)
    {    
        if (ppContext)
        {
            if (pBackBuffer)
    {
        pBackBuffer->Release();

        if (nk_begin(ctx, "Show", nk_rect(100, 100, 220, 220),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {

        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button")) { //button
            printf("button\n");
            }
        nk_layout_row_begin(ctx, NK_STATIC, 30, 2); // slider
        {
            nk_layout_row_push(ctx, 50);
            nk_label(ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 110);
            nk_slider_float(ctx, 0, &volume, 100.0f, 1.0f);
        }
        nk_layout_row_end(ctx);

        nk_layout_row_dynamic(ctx, 20, 1); //color picker
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        nk_input_begin(ctx); //call in back buffer is bad

        nk_d3d11_render(ppContext, NK_ANTI_ALIASING_OFF);

        if (mainRenderTargetView) mainRenderTargetView->Release();
        ppContext->Release();
    }
    }
    }
    return original_present(swap_chain, sync_interval, flags);
}

void install_hook() {
    auto sig = Utils::findSig("48 8b c4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 ec ? 44 0f b7 bc 24");
        if (sig) {
            if (MH_CreateHook((void*)sig, &mouseClickCallback, reinterpret_cast<LPVOID*>(&_Mouse)) == MH_OK)
            {
                MH_EnableHook((void*)sig);
                printf("work2\n");
            }
        }

    // the game prefers using D3D12 over D3D11, so we'll try to hook in that same order
    if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
        // Present and ResizeBuffers live at indexes 140 and 145 respectively
        kiero::bind(140, (void**)&original_present, present_callback);
        printf("Hooked D3D12.\n");
        return;
    }
    
    if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
        // indexes are 8 and 13 for D3D11 instead
        kiero::bind(8, (void**)&original_present, present_callback);
        printf("Hooked D3D11.\n");
        return;
    }
    
    // something weird happened
    printf("Failed to hook.\n");
}