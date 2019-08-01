// This is the main DLL file.
#pragma unmanaged
#include "D3DVisualization.h"

using namespace DirectX;

PdfRender * pApplication;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct ConstantBuffer
{
	XMMATRIX  WVP;
	float vInversion;
	int fBookmark;
};


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fwdReason, LPVOID lpvReserved)
{
	return TRUE;
}

extern HRESULT __cdecl Init()
{
	pApplication = new PdfRender();

	HRESULT hr = S_OK;

	if (FAILED(hr = pApplication->InitDevice()))
	{
		return hr;
	}

	return hr;
}

extern void __cdecl Cleanup()
{
	delete pApplication;
	pApplication = NULL;
}

extern HRESULT __cdecl Render(void * pResource, bool isNewSurface)
{
	if (NULL == pApplication)
	{
		return E_FAIL;
	}

	return pApplication->Render(pResource, isNewSurface);
}

extern void __cdecl SetLeft(bool aState)
{
	pApplication->SetLeft(aState);
}

extern void __cdecl SetRight(bool aState)
{
	pApplication->SetRight(aState);
}

extern void __cdecl SetDay(bool aState)
{
	pApplication->SetDay(aState);
}

extern void __cdecl SetNight(bool aState)
{
	pApplication->SetNight(aState);
}

extern void __cdecl SetReverse()
{
	pApplication->SetReverse();
}

extern void __cdecl OpenDocument(wchar_t *filename)
{
	pApplication->OpenDocument(filename);
}

extern int __cdecl GetNowPageWidth()
{
	return pApplication->GetNowPageWidth();
}

extern int __cdecl GetNowPageHeight()
{
	return pApplication->GetNowPageHeight();
}

extern void __cdecl SaveImage(int percent, wchar_t *filename)
{
	pApplication->SaveImage(percent, filename);
}

extern bool __cdecl Search(wchar_t *searchstr, int search_page, bool direction, bool case_sensetive)
{
	return pApplication->Search(searchstr, search_page, direction, case_sensetive);
}


extern int __cdecl GetDocSize()
{
	return pApplication->GetDocSize();
}

extern int __cdecl GetNowPage()
{
	return pApplication->GetNowPage();
}

extern void __cdecl ViewBookmark(int page_num)
{
	pApplication->ViewBookmark(page_num);
}

extern void __cdecl AddBookmark()
{
	pApplication->AddBookmark();
}

extern void __cdecl RemoveBookmark()
{
	pApplication->RemoveBookmark();
}


PdfRender::PdfRender() :
	  m_pagesNum(PAGE_NUM)
	, m_left(false)
	, m_right(false)
	, m_day(true)
	, m_night(false)
	, m_Height(0)
	, m_Width(0)
{
	m_driverType = D3D_DRIVER_TYPE_NULL;
	m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	m_pd3dDevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pSwapChain = nullptr;
	m_pRenderTargetView = nullptr;
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pVertexLayout = nullptr;
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pConstantBuffer = nullptr;
	m_pDepthStencil = nullptr;
	m_pDepthStencilView = nullptr;
	m_pSamplerLinear = nullptr;
	m_pTextureWithBookmark[0] = nullptr;
	m_pTextureWithBookmark[1] = nullptr;
	m_pTextureNullBM = nullptr;
	m_pTextureFullBM = nullptr;

	for (int i = 0; i < m_pagesNum; i++)
		m_pTextureRV[i] = nullptr;

	animationInit();

	m_bookmarks.clear();
}

PdfRender::~PdfRender()
{
	if (m_pImmediateContext)
	{
		m_pImmediateContext->ClearState();
	}

	SAFE_RELEASE(m_pd3dDevice);
	SAFE_RELEASE(m_pImmediateContext);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pSamplerLinear);
	SAFE_RELEASE(m_pTextureWithBookmark[0]);
	SAFE_RELEASE(m_pTextureWithBookmark[1]);
	SAFE_RELEASE(m_pTextureNullBM);
	SAFE_RELEASE(m_pTextureFullBM);

	for (int i = 0; i < m_pagesNum; i++)
		SAFE_RELEASE(m_pTextureRV[i]);
}

