/*
 *  Scheme.cpp
 *  OpenEngine
 *
 *  Created by Peter Kristensen on 24/10/09.
 *  Copyright 2009 Lucky Software. All rights reserved.
 *
 */


#include "Scheme.h"
#include <Logging/Logger.h>

using namespace std;

const char *loadStr = 
"(define process (lambda (n) "
"(callback)"
"(display n)"
"(display \", Process\n\")))";
const char *loadStr2 = 
"(define getp (lambda () "
"process))";



namespace OpenEngine {
namespace Script {

    class Callback {
        
    public:
        pointer operator()(scheme *s, pointer args) {
            logger.info << "CPP callback" << logger.end;
            return s->NIL;
        }
        
        static pointer test_ff(scheme *s, void* context, pointer args) {
            if (context) {
                logger.info << "context: " << context << logger.end;
                Callback *cb = (Callback*)context;
                return (*cb)(s,args);
                
            } 
            logger.info << "callback" << logger.end;
            return s->NIL;
        }
        
    };
    
    
    Scheme::Scheme() {
        sc = scheme_init_new();
        time.Start();
        logger.info << "Scheme: " << sc << logger.end;
        //scheme_set_input_port_file(sc, stdin);
        scheme_set_output_port_file(sc, stdout);
                
        scheme_load_string(sc, loadStr);
        scheme_load_string(sc, loadStr2);
        
        Callback *cb = new Callback();
        
        scheme_define(sc,sc->global_env,mk_symbol(sc,"callback"),mk_foreign_func(sc, (void*)cb, Callback::test_ff)); 


        
        fProcess = scheme_apply0(sc, "getp");

        logger.info << "pointer: " << fProcess << logger.end;
        
    }
    
    void Scheme::EvalAndPrint(string str) {
        logger.info << "[SCHEME] Eval: " << str << logger.end;
        scheme_load_string(sc, str.c_str());
    }
    void Scheme::Handle(ProcessEventArg arg) {
        int dt = time.GetElapsedIntervals(1);
        if (time.GetElapsedIntervals(100000)) {
            time.Reset();
            scheme_call(sc, fProcess, _cons(sc, mk_integer(sc, dt), sc->NIL, 0));
                    
            //scheme_apply0(sc, "process");
        }
    }
    
}
}