// D3DVisualization.h

#pragma once
#pragma warning(disable : 4561)

#include <d3d11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "DX11Utils.h"
#include "resource.h"
#define PAGE_NUM 7
#include "PageBuilder.h"
#include <list>

extern "C" {
	__declspec(dllexport) HRESULT __cdecl Init();
}

extern "C" {
	__declspec(dllexport) void __cdecl Cleanup();
}

extern "C" {
	__declspec(dllexport) HRESULT __cdecl Render(void * pResource, bool isNewSurface);
}

extern "C" {
	__declspec(dllexport) void __cdecl SetLeft(bool aState);
}

extern "C" {
	__declspec(dllexport) void __cdecl SetRight(bool aState);
}

extern "C" {
	__declspec(dllexport) void __cdecl SetDay(bool aState);
}

extern "C" {
	__declspec(dllexport) void __cdecl SetNight(bool aState);
}

extern "C" {
	__declspec(dllexport) void __cdecl SetReverse();
}

extern "C" {
	__declspec(dllexport) void __cdecl OpenDocument(wchar_t *filename);
}

extern "C" {
	__declspec(dllexport) int __cdecl GetNowPageWidth();
}

extern "C" {
	__declspec(dllexport) int __cdecl GetNowPageHeight();
}

extern "C" {
	__declspec(dllexport) void __cdecl SaveImage(int percent, wchar_t *filename);
}

extern "C" {
	__declspec(dllexport) int __cdecl GetDocSize();
}

extern "C" {
	__declspec(dllexport) bool __cdecl Search(wchar_t *searchstr, int search_page, bool direction, bool case_sensetive);
}

extern "C" {
	__declspec(dllexport) int __cdecl GetNowPage();
}

extern "C" {
	__declspec(dllexport) void __cdecl ViewBookmark(int page_num);
}

extern "C" {
	__declspec(dllexport) void __cdecl AddBookmark();
}

extern "C" {
	__declspec(dllexport) void __cdecl RemoveBookmark();
}

extern "C" {
	__declspec(dllexport) void __cdecl AddBookmarksFromFile(int page_num);
}

extern "C" {
	__declspec(dllexport) void __cdecl ClearBookmarks();
}

struct AnimationCounter
{
	float cTR;
	float cW;
	float cH;
};


public class PdfRender
{
private:
	typedef std::shared_ptr<PageBuilder> PageBuilder_t;

public:
	PdfRender();
	~PdfRender();

	HRESULT InitDevice();
	HRESULT Render(void * pResource, bool isNewSurface);
	void OpenDocument(std::wstring filename);

	void SetLeft(bool aState);
	void SetRight(bool aState);
	void SetDay(bool aState);
	void SetNight(bool aState);
	void SetReverse();

	int GetNowPageWidth();
	int GetNowPageHeight();
	void SaveImage(int percent, std::wstring filename);
	int GetDocSize();
	bool Search(std::wstring searchstr, int search_page, bool direction, bool case_sensetive);
	int GetNowPage();
	void ViewBookmark(int page_num);
	void AddBookmark();
	void RemoveBookmark();
	void AddBookmarksFromFile(int page_num);
	void ClearBookmarks();


	// Special function definitions to ensure alignment between c# and c++ 
	void* operator new(size_t size)
	{
		return _aligned_malloc(size, 16);
	}

	void operator delete(void *p)
	{
		_aligned_free(p);
	}

	void RenderDocument(PageBuilder_t pages);

private:

	HRESULT InitRenderTarget(void * pResource);
	void SetUpViewport();

	HRESULT createOnePage(bool push_back, bool push_front);
	HRESULT createStartPages(int i);
	void SetSearchPages();
	bool animationRight();
	bool animationLeft();
	void animCountersSetDefault();
	void SetUpTranslationMatrixes();
	void setNextPosValues();
	void setStartPosValues();
	void animationInit();
	void setupRenderCounters(int *worldC, int *end);
	HRESULT LoadShaders();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D10Blob** ppBlobOut);


	D3D_DRIVER_TYPE                     m_driverType;
	D3D_FEATURE_LEVEL                   m_featureLevel;

	ID3D11Device*                       m_pd3dDevice;
	ID3D11DeviceContext*                m_pImmediateContext;
	IDXGISwapChain*                     m_pSwapChain;
	ID3D11RenderTargetView*             m_pRenderTargetView;
	ID3D11InputLayout*                  m_pVertexLayout;
	ID3D11Buffer*                       m_pVertexBuffer;
	ID3D11Buffer*                       m_pIndexBuffer;
	ID3D11Buffer*                       m_pConstantBuffer;
	ID3D11Texture2D*					m_pDepthStencil;
	ID3D11DepthStencilView*				m_pDepthStencilView;
	ID3D11SamplerState*					m_pSamplerLinear;

	ID3D11ShaderResourceView*			m_pTextureRV[PAGE_NUM];
	ID3D11ShaderResourceView*			m_pTextureNullBM;
	ID3D11ShaderResourceView*			m_pTextureFullBM;
	ID3D11ShaderResourceView*			m_pTextureWithBookmark[2];

	DirectX::XMMATRIX					m_World[7];
	DirectX::XMMATRIX					m_View;
	DirectX::XMMATRIX					m_Projection;

	ID3D11VertexShader*                 m_pVertexShader;
	ID3D11PixelShader*                  m_pPixelShader;

	// Initial window resolution
	UINT                                m_Width;
	UINT                                m_Height;

	PageBuilder_t m_Doc;

	int m_pagesNum;
	int m_nowPage;

	bool m_left;
	bool m_right;
	bool m_day;
	bool m_night;

	AnimationCounter m_acounters[PAGE_NUM];
	AnimationCounter m_startpos[PAGE_NUM];
	AnimationCounter m_nextpos[PAGE_NUM];
	int m_div[PAGE_NUM];
	DirectX::XMMATRIX Translation[PAGE_NUM];
	DirectX::XMMATRIX Scale[PAGE_NUM];

	std::list<int> m_bookmarks;
};

