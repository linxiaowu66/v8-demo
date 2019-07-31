//
// Created by 林小兀 on 2019-07-31.
//

#include "v8.h"
#include "libplatform/libplatform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

using std::map;
using std::string;
using v8::Local;
using v8::Name;
using v8::PropertyCallbackInfo;
using v8::Value;
using v8::Isolate;
using v8::String;
using v8::NewStringType;
using v8::Object;

extern string ObjectToString(Isolate* isolate, Local<Value> value);
extern map<string, string>* UnwrapMap(Local<Object> obj);

void PointInterceptorGetter(
    Local<Name> name, const PropertyCallbackInfo<Value>& info) {
  if (name->IsSymbol()) return;

  // Fetch the map wrapped by this object.
  map<string, string>* obj = UnwrapMap(info.Holder());

  // Convert the JavaScript string to a std::string.
  string key = ObjectToString(info.GetIsolate(), Local<String>::Cast(name));

  printf("interceptor Getting for Point property has called, name[%s]\n", key.c_str());

  // 如果调用这个设置return，那么就不会再执行后面的Getter
//  info.GetReturnValue().Set(11);
}

void PointInterceptorSetter(
    Local<Name> name, Local<Value> value_obj,
    const PropertyCallbackInfo<Value>& info) {
  if (name->IsSymbol()) return;
  // Fetch the map wrapped by this object.
  map<string, string>* obj = UnwrapMap(info.Holder());


  // Convert the key and value to std::strings.
  string key = ObjectToString(info.GetIsolate(), Local<String>::Cast(name));
  string value = ObjectToString(info.GetIsolate(), value_obj);

  printf("interceptor Setting for Point property has called, name[%s] = value[%s]\n", key.c_str(), value.c_str());
}