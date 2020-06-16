#include "Urho2dTest.h"

#include <Urho3D/Scene/SceneEvents.h>

#include <Urho3D/Core/CoreEvents.h>

#include <Urho3D/Scene/SceneEvents.h>

#include <Urho3D/Scene/SceneEvents.h>
#include "Global.h"

#include <Urho3D/Graphics/GraphicsImpl.h>
#include <Urho3D/Math/MathDefs.h>


#include "i/imgui.h"
#include <Urho3D/ThirdParty/SDL/SDL_events.h>
#include "i/imgui_impl_sdl.h"
#include "i/imgui_impl_dx11.h"
#include <d3d11.h>
#include <stdio.h>
//#include <SDL.h>
//#include <SDL_syswm.h>
//#define TESTDATA 
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
URHO3D_DEFINE_APPLICATION_MAIN(Urho2DTest)

Urho2DTest::Urho2DTest(Context* context) :
	Application(context), yaw_(0.0f),
	pitch_(0.f),
	useMouseMode_(MM_ABSOLUTE),
	screenJoystickIndex_(M_MAX_UNSIGNED),
	screenJoystickSettingsIndex_(M_MAX_UNSIGNED),
	paused_(false)
{
	context->RegisterFactory<CameraController>();
	context->RegisterSubsystem<Global>();
}

void Urho2DTest::Setup()
{
	engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
	engineParameters_[EP_LOG_NAME] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
	engineParameters_[EP_FULL_SCREEN] = false;
	engineParameters_[EP_HEADLESS] = false;
	engineParameters_[EP_SOUND] = false;
	engineParameters_[EP_WINDOW_HEIGHT] = 900;
	engineParameters_[EP_WINDOW_WIDTH] = 1600;
	engineParameters_[EP_WINDOW_RESIZABLE] = true;
//	engineParameters_[EP_FRAME_LIMITER] = false;
	engineParameters_[EP_VSYNC] = true;

	if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS))
		engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../../Resources;./Resources";

	engineParameters_[EP_RESOURCE_PATHS] = "GameData;CoreData;Data";
}

void Urho2DTest::Start()
{
	CACHE->SetAutoReloadResources(true);
	CreateConsoleAndDebugHud();
	InitMouseModeAndLoadL10nFiles(MM_FREE, "Lang.json");
	CreateUI();
	CreateImgui();
	CreateScene();
	SetupViewport();
	SubscribeToEvents();

	SETDEV(0, "LeftClick   ----   HardLight\n\nRightClick   ----   SoftLight\n\nJ   ----   ShowDepthMap");

	SETDEV(2,"WS:  " + String(GetSubsystem<WorkQueue>()->GetNumThreads()));
}

void Urho2DTest::Stop()
{
	engine_->DumpResources(true);
}




void Urho2DTest::InitMouseModeAndLoadL10nFiles(MouseMode mode, const String& FileNames)
{

	useMouseMode_ = mode;

	Input* input = GetSubsystem<Input>();

	if (GetPlatform() != "Web")
	{
		if (useMouseMode_ == MM_FREE)
			input->SetMouseVisible(true);

		Console* console = GetSubsystem<Console>();
		if (useMouseMode_ != MM_ABSOLUTE)
		{
			input->SetMouseMode(useMouseMode_);
			if (console && console->IsVisible())
				input->SetMouseMode(MM_ABSOLUTE, true);
		}
	}
	else
	{
		input->SetMouseVisible(true);
		SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Urho2DTest, HandleMouseModeRequest));
		SubscribeToEvent(E_MOUSEMODECHANGED, URHO3D_HANDLER(Urho2DTest, HandleMouseModeChange));
	}


	auto l10n = LOCALIZATION;

	l10n->LoadJSONFile(FileNames);
}

void Urho2DTest::CreateConsoleAndDebugHud()
{

	// Get default style
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

	// Create console
	Console* console = engine_->CreateConsole();
	console->SetDefaultStyle(xmlFile);
	console->GetBackground()->SetOpacity(0.8f);

	// Create debug HUD.
	DebugHud* debugHud = engine_->CreateDebugHud();
	debugHud->SetDefaultStyle(xmlFile);
}

void Urho2DTest::SubscribeToEvents()
{
	// Subscribe key down event
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Urho2DTest, HandleKeyDown));
	// Subscribe key up event
	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Urho2DTest, HandleKeyUp));
	// Subscribe scene update event
	SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Urho2DTest, HandleSceneUpdate));


	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Urho2DTest, HandleUpdate));
	SubscribeToEvent(E_POSTRENDERUPDATE, [=](StringHash eventType, VariantMap& eventData)
		{
			//if (!DrawDebugGeometry_)return;

			DebugRenderer* dbgRenderer = scene_->GetOrCreateComponent<DebugRenderer>();
			if (dbgRenderer)
			{
				//»æÖÆnavmeshÊý¾Ý
				//	auto navMesh = scene_->GetComponent<PhysicsWorld2D>();
				//`	navMesh->DrawDebugGeometry(dbgRenderer, false);
			}
		});
	SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Urho2DTest, HandleMouseButtonDown));
	SubscribeToEvent(E_ENDRENDERING, URHO3D_HANDLER(Urho2DTest, HandleAllViewRenderEnd));
}

