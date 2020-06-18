#pragma once
#include <Urho3D/Urho3DAll.h>
#include <d3d11.h>
class MyUI:public Object
{
	URHO3D_OBJECT(MyUI,Object)
private:

	ID3D11Device* g_pd3dDevice = NULL;
	ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;
	ID3D11RenderTargetView* g_mainRenderTargetView = NULL;


	void HandleSDLRawInput(StringHash eventType, VariantMap& eventData);
	void HandleAllViewRenderEnd(StringHash eventType, VariantMap& eventData);
	void RenderUI();
public:
	MyUI(Context * context);
	void Init();
	
};
