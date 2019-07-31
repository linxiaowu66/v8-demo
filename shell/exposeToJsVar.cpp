#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include "shell.h"

extern const char *ToCString(const v8::String::Utf8Value &value);

void VersionGetter(v8::Local<v8::String> property,
                   const v8::PropertyCallbackInfo<v8::Value> &info) {
  info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), version).ToLocalChecked());
}

void
VersionSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info) {
  v8::String::Utf8Value str(info.GetIsolate(), value);
  const char *result = ToCString(str);
  strncpy(version, result, sizeof(version));
}

void GetPointX(v8::Local<v8::String> property,
                   const v8::PropertyCallbackInfo<v8::Value> &info) {
  v8::Local<v8::Object> self = info.Holder();
  v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<Point*>(ptr)->x_;
  info.GetReturnValue().Set(value);
}
void SetPointX(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info) {
  v8::Local<v8::Object> self = info.Holder();
  v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<Point*>(ptr)->x_ = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
}


void GetPointY(v8::Local<v8::String> property,
               const v8::PropertyCallbackInfo<v8::Value> &info) {
  v8::Local<v8::Object> self = info.Holder();
  v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<Point*>(ptr)->y_;
  info.GetReturnValue().Set(value);
}
void SetPointY(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info) {
  v8::Local<v8::Object> self = info.Holder();
  v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<Point*>(ptr)->y_ = value->Int32Value(info.GetIsolate()->GetCurrentContext()).ToChecked();
}