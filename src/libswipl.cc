#include <cstdlib>
#include <sstream>
#include <nan.h>
#include <SWI-Prolog.h>

using v8::Array;
using v8::String;
using v8::Object;
using v8::Value;
using v8::Local;
using v8::Number;
using v8::FunctionTemplate;
using v8::Function;
using Nan::FunctionCallbackInfo;
using Nan::Persistent;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::Set;
using Nan::ThrowError;
using Nan::GetFunction;

// Installs wrapper that extracts bindings.

void InstallWrapper() {
    char const *goal = "assert(system:nswi_(A,Bs):-(atom_to_term(A,G,Bs),call(G)))";
    fid_t fid = PL_open_foreign_frame();
    term_t g = PL_new_term_ref();
    PL_chars_to_term(goal, g);
    PL_call(g, NULL);
    PL_discard_foreign_frame(fid);
}

// Initializes the SWI-Prolog engine.

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

    info.GetReturnValue().Set(New<Number>(rval));
}

// Shuts down the SWI-Prolog engine.

void Cleanup(const FunctionCallbackInfo<Value>& args) {
    HandleScope scope;
    int rval = PL_cleanup(0);
    args.GetReturnValue().Set(New<Number>(rval));
}

Local<Value> ExportTermValue(term_t t);

// Exports the compound term into an object.

Local<Value> ExportCompound(term_t t) {
    Local<Object> compound = New<Object>();
    atom_t n;
    int arity;
    const char *name;
    if (!PL_get_compound_name_arity(t, &n, &arity)) {
        ThrowError("PL_get_compound_name_arity failed.");
        return Null();
    }
    name = PL_atom_chars(n);
    Local<Array> args = New<Array>();
    for (int i = 1; i <= arity; ++i) {
        term_t arg_t = PL_new_term_ref();
        if (!PL_get_arg(i, t, arg_t)) {
            ThrowError("PL_get_arg failed.");
            return Null();
        }
        args->Set(i - 1, ExportTermValue(arg_t));
    }
    compound->Set(New<String>("name").ToLocalChecked(),
        New<String>(name).ToLocalChecked());
    compound->Set(New<String>("args").ToLocalChecked(), args);
    return compound;
}

// Exports PL_FLOAT as number.

Local<Value> ExportFloat(term_t t) {
    double d = 0.0;
    if (!PL_get_float(t, &d)) {
        ThrowError("PL_get_float failed.");
        return Null();
    }
    return New<Number>(d);
}

// Exports PL_INTEGER as number.

Local<Value> ExportInteger(term_t t) {
    int i = 0;
    if (!PL_get_integer(t, &i)) {
        ThrowError("PL_get_integer failed.");
        return Null();
    }
    return Nan::New<Number>(i);
}

// Exports PL_ATOM.

Local<Value> ExportAtom(term_t t) {
    char *c;
    if (!PL_get_atom_chars(t, &c)) {
        ThrowError("PL_get_atom_chars failed.");
        return Null();
    }
    return New<String>(c).ToLocalChecked();
}

// Exports PL_STRING.

Local<Value> ExportString(term_t t) {
    char *c;
    size_t len;
    if (!PL_get_string_chars(t, &c, &len)) {
        ThrowError("PL_get_string_chars failed.");
    }
    return New<String>(c).ToLocalChecked();
}

// Exports PL_LIST_PAIR.

Local<Value> ExportListPair(term_t t) {
    Local<Object> pair = New<Object>();
    term_t head = PL_new_term_ref();
    term_t tail = PL_new_term_ref();
    if (!PL_get_list(t, head, tail)) {
        ThrowError("PL_get_list failed.");
    };
    pair->Set(New<String>("head").ToLocalChecked(),
        ExportTermValue(head));
    pair->Set(New<String>("tail").ToLocalChecked(),
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
            return New<String>("[]").ToLocalChecked();
        case PL_LIST_PAIR:
            return ExportListPair(t);
        case PL_ATOM:
            return ExportAtom(t);
        case PL_VARIABLE:
            return Null();
        case PL_STRING:
            return ExportString(t);
        case PL_TERM:
            return ExportCompound(t);
        case PL_BLOB:
            ThrowError("Term PL_BLOB cannot be exported yet.");
            return Null();
        case PL_DICT:
            ThrowError("Term PL_DICT cannot be exported yet.");
            return Null();
        default:
            ThrowError("Unknown exported term.");
            return Null();
    }
    return Null();
}