void Urho2DTest::CreateCollFromJson(Node* node, const String& FileName, const Sprite2D* spr2d, int index)
{

	auto jroot = GET_JSON_FILE(FileName)->GetRoot();
	auto height = spr2d->GetTexture()->GetHeight();
	auto width = spr2d->GetTexture()->GetWidth();
	auto size = height > width ? height : width;

	auto jarray = jroot.Get("rigidBodies").GetArray()[index].Get("polygons").GetArray();

	for (auto jp : jarray)
	{
		auto jarray2 = jp.GetArray();
		PODVector<Vector2> vs;
		for (auto j : jarray2)
		{
			vs.Push(Vector2(j.Get("x").GetFloat() * size / 100.f - size / 200.f, j.Get("y").GetFloat() * size / 100.f - height / 200.f));
		}

		node->CreateComponent<CollisionPolygon2D>()->SetVertices(vs);
		node->CreateComponent<CollisionPolygon2D>()->SetFriction(1.f);

	}
}

void Urho2DTest::HandleMouseModeRequest(StringHash eventType, VariantMap& eventData)
{

	Console* console = GetSubsystem<Console>();
	if (console && console->IsVisible())
		return;
	Input* input = GetSubsystem<Input>();
	if (useMouseMode_ == MM_ABSOLUTE)
		input->SetMouseVisible(false);
	else if (useMouseMode_ == MM_FREE)
		input->SetMouseVisible(true);
	input->SetMouseMode(useMouseMode_);

}

void Urho2DTest::HandleMouseModeChange(StringHash eventType, VariantMap& eventData)
{
	Input* input = GetSubsystem<Input>();
	bool mouseLocked = eventData[MouseModeChanged::P_MOUSELOCKED].GetBool();
	input->SetMouseVisible(!mouseLocked);
}

void Urho2DTest::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{

	using namespace KeyDown;

	if (eventData[P_REPEAT].GetBool())return;
	int key = eventData[P_KEY].GetInt();

	// Toggle console with F1
	if (key == KEY_F1)
		GetSubsystem<Console>()->Toggle();

	// Toggle debug HUD with F2
	else if (key == KEY_F2)
		GetSubsystem<DebugHud>()->ToggleAll();
	// Common rendering quality controls, only when UI has no focused element
	else if (key == KEY_SPACE)
	{

		JSONFile f(context_);
		scene_->SaveJSON(f.GetRoot());
		f.SaveFile("d:/11.json");
	}
	else if (key == KEY_1)
	{
		auto node = scene_->GetChild("Floors");
		node->SetEnabledRecursive(!node->IsEnabled());
	}
	else if (key == KEY_2)
	{
		auto node = scene_->GetChild("Floors2");
		node->SetEnabledRecursive(!node->IsEnabled());
	}
	else if (key == KEY_F)
	{
	}
	
	else if (key == KEY_V)
	{
	}
	else if (key == KEY_F5)
	{
		JSONFile jf(context_);
		scene_->SaveJSON(jf.GetRoot());
		jf.SaveFile("D:/111.json");
	}
	else if (key == KEY_F7)
	{
		JSONFile jf(context_);
		jf.LoadFile("D:/111.json");
		scene_->LoadJSON(jf.GetRoot());
	}
	else if (key == KEY_F12)
	{
		Image img(context_);

		GRAPHICS->TakeScreenShot(img);

		img.SavePNG("d:/11/111.png");
	}

	else if (!UI->GetFocusElement())
	{
		Renderer* renderer = GetSubsystem<Renderer>();

		// Preferences / Pause
		if (key == KEY_SELECT)
		{
			paused_ = !paused_;

			Input* input = GetSubsystem<Input>();
			if (screenJoystickSettingsIndex_ == M_MAX_UNSIGNED)
			{
				// Lazy initialization
				ResourceCache* cache = GetSubsystem<ResourceCache>();
				screenJoystickSettingsIndex_ = (unsigned)input->AddScreenJoystick(cache->GetResource<XMLFile>("UI/ScreenJoystickSettings_Samples.xml"), cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
			}
			else
				input->SetScreenJoystickVisible(screenJoystickSettingsIndex_, paused_);
		}

		// Texture quality
		else if (key == '1')
		{
			//int quality = renderer->GetTextureQuality();
			//++quality;
			//if (quality > QUALITY_HIGH)
			//	quality = QUALITY_LOW;
			//renderer->SetTextureQuality(quality);
		}

		// Material quality
		else if (key == '2')
		{
			//int quality = renderer->GetMaterialQuality();
			//++quality;
			//if (quality > QUALITY_HIGH)
			//	quality = QUALITY_LOW;
			//renderer->SetMaterialQuality(quality);
		}

		// Specular lighting
		else if (key == '3')
			renderer->SetSpecularLighting(!renderer->GetSpecularLighting());

		// Shadow rendering
		else if (key == '4')
			renderer->SetDrawShadows(!renderer->GetDrawShadows());

		// Shadow map resolution
		else if (key == '5')
		{
			int shadowMapSize = renderer->GetShadowMapSize();
			shadowMapSize *= 2;
			if (shadowMapSize > 2048)
				shadowMapSize = 512;
			renderer->SetShadowMapSize(shadowMapSize);
		}

		// Shadow depth and filtering quality
		else if (key == '6')
		{
			ShadowQuality quality = renderer->GetShadowQuality();
			quality = (ShadowQuality)(quality + 1);
			if (quality > SHADOWQUALITY_BLUR_VSM)
				quality = SHADOWQUALITY_SIMPLE_16BIT;
			renderer->SetShadowQuality(quality);
		}

		// Occlusion culling
		else if (key == '7')
		{
			bool occlusion = renderer->GetMaxOccluderTriangles() > 0;
			occlusion = !occlusion;
			renderer->SetMaxOccluderTriangles(occlusion ? 5000 : 0);
		}

		// Instancing
		else if (key == '8')
			renderer->SetDynamicInstancing(!renderer->GetDynamicInstancing());

		else if (key == KEY_P)
		{
			File file(context_, "D:/test.xml", FILE_WRITE);



			scene_->SaveXML(file);

		}

		else if (key == KEY_O)
		{
			File file(context_, "D:/testui.json", FILE_WRITE);


		}

	}
}
void Urho2DTest::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{


	using namespace KeyUp;

	int key = eventData[P_KEY].GetInt();

	// Close console (if open) or exit when ESC is pressed
	if (key == KEY_ESCAPE)
	{
		Console* console = GetSubsystem<Console>();
		if (console->IsVisible())
			console->SetVisible(false);
		else
		{
			if (GetPlatform() == "Web")
			{
				GetSubsystem<Input>()->SetMouseVisible(true);
				if (useMouseMode_ != MM_ABSOLUTE)
					GetSubsystem<Input>()->SetMouseMode(MM_FREE);
			}
			else
				engine_->Exit();
		}
	}
}

void Urho2DTest::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{


}
void Urho2DTest::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(INPUT->GetKeyDown(KEY_U))
	 scene_->GetChild("Floors")->GetComponent<MultiStaticSprite2D>()->SetAlpha(Random(0.999f,1.f));

}
void Urho2DTest::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseButtonDown;
	auto button = eventData[P_BUTTON].GetInt();
	if (button == 1 || button == 4)
	{

	

	}
}
void Urho2DTest::HandleAllViewRenderEnd(StringHash eventType, VariantMap& eventData)
{
	RenderImGui();
}
void Urho2DTest::RenderImGui()
{

	URHO3D_PROFILE(RenderImGui);
	if (GRAPHICS->IsDeviceLost())return;

	auto window = GRAPHICS->GetWindow();
	auto g_pd3dDeviceContext = GRAPHICS->GetImpl()->GetDeviceContext();
	g_mainRenderTargetView = (ID3D11RenderTargetView*)*(((int**)GRAPHICS->GetImpl()) + 3);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		if (ImGui::ColorEdit3("Fog color", (float*)&clear_color))// Edit 3 floats representing a color
		{
			RENDERER->GetDefaultZone()->SetFogColor(Color((const float*)&clear_color));
		}


		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	//	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//	g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync
}
Vector2 Urho2DTest::GetMousePosInWorld()
{
	auto pos = INPUT->GetMousePosition();
	Graphics* graphics = GetSubsystem<Graphics>();
	Camera* camera = cameraNode_->GetComponent<Camera>();
	Vector3 v3 = camera->ScreenToWorldPoint(Vector3((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight(), -10));

	return Vector2(v3.x_, v3.y_);
}
void Urho2DTest::CreateUI()
{

	auto root = UI_ROOT;

	auto style = CACHE->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(style);

	auto hub = UI->LoadLayout(GET_XML_FILE("UI/Hub.xml"));
	UI_ROOT->AddChild(hub);


}

void Urho2DTest::CreateScene()
{

	scene_ = new Scene(context_);

	scene_->LoadJSON(GET_JSON_FILE("scene.json")->GetRoot());
	cameraNode_ = scene_->GetChild("CameraNode");
	cameraNode_->CreateComponent<CameraController>();

	auto hehe = scene_->CreateChild("Hehe");
	hehe->LoadJSON(GET_JSON_FILE("hehe.json")->GetRoot());

	//auto my = scene_->CreateChild("Floors");
	//my->SetPosition2D(0.f, 0.f);
	//my->SetPosition2D(500.f, 0.f);
	//auto mspr = my->CreateComponent<MultiStaticSprite2D>();

	//auto w = 100, h = 50;
	//auto data = (char*)malloc(w * h + 8);
	//auto wh = (int*)data;
	//wh[0] = w;
	//wh[1] = h;
	//mspr->SetGridData(data);
	//data += 8;
	//memset(data, 1, w * h);
	//List<Vector2> list;
	//for (int i = 0; i < 20000; i++)
	//	list.Insert(list.Begin(), Vector2(Random(-100.f, 100.f), Random(-100.f, 100.f)));
	//mspr->SetPoints(list);
	//mspr->SetBatchNum(4);
	//mspr->SetSprite(0, GET_SPRITE2D("Textures/Floor1.png"));

	//mspr->SetSprite(1, GET_SPRITE2D("Textures/Floor2.png"));

	//mspr->SetSprite(2, GET_SPRITE2D("Textures/Wall.png"));

	//mspr->SetSprite(3, GET_SPRITE2D("Textures/hehe.png"));
//	mspr->SetDataType(MultiStaticSprite2D::DataType::List);
}
void Urho2DTest::CreateImgui()
{
	main22(); 
	

	SubscribeToEvent(E_SDLRAWINPUT, [=](StringHash eventType, VariantMap& eventData)
		{
	using namespace UIMouseClick;
			using namespace SDLRawInput;
			auto sdl_event = static_cast<SDL_Event*>(eventData[P_SDLEVENT].GetVoidPtr() );
			ImGui_ImplSDL2_ProcessEvent(sdl_event);

		});
}
void Urho2DTest::SetupViewport()
{
	Renderer* renderer = GetSubsystem<Renderer>();


	auto camera = cameraNode_->GetComponent<Camera>();
	// Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
	SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));


	renderer->SetViewport(0, viewport);
}
// dear imgui: standalone example application for SDL2 + DirectX 11
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)


