#include "JsDebugger.h"
#include "V8GlobalHelpers.h"
#include "JniLocalRef.h"
#include <assert.h>
#include <android/log.h>

using namespace std;
using namespace tns;

JsDebugger::JsDebugger()
{
}

void JsDebugger::Init(v8::Isolate *isolate, const string& packageName, jobject jsDebugger)
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::Init called");

	s_isolate = isolate;
	s_packageName = packageName;

	JEnv env;
	s_jsDebugger = env.NewGlobalRef(jsDebugger);

	s_JsDebuggerClass = env.FindClass("com/tns/JsDebugger");
	assert(s_JsDebuggerClass != nullptr);

	//what is enqueue message used for
	s_EnqueueMessage = env.GetMethodID(s_JsDebuggerClass, "enqueueMessage", "(Ljava/lang/String;)V");
	assert(s_EnqueueMessage != nullptr);

	//enableAgent.start(.stop) what is it used for ?
	s_EnableAgent = env.GetMethodID(s_JsDebuggerClass, "enableAgent", "(Z)V");
	assert(s_EnableAgent != nullptr);


}

string JsDebugger::GetPackageName()
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::GetPackageName");
	return s_packageName;
}

/* *
 * private method that takes debug message as json from v8
 * after it gets the message the message handler passes it to enqueueMessage method in java
 */
void JsDebugger::MyMessageHandler(const v8::Debug::Message& message)
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::MyMessageHandler");

	if (s_jsDebugger == nullptr)
	{
		__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger s_jsDebugger is null...returning....");
		return;
	}

	auto json = message.GetJSON();
	auto str = ConvertToString(json);

	JEnv env;
	JniLocalRef s(env.NewStringUTF(str.c_str()));

	env.CallVoidMethod(s_jsDebugger, s_EnqueueMessage, (jstring)s);
}

/* *
 * sets who will handle the messages when they start comming from v8
 */
void JsDebugger::Enable()
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::Enable called");
	auto isolate = s_isolate;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

	v8::Debug::SetMessageHandler(MyMessageHandler);
}

/* *
 * the message that come from v8 will not be handled anymore
 */
void JsDebugger::Disable()
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::Disable called");
	auto isolate = s_isolate;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

	v8::Debug::SetMessageHandler(nullptr);
}

/* *
 * schedule a debugger break to happen when JavaScript code is run in the given isolate
 * (cli command: tns debug android --debug-brk) ?
 */
void JsDebugger::DebugBreak()
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::DebugBreak called");
	auto isolate = s_isolate;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

	v8::Debug::DebugBreak(isolate);
}

void JsDebugger::ProcessDebugMessages()
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::ProcessDebugMessages called");
	auto isolate = s_isolate;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handleScope(isolate);

	v8::Debug::ProcessDebugMessages();
}

void JsDebugger::SendCommand(uint16_t *cmd, int length)
{
	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::SendCommand called");
	auto isolate = s_isolate;

	v8::Debug::SendCommand(isolate, cmd, length, nullptr);
}

void JsDebugger::DebugBreakCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{

	__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger::DebugBreakCallback called");

	if (s_jsDebugger == nullptr)
	{
		__android_log_print(ANDROID_LOG_INFO, "TNS.JsDebugger", "JsDebugger s_jsDebugger is null...returning");
		return;
	}

	JEnv env;
	env.CallVoidMethod(s_jsDebugger, s_EnableAgent, JNI_FALSE);

	DebugBreak();
}


v8::Isolate* JsDebugger::s_isolate = nullptr;
jobject JsDebugger::s_jsDebugger = nullptr;
string JsDebugger::s_packageName = "";
jclass JsDebugger::s_JsDebuggerClass = nullptr;
jmethodID JsDebugger::s_EnqueueMessage = nullptr;
jmethodID JsDebugger::s_EnableAgent = nullptr;

