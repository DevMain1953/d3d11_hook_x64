#ifndef HEADER_H
#define HEADER_H

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "detours.h"
#pragma comment(lib, "detours.X64/detours.lib")

typedef void(WINAPI* D3D11DrawHook) (ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation);
typedef void(WINAPI* D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(WINAPI* D3D11DrawIndexedInstancedHook) (ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

typedef HRESULT(WINAPI* D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

D3D11DrawHook oD3D11Draw = nullptr;
D3D11DrawIndexedHook oD3D11DrawIndexed = nullptr;
D3D11DrawIndexedInstancedHook oD3D11DrawIndexedInstanced = nullptr;

D3D11PresentHook oD3D11Present = nullptr;

//DWORD_PTR is alias for ULONG_PTR, we use LONG because target game and DLL are x64
DWORD_PTR* pContextVTable = NULL;
DWORD_PTR* pSwapChainVtable = NULL;

//Intializes pointers to device, context and swapChain
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
IDXGISwapChain* pSwapChain = NULL;

ID3D11Device* pDeviceInGame = NULL;

//The rasterizer - state interface holds a description for rasterizer state that you can bind to the rasterizer stage
ID3D11RasterizerState* rasterizerStateNormal = NULL; //Normal state
ID3D11RasterizerState* rasterizerStateModified = NULL; //Modified state

//Describes rasterizer state
D3D11_RASTERIZER_DESC nrasterizer_desc;

bool isEnabled = false;

bool firstTime = true;

#endif