/*  $Id$

 node.js addon for SWI-Prolog

 Author:        Tom Klonikowski
 E-mail:        klonik_t@informatik.haw-hamburg.de
 Copyright (C): 2012, Tom Klonikowski

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cstdlib>
#include <SWI-Prolog.h>

#include "libswipl.h"

template<typename T>
using Handle = v8::Handle<T>;

using v8::FunctionTemplate;
using v8::Local;
using v8::Array;

Handle<Object> ExportSolution(term_t t, int len, Handle<Object> result_terms,
    Handle<Object> varnames);

module_t GetModule(const FunctionCallbackInfo& args, int idx) {
  module_t mo = NULL;
  if (args.Length() > idx && !(args[idx]->IsUndefined() || args[idx]->IsNull())
      && args[idx]->IsString()) {
    mo = PL_new_module(PL_new_atom(*String::Utf8Value(args[idx])));
  }
  return mo;
}

void Initialise(const FunctionCallbackInfo& info) {
  int rval;
  const char *plav[3];
  HandleScope scope;

  /* make the argument vector for Prolog */

  String::Utf8Value str(info[0]);
  plav[0] = *str;
  plav[1] = "--quiet";
  plav[2] = nullptr;

  /* initialise Prolog */

  rval = PL_initialise(2, (char **) plav);

  info.GetReturnValue().Set(Nan::New<Number>(rval));
}

Handle<Value> CreateException(const char *msg) {
  Handle<Object> result = Nan::New<Object>();
  result->Set(Nan::New<String>("exc").ToLocalChecked(), Nan::New<String>(msg).ToLocalChecked());
  return result;
}

const char* GetExceptionString(term_t term) {
  char *msg;
  term_t msgterms = PL_new_term_refs(2);
  PL_put_term(msgterms, term);
  int rval = PL_call_predicate(NULL, PL_Q_NODEBUG,
      PL_predicate("message_to_string", 2, NULL), msgterms);
  if (rval) {
    rval = PL_get_chars(msgterms + 1, &msg, CVT_ALL);
    return msg;
  } else {
    return "unknown error";
  }
}

/**
 *
 */
void TermType(const FunctionCallbackInfo& info) {
  int rval = 0;
  HandleScope scope;
  term_t term = PL_new_term_ref();
  rval = PL_chars_to_term(*String::Utf8Value(info[0]), term);
  if (rval) {
    rval = PL_term_type(term);
  }
  info.GetReturnValue().Set(Nan::New<Number>(rval));
}

void ExportTerm(term_t t, Handle<Object> result, Handle<Object> varnames) {
  int rval = 0;
  Handle<Value> key = varnames->Get(t);
  Handle<Value> val;
  char *c;
  int i = 0;
  double d = 0.0;
  int type = PL_term_type(t);
  if (!key->IsUndefined()) {
    switch (type) {
    case PL_FLOAT:
      rval = PL_get_float(t, &d);
      val = Nan::New<Number>(d);
      break;
    case PL_INTEGER:
      rval = PL_get_integer(t, &i);
      val = Nan::New<v8::Integer>(i);
      break;
    default:
      rval = PL_get_chars(t, &c, CVT_ALL);
      val = Nan::New<String>(c).ToLocalChecked();
      break;
    }
    if (rval) {
      result->Set(key, val);
    }
  }
}

Handle<Object> ExportSolution(term_t t, int len, Handle<Object> result_terms,
    Handle<Object> varnames) {
  for (int j = 0; j < len; j++) {
    ExportTerm(t + j, result_terms, varnames);
  }
  return result_terms;
}

void Cleanup(const FunctionCallbackInfo& args) {
  HandleScope scope;
  int rval = PL_cleanup(0);
  args.GetReturnValue().Set(Nan::New<Number>(rval));
}

Query::Query() {
}

Query::~Query() {
}


void Query::Init(Handle<Object> target) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(Open);
  tpl->SetClassName(Nan::New<v8::String>("Query").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(3);
  // Prototype
  tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("next_solution").ToLocalChecked(),
	  Nan::New<FunctionTemplate>(NextSolution));
  tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("close").ToLocalChecked(),
	  Nan::New<FunctionTemplate>(Close));
  tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("exception").ToLocalChecked(),
	  Nan::New<FunctionTemplate>(Exception));

  
  auto constructor = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(target, Nan::New<v8::String>("Query").ToLocalChecked(), constructor);
}

