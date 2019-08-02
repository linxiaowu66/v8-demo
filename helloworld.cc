#include <stdio.h>
#include <stdlib.h>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"


using namespace v8;
using namespace std;

int main(int argc, char * argv[]) {
  // 1、初始化V8
  V8::InitializeICUDefaultLocation(argv[0]);
  V8::InitializeExternalStartupData(argv[0]);
  unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  // 2、创建一个新的隔离区，并将这个隔离区置为当前使用
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);

    // 3、创建一个栈分配的句柄范围
    HandleScope handle_scope(isolate);

    // 4、创建一个上下文
    Local<Context> context = Context::New(isolate);

    // 5、进入上下文编译和运行脚本
    Context::Scope context_scope(context);
    {
      Local<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World'", NewStringType::kNormal).ToLocalChecked();

      Local<Script> script = Script::Compile(context, source).ToLocalChecked();

      Local<Value> result = script->Run(context).ToLocalChecked();

      String::Utf8Value utf8(isolate, result);

      printf("%s\n", *utf8);
    }
  }

  // 6、销毁isolate以及使用过的buffer,并关掉进程
  isolate->Dispose();
  V8::Dispose();
  V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}
