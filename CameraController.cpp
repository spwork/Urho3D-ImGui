#include "CameraController.h"
#include "Urho3DAliases.h"	


CameraController::CameraController(Context* context):
	LogicComponent(context),keyUp_(KEY_W), keyDown_(KEY_S), keyLeft_(KEY_A), keyRight_(KEY_D),moveSpeed_(16.f)
{
	
}

void CameraController::Update(StringHash eventType, VariantMap & eventData)//(float timeStep)
{

	if (UI->GetFocusElement())return;

	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
	auto input = INPUT;

	if (input->GetKeyDown(keyUp_))
	{
		node_->Translate2D(Vector2::UP * moveSpeed_ * timeStep);
		
	}
	else if (input->GetKeyDown(keyDown_))
		node_->Translate2D(Vector2::DOWN * moveSpeed_ * timeStep);
	if (input->GetKeyDown(keyLeft_))
		node_->Translate2D(Vector2::LEFT * moveSpeed_ * timeStep);
	else if (input->GetKeyDown(keyRight_))
		node_->Translate2D(Vector2::RIGHT * moveSpeed_ * timeStep);
	else if (input->GetKeyDown(KEY_Q))
		node_->Rotate2D(-timeStep * 50.f);
	else if (input->GetKeyDown(KEY_E))
		node_->Rotate2D(timeStep * 50.f);

	static Camera* camera = 0;
	if (camera == 0)
		camera = node_->GetComponent<Camera>();

	newZoom *=  1 +input->GetMouseMoveWheel()*.2f;

	if (newZoom == camera->GetZoom())return;

	const float ZoomSpeed = 1.f;
	const float MaxZoom = 1.f;//1.f
	const float MinZoom = .02f;//.2f;

	if (newZoom > MaxZoom)
		newZoom = MaxZoom;
	else if (newZoom < MinZoom)
		newZoom = MinZoom;

	if (Abs(camera->GetZoom() - newZoom) < ZoomSpeed * timeStep)
		camera->SetZoom(newZoom);
	else if (camera->GetZoom() < newZoom)
		camera->SetZoom(camera->GetZoom() + ZoomSpeed * timeStep);
	else if (camera->GetZoom() > newZoom)
		camera->SetZoom(camera->GetZoom() - ZoomSpeed * timeStep);


	
}

void CameraController::Start()
{

	newZoom = GetComponent<Camera>()->GetZoom();
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CameraController, Update));
	SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(CameraController, HandleMouseMove));

}

void CameraController::HandleMouseMove(StringHash eventType, VariantMap & eventData)
{
	if (eventData[MouseMove::P_BUTTONS].GetInt() & 4)
	{
		auto graphics = GRAPHICS;
		int dx = eventData[MouseMove::P_DX].GetInt();
		int dy = eventData[MouseMove::P_DY].GetInt();

		float zoom = GetComponent<Camera>()->GetZoom();
		zoom = 1 / zoom;
		node_->Translate2D(Vector2(-dx * zoom * .01f,dy * zoom * .01f));
	}
	
}

void CameraController::MoveTo(const Vector2 & To)
{
	auto pos = node_->GetPosition();
	pos.x_ = To.x_;
	pos.y_ = To.y_;
	node_->SetPosition(pos);
}