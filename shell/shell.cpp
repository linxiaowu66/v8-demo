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
/**
 * This sample program shows how to implement a simple javascript shell
 * based on V8.  This includes initializing V8 with command line options,
 * creating global functions, compiling and executing strings.
 *
 * For a more sophisticated shell, consider using the debug shell D8.
 */
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate);
void VersionGetter(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
void VersionSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info);
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform);
extern bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
                   v8::Local<v8::Value> name, bool print_result,
                   bool report_exceptions);
void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
extern void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);
extern const char* ToCString(const v8::String::Utf8Value& value);

static char version[100] = "";

int main(int argc, char* argv[]) {
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

    v8::Isolate* isolate = v8::Isolate::New(create_params);
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
v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate) {
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

    global->SetAccessor(v8::String::NewFromUtf8(isolate, "version", v8::NewStringType::kInternalized).ToLocalChecked(), VersionGetter, VersionSetter);

    return v8::Context::New(isolate, NULL, global);
}

void VersionGetter(v8::Local<v8::String> property,
                   const v8::PropertyCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(version);
}

void VersionSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info) {
    strncpy(version, value->, sizeof(version));
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        const char* cstr = ToCString(str);
        printf("%s", cstr);
    }
    printf("\n");
    fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        v8::String::Utf8Value file(args.GetIsolate(), args[i]);
        if (*file == NULL) {
            args.GetIsolate()->ThrowException(
                    v8::String::NewFromUtf8(args.GetIsolate(), "Error loading file",
                                            v8::NewStringType::kNormal).ToLocalChecked());
            return;
        }
        v8::Local<v8::String> source;
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
void Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // If not arguments are given args[0] will yield undefined which
    // converts to the integer value 0.
    int exit_code =
            args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    fflush(stdout);
    fflush(stderr);
    exit(exit_code);
}


// Reads a file into a v8 string.
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
    FILE* file = fopen(name, "rb");
    if (file == NULL) return v8::MaybeLocal<v8::String>();
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char* chars = new char[size + 1];
    chars[size] = '\0';
    for (size_t i = 0; i < size;) {
        i += fread(&chars[i], 1, size - i, file);
        if (ferror(file)) {
            fclose(file);
            return v8::MaybeLocal<v8::String>();
        }
    }
    fclose(file);
    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
            isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
    delete[] chars;
    return result;
}

// The read-eval-execute loop of the shell.
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform) {
    fprintf(stderr, "V8 version %s [sample shell]\n", v8::V8::GetVersion());
    static const int kBufferSize = 256;
    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(
            v8::String::NewFromUtf8(context->GetIsolate(), "(shell)",
                                    v8::NewStringType::kNormal).ToLocalChecked());
    while (true) {
        char buffer[kBufferSize];
        fprintf(stderr, "> ");
        char* str = fgets(buffer, kBufferSize, stdin);
        if (str == NULL) break;
        v8::HandleScope handle_scope(context->GetIsolate());
        ExecuteString(
                context->GetIsolate(),
                v8::String::NewFromUtf8(context->GetIsolate(), str,
                                        v8::NewStringType::kNormal).ToLocalChecked(),
                name, true, true);
        while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
            continue;
    }
    fprintf(stderr, "\n");
}