// Data

// Forward declarations of helper functions
//bool CreateDeviceD3D(HWND hWnd);
//void CleanupDeviceD3D();
//void CreateRenderTarget();
//void CleanupRenderTarget();

// Main code
int Urho2DTest:: main22()
{
	// Setup SDL
	// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
	// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
	auto window = GRAPHICS->GetWindow();
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark(); 
	//ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForD3D(GRAPHICS->GetWindow() );
	ImGui_ImplDX11_Init(GRAPHICS->GetImpl()->GetDevice() ,GRAPHICS->GetImpl()->GetDeviceContext() );

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	//bool done = false;
	//while (!done)
	//{
	//	// Poll and handle events (inputs, window resize, etc.)
	//	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	//	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
	//	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
	//	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

	//	// Start the Dear ImGui frame
	//
	//	//g_pSwapChain->Present(0, 0); // Present without vsync
	//}

	// Cleanup
	//ImGui_ImplDX11_Shutdown();
	//ImGui_ImplSDL2_Shutdown();
	//ImGui::DestroyContext();

	//CleanupDeviceD3D();

	return 0;
}

// Helper functions
//
//bool CreateDeviceD3D(HWND hWnd)
//{
//	// Setup swap chain
//	DXGI_SWAP_CHAIN_DESC sd;
//	ZeroMemory(&sd, sizeof(sd));
//	sd.BufferCount = 2;
//	sd.BufferDesc.Width = 0;
//	sd.BufferDesc.Height = 0;
//	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	sd.BufferDesc.RefreshRate.Numerator = 60;
//	sd.BufferDesc.RefreshRate.Denominator = 1;
//	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	sd.OutputWindow = hWnd;
//	sd.SampleDesc.Count = 1;
//	sd.SampleDesc.Quality = 0;
//	sd.Windowed = TRUE;
//	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//
//	UINT createDeviceFlags = 0;
//	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//	D3D_FEATURE_LEVEL featureLevel;
//	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
//	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
//		return false;
//
//	CreateRenderTarget();
//	return true;
//}
//
//void CleanupDeviceD3D()
//{
//	CleanupRenderTarget();
//	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
//	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
//	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
//}
//
//void CreateRenderTarget()
//{
//	ID3D11Texture2D* pBackBuffer;
//	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
//	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
//	pBackBuffer->Release();
//}
//
//void CleanupRenderTarget()
//{
//	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
//}
