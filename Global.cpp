#include "Global.h"
#include "Urho3DAliases.h"

#include "../Precompiled.h"

#include "../AngelScript/APITemplates.h"
#include "../Input/Controls.h"
#include "../Input/Input.h"

void Global::Test(const String & in)
{

	GRAPHICS->SetWindowTitle(in);
}

void Global::Test2(const String & in)
{
	auto s = CACHE->GetResource<ScriptFile>("d:/1.as");
	s->Execute(in);
}

Global::Global(Context* context) :Object(context)
{
}
void Global::Test()
{
	GRAPHICS->SetWindowTitle("ZHANGKAI 's Test() from script.");
}
static Global* GetGlobal()
{
	return GetScriptContext()->GetSubsystem<Global>();
}
void Global:: RegisterControls(asIScriptEngine*engine)
{
	RegisterObject<Global>(engine, "Global");
	engine->RegisterObjectMethod("Global", "void Test()", asMETHOD(Global, Test), asCALL_THISCALL);
	engine->RegisterObjectMethod("Global", "void Test(const String &in)", asMETHODPR(Global, Test,(const String &),void), asCALL_THISCALL);
	engine->RegisterGlobalFunction("Global@+ get_global()", asFUNCTION(GetGlobal), asCALL_CDECL);
	engine->RegisterObjectMethod("Global", "void Test2(const String &in)", asMETHODPR(Global, Test2, (const String &), void), asCALL_THISCALL);
}