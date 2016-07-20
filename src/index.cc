// hello.cc
#include <node.h>
#include <iostream>
#include "word.h"

using namespace v8;
using namespace std;
using namespace word;


void WordInit(Local<Object> exports ,Local<Object> module) {
	Word::Init(exports);
	
}
NODE_MODULE(word , WordInit);

/**
namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Handle;
using  namespace std;

class Word: public node::ObjectWrap {
public:
	static void Init(v8::Local<v8::Object> exports);
private:
	explicit Word(Handle<Object> config);
	~Word();
	static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
};

void Method(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  //args[0] Value
  Handle<Object> object = Handle<Object>::Cast(args[0]);
  Handle<Value> fieldValue = object->Get(String::NewFromUtf8(isolate, "file"));
  String::Utf8Value ascii2(fieldValue);
  printf("%s\n", *ascii2);
  args.GetReturnValue().Set( fieldValue);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "latte", Method);
}

NODE_MODULE(addon, init)

}
*/ 