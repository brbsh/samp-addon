#pragma once



#include "client.hpp"





extern boost::shared_ptr<addonD3Device> gD3Device;





ProxyIDirect3DDevice9::ProxyIDirect3DDevice9(IDirect3DDevice9 *_original, UINT _adapter, D3DDEVTYPE _deviceType, HWND _focusWindow, DWORD _behaviorFlags, D3DPRESENT_PARAMETERS *_presentationParameters) :
	original(_original),
	adapter(_adapter),
	deviceType(_deviceType),
	focusWindow(_focusWindow),
	behaviorFlags(_behaviorFlags),
	presentationParameters(*_presentationParameters)
{

}



ProxyIDirect3DDevice9::~ProxyIDirect3DDevice9()
{

}



HRESULT ProxyIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
	*ppvObj = NULL;

	HRESULT result = original->QueryInterface(riid, ppvObj);
	if (result == S_OK)
	{
		ULONG count = original->AddRef()-1;
		original->Release();

		*ppvObj = this;
	}

	return result;
}

ULONG ProxyIDirect3DDevice9::AddRef()
{
    ULONG count = original->AddRef();

	return count;
}

ULONG ProxyIDirect3DDevice9::Release()
{
	ULONG count = original->Release();

	if (count == 0)
	{		
		delete(this);
	}

	return count;
}

HRESULT ProxyIDirect3DDevice9::TestCooperativeLevel()
{
    return original->TestCooperativeLevel();
}

UINT ProxyIDirect3DDevice9::GetAvailableTextureMem()
{
    return original->GetAvailableTextureMem();
}

HRESULT ProxyIDirect3DDevice9::EvictManagedResources()
{
    return original->EvictManagedResources();
}

HRESULT ProxyIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
    return original->GetDirect3D(ppD3D9);
}

HRESULT ProxyIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
    return original->GetDeviceCaps(pCaps);
}

HRESULT ProxyIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
    return original->GetDisplayMode(iSwapChain, pMode);
}

HRESULT ProxyIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return original->GetCreationParameters(pParameters);
}

HRESULT ProxyIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
	return original->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void ProxyIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{
    return original->SetCursorPosition(X,Y,Flags);
}

BOOL ProxyIDirect3DDevice9::ShowCursor(BOOL bShow)
{
    return original->ShowCursor(bShow);
}

HRESULT ProxyIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  
{
	return original->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT ProxyIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
	return original->GetSwapChain(iSwapChain, pSwapChain);
}

UINT ProxyIDirect3DDevice9::GetNumberOfSwapChains()
{
    return original->GetNumberOfSwapChains();
}

HRESULT ProxyIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    return original->Reset(pPresentationParameters);
}

STDMETHODIMP ProxyIDirect3DDevice9::Present(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
	return original->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT ProxyIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
	return original->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT ProxyIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
    return original->GetRasterStatus(iSwapChain,pRasterStatus);
}

HRESULT ProxyIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
    return original->SetDialogBoxMode(bEnableDialogs);
}

void ProxyIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
    return original->SetGammaRamp(iSwapChain,Flags,pRamp);
}

void ProxyIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
    return original->GetGammaRamp(iSwapChain,pRamp);
}

HRESULT ProxyIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
	return original->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
	return original->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
	return original->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
	return original->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
	return original->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	return original->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	return original->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
	return original->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

HRESULT ProxyIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{;
	return original->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT ProxyIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
	return original->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT ProxyIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
	return original->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT ProxyIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{ 
	return original->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT ProxyIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
	return original->ColorFill(pSurface, pRect ,color);
}

HRESULT ProxyIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
	return original->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT ProxyIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
	return original->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT ProxyIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
	return original->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT ProxyIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return original->SetDepthStencilSurface(pNewZStencil);
}

HRESULT ProxyIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	return original->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT ProxyIDirect3DDevice9::BeginScene()
{
	gD3Device->initRender();

	return original->BeginScene();
}

HRESULT ProxyIDirect3DDevice9::EndScene()
{
	gD3Device->processRender();

	return original->EndScene();
}

HRESULT ProxyIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
    return original->Clear(Count,pRects,Flags,Color,Z,Stencil);
}

HRESULT ProxyIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return original->SetTransform(State, pMatrix);
}

HRESULT ProxyIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
    return original->GetTransform(State,pMatrix);
}

HRESULT ProxyIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return original->MultiplyTransform(State,pMatrix);
}

HRESULT ProxyIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
    return original->SetViewport(pViewport);
}

HRESULT ProxyIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
    return original->GetViewport(pViewport);
}

HRESULT ProxyIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
    return original->SetMaterial(pMaterial);
}

HRESULT ProxyIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
    return original->GetMaterial(pMaterial);
}

HRESULT ProxyIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
    return original->SetLight(Index,pLight);
}

HRESULT ProxyIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
    return original->GetLight(Index,pLight);
}

