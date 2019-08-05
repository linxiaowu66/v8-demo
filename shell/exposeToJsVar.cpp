#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include "shell.h"

using v8::Local;
using v8::String;
using v8::PropertyCallbackInfo;
using v8::External;
using v8::Object;
using v8::Value;

extern char version[100];
extern const char *ToCString(const String::Utf8Value &value);

void VersionGetter(Local<String> property,
                   const PropertyCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), version).ToLocalChecked());
}

void
VersionSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> &info) {
  String::Utf8Value str(info.GetIsolate(), value);
  const char *result = ToCString(str);
  strncpy(version, result, sizeof(version));
}

void GetPointX(Local<String> property,
                   const PropertyCallbackInfo<Value> &info) {
  printf("GetPointX is calling\n");

  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<Point*>(ptr)->x_;
  info.GetReturnValue().Set(value);
}
void SetPointX(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> &info) {
  printf("SetPointX is calling\n");

  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<Point*>(ptr)->x_ = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
}


void GetPointY(Local<String> property,
               const PropertyCallbackInfo<Value> &info) {
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<Point*>(ptr)->y_;
  info.GetReturnValue().Set(value);
}
void SetPointY(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void> &info) {
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<Point*>(ptr)->y_ = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
}