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