HRESULT PdfRender::InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// DX10 or 11 devices are suitable
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		if (SUCCEEDED(hr))
		{
			m_driverType = driverTypes[driverTypeIndex];
			break;
		}
	}

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Could not create a Direct3D 10 or 11 device.", L"Error", MB_ICONHAND | MB_OK);
		return hr;
	}

	hr = LoadShaders();

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Could not load shaders.", L"Error", MB_ICONHAND | MB_OK);
		return hr;
	}

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
	};
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Create index buffer
	WORD indices[] =
	{
		0,1,2,
		2,1,3
	};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))
		return false;

	// Initialize the world matrix
	for (int i = 0; i < m_pagesNum; i++)
		m_World[i] = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -2.5f, 1.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	PageBuilder_t tmp = std::make_shared<PageBuilder>();
	RenderDocument(tmp);

	return S_OK;
}

HRESULT PdfRender::Render(void * pResource, bool isNewSurface)
{
	HRESULT hr = S_OK;
	// If we've gotten a new Surface, need to initialize the renderTarget.
	// One of the times that this happens is on a resize.
	if (isNewSurface)
	{
		m_pImmediateContext->OMSetRenderTargets(0, NULL, NULL);
		hr = InitRenderTarget(pResource);
		if (FAILED(hr))
		{
			return hr;
		}
	}


	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	SetUpTranslationMatrixes();

	if (m_right == true && m_Doc->NowView() < m_Doc->size() - 1)
	{
		if (animationRight())
		{
			createOnePage(true, false);
			animCountersSetDefault();
			m_right = false;
		}
	}

	if (m_left == true && m_Doc->NowView() > 0)
	{
		if (animationLeft())
		{
			createOnePage(false, true);
			animCountersSetDefault();
			m_left = false;
		}
	}


	m_Doc->CheckBookmarks(m_bookmarks);

	XMMATRIX WVP;
	ConstantBuffer cb;

	if (m_day)
		cb.vInversion = 0.0f;
	if (m_night)
		cb.vInversion = 1.0f;

	int worldC, end;

	setupRenderCounters(&worldC, &end);
	if (m_left)
	{
		int tmp = worldC;
		worldC = end - 1;
		end = tmp - 1;
		while (worldC > end)
		{
			/*m_pTextureWithBookmark[0] = m_pTextureRV[worldC];
			if (m_Doc->Bookmark(worldC))
				m_pTextureWithBookmark[1] = m_pTextureFullBM;
			else
				m_pTextureWithBookmark[1] = m_pTextureNullBM;*/
			if (m_Doc->Bookmark(worldC))
				cb.fBookmark = 1;
			else
				cb.fBookmark = 0;
			WVP = m_World[worldC] * m_View * m_Projection;
			cb.WVP = XMMatrixTranspose(WVP);
			m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
			m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
			m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
			m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV[worldC]);
			m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
			m_pImmediateContext->DrawIndexed(6, 0, 0);
			worldC--;
		}
	}
	else
	{
		while (worldC < end)
		{
			/*m_pTextureWithBookmark[0] = m_pTextureRV[worldC];
			if (m_Doc->Bookmark(worldC))
			m_pTextureWithBookmark[1] = m_pTextureFullBM;
			else
			m_pTextureWithBookmark[1] = m_pTextureNullBM;*/
			if (m_Doc->Bookmark(worldC))
				cb.fBookmark = 1;
			else
				cb.fBookmark = 0;

			WVP = m_World[worldC] * m_View * m_Projection;
			cb.WVP = XMMatrixTranspose(WVP);
			m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb, 0, 0);
			m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
			m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
			m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV[worldC]);
			m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
			m_pImmediateContext->DrawIndexed(6, 0, 0);
			worldC++;
		}
	}

	if (NULL != m_pImmediateContext)
	{
		m_pImmediateContext->Flush();
	}

	return 0;
}

void PdfRender::OpenDocument(std::wstring filename)
{
	PageBuilder_t tmp = std::make_shared<PageBuilder>(filename);
	RenderDocument(tmp);
}

void PdfRender::SetLeft(bool aState)
{
	m_left = aState;
	m_right = !aState;
}

void PdfRender::SetRight(bool aState)
{
	m_right = aState;
	m_left = !aState;
}

