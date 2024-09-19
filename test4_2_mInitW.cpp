#include "mInitW.h"
#include <cassert>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}
D3DApp* D3DApp::mApp = nullptr;
D3DApp* D3DApp::GetApp() {
	return mApp;
}
D3DApp::D3DApp(HINSTANCE hInstance) : mhAppInstance(hInstance) {
	assert(mApp == nullptr);  // ��������һ��ʵ��
	mApp = this;  // ���鵱ǰʵ����Ӧ��ָ�� this �Ĳ�����δ��ֵ���� this ���� mApp
}
D3DApp::~D3DApp() {
	// @TODO: ��������豸����ȴ� CommandQueue ִ��������ͷ�
	if (md3dDevice != nullptr) {  }
}
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::Initialize() {
	if (!InitMainWindow()) { return false; }

	return true;
}


bool D3DApp::InitMainWindow() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);  // �� HGDIOBJ ������תΪ��ˢ���
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return false;
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;
	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInstance, 0);
	if (!mhMainWnd) {
		MessageBox(0, L"CreateWindow Failed", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

int D3DApp::Run() {
	MSG msg = { 0 };
	/*while (msg.message != WM_QUIT) {

	}*/
	while (GetMessage(&msg, 0, 0, 0) != 0) {

	}
	return (int)msg.wParam;
}