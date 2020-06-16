#pragma once


#include <Urho3D/Urho3DAll.h>
#include "Urho3DAliases.h"
#include "CameraController.h"
#include "d3d11.h"
class Urho2DTest :public Application
{

	URHO3D_OBJECT(Urho2DTest,Application)

public:
	Urho2DTest(Context* context);
	virtual void Setup();
	virtual void Start();
	virtual void Stop();
protected:

	virtual String GetScreenJoystickPatchString()const { return String::EMPTY; }

	void InitMouseModeAndLoadL10nFiles(MouseMode mode, const String&  FileNames);

	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;
	Camera* camera_;

	float yaw_;
	float pitch_;
	Node* pickedNode = nullptr;
	bool IsInBasket = false;
	bool clockPause = false;

	MouseMode useMouseMode_;




private:
	void CreateConsoleAndDebugHud();
	void SubscribeToEvents();
	void CreateCollFromJson(Node* node, const String& FileName, const Sprite2D* spr2d,int index = 0);
	void HandleMouseModeRequest(StringHash eventType, VariantMap& eventData);
	/// Handle request for mouse mode change on web platform.
	void HandleMouseModeChange(StringHash eventType, VariantMap& eventData);
	/// Handle key down event to process key controls common to all samples.
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	/// Handle key up event to process key controls common to all samples.
	void HandleKeyUp(StringHash eventType, VariantMap& eventData);
	/// Handle scene update event to control camera's pitch and yaw for all samples.
	void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);

	void HandleUpdate(StringHash eventType, VariantMap& eventData);

	void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData);
	void HandleAllViewRenderEnd(StringHash eventType, VariantMap& eventData);
	void RenderImGui();
	Vector2 GetMousePosInWorld();
	/// Handle touch begin event to initialize touch input on desktop platform.
	void CreateUI();

	void CreateScene();
	void CreateImgui();

	void SetupViewport();
	int main22();
	/// Screen joystick index for navigational controls (mobile platforms only).
	unsigned screenJoystickIndex_;
	/// Screen joystick index for settings (mobile platforms only).
	unsigned screenJoystickSettingsIndex_;
	/// Pause flag.
	bool paused_;


	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
};

