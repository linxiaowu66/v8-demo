#include <include/v8.h>
#include <include/libplatform/libplatform.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#include "./shell.h"

using std::map;
using std::string;
using v8::Isolate;
using v8::String;
using v8::Local;
using v8::TryCatch;
using v8::HandleScope;
using v8::Context;
using v8::Script;
using v8::Message;
using v8::ScriptOrigin;
using v8::MaybeLocal;
using v8::Value;
using v8::NewStringType;
using v8::Platform;
using v8::External;
using v8::Object;

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

// Convert a JavaScript string to a std::string.  To not bother too
// much with string encodings we just use ascii.
string ObjectToString(Isolate* isolate, Local<Value> value) {
  String::Utf8Value utf8_value(isolate, value);
  return string(*utf8_value);
}

void ReportException(Isolate* isolate, TryCatch* try_catch) {
    HandleScope handle_scope(isolate);
    String::Utf8Value exception(isolate, try_catch->Exception());
    const char* exception_string = ToCString(exception);
    Local<Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        fprintf(stderr, "%s\n", exception_string);
    } else {
        // Print (filename):(line number): (message).
        String::Utf8Value filename(isolate,
                                       message->GetScriptOrigin().ResourceName());
        Local<Context> context(isolate->GetCurrentContext());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber(context).FromJust();
        fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        String::Utf8Value sourceline(
                isolate, message->GetSourceLine(context).ToLocalChecked());
        const char* sourceline_string = ToCString(sourceline);
        fprintf(stderr, "%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn(context).FromJust();
        for (int i = 0; i < start; i++) {
            fprintf(stderr, " ");
        }
        int end = message->GetEndColumn(context).FromJust();
        for (int i = start; i < end; i++) {
            fprintf(stderr, "^");
        }
        fprintf(stderr, "\n");
        Local<Value> stack_trace_string;
        if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString() &&
            Local<String>::Cast(stack_trace_string)->Length() > 0) {
            String::Utf8Value stack_trace(isolate, stack_trace_string);
            const char* stack_trace_string = ToCString(stack_trace);
            fprintf(stderr, "%s\n", stack_trace_string);
        }
    }
}

// Executes a string within the current v8 context.
bool ExecuteString(Isolate* isolate, Local<String> source,
                   Local<Value> name, bool print_result,
                   bool report_exceptions) {
    HandleScope handle_scope(isolate);
    TryCatch try_catch(isolate);
    ScriptOrigin origin(name);
    Local<Context> context(isolate->GetCurrentContext());
    Local<Script> script;
    if (!Script::Compile(context, source, &origin).ToLocal(&script)) {
        // Print errors that happened during compilation.
        if (report_exceptions)
            ReportException(isolate, &try_catch);
        return false;
    } else {
        Local<Value> result;
        if (!script->Run(context).ToLocal(&result)) {
            assert(try_catch.HasCaught());
            // Print errors that happened during execution.
            if (report_exceptions)
                ReportException(isolate, &try_catch);
            return false;
        } else {
            assert(!try_catch.HasCaught());
            if (print_result && !result->IsUndefined()) {
                // If all went well and the result wasn't undefined then print
                // the returned value.
                String::Utf8Value str(isolate, result);
                const char* cstr = ToCString(str);
                printf("%s\n", cstr);
            }
            return true;
        }
    }
}

// Reads a file into a v8 string.
MaybeLocal<String> ReadFile(Isolate *isolate, const char *name) {
  FILE *file = fopen(name, "rb");
  if (file == NULL) return MaybeLocal<String>();
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);
  char *chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&chars[i], 1, size - i, file);
    if (ferror(file)) {
      fclose(file);
      return MaybeLocal<String>();
    }
  }
  fclose(file);
  MaybeLocal<String> result = String::NewFromUtf8(
      isolate, chars, NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}

// The read-eval-execute loop of the shell.
void RunShell(Local<Context> context, Platform *platform) {
  fprintf(stderr, "V8 version %s [sample shell]\n", v8::V8::GetVersion());
  static const int kBufferSize = 256;
  // Enter the execution environment before evaluating any code.
  Context::Scope context_scope(context);
  Local<String> name(
      String::NewFromUtf8(context->GetIsolate(), "(shell)",
                              NewStringType::kNormal).ToLocalChecked());
  while (true) {
    char buffer[kBufferSize];
    fprintf(stderr, "> ");
    char *str = fgets(buffer, kBufferSize, stdin);
    if (str == NULL) break;
    HandleScope handle_scope(context->GetIsolate());
    ExecuteString(
        context->GetIsolate(),
        String::NewFromUtf8(context->GetIsolate(), str,
                                NewStringType::kNormal).ToLocalChecked(),
        name, true, true);
    while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
      continue;
  }
  fprintf(stderr, "\n");
}

// Utility function that extracts the C++ map pointer from a wrapper
// object.
map<string, string>* UnwrapMap(Local<Object> obj) {
  Local<External> field = Local<External>::Cast(obj->GetInternalField(0));
  void* ptr = field->Value();
  return static_cast<map<string, string>*>(ptr);
}