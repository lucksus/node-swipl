#include <cstdlib>
#include <nan.h>
#include <SWI-Prolog.h>

using v8::Array;
using v8::String;
using v8::Object;
using v8::Value;
using v8::Local;
using v8::Number;
using v8::FunctionTemplate;
using Nan::FunctionCallbackInfo;
using Nan::Persistent;
using Nan::HandleScope;

module_t GetModule(const FunctionCallbackInfo<Value>& args, int idx) {
    module_t mo = NULL;
    if (args.Length() > idx &&
        !(args[idx]->IsUndefined() || args[idx]->IsNull()) &&
        args[idx]->IsString()) {
        mo = PL_new_module(PL_new_atom(*String::Utf8Value(args[idx])));
    }
    return mo;
}

// Installs wrapper that extracts bindings.

void InstallWrapper() {
    char const *goal = "assert(nswi_(A,Bs):-(atom_to_term(A,G,Bs),call(G)))";
    fid_t fid = PL_open_foreign_frame();
    term_t g = PL_new_term_ref();
    PL_chars_to_term(goal, g);
    PL_call(g, NULL);
    PL_discard_foreign_frame(fid);
}

void Initialise(const FunctionCallbackInfo<Value>& info) {
    int rval;
    const char *plav[3];
    HandleScope scope;

    /* Make the argument vector for Prolog. */

    String::Utf8Value str(info[0]);
    plav[0] = *str;
    plav[1] = "--quiet";
    plav[2] = nullptr;

    /* Initialise Prolog */

    rval = PL_initialise(2, (char **) plav);

    if (rval == 1) {
        InstallWrapper();
    }

    info.GetReturnValue().Set(Nan::New<Number>(rval));
}

void Cleanup(const FunctionCallbackInfo<Value>& args) {
    HandleScope scope;
    int rval = PL_cleanup(0);
    args.GetReturnValue().Set(Nan::New<Number>(rval));
}

