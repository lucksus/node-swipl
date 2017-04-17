#ifndef NODE_LIBSWIPL_H_
#define NODE_LIBSWIPL_H_

#include <nan.h>
#include <SWI-Prolog.h>

using v8::String;
using v8::Object;
using v8::Value;
using Nan::HandleScope;
using v8::Number;
using FunctionCallbackInfo = Nan::FunctionCallbackInfo<v8::Value>;
using Nan::Persistent;

void Initialise(const FunctionCallbackInfo&);

void TermType(const FunctionCallbackInfo&);

void Cleanup(const FunctionCallbackInfo&);

class Query : public Nan::ObjectWrap {
 public:
  static const int OPEN = 1;
  static const int CLOSED = 0;
  static void Init(v8::Handle<v8::Object> target);

 private:
  Query();
  ~Query();

  static void Open(const FunctionCallbackInfo& args);
  static void NextSolution(const FunctionCallbackInfo& args);
  static void Close(const FunctionCallbackInfo& args);
  static void Exception(const FunctionCallbackInfo& args);

  int open;
  qid_t qid;
  term_t term;
  Persistent<Object> varnames;
  int term_len;
  int (*cb_log)(const char*, ...);
};

#endif /* NODE_LIBSWIPL_H_ */