void Query::Open(const FunctionCallbackInfo& args) {
  HandleScope scope;

  Query* obj = new Query();
  obj->cb_log = NULL; //&printf;

  module_t module = GetModule(args, 2);
  const char *module_name =
      module ? PL_atom_chars(PL_module_name(module)) : NULL;

  if (args.Length() > 1 && args[0]->IsString() && args[1]->IsArray()) {
    int rval = 0;
    String::Utf8Value predicate(args[0]);
	if (obj->cb_log) {
		obj->cb_log("Query::Open predicate: %s(", *predicate);
	}
    Handle<Array> terms = Handle<Array>::Cast(args[1]);
    predicate_t p = PL_predicate(*predicate, terms->Length(), module_name);
    obj->term = PL_new_term_refs(terms->Length());
    obj->term_len = terms->Length();
	obj->varnames.Reset(Nan::New<Object>());
    term_t t = obj->term;
    for (unsigned int i = 0; i < terms->Length(); i++) {
      Local<Value> v = terms->Get(i);
      if (v->IsInt32()) {
        if (obj->cb_log)
          obj->cb_log("%i", v->Int32Value());
        rval = PL_put_integer(t, v->Int32Value());
      } else if (v->IsNumber()) {
        if (obj->cb_log)
          obj->cb_log("%f", v->NumberValue());
        rval = PL_put_float(t, v->NumberValue());
      } else {
        String::Utf8Value s(v);
        if (obj->cb_log)
          obj->cb_log("%s", *s);
        rval = PL_chars_to_term(*s, t);
        int type = PL_term_type(t);
        if (obj->cb_log)
          obj->cb_log(" [%i]", type);
        switch (type) {
        case PL_VARIABLE:
          Nan::New(obj->varnames)->Set(t, Nan::New<String>(*s).ToLocalChecked());
          break;
        case PL_ATOM:
        case PL_TERM:
        default:
          break;
        }
      }
      if (obj->cb_log)
        obj->cb_log(", ");
      t = t + 1;
    }

    obj->qid = PL_open_query(module, PL_Q_CATCH_EXCEPTION, p, obj->term);

    if (obj->cb_log)
      obj->cb_log(") #%li\n", obj->qid);

    if (obj->qid == 0) {
      Nan::ThrowError(
          v8::Exception::Error(
              Nan::New<String>("not enough space on the environment stack").ToLocalChecked()));
	  args.GetReturnValue().SetUndefined();
    } else if (rval == 0) {
		Nan::ThrowError(
          v8::Exception::Error(
              Nan::New<String>(GetExceptionString(PL_exception(obj->qid))).ToLocalChecked()));
		args.GetReturnValue().SetUndefined();
    } else {
      obj->open = OPEN;
      obj->Wrap(args.This());
	  args.GetReturnValue().Set(args.This());
    }
  } else {
    Nan::ThrowError(
        v8::Exception::SyntaxError(Nan::New<String>("invalid arguments (pred, [ args ], module)").ToLocalChecked()));
	args.GetReturnValue().SetUndefined();
  }
}

void Query::NextSolution(const FunctionCallbackInfo& args) {
  HandleScope scope;
  int rval = 0;

  Query* obj = ObjectWrap::Unwrap<Query>(args.This());

  if (obj->cb_log)
    obj->cb_log("Query::NextSolution #%li", obj->qid);

  if (obj->open == OPEN) {
    rval = PL_next_solution(obj->qid);
    if (obj->cb_log)
      obj->cb_log(": %i\n", rval);

    if (rval) {
		args.GetReturnValue().Set(ExportSolution(obj->term, obj->term_len, Nan::New<Object>(), Nan::New(obj->varnames)));
    } else {
		args.GetReturnValue().Set(false);
    }
  } else {
	  Nan::ThrowError(v8::Exception::Error(Nan::New<String>("query is closed").ToLocalChecked()));
	  args.GetReturnValue().SetUndefined();
  }
}

void Query::Exception(const FunctionCallbackInfo& args) {
  HandleScope scope;

  Query* obj = ObjectWrap::Unwrap<Query>(args.This());
  if (obj->cb_log)
    obj->cb_log("Query::Exception #%li\n", obj->qid);
  term_t term = PL_exception(obj->qid);

  if (term) {    
	args.GetReturnValue().Set(CreateException(GetExceptionString(term)));
  } else {
	  args.GetReturnValue().Set(Nan::True());
  }
}

void Query::Close(const FunctionCallbackInfo& args) {
  HandleScope scope;

  Query* obj = ObjectWrap::Unwrap<Query>(args.This());
  if (obj->open == OPEN) {
    if (obj->cb_log)
      obj->cb_log("Query::Close #%li\n", obj->qid);
    PL_close_query(obj->qid);
    obj->open = CLOSED;
  }

  args.GetReturnValue().Set(true);
}


NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New<String>("initialise").ToLocalChecked(),
	  Nan::GetFunction(Nan::New<FunctionTemplate>(Initialise)).ToLocalChecked());

  Nan::Set(target, Nan::New<String>("term_type").ToLocalChecked(),
	  Nan::GetFunction(Nan::New<FunctionTemplate>(TermType)).ToLocalChecked());

  Nan::Set(target, Nan::New<String>("cleanup").ToLocalChecked(),
	  Nan::GetFunction(Nan::New<FunctionTemplate>(Cleanup)).ToLocalChecked());
  
  Query::Init(target);
}

NODE_MODULE(libswipl, init)
