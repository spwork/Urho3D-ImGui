#pragma once
#include <Urho3D/Urho3DAll.h>
class Global:public Object
{
	URHO3D_OBJECT(Global,Object)
private:

public:
	void Test();
	void Test(const String &in);
	void Test2(const String &in);
	Global(Context * context);
	static void RegisterControls(asIScriptEngine * engine);
};