Local<Value> CreateException(const char *msg) {
    Local<Object> result = Nan::New<Object>();
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

Local<Value> ExportTermValue(term_t t);

// Exports the compound term into an object.

Local<Value> ExportCompound(term_t t) {
    Local<Object> compound = Nan::New<Object>();
    atom_t n;
    int arity;
    const char *name;
    if (!PL_get_compound_name_arity(t, &n, &arity)) {
        Nan::ThrowError("PL_get_compound_name_arity failed.");
        return Nan::Null();
    }
    name = PL_atom_chars(n);
    Local<Array> args = Nan::New<Array>();
    for (int i = 1; i <= arity; ++i) {
        term_t arg_t = PL_new_term_ref();
        if (!PL_get_arg(i, t, arg_t)) {
            Nan::ThrowError("PL_get_arg failed.");
            return Nan::Null();
        }
        args->Set(i - 1, ExportTermValue(arg_t));
    }
    compound->Set(Nan::New<String>("name").ToLocalChecked(),
        Nan::New<String>(name).ToLocalChecked());
    compound->Set(Nan::New<String>("args").ToLocalChecked(), args);
    return compound;
}

// Exports PL_FLOAT as number.

Local<Value> ExportFloat(term_t t) {
    double d = 0.0;
    if (!PL_get_float(t, &d)) {
        Nan::ThrowError("PL_get_float failed.");
        return Nan::Null();
    }
    return Nan::New<Number>(d);
}

// Exports PL_INTEGER as number.

Local<Value> ExportInteger(term_t t) {
    int i = 0;
    if (!PL_get_integer(t, &i)) {
        Nan::ThrowError("PL_get_integer failed.");
        return Nan::Null();
    }
    return Nan::New<Number>(i);
}

// Exports PL_ATOM.

Local<Value> ExportAtom(term_t t) {
    char *c;
    if (!PL_get_atom_chars(t, &c)) {
        Nan::ThrowError("PL_get_atom_chars failed.");
        return Nan::Null();
    }
    return Nan::New<String>(c).ToLocalChecked();
}

// Exports PL_STRING.

Local<Value> ExportString(term_t t) {
    char *c;
    size_t len;
    if (!PL_get_string_chars(t, &c, &len)) {
        Nan::ThrowError("PL_get_string_chars failed.");
    }
    return Nan::New<String>(c).ToLocalChecked();
}

// Exports PL_LIST_PAIR.

Local<Value> ExportListPair(term_t t) {
    Local<Object> pair = Nan::New<Object>();
    term_t head = PL_new_term_ref();
    term_t tail = PL_new_term_ref();
    if (!PL_get_list(t, head, tail)) {
        Nan::ThrowError("PL_get_list failed.");
    };
    pair->Set(Nan::New<String>("head").ToLocalChecked(),
        ExportTermValue(head));
    pair->Set(Nan::New<String>("tail").ToLocalChecked(),
        ExportTermValue(tail));
    return pair;
};

// Exports the term as a primitive value or object.
// More info: http://www.swi-prolog.org/pldoc/man?CAPI=PL_term_type

Local<Value> ExportTermValue(term_t t) {
    int type = PL_term_type(t);
    switch (type) {
        case PL_FLOAT:            
            return ExportFloat(t);
        case PL_INTEGER:            
            return ExportInteger(t);
        case PL_NIL:
            return Nan::New<String>("[]").ToLocalChecked();
        case PL_LIST_PAIR:
            return ExportListPair(t);
        case PL_ATOM:
            return ExportAtom(t);
        case PL_VARIABLE:
            return Nan::Null();
        case PL_STRING:
            return ExportString(t);
        case PL_TERM:
            return ExportCompound(t);
        case PL_BLOB:
            Nan::ThrowError("Term PL_BLOB cannot be exported yet.");
            return Nan::Null();
        default:
            Nan::ThrowError("Unknown exported term.");
            return Nan::Null();
    }
    return Nan::Null();
}

// Sets variable bindings in the original query.

Local<Object> ExportSolution(term_t t, int len, Local<Object> vars) {
    Local<Object> solution = Nan::New<Object>();
    for (int j = 0; j < len; j++) {
        int tj = t + j;
        Local<Value> key = vars->Get(tj);
        if (key->IsUndefined()) {
            continue;
        }
        solution->Set(key, ExportTermValue(tj));
    }
    return solution;
}

class InternalQuery : public Nan::ObjectWrap {
    public:
        static const int OPEN = 1;
        static const int CLOSED = 0;
        static void Init(Local<Object> target);

    private:
        InternalQuery() {};
        ~InternalQuery() {};

        // Closes the query.

        static void Close(const FunctionCallbackInfo<Value>& args) {
            HandleScope scope;
            InternalQuery* queryObject = ObjectWrap::Unwrap<InternalQuery>(args.This());
            if (queryObject->open == OPEN) {
                PL_close_query(queryObject->qid);
                queryObject->open = CLOSED;
            }
            args.GetReturnValue().Set(true);
        }

        static void Exception(const FunctionCallbackInfo<Value>& args);

        // Opens wrapped query. Wrapped query returns
        // bindings.

        static void Open(const FunctionCallbackInfo<Value>& args) {
            HandleScope scope;
            InternalQuery* queryObject = new InternalQuery();
            String::Utf8Value string(args[0]);
            atom_t query = PL_new_atom(*string);
            term_t refs = PL_new_term_refs(2);
            if (!PL_put_atom(refs, query)) {
                Nan::ThrowError("PL_put_atom failed.");
                return;
            }
            atom_t wrapper = PL_new_atom("nswi_");
            functor_t f = PL_new_functor(wrapper, 2);
            predicate_t p = PL_pred(f, NULL);
            int flags = PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION;
            qid_t q = PL_open_query(NULL, flags, p, refs);
            if (q == 0) {
                Nan::ThrowError("Not enough space on the environment stack.");
                return;
            }
            queryObject->qid = q;
            queryObject->bindings = refs + 1;
            queryObject->open = OPEN;
            queryObject->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }

        // Triggers finding next solution. Returns bindings list.

        static void Next(const FunctionCallbackInfo<Value>& args) {
            HandleScope scope;
            InternalQuery* queryObject = ObjectWrap::Unwrap<InternalQuery>(args.This());
            if (queryObject->open == OPEN) {
                if (PL_next_solution(queryObject->qid)) {
                    args.GetReturnValue().Set(ExportTermValue(queryObject->bindings));
                } else {
                    args.GetReturnValue().Set(false);
                }                
            } else {
                Nan::ThrowError("Query is closed.");
                args.GetReturnValue().SetUndefined();
            }
        }

        int open;
        qid_t qid;
        term_t bindings;
};

void InternalQuery::Init(Local<Object> target) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(Open);
    tpl->SetClassName(Nan::New<v8::String>("InternalQuery").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(3);
    // Prototype
    tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("next").ToLocalChecked(),
        Nan::New<FunctionTemplate>(Next));
    tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("close").ToLocalChecked(),
        Nan::New<FunctionTemplate>(Close));
    tpl->PrototypeTemplate()->Set(Nan::New<v8::String>("exception").ToLocalChecked(),
        Nan::New<FunctionTemplate>(Exception));


    auto constructor = Nan::GetFunction(tpl).ToLocalChecked();
    Nan::Set(target, Nan::New<v8::String>("InternalQuery").ToLocalChecked(), constructor);
}

void InternalQuery::Exception(const FunctionCallbackInfo<Value>& args) {
    HandleScope scope;
    InternalQuery* obj = ObjectWrap::Unwrap<InternalQuery>(args.This());
    term_t term = PL_exception(obj->qid);
    if (term) {    
        args.GetReturnValue().Set(CreateException(GetExceptionString(term)));
    } else {
        args.GetReturnValue().Set(Nan::True());
    }
}

NAN_MODULE_INIT(init) {
    Nan::Set(target, Nan::New<String>("initialise").ToLocalChecked(),
        Nan::GetFunction(Nan::New<FunctionTemplate>(Initialise)).ToLocalChecked());

    Nan::Set(target, Nan::New<String>("cleanup").ToLocalChecked(),
        Nan::GetFunction(Nan::New<FunctionTemplate>(Cleanup)).ToLocalChecked());

    InternalQuery::Init(target);
}

NODE_MODULE(libswipl, init)