// Sets variable bindings in the original query.

Local<Object> ExportSolution(term_t t, int len, Local<Object> vars) {
    Local<Object> solution = New<Object>();
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

// Extracts exception string from prolog.

const char* ExceptionString(term_t term) {
    char *msg;
    term_t msgterms = PL_new_term_refs(2);
    PL_put_term(msgterms, term);
    if (!PL_call_predicate(NULL, PL_Q_NODEBUG,
        PL_predicate("message_to_string", 2, NULL), msgterms)) {
        return "Error while getting the exception message.";
    };
    if (!PL_get_chars(msgterms + 1, &msg, CVT_ALL)) {
        return "Error while extracting the exception message.";
    }
    return msg;
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
                PL_discard_foreign_frame(queryObject->fid);
                queryObject->open = CLOSED;
            }
            args.GetReturnValue().Set(true);
        }

        // Opens wrapped query. Wrapped query returns
        // bindings.

        static void Open(const FunctionCallbackInfo<Value>& args) {
            HandleScope scope;
            InternalQuery* queryObject = new InternalQuery();
            queryObject->open = CLOSED;
            fid_t fid = PL_open_foreign_frame();
            String::Utf8Value string(args[0]);
            atom_t query = PL_new_atom(*string);
            term_t refs = PL_new_term_refs(2);
            if (!PL_put_atom(refs, query)) {
                PL_discard_foreign_frame(fid);
                ThrowError("PL_put_atom failed.");
                return;
            }
            atom_t wrapper = PL_new_atom("nswi_");
            functor_t f = PL_new_functor(wrapper, 2);
            predicate_t p = PL_pred(f, NULL);
            int flags = PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION;
            qid_t q = PL_open_query(NULL, flags, p, refs);
            if (q == 0) {
                PL_discard_foreign_frame(fid);
                ThrowError("Not enough space on the environment stack.");
                return;
            }
            queryObject->qid = q;
            queryObject->fid = fid;
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
                    Nan::TryCatch tc;
                    args.GetReturnValue().Set(ExportTermValue(queryObject->bindings));
                    if (tc.HasCaught()) {
                        PL_close_query(queryObject->qid);
                        PL_discard_foreign_frame(queryObject->fid);
                        queryObject->open = CLOSED;
                        tc.ReThrow();
                        args.GetReturnValue().SetUndefined();
                    }                    
                } else {
                    // Check if exception was raised during execution.
                    term_t exception = PL_exception(queryObject->qid);
                    if (exception) {
                        std::stringstream err;
                        err << "Error during query execution. " << ExceptionString(exception);
                        PL_close_query(queryObject->qid);
                        PL_discard_foreign_frame(queryObject->fid);
                        std::string strErr = err.str();
                        queryObject->open = CLOSED;     
                        ThrowError(strErr.c_str());
                        args.GetReturnValue().SetUndefined();
                    } else {
                        // Last solution. Close query and frame.
                        PL_close_query(queryObject->qid);
                        PL_discard_foreign_frame(queryObject->fid);
                        args.GetReturnValue().Set(false);
                    }
                }
            } else {
                ThrowError("Query is closed.");
                args.GetReturnValue().SetUndefined();
            }
        }

        int open;
        qid_t qid;
        fid_t fid;
        term_t bindings;
};

void InternalQuery::Init(Local<Object> target) {
    // Prepare constructor template.
    Local<FunctionTemplate> tpl = New<FunctionTemplate>(Open);
    tpl->SetClassName(New<String>("InternalQuery").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(3);
    // Prototype
    tpl->PrototypeTemplate()->Set(New<String>("next").ToLocalChecked(),
        New<FunctionTemplate>(Next));
    tpl->PrototypeTemplate()->Set(New<String>("close").ToLocalChecked(),
        New<FunctionTemplate>(Close));

    Local<Function> constructor = GetFunction(tpl).ToLocalChecked();
    Set(target, New<String>("InternalQuery").ToLocalChecked(), constructor);
}

NAN_MODULE_INIT(init) {
    Set(target, New<String>("initialise").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Initialise)).ToLocalChecked());

    Set(target, New<String>("cleanup").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Cleanup)).ToLocalChecked());

    InternalQuery::Init(target);
}

NODE_MODULE(libswipl, init)
