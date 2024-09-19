/*
	本文件主要整理实现思路，不作封装，
	所以实现 DX12 时未关注变量作用域
	仅前半部分调用 Win API 的代码可以运行
*/

#include <Windows.h>
#include <wrl.h>
#include <comdef.h>
#include <string>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


HWND mhMainWnd = 0;  //某个窗口的句柄，ShowWindow 和 UpdateWindow 函数均要调用此句柄
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);  //窗口过程函数的声明,HWND 是主窗口句柄

// windows 主函数入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCMD) {
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// 窗口初始化描述结构体
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	//当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = MainWndProc;	    //指定窗口过程
	wc.cbClsExtra = 0;	                //借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	                //借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInstance;         	//应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	            //没有菜单栏
	wc.lpszClassName = L"MainWnd";	    //窗口名
	if (!RegisterClass(&wc)) {
		// 窗口对象注册失败返回：
		// 参数： 消息框所属窗口句柄，文本信息，标题文本，消息框样式
		MessageBox(0, L"RegisterClass failed", 0, 0);
		return 0;
	}  // 注册成功则继续运行

	RECT R;	 //裁剪矩形
	R.left = 0;
	R.top = 0;
	R.right = 1280;
	R.bottom = 720;
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);  //根据窗口的客户区大小计算窗口的大小
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	// 创建窗口并返回 bool 值
	// 参数：标题，样式，左上角坐标 X，左上角坐标 Y，父窗口句柄，菜单 ID，HINSTANCE 实例，任意数据指针（传参）
	mhMainWnd = CreateWindow(L"MainWnd", L"DX12Initialize", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hInstance, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}

	//窗口创建成功,则显示并更新窗口
	ShowWindow(mhMainWnd, nShowCMD);
	UpdateWindow(mhMainWnd);

	// 消息循环
	// 定义消息结构体
	MSG msg = { 0 };
	BOOL bRet = 0;
	// 如果GetMessage函数不等于0，说明没有接受到WM_QUIT
	while (bRet = GetMessage(&msg, 0, 0, 0) != 0) {
		// 如果等于 -1，说明 GetMessage 函数出错了，弹出错误框
		if (bRet == -1) { MessageBox(0, L"GetMessage Failed", L"Error", MB_OK); }
		// 如果等于其他值，说明接收到了消息
		else {
			TranslateMessage(&msg);	//键盘按键转换，将虚拟键消息转换为字符消息
			DispatchMessage(&msg);	//把消息分派给相应的窗口过程
		}
	}
	return (int)msg.wParam;

	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// 消息处理
	switch (msg) {
		// 当窗口被销毁时，终止消息循环
	case WM_DESTROY:
		PostQuitMessage(0);	// 终止消息循环，并发出WM_QUIT消息
		return 0;
	default:
		break;
	}
	// 将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hwnd, msg, wParam, lParam);
}



// 定义 ThrowIfFailed 宏
// D3D12大多数创建函数会返回 HRESULT，ThrowIfFailed 宏可以接收 HRESULT 值，从而判断是否创建成功。
// 这个宏展开后，是个可以抛出异常的函数，并能将对应文件名行号显示出来
// 注意：这里的 ToString 函数用到了 <comdef.h> 头文件

// AnsiToWString 函数（转换成宽字符类型的字符串，wstring）
// 在 Windows 平台上应使用 wstring 和 wchar_t，处理方式是在字符串前 +L
inline std::wstring AnsiToWString(const std::string& str) {
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

// DxException类
class DxException {
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	std::wstring ToString()const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	Filename(filename),
	LineNumber(lineNumber) {  }

std::wstring DxException::ToString() const {
	// Get the string description of the error code.
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

// 宏定义 ThrowIfFailed
#ifndef ThrowIfFailed
#define ThrowIfFailed(x) {                                            \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif


// ---------------------------------- INITIALIZE D3D12 ----------------------------------
// 开启D3D12调试层。
// 创建设备。
// 创建围栏，同步CPU和GPU。
// 获取描述符大小。
// 设置MSAA抗锯齿属性。
// 创建命令队列、命令列表、命令分配器。
// 创建交换链。
// 创建描述符堆。
// 创建描述符。
// 资源转换。
// 设置视口和裁剪矩形。
// 设置围栏刷新命令队列。
// 将命令从列表传至队列。


// 创建设备
void CreateDevice() {
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
	ThrowIfFailed(
		D3D12CreateDevice(
			nullptr,                 // 此参数如果设置为 nullptr，则使用主适配器
			D3D_FEATURE_LEVEL_12_0,  // 应用程序需要硬件所支持的最低功能级别
			IID_PPV_ARGS(&d3dDevice)
		)
	);  // 返回所建设备
}

// 创建围栏
void CreateFence() {
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

// 获取描述符大小，这个大小可以让我们知道描述符堆中每个元素的大小
// （描述符在不同的GPU平台上的大小各异），方便之后在地址中做偏移来找到堆中的描述符元素
// 获取：
// RTV（渲染目标缓冲区描述符）
// DSV（深度模板缓冲区描述符）
// CBV_SRV_UAV（常量缓冲区描述符、着色器资源缓冲描述符和随机访问缓冲描述符）
void GetDescriptorSize() {
	UINT rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	UINT dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	UINT cbv_srv_uavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// 设置 MSAA 抗锯齿属性：
// 填充多重采样属性结构体，
// 通过 CheckFeatureSupport 函数设置 NumQualityLevels
void SetMSAA() {
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// UNORM是归一化处理的无符号整数
	msaaQualityLevels.SampleCount = 1;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msaaQualityLevels.NumQualityLevels = 0;
	// 当前图形驱动对MSAA多重采样的支持（注意：第二个参数即是输入又是输出）
	ThrowIfFailed(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels)));
	// NumQualityLevels 在 Check 函数里会进行设置
	// 如果支持 MSAA，则 Check 函数返回的 NumQualityLevels > 0
	// expression 为假（即为 0），则终止程序运行，并打印一条出错信息
	assert(msaaQualityLevels.NumQualityLevels > 0);
}

// 创建命令队列、命令列表和命令分配器
// CPU 创建命令列表，
// 将关联在命令分配器上的命令传入命令列表，
// 将命令列表传入命令队列给 GPU 处理
void CreateCommandObject() {
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue;
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&cmdQueue)));
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator)));//&cmdAllocator等价于cmdAllocator.GetAddressOf
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;	// 注意此处的接口名是 ID3D12GraphicsCommandList，而不是 ID3D12CommandList
	ThrowIfFailed(
		d3dDevice->CreateCommandList(
			0,                              // 掩码值为0，单GPU
			D3D12_COMMAND_LIST_TYPE_DIRECT, // 命令列表类型
			cmdAllocator.Get(),             // 命令分配器接口指针
			nullptr,                        // 流水线状态对象PSO，这里不绘制，所以空指针
			IID_PPV_ARGS(&cmdList)
		)
	);  // 返回创建的命令列表
	cmdList->Close();  // 重置命令列表前必须将其关闭
}

// 创建交换链
// 调用 DXGIFactory
// 直接设置 MSAA 会出错，所以 count 还是为 1，质量为 0
// 注意：CreateSwapChain 函数的第一个参数其实是命令队列接口指针，不是设备接口指针，参数描述有误导。
void CreateSwapChain() {
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	swapChain.Reset();
	DXGI_SWAP_CHAIN_DESC swapChainDesc;                             // 交换链描述结构体
	swapChainDesc.BufferDesc.Width = 1280;                          // 缓冲区分辨率的宽度
	swapChainDesc.BufferDesc.Height = 720;                          // 缓冲区分辨率的高度
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // 缓冲区的显示格式
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;           // 刷新率的分子
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;            // 刷新率的分母
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;  // 逐行扫描VS隔行扫描(未指定的)
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;  // 图像相对屏幕的拉伸（未指定的）
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // 将数据渲染至后台缓冲区（即作为渲染目标）
	swapChainDesc.OutputWindow = mhMainWnd;                         // 渲染窗口句柄
	swapChainDesc.SampleDesc.Count = 1;                             // 多重采样数量
	swapChainDesc.SampleDesc.Quality = 0;                           // 多重采样质量
	swapChainDesc.Windowed = true;                                  // 是否窗口化
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;       // 固定写法
	swapChainDesc.BufferCount = 2;                                  // 后台缓冲区数量（双缓冲）
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;   // 自适应窗口模式（自动选择最适于当前窗口尺寸的显示模式）
	
	// 利用DXGI接口下的工厂类创建交换链
	ThrowIfFailed(dxgiFactory->CreateSwapChain(cmdQueue.Get(), &swapChainDesc, swapChain.GetAddressOf()));
}

// 创建描述符堆
// 由于是双后台缓冲，所以要创建存放 2 个 RTV 堆
// 深度模板缓存只有一个，所以创建 1 个 DSV 堆
// 先填充描述符堆属性结构体
// 然后通过设备创建描述符堆
void CreateDescriptorHeap() {
	// 创建 RTV 堆
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap)));
	// 创建 DSV 堆
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NodeMask = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap)));
}

// 创建 RTV 描述符
// 从 RTV 堆中拿到首个 RTV 句柄
// 获得存于交换链中的 RT 资源
// 创建 RTV 将 RT 资源和 RTV 句柄联系起来，并根据 RTV 大小在堆中作地址偏移
// 注：用到 CD3DX12_CPU_DESCRIPTOR_HANDL，这个变体在 d3dx12.h 头文件中定义
// CD3DX12_ 开头的变体常用与简化代码
void CreateRTV() {
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainBuffer[2];
	for (int i = 0; i < 2; i++) {
		// 获得存于交换链中的后台缓冲区资源
		swapChain->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer[i].GetAddressOf()));
		// 创建RTV
		d3dDevice->CreateRenderTargetView(
			swapChainBuffer[i].Get(),
			nullptr,  // 在交换链创建中已经定义了该资源的数据格式，所以这里指定为空指针
			rtvHeapHandle
		);  // 描述符句柄结构体（这里是变体，继承自 CD3DX12_CPU_DESCRIPTOR_HANDLE）
		// 偏移到描述符堆中的下一个缓冲区
		rtvHeapHandle.Offset(1, rtvDescriptorSize);
	}
}

// 创建 DSV 描述符
// 在 CPU 中创建好 DS 资源
// 通过 CreateCommittedResource 函数将 DS 资源提交至 GPU 显存中
// 创建 DSV 将显存中的 DS 资源和 DSV 句柄联系起来
void CreateDSV() {
	// 在 CPU 中创建好深度模板数据资源
	D3D12_RESOURCE_DESC dsvResourceDesc;
	dsvResourceDesc.Alignment = 0;                          // 指定对齐
	dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 指定资源维度（类型）为 TEXTURE2D
	dsvResourceDesc.DepthOrArraySize = 1;                   // 纹理深度为 1
	dsvResourceDesc.Width = 1280;                           // 资源宽
	dsvResourceDesc.Height = 720;                           // 资源高
	dsvResourceDesc.MipLevels = 1;                          // MIPMAP 层级数量
	dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // 指定纹理布局（这里不指定）
	dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;  //深度模板资源的 Flag
	dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24 位深度，8 位模板,还有个无类型的格式 DXGI_FORMAT_R24G8_TYPELESS 也可以使用
	dsvResourceDesc.SampleDesc.Count = 4;                   // 多重采样数量
	dsvResourceDesc.SampleDesc.Quality = msaaQualityLevels.NumQualityLevels - 1;  // 多重采样质量
	CD3DX12_CLEAR_VALUE optClear;                           // 清除资源的优化值，提高清除操作的执行速度（CreateCommittedResource 函数中传入）
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;        // 24 位深度，8 位模板,还有个无类型的格式 DXGI_FORMAT_R24G8_TYPELESS 也可以使用
	optClear.DepthStencil.Depth = 1;                        // 初始深度值为 1
	optClear.DepthStencil.Stencil = 0;                      // 初始模板值为 0
	// 创建一个资源和一个堆，并将资源提交至堆中（将深度模板数据提交至 GPU 显存中）
	ComPtr<ID3D12Resource> depthStencilBuffer;
	ThrowIfFailed(
		d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),  // 堆类型为默认堆（不能写入）
			D3D12_HEAP_FLAG_NONE,         // Flag
			&dsvResourceDesc,             // 上面定义的 DSV 资源指针
			D3D12_RESOURCE_STATE_COMMON,  // 资源的状态为初始状态
			&optClear,                    // 上面定义的优化值指针
			IID_PPV_ARGS(&depthStencilBuffer)
		)
	);  // 返回深度模板资源
	// 创建DSV(必须填充 DSV 属性结构体，和创建 RTV 不同，RTV 是通过句柄)
	// D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	// dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	// dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// dsvDesc.Texture2D.MipSlice = 0;
	d3dDevice->CreateDepthStencilView(
		depthStencilBuffer.Get(),
		nullptr,  // D3D12_DEPTH_STENCIL_VIEW_DESC 类型指针，可填 &dsvDesc，
		          // 由于在创建 深度模板资源 时已经定义深度模板数据属性，所以这里可以指定为空指针
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);	// DSV句柄
}

// 标记 DS 资源的状态
// 资源在过程中会有不同的状态，只读或可写等
// 将 ResourceBarrier 下的 Transition 传递状态到命令列表以转换资源状态
void StateTransition() {
	cmdList->ResourceBarrier(
		1,  // Barrier屏障个数
		&CD3DX12_RESOURCE_BARRIER::Transition(
			depthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,      // 转换前状态（创建时的状态，即CreateCommittedResource函数中定义的状态）
			D3D12_RESOURCE_STATE_DEPTH_WRITE  // 转换后状态为可写入的深度图，还有一个D3D12_RESOURCE_STATE_DEPTH_READ是只可读的深度图
		)
	);
}

// 等所有命令都进入 cmdList 后，还须用 ExecuteCommandLists 函数，
// 将命令从命令列表传入命令队列，CPU -> GPU
// 注意：在传入命令队列前必须关闭命令列表
void CallExecuteCommandList() {
	ThrowIfFailed(cmdList->Close());                              //命令添加完后将其关闭
	ID3D12CommandList* cmdLists[] = { cmdList.Get() };            //声明并定义命令列表数组
	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);  //将命令从命令列表传至命令队列
}

// 实现 CPU、GPU 同步
// 围栏接口已实现，以下实现围栏代码：
// 围栏初始值为 0（GPU 端的围栏值），定义一个当前围栏值也为 0（CPU 端的围栏值），
// 当 CPU 将命令传递至 GPU 后，当前围栏值++（CPU围栏++），
// 当 GPU 处理完 CPU 传来的命令后，围栏值++（GPU围栏++），
// 然后判定围栏值（CPU 围栏值）和当前围栏值（GPU 围栏值）的大小，来确定 GPU 是否命中围栏点，
// 如果没有命中，则等待命中后触发事件

int mCurrentFence = 0;  // 初始CPU上的围栏点为0

void FlushCmdQueue() {
	mCurrentFence++;  // CPU 传完命令并关闭后，将当前围栏值 +1
	cmdQueue->Signal(fence.Get(), mCurrentFence);     // 当 GPU 处理完 CPU 传入的命令后，将 fence 接口中的围栏值 +1，即 fence->GetCompletedValue()+1
	if (fence->GetCompletedValue() < mCurrentFence) { // 如果小于，说明 GPU 没有处理完所有命令
		HANDLE eventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");  // 创建事件
		fence->SetEventOnCompletion(mCurrentFence, eventHandle);  // 当围栏达到 mCurrentFence 值（即执行到 Signal() 指令修改了围栏值）时触发的 eventHandle 事件
		WaitForSingleObject(eventHandle, INFINITE);  // 等待 GPU 命中围栏，激发事件（阻塞当前线程直到事件触发，注意此 Enent 需先设置再等待，
		                                             // 如果没有 Set 就 Wait，就死锁了，Set 永远不会调用，所以也就没线程可以唤醒这个线程）
		CloseHandle(eventHandle);
	}
}

// 设置视口 & 裁剪矩形
void CreateViewPortAndScissorRect() {
	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;
	// 视口设置
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = 1280;
	viewPort.Height = 720;
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	// 裁剪矩形设置（矩形外的像素都将被剔除）
	// 前两个为左上点坐标，后两个为右下点坐标
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = 1280;
	scissorRect.bottom = 720;
}