#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./shell.h"

extern const char *ToCString(const v8::String::Utf8Value &value);
extern bool ExecuteString(v8::Isolate *isolate, v8::Local<v8::String> source,
                          v8::Local<v8::Value> name, bool print_result,
                          bool report_exceptions);
extern v8::MaybeLocal<v8::String> ReadFile(v8::Isolate *isolate, const char *name);

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo <v8::Value> &args) {
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args.GetIsolate(), args[i]);
    const char *cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const v8::FunctionCallbackInfo <v8::Value> &args) {
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value file(args.GetIsolate(), args[i]);
    if (*file == NULL) {
      args.GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file",
                                  v8::NewStringType::kNormal).ToLocalChecked());
      return;
    }
    v8::Local <v8::String> source;
    if (!ReadFile(args.GetIsolate(), *file).ToLocal(&source)) {
      args.GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file",
                                  v8::NewStringType::kNormal).ToLocalChecked());
      return;
    }
    if (!ExecuteString(args.GetIsolate(), source, args[i], false, false)) {
      args.GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(args.GetIsolate(), "Error executing file",
                                  v8::NewStringType::kNormal).ToLocalChecked());
      return;
    }
  }
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
void Quit(const v8::FunctionCallbackInfo <v8::Value> &args) {
  // If not arguments are given args[0] will yield undefined which
  // converts to the integer value 0.
  int exit_code =
      args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
  fflush(stdout);
  fflush(stderr);
  exit(exit_code);
}

void constructPoint(const v8::FunctionCallbackInfo <v8::Value> &args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  //start a handle scope
  v8::EscapableHandleScope handle_scope(isolate);

  //get an x and y
  double x = args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked();
  double y = args[1]->NumberValue(isolate->GetCurrentContext()).ToChecked();

  //generate a new point
  Point *point = new Point(x, y);

  args.This()->SetInternalField(0, v8::External::New(isolate, point));
}

void PointMulti(const v8::FunctionCallbackInfo <v8::Value> &args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  //start a handle scope
  v8::EscapableHandleScope handle_scope(isolate);


  v8::Local<v8::Object> self = args.Holder();
  v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<Point*>(ptr)->multi();

  args.GetReturnValue().Set(value);
}