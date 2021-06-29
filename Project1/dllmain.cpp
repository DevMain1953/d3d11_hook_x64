#include "header.h"

//Hook function that will be executed instead of original Present
HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		firstTime = false; //Only once

		//Gets device in the current game so we can use it to make our changes in the current game 
		if (pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDeviceInGame) == S_OK)
		{
			pSwapChain->GetDevice(__uuidof(pDeviceInGame), (void**)&pDeviceInGame);
		}

		//Describes rasterizer state
		ZeroMemory(&nrasterizer_desc, sizeof(nrasterizer_desc));
		nrasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		nrasterizer_desc.CullMode = D3D11_CULL_NONE;
		nrasterizer_desc.FrontCounterClockwise = false;
		nrasterizer_desc.DepthBias = 0.0f;
		nrasterizer_desc.SlopeScaledDepthBias = 0.0f;
		nrasterizer_desc.DepthBiasClamp = 0.0f;
		nrasterizer_desc.DepthClipEnable = true;
		nrasterizer_desc.ScissorEnable = false;
		nrasterizer_desc.MultisampleEnable = false;
		nrasterizer_desc.AntialiasedLineEnable = false;

		//Creates a rasterizer state object that tells the rasterizer stage how to behave
		pDeviceInGame->CreateRasterizerState(&nrasterizer_desc, &rasterizerStateModified);
		
		//Describes rasterizer state
		ZeroMemory(&nrasterizer_desc, sizeof(nrasterizer_desc));
		nrasterizer_desc.FillMode = D3D11_FILL_SOLID;
		nrasterizer_desc.CullMode = D3D11_CULL_NONE;
		nrasterizer_desc.FrontCounterClockwise = false;
		nrasterizer_desc.DepthBias = 0.0f;
		nrasterizer_desc.SlopeScaledDepthBias = 0.0f;
		nrasterizer_desc.DepthBiasClamp = 0.0f;
		nrasterizer_desc.DepthClipEnable = true;
		nrasterizer_desc.ScissorEnable = false;
		nrasterizer_desc.MultisampleEnable = false;
		nrasterizer_desc.AntialiasedLineEnable = false;

		//Creates a rasterizer state object that tells the rasterizer stage how to behave
		pDeviceInGame->CreateRasterizerState(&nrasterizer_desc, &rasterizerStateNormal);
	}

	return oD3D11Present(pSwapChain, SyncInterval, Flags);
}

//Hook function that will be executed instead of original DrawIndexedInstanced
void __stdcall hookD3D11DrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	return oD3D11DrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

//Hook function that will be executed instead of original DrawIndexed
void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	if (isEnabled)
	{
		if (IndexCount > 10)
		{
			//Sets the rasterizer state for the rasterizer stage of the pipeline
			pContext->RSSetState(rasterizerStateModified);
		}
	}
	else
	{
		pContext->RSSetState(rasterizerStateNormal);
	}

	return oD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

//Hook function that will be executed instead of original Draw
void __stdcall hookD3D11Draw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	return oD3D11Draw(pContext, VertexCount, StartVertexLocation);
}

//Initializes hooks
DWORD __stdcall InitHooks(LPVOID)
{
	//Creates window
	HWND hWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 0, 0, 0, 0, 0, 0);

	//Describes the set of features targeted by a Direct3D device
	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;

	//Describes a swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	//Creates device, swapChain and context
	if ((D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)) != S_OK)
	{
		MessageBox(hWnd, L"Failed to create directX device and swapchain!", L"Error", MB_ICONERROR);
		return 0;
	}

	//Gets start of virtual table from context
	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];

	//Gets start of virtual table from swapChain
	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	//Gets addresses of original functions from virtual table from context
	oD3D11Draw = (D3D11DrawHook)(DWORD_PTR*)pContextVTable[13];
	oD3D11DrawIndexed = (D3D11DrawIndexedHook)(DWORD_PTR*)pContextVTable[12];
	oD3D11DrawIndexedInstanced = (D3D11DrawIndexedInstancedHook)(DWORD_PTR*)pContextVTable[20];

	//Gets addresses of original functions from virtual table from swapChain
	oD3D11Present = (D3D11PresentHook)(DWORD_PTR*)pSwapChainVtable[8];

	DetourTransactionBegin(); //Begins a new transaction for attaching or detaching detours
	DetourUpdateThread(GetCurrentThread()); //Enlists a thread for update in the current transaction
	
	//Attaches a detour to a target function
	//param1 - Pointer to the target pointer to which the detour will be attached
	//param2 - Pointer to the detour function
	DetourAttach(&(LPVOID&)oD3D11Draw, (PBYTE)hookD3D11Draw);
	DetourAttach(&(LPVOID&)oD3D11DrawIndexed, (PBYTE)hookD3D11DrawIndexed);
	DetourAttach(&(LPVOID&)oD3D11DrawIndexedInstanced, (PBYTE)hookD3D11DrawIndexedInstanced);
	DetourAttach(&(LPVOID&)oD3D11Present, (PBYTE)hookD3D11Present);

	DetourTransactionCommit(); //Commits the current transaction

	//Releases device, context and swapChain
	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	//Closes window
	CloseHandle(hWnd);

	return 0;
}

DWORD WINAPI KeyboardHook(void* lParam)
{
	while (1)
	{
		if (GetAsyncKeyState(VK_NUMPAD1))
		{
			isEnabled = !isEnabled;
		}
		Sleep(100);
	}
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, &InitHooks, 0, 0, 0);
		CreateThread(0, 0, &KeyboardHook, 0, 0, 0);
		break;
	}
	return 1;
}