#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./shell.h"

using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Name;
using v8::String;
using v8::MaybeLocal;
using v8::FunctionCallbackInfo;
using v8::NewStringType;
using v8::EscapableHandleScope;
using v8::External;
using v8::Object;
using v8::Value;

extern const char *ToCString(const String::Utf8Value &value);
extern bool ExecuteString(Isolate *isolate, Local<String> source,
                          Local<Value> name, bool print_result,
                          bool report_exceptions);
extern MaybeLocal<String> ReadFile(Isolate *isolate, const char *name);

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const FunctionCallbackInfo <Value> &args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    String::Utf8Value str(args.GetIsolate(), args[i]);
    const char *cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const FunctionCallbackInfo <Value> &args) {
  for (int i = 0; i < args.Length(); i++) {
    HandleScope handle_scope(args.GetIsolate());
    String::Utf8Value file(args.GetIsolate(), args[i]);
    if (*file == NULL) {
      args.GetIsolate()->ThrowException(
          String::NewFromUtf8(args.GetIsolate(), "Error loading file",
                                  NewStringType::kNormal).ToLocalChecked());
      return;
    }
    Local <String> source;
    if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
      args.GetIsolate()->ThrowException(
          String::NewFromUtf8(args.GetIsolate(), "Error loading file",
                                  NewStringType::kNormal).ToLocalChecked());
      return;
    }
    if (!ExecuteString(args.GetIsolate(), source, args[i], false, false)) {
      args.GetIsolate()->ThrowException(
          String::NewFromUtf8(args.GetIsolate(), "Error executing file",
                                  NewStringType::kNormal).ToLocalChecked());
      return;
    }
  }
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
void Quit(const FunctionCallbackInfo <Value> &args) {
  // If not arguments are given args[0] will yield undefined which
  // converts to the integer value 0.
  int exit_code =
      args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
  fflush(stdout);
  fflush(stderr);
  exit(exit_code);
}

void constructPoint(const FunctionCallbackInfo <Value> &args) {
  Isolate* isolate = Isolate::GetCurrent();

  //get an x and y
  double x = args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked();
  double y = args[1]->NumberValue(isolate->GetCurrentContext()).ToChecked();

  //generate a new point
  Point *point = new Point(x, y);

  args.This()->SetInternalField(0, External::New(isolate, point));
}

void PointMulti(const FunctionCallbackInfo <Value> &args) {
  Isolate* isolate = Isolate::GetCurrent();
  //start a handle scope
  HandleScope handle_scope(isolate);


  Local<Object> self = args.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();

  // 这里直接调用已经实例化的Point类的成员方法multi，并拿到结果
  int value = static_cast<Point*>(ptr)->multi();

  args.GetReturnValue().Set(value);
}