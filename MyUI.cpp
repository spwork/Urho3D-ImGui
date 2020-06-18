#include "MyUI.h"
#include "Urho3DAliases.h"

#include "../Precompiled.h"

#include "../AngelScript/APITemplates.h"
#include "../Input/Controls.h"
#include "../Input/Input.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_dx11.h"

#include <Urho3D/Graphics/GraphicsImpl.h>
void MyUI::HandleSDLRawInput(StringHash eventType, VariantMap& eventData)
{

	using namespace SDLRawInput;
	auto sdl_event = static_cast<SDL_Event*>(eventData[P_SDLEVENT].GetVoidPtr());
	ImGui_ImplSDL2_ProcessEvent(sdl_event);
}
void MyUI::HandleAllViewRenderEnd(StringHash eventType, VariantMap& eventData)
{
	RenderUI();
}
void MyUI::RenderUI()
{

	// Our state
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	URHO3D_PROFILE(RenderImGui);
	if (GRAPHICS->IsDeviceLost())return;


	SetGlobalVar("IsInWindow", ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));
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


	{
		auto loc = LOCALIZATION;

		ImGui::Begin(loc->Get("MyWindow").CString());

		{

			auto id = CACHE->GetResource<Texture2D>("Textures/hehe.png")->GetShaderResourceView();
			ImGui::Image(id, ImVec2(50.f, 50.f));

		}

		int i = loc->GetLanguageIndex();
		if (ImGui::Button(loc->Get("lang").CString(), ImVec2(200, 50)))
		{
			if (++i >= loc->GetNumLanguages())i = 0;
			loc->SetLanguage(i);
		}



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
MyUI::MyUI(Context* context) :Object(context)
{
}

void MyUI::Init()
{
        auto g = GRAPHICS;
        g_pd3dDevice = g->GetImpl()->GetDevice();
        g_pd3dDeviceContext = g->GetImpl()->GetDeviceContext();
        g_pSwapChain = g->GetImpl()->GetSwapChain();

        auto window = g->GetWindow();



        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplSDL2_InitForD3D(window);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

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



        SubscribeToEvent(E_ENDRENDERING, URHO3D_HANDLER(MyUI, HandleAllViewRenderEnd));
        SubscribeToEvent(E_SDLRAWINPUT, URHO3D_HANDLER(MyUI, HandleSDLRawInput));
}