void PdfRender::SetDay(bool aState)
{
	m_day = aState;
	m_night = !aState;
}

void PdfRender::SetNight(bool aState)
{
	m_night = aState;
	m_day = !aState;
}

void PdfRender::SetReverse()
{
	m_day = !m_day;
	m_night = !m_night;
}

int PdfRender::GetNowPageWidth()
{
	return m_Doc->NowWidth();
}

int PdfRender::GetNowPageHeight()
{
	return m_Doc->NowHeight();
}

void PdfRender::SaveImage(int percent, std::wstring filename)
{
	m_Doc->SaveImage(percent, filename);
}

int PdfRender::GetDocSize()
{
	return m_Doc->size();
}

bool PdfRender::Search(std::wstring searchstr, int search_page, bool direction, bool case_sensetive)
{
	m_Doc->SetCaseSensitiveSearch(case_sensetive);
	bool result = false;
	if (!direction)
		result = m_Doc->SearchOnPageForward(search_page, searchstr);
	else
		result = m_Doc->SearchOnPageBackward(search_page, searchstr);

	if (result)
	{
		SetSearchPages();
		return result;
	}
	else
		return result;
}

int PdfRender::GetNowPage()
{
	return m_Doc->NowView();
}

void PdfRender::ViewBookmark(int page_num)
{
	if (page_num > m_Doc->NowView())
	{
		while (m_Doc->NowView() != page_num)
		{
			createOnePage(true, false);
		}
		m_Doc->SetBookmark(true);
	}
	else if (page_num < m_Doc->NowView())
	{
		while (m_Doc->NowView() != page_num)
		{
			createOnePage(false, true);
		}
		m_Doc->SetBookmark(true);
	}
}

void PdfRender::AddBookmark()
{
	m_bookmarks.push_back(m_Doc->NowView());
}

void PdfRender::RemoveBookmark()
{
	std::list<int>::iterator it;
	for (it = m_bookmarks.begin(); *it != m_Doc->NowView(); ++it);
		m_bookmarks.erase(it);
}

void PdfRender::SetSearchPages()
{
	for (int i = 0; i < m_pagesNum; i++)
		createStartPages(i);

}

void PdfRender::RenderDocument(PageBuilder_t pages)
{
	m_Doc = pages;
	m_left = false;
	m_right = false;
	animationInit();
	for (int i = 0; i < m_pagesNum; i++)
		createStartPages(i);
	
	/*HRESULT hr = CreateWICTextureFromFile(m_pd3dDevice, L"D:\\work\\dx11pdfReader_v4\\dx11pdfReader_v3\\images\\bookmark.png", nullptr, &m_pTextureFullBM);
	if (FAILED(hr))
	MessageBox(nullptr, L"Error", L"Не удалось создать текстуру", MB_OK);
	hr = CreateWICTextureFromFile(m_pd3dDevice, L"D:\\work\\dx11pdfReader_v4\\dx11pdfReader_v3\\images\\nullbookmark.png", nullptr, &m_pTextureNullBM);
	if (FAILED(hr))
	MessageBox(nullptr, L"Error", L"Не удалось создать текстуру", MB_OK);*/

	for (int i = 0; i < m_pagesNum; i++)
	{
		m_acounters[i].cH /= m_div[i];
		m_acounters[i].cW /= m_div[i];
		m_startpos[i].cH /= m_div[i];
		m_startpos[i].cW /= m_div[i];
	}

	setNextPosValues();
	m_nowPage = PAGE_NUM;
}

HRESULT PdfRender::InitRenderTarget(void * pResource)
{
	HRESULT hr = S_OK;

	IUnknown *pUnk = (IUnknown*)pResource;

	IDXGIResource * pDXGIResource;
	hr = pUnk->QueryInterface(__uuidof(IDXGIResource), (void**)&pDXGIResource);
	if (FAILED(hr))
	{
		return hr;
	}

	HANDLE sharedHandle;
	hr = pDXGIResource->GetSharedHandle(&sharedHandle);
	if (FAILED(hr))
	{
		return hr;
	}

	pDXGIResource->Release();

	IUnknown * tempResource11;
	hr = m_pd3dDevice->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11));
	if (FAILED(hr))
	{
		return hr;
	}

	ID3D11Texture2D * pOutputResource;
	hr = tempResource11->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&pOutputResource));
	if (FAILED(hr))
	{
		return hr;
	}
	tempResource11->Release();

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;

	hr = m_pd3dDevice->CreateRenderTargetView(pOutputResource, &rtDesc, &m_pRenderTargetView);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_TEXTURE2D_DESC outputResourceDesc;
	pOutputResource->GetDesc(&outputResourceDesc);
	if (outputResourceDesc.Width != m_Width || outputResourceDesc.Height != m_Height)
	{
		m_Width = outputResourceDesc.Width;
		m_Height = outputResourceDesc.Height;

		SetUpViewport();
	}

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_Width;
	descDepth.Height = m_Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &m_pDepthStencil);
	if (FAILED(hr))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
	if (FAILED(hr))
		return false;

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	if (NULL != pOutputResource)
	{
		pOutputResource->Release();
	}

	return hr;
}

void PdfRender::SetUpViewport()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float)m_Width;
	vp.Height = (float)m_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	// Initialize the projection matrix
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_Width / (FLOAT)m_Height, 0.01f, 100.0f);
}

HRESULT PdfRender::createOnePage(bool push_back, bool push_front)
{
	HRESULT hr = S_OK;
	int bpp = 4;
	int nb_color = 4;

	int nowPage = 0;
	if (push_back)
	{
		m_Doc->GetNext();
		nowPage = m_nowPage - 1;
	}
	if (push_front)
	{
		m_Doc->GetPrevious();
		nowPage = m_nowPage - PAGE_NUM;
	}


	D3D11_SUBRESOURCE_DATA tbsd;

	tbsd.pSysMem = (void *)m_Doc->GetPage(nowPage);
	tbsd.SysMemPitch = m_Doc->width(nowPage) * bpp;
	tbsd.SysMemSlicePitch = m_Doc->width(nowPage) * m_Doc->height(nowPage) * bpp;

	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = m_Doc->width(nowPage);
	desc.Height = m_Doc->height(nowPage);
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D *tex;
	hr = m_pd3dDevice->CreateTexture2D(&desc, nullptr, &tex);

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = -1;

		if (push_back == true)
		{
			for (int i = 0; i < m_pagesNum - 1; i++)
				m_pTextureRV[i] = m_pTextureRV[i + 1];
			hr = m_pd3dDevice->CreateShaderResourceView(tex, &SRVDesc, &m_pTextureRV[m_pagesNum - 1]);
			m_nextpos[m_pagesNum - 1].cH = m_Doc->height(nowPage) * m_nextpos[m_pagesNum - 1].cW / m_Doc->width(nowPage);
			m_pImmediateContext->UpdateSubresource(tex, 0, 0, tbsd.pSysMem, tbsd.SysMemPitch, 0);
			m_pImmediateContext->GenerateMips(m_pTextureRV[m_pagesNum - 1]);
		}
		if (push_front == true)
		{
			for (int i = m_pagesNum - 1; i > 0; i--)
				m_pTextureRV[i] = m_pTextureRV[i - 1];
			hr = m_pd3dDevice->CreateShaderResourceView(tex, &SRVDesc, &m_pTextureRV[0]);
			m_startpos[0].cH = m_Doc->height(nowPage) * m_startpos[0].cW / m_Doc->width(nowPage);
			m_pImmediateContext->UpdateSubresource(tex, 0, 0, tbsd.pSysMem, tbsd.SysMemPitch, 0);
			m_pImmediateContext->GenerateMips(m_pTextureRV[0]);
		}
	}

	tex->Release();

	if (FAILED(hr))
		return hr;
	return hr;
}

HRESULT PdfRender::createStartPages(int i)
{
	HRESULT hr = S_OK;
	int bpp = 4;
	int nb_color = 4;


	if (m_Doc->Bookmark(i))
		m_pTextureRV[PAGE_NUM] = m_pTextureWithBookmark[1];


	D3D11_SUBRESOURCE_DATA tbsd;

	tbsd.pSysMem = (void *)m_Doc->GetPage(i);
	tbsd.SysMemPitch = m_Doc->width(i) * bpp;
	tbsd.SysMemSlicePitch = m_Doc->width(i) * m_Doc->height(i) * bpp;

	D3D11_TEXTURE2D_DESC desc = {};

	desc.Width = m_Doc->width(i);
	desc.Height = m_Doc->height(i);
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D *tex;
	hr = m_pd3dDevice->CreateTexture2D(&desc, nullptr, &tex);

	if (SUCCEEDED(hr))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = -1;

		hr = m_pd3dDevice->CreateShaderResourceView(tex, &SRVDesc, &m_pTextureRV[i]);
		m_pImmediateContext->UpdateSubresource(tex, 0, 0, tbsd.pSysMem, tbsd.SysMemPitch, 0);
		m_pImmediateContext->GenerateMips(m_pTextureRV[i]);
	}

	m_acounters[i].cH = m_Doc->height(i) * m_acounters[i].cW / m_Doc->width(i);
	m_startpos[i].cH = m_Doc->height(i) * m_startpos[i].cW / m_Doc->width(i);

	tex->Release();

	return hr;
}

bool PdfRender::animationRight()
{
	float animationStep = 0.004f;

	m_acounters[1].cTR -= animationStep;
	if (m_acounters[1].cW > m_nextpos[0].cW)
		m_acounters[1].cW -= animationStep;
	if (m_acounters[1].cH > m_nextpos[0].cH)
		m_acounters[1].cH -= animationStep;

	if (m_acounters[2].cTR > m_nextpos[1].cTR)
		m_acounters[2].cTR -= animationStep;
	if (m_acounters[2].cW > m_nextpos[1].cW)
		m_acounters[2].cW -= animationStep;
	if (m_acounters[2].cH > m_nextpos[1].cH)
		m_acounters[2].cH -= animationStep;

	if (m_acounters[3].cTR > m_nextpos[2].cTR)
		m_acounters[3].cTR -= animationStep;
	if (m_acounters[3].cW > m_nextpos[2].cW)
		m_acounters[3].cW -= animationStep;
	if (m_acounters[3].cH > m_nextpos[2].cH)
		m_acounters[3].cH -= animationStep;

	if (m_acounters[4].cTR > m_nextpos[3].cTR)
		m_acounters[4].cTR -= animationStep;
	if (m_acounters[4].cW < m_nextpos[3].cW)
		m_acounters[4].cW += animationStep;
	if (m_acounters[4].cH < m_nextpos[3].cH)
		m_acounters[4].cH += animationStep;

	if (m_acounters[5].cTR > m_nextpos[4].cTR)
		m_acounters[5].cTR -= animationStep;
	if (m_acounters[5].cW < m_nextpos[4].cW)
		m_acounters[5].cW += animationStep;
	if (m_acounters[5].cH < m_nextpos[4].cH)
		m_acounters[5].cH += animationStep;

	if (m_acounters[6].cTR > m_nextpos[5].cTR)
		m_acounters[6].cTR -= animationStep;
	if (m_acounters[6].cW < m_nextpos[5].cW)
		m_acounters[6].cW += animationStep;
	if (m_acounters[6].cH < m_nextpos[5].cH)
		m_acounters[6].cH += animationStep;

	if (m_acounters[4].cTR > m_nextpos[3].cTR)
		return false;
	else
		return true;
}

bool PdfRender::animationLeft()
{
	float animationStep = 0.004f;

	if (m_acounters[0].cTR < m_startpos[1].cTR)
		m_acounters[0].cTR += animationStep;
	if (m_acounters[0].cW < m_startpos[1].cW)
		m_acounters[0].cW += animationStep;
	if (m_acounters[0].cH < m_startpos[1].cH)
		m_acounters[0].cH += animationStep;

	if (m_acounters[1].cTR < m_startpos[2].cTR)
		m_acounters[1].cTR += animationStep;
	if (m_acounters[1].cW < m_startpos[2].cW)
		m_acounters[1].cW += animationStep;
	if (m_acounters[1].cH < m_startpos[2].cH)
		m_acounters[1].cH += animationStep;

	if (m_acounters[2].cTR < m_startpos[3].cTR)
		m_acounters[2].cTR += animationStep;
	if (m_acounters[2].cW < m_startpos[3].cW)
		m_acounters[2].cW += animationStep;
	if (m_acounters[2].cH < m_startpos[3].cH)
		m_acounters[2].cH += animationStep;

	if (m_acounters[3].cTR < m_startpos[4].cTR)
		m_acounters[3].cTR += animationStep;
	if (m_acounters[3].cW > m_startpos[4].cW)
		m_acounters[3].cW -= animationStep;
	if (m_acounters[3].cH > m_startpos[4].cH)
		m_acounters[3].cH -= animationStep;

	if (m_acounters[4].cTR < m_startpos[5].cTR)
		m_acounters[4].cTR += animationStep;
	if (m_acounters[4].cW > m_startpos[5].cW)
		m_acounters[4].cW -= animationStep;
	if (m_acounters[4].cH > m_startpos[5].cH)
		m_acounters[4].cH -= animationStep;

	//if (m_acounters[5].cTR < m_startpos[6].cTR)
	m_acounters[5].cTR += animationStep;
	if (m_acounters[5].cW < m_startpos[6].cW)
		m_acounters[5].cW += animationStep;
	if (m_acounters[5].cH < m_startpos[6].cH)
		m_acounters[5].cH += animationStep;



	if (m_acounters[3].cTR < m_startpos[4].cTR)
		return false;
	else
		return true;
}

void PdfRender::animCountersSetDefault()
{
	if (m_right)
	{
		for (int i = 0; i < m_pagesNum; i++)
			m_acounters[i] = m_nextpos[i];

		setStartPosValues();
		setNextPosValues();
	}

	if (m_left)
	{
		for (int i = 0; i < m_pagesNum; i++)
			m_acounters[i] = m_startpos[i];

		setStartPosValues();
		setNextPosValues();
	}
}

void PdfRender::SetUpTranslationMatrixes()
{
	Translation[0] = XMMatrixTranslation(m_acounters[0].cTR, 0.0f, 0.0f);
	Scale[0] = XMMatrixScaling(m_acounters[0].cW, m_acounters[0].cH, 0.0f);

	Translation[1] = XMMatrixTranslation(m_acounters[1].cTR, 0.0f, 0.0f);
	Scale[1] = XMMatrixScaling(m_acounters[1].cW, m_acounters[1].cH, 0.0f);

	Translation[2] = XMMatrixTranslation(m_acounters[2].cTR, 0.0f, 0.0f);
	Scale[2] = XMMatrixScaling(m_acounters[2].cW, m_acounters[2].cH, 0.0f);

	Translation[3] = XMMatrixTranslation(m_acounters[3].cTR, 0.0f, 0.0f);
	Scale[3] = XMMatrixScaling(m_acounters[3].cW, m_acounters[3].cH, 0.0f);

	Translation[4] = XMMatrixTranslation(m_acounters[4].cTR, 0.0f, 0.0f);
	Scale[4] = XMMatrixScaling(m_acounters[4].cW, m_acounters[4].cH, 0.0f);

	Translation[5] = XMMatrixTranslation(m_acounters[5].cTR, 0.0f, 0.0f);
	Scale[5] = XMMatrixScaling(m_acounters[5].cW, m_acounters[5].cH, 0.0f);

	Translation[6] = XMMatrixTranslation(m_acounters[6].cTR, 0.0f, 0.0f);
	Scale[6] = XMMatrixScaling(m_acounters[6].cW, m_acounters[6].cH, 0.0f);

	m_World[0] = Scale[0] * Translation[0];

	m_World[1] = Scale[1] * Translation[1];
	m_World[2] = Scale[2] * Translation[2];

	m_World[3] = Scale[3] * Translation[3];
	m_World[4] = Scale[4] * Translation[4];
	m_World[5] = Scale[5] * Translation[5];
	m_World[6] = Scale[6] * Translation[6];
}

void PdfRender::setNextPosValues()
{
	m_nextpos[0].cW = m_acounters[1].cW / 2;
	m_nextpos[1].cW = m_acounters[2].cW / 2;
	m_nextpos[2].cW = m_acounters[3].cW / 2;
	m_nextpos[3].cW = m_acounters[4].cW * 2;
	m_nextpos[4].cW = m_acounters[5].cW * 2;
	m_nextpos[5].cW = m_acounters[6].cW * 2;
	m_nextpos[6].cW = m_acounters[6].cW;

	m_nextpos[0].cH = m_acounters[1].cH / 2;
	m_nextpos[1].cH = m_acounters[2].cH / 2;
	m_nextpos[2].cH = m_acounters[3].cH / 2;
	m_nextpos[3].cH = m_acounters[4].cH * 2;
	m_nextpos[4].cH = m_acounters[5].cH * 2;
	m_nextpos[5].cH = m_acounters[6].cH * 2;
	m_nextpos[6].cH = m_acounters[6].cH;
}

