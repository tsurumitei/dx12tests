#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <Windows.h>
#include <wrl.h>
#include <comdef.h>
#include <d3d12.h>
#include <string>
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp {
protected:
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();
public:
	static D3DApp* GetApp();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual bool Initialize();
	int Run();

protected:
	bool InitMainWindow();

// Parameters
protected:
	static D3DApp* mApp;
	HINSTANCE mhAppInstance = nullptr;  // 实例由 D3DApp 或继承类的构造函数提供
	HWND mhMainWnd = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
protected:
	int mClientWidth = 800;
	int mClientHeight = 600;
	std::wstring mMainWndCaption = L"d3d App";
};