/**
 * v8/samples/shell.cpp的简化版本，完整版本：https://chromium.googlesource.com/v8/v8/+/branch-heads/6.8/samples/shell.cc
 */

#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

/**
 * This sample program shows how to implement a simple javascript shell
 * based on V8.  This includes initializing V8 with command line options,
 * creating global functions, compiling and executing strings.
 *
 * For a more sophisticated shell, consider using the debug shell D8.
 */
extern void VersionGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
extern void VersionSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info);
extern void PointMulti(const v8::FunctionCallbackInfo <v8::Value> &args);
extern void GetPointX(v8::Local<v8::String> property,
               const v8::PropertyCallbackInfo<v8::Value> &info);
extern void SetPointX(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info);
extern void GetPointY(v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Value> &info);
extern void SetPointY(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info);
extern void Print(const v8::FunctionCallbackInfo<v8::Value> &args);
extern void Load(const v8::FunctionCallbackInfo<v8::Value> &args);
extern void Quit(const v8::FunctionCallbackInfo<v8::Value> &args);
extern void constructPoint(const v8::FunctionCallbackInfo <v8::Value> &args);
extern void ReportException(v8::Isolate *isolate, v8::TryCatch *handler);

extern const char *ToCString(const v8::String::Utf8Value &value);

void RunShell(v8::Local<v8::Context> context, v8::Platform *platform);
v8::Local<v8::Context> CreateShellContext(v8::Isolate *isolate);


int main(int argc, char *argv[]) {
  // 初始化v8
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();

  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    // 初始化version
    strncpy(version, v8::V8::GetVersion(), sizeof(version));

    v8::Local<v8::Context> context = CreateShellContext(isolate);
    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }
    v8::Context::Scope context_scope(context);
    RunShell(context, platform.get());
  }
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}

// Creates a new execution environment containing the built-in
// functions.
v8::Local<v8::Context> CreateShellContext(v8::Isolate *isolate) {
  // Create a template for the global object.
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

  // Bind the global 'print' function to the C++ Print callback.
  global->Set(
      v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      v8::FunctionTemplate::New(isolate, Print));


  // Bind the global 'load' function to the C++ Load callback.
  global->Set(v8::String::NewFromUtf8(
      isolate, "load", v8::NewStringType::kNormal).ToLocalChecked(),
              v8::FunctionTemplate::New(isolate, Load));


  // Bind the 'quit' function
  global->Set(v8::String::NewFromUtf8(
      isolate, "quit", v8::NewStringType::kNormal).ToLocalChecked(),
              v8::FunctionTemplate::New(isolate, Quit));

  // Bind the 'version' variable
  global->SetAccessor(v8::String::NewFromUtf8(isolate, "version").ToLocalChecked(), VersionGetter, VersionSetter);

  /* Binding the Point Class */
  //create a pointer to a class template
  v8::Handle<v8::FunctionTemplate> point_templ = v8::FunctionTemplate::New(isolate, constructPoint);

  //assign the "Point" name to the new class template
  point_templ->SetClassName(v8::String::NewFromUtf8(isolate, "Point").ToLocalChecked());

  global->Set(v8::String::NewFromUtf8(
      isolate, "Point", v8::NewStringType::kNormal).ToLocalChecked(), point_templ);

  //access the class template
  v8::Handle<v8::ObjectTemplate> point_proto = point_templ->PrototypeTemplate();

  point_proto->Set(v8::String::NewFromUtf8(
      isolate, "multi", v8::NewStringType::kNormal).ToLocalChecked(), v8::FunctionTemplate::New(isolate, PointMulti));

  //access the instance pointer of our new class template
  v8::Handle<v8::ObjectTemplate> point_inst = point_templ->InstanceTemplate();

  //set the internal fields of the class as we have the Point class internally
  point_inst->SetInternalFieldCount(1);

  //associates the name "x" with its Get/Set functions
  point_inst->SetAccessor(v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), GetPointX, SetPointX);

  const v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
#ifdef USING_ADVANCED_GUIDE
  /* 这个版本是依据Advanced guide 写的，不过只能提供该变量在Js端的使用，不能新建class*/
  v8::Local<v8::ObjectTemplate> point_templ = v8::ObjectTemplate::New(isolate);
  point_templ->SetInternalFieldCount(1);
  point_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), GetPointX, SetPointX);
  point_templ->SetAccessor(v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), GetPointY, SetPointY);
  Point* p = new Point(11, 22);
  v8::Local<v8::Object> obj = point_templ->NewInstance(context).ToLocalChecked();
  obj->SetInternalField(0, v8::External::New(isolate, p));
  context->Global()->Set(context, v8::String::NewFromUtf8(isolate, "p").ToLocalChecked(), obj).ToChecked();
#endif
  return context;
}