void PdfRender::setStartPosValues()
{
	m_startpos[0].cW = m_acounters[0].cW;
	m_startpos[1].cW = m_acounters[0].cW * 2;
	m_startpos[2].cW = m_acounters[1].cW * 2;
	m_startpos[3].cW = m_acounters[2].cW * 2;
	m_startpos[4].cW = m_acounters[3].cW / 2;
	m_startpos[5].cW = m_acounters[4].cW / 2;
	m_startpos[6].cW = m_acounters[5].cW / 2;

	m_startpos[0].cH = m_acounters[0].cH;
	m_startpos[1].cH = m_acounters[0].cH * 2;
	m_startpos[2].cH = m_acounters[1].cH * 2;
	m_startpos[3].cH = m_acounters[2].cH * 2;
	m_startpos[4].cH = m_acounters[3].cH / 2;
	m_startpos[5].cH = m_acounters[4].cH / 2;
	m_startpos[6].cH = m_acounters[5].cH / 2;
}

void PdfRender::animationInit()
{
	m_acounters[0].cTR = -1.925f;
	m_acounters[1].cTR = -1.65f;
	m_acounters[2].cTR = -1.1f;
	m_acounters[3].cTR = 0.0f;
	m_acounters[4].cTR = 1.1f;
	m_acounters[5].cTR = 1.65f;
	m_acounters[6].cTR = 1.925f;

	for (int i = 0; i < m_pagesNum; i++)
		m_acounters[i].cW = 0.71f;

	m_div[0] = 8;
	m_div[1] = 4;
	m_div[2] = 2;
	m_div[3] = 1;
	m_div[4] = 2;
	m_div[5] = 4;
	m_div[6] = 8;

	for (int i = 0; i < m_pagesNum; i++)
	{
		m_startpos[i] = m_acounters[i];
		m_nextpos[i] = m_acounters[i];
	}
}

void PdfRender::setupRenderCounters(int * worldC, int * end)
{
	*end = m_pagesNum - 1;
	if (m_right)
		*end = m_pagesNum;

	*worldC = 1;
	if (m_left)
		*worldC = 0;

	if (m_Doc->NowView() == 0)
		*worldC = 3;
	else if (m_Doc->NowView() == 1)
		*worldC = 2;
	else if (m_Doc->NowView() == 2)
		*worldC = 1;
	else if (m_Doc->NowView() == m_Doc->size() - 3)
		*end = m_pagesNum - 1;
	else if (m_Doc->NowView() == m_Doc->size() - 2)
		*end = m_pagesNum - 2;
	else if (m_Doc->NowView() == m_Doc->size() - 1)
		*end = m_pagesNum - 3;

	if (m_Doc->size() == 4 && m_Doc->NowView() == 0 && m_right == false)
		*end = m_Doc->size() + 2;
	else if (m_Doc->size() == 4 && m_Doc->NowView() == m_Doc->size() - 1)
		*end = m_Doc->size();

	else if (m_Doc->size() == 5 && m_Doc->NowView() == m_Doc->size() - 3 && m_right == true)
		*end = m_pagesNum - 1;


	else if (m_Doc->size() < PAGE_NUM - 2)
		*end = m_Doc->size() + *worldC;
}

HRESULT PdfRender::LoadShaders()
{
	HRESULT hr = S_OK;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"D3DVisualization.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"D3DVisualization.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Set the input layout
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

	return hr;
}

HRESULT PdfRender::CompileShaderFromFile(WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D10Blob ** ppBlobOut)
{
	HRESULT hr = S_OK;

	ID3D10Blob* pErrorBlob = NULL;

	hr = D3DCompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		0, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (NULL != pErrorBlob)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}
	}

	SAFE_RELEASE(pErrorBlob);

	return hr;
}