HRESULT ProxyIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{
    return original->LightEnable(Index,Enable);
}

HRESULT ProxyIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{
    return original->GetLightEnable(Index, pEnable);
}

HRESULT ProxyIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{
    return original->SetClipPlane(Index, pPlane);
}

HRESULT ProxyIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{
    return original->GetClipPlane(Index,pPlane);
}

HRESULT ProxyIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
    return original->SetRenderState(State, Value);
}

HRESULT ProxyIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
    return original->GetRenderState(State, pValue);
}

HRESULT ProxyIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{	
	return original->CreateStateBlock(Type, ppSB);
}

HRESULT ProxyIDirect3DDevice9::BeginStateBlock()
{
    return original->BeginStateBlock();
}

HRESULT ProxyIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{	
	return original->EndStateBlock(ppSB);
}

HRESULT ProxyIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
    return original->SetClipStatus(pClipStatus);
}

HRESULT ProxyIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
    return original->GetClipStatus(pClipStatus);
}

HRESULT ProxyIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
	return original->GetTexture(Stage, ppTexture);
}

HRESULT ProxyIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
	return original->SetTexture(Stage, pTexture);
}

HRESULT ProxyIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
    return original->GetTextureStageState(Stage,Type, pValue);
}

HRESULT ProxyIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
    return original->SetTextureStageState(Stage,Type,Value);
}

HRESULT ProxyIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
    return original->GetSamplerState(Sampler,Type, pValue);
}

HRESULT ProxyIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
    return original->SetSamplerState(Sampler,Type,Value);
}

HRESULT ProxyIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
    return original->ValidateDevice(pNumPasses);
}

HRESULT ProxyIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
    return original->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT ProxyIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
    return original->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT ProxyIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
    return original->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT ProxyIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
    return original->GetCurrentTexturePalette(PaletteNumber);
}

HRESULT ProxyIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
    return original->SetScissorRect(pRect);
}

HRESULT ProxyIDirect3DDevice9::GetScissorRect( RECT* pRect)
{
    return original->GetScissorRect(pRect);
}

HRESULT ProxyIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
    return original->SetSoftwareVertexProcessing(bSoftware);
}

BOOL ProxyIDirect3DDevice9::GetSoftwareVertexProcessing()
{
    return original->GetSoftwareVertexProcessing();
}

HRESULT ProxyIDirect3DDevice9::SetNPatchMode(float nSegments)
{
    return original->SetNPatchMode(nSegments);
}

float ProxyIDirect3DDevice9::GetNPatchMode()
{
    return original->GetNPatchMode();
}

HRESULT ProxyIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	return original->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount);
}

HRESULT ProxyIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
    return original->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount);
}

HRESULT ProxyIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{	
	return original->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT ProxyIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	return original->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride);
}

HRESULT ProxyIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	return original->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT ProxyIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	return original->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT ProxyIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return original->SetVertexDeclaration(pDecl);
}

HRESULT ProxyIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
    return original->GetVertexDeclaration(ppDecl);
}

HRESULT ProxyIDirect3DDevice9::SetFVF(DWORD FVF)
{
    return original->SetFVF(FVF);
}

HRESULT ProxyIDirect3DDevice9::GetFVF(DWORD* pFVF)
{
    return original->GetFVF(pFVF);
}

HRESULT ProxyIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	return original->CreateVertexShader(pFunction, ppShader);
}

HRESULT ProxyIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return original->SetVertexShader(pShader);
}

HRESULT ProxyIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return original->GetVertexShader(ppShader);
}

HRESULT ProxyIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return original->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount);
}

HRESULT ProxyIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return original->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT ProxyIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return original->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT ProxyIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return original->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT ProxyIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return original->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT ProxyIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return original->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT ProxyIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
	return original->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT ProxyIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
    return original->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT ProxyIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
    return original->SetStreamSourceFreq(StreamNumber,Divider);
}

HRESULT ProxyIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
    return original->GetStreamSourceFreq(StreamNumber,Divider);
}

HRESULT ProxyIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return original->SetIndices(pIndexData);
}

HRESULT ProxyIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
    return original->GetIndices(ppIndexData);
}

HRESULT ProxyIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	return original->CreatePixelShader(pFunction, ppShader);
}

HRESULT ProxyIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return original->SetPixelShader(pShader);
}

HRESULT ProxyIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return original->GetPixelShader(ppShader);
}

HRESULT ProxyIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return original->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT ProxyIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return original->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount);
}

HRESULT ProxyIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return original->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT ProxyIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return original->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount);
}

HRESULT ProxyIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return original->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT ProxyIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	return original->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount);
}

HRESULT ProxyIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return original->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo);
}

HRESULT ProxyIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return original->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT ProxyIDirect3DDevice9::DeletePatch(UINT Handle)
{
    return original->DeletePatch(Handle);
}

HRESULT ProxyIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	return original->CreateQuery(Type, ppQuery);
}