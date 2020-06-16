#pragma once
#include <Urho3D/Urho3DAll.h>


class CameraController:public LogicComponent
{

	URHO3D_OBJECT(CameraController, LogicComponent)

	float newZoom;
	IntVector2 pos;

public:
	CameraController(Context* context);
	void Update(StringHash eventType, VariantMap & eventData);
	virtual void Start();

	Urho3D::Key keyUp_, keyDown_, keyLeft_, keyRight_;

	float moveSpeed_;

	void HandleMouseMove(StringHash eventType, VariantMap & eventData);
	void MoveTo(const Vector2& To);
	
};