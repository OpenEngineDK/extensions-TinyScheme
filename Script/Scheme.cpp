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
#include <Resources/DirectoryManager.h>
#include "boost/filesystem/operations.hpp"
#include <Scene/TransformationNode.h>

using namespace std;

const char *loadStr = 
"(define process (lambda (n) "
//"(callback)"
//"(display n)"
//"(display \", Process\n\")"
"1"
"))";
const char *loadStr2 = 
"(define getp (lambda () "
"process))";



namespace OpenEngine {
namespace Script {

    using namespace OpenEngine::Resources;
    using namespace OpenEngine::Scene;
    using namespace boost::filesystem;

    template <class C>class Callback {
    public:
        
        typedef pointer (C::*Method)(scheme* s, pointer args);
                
        
        Callback(C* inst, Method m) {
            cl_inst = inst;
            method = m;
        }
        
        pointer operator()(scheme *s, pointer args) {
            return (cl_inst->*method)(s,args);
        }
        
        static pointer scheme_ff(scheme *s, void* context, pointer args) {
            if (context) {
                logger.info << "context: " << context << logger.end;
                Callback *cb = (Callback*)context;
                return (*cb)(s,args);
                
            } 
            logger.info << "callback" << logger.end;
            return s->NIL;
        }
        
    private:
        C* cl_inst;
        Method method;
    };
    

    
    
    Scheme::Scheme() {
        sc = scheme_init_new();
        time.Start();
        reloadTimer.Start();
        logger.info << "Scheme: " << sc << logger.end;
        //scheme_set_input_port_file(sc, stdin);
        scheme_set_output_port_file(sc, stdout);
                
        scheme_load_string(sc, loadStr);
        scheme_load_string(sc, loadStr2);
        
        Callback<Scheme> *cb = new Callback<Scheme>(this,&Scheme::CbQuit);
        
        scheme_define(sc,sc->global_env,mk_symbol(sc,"quit"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbQuit)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"tn-get-pos"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbTnGetPos)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"vec-to-list"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbVecToList)), Callback<Scheme>::scheme_ff)); 
        

        
        fProcess = scheme_apply0(sc, "getp");

        logger.info << "pointer: " << fProcess << logger.end;
        
    }
    
    void Scheme::EvalAndPrint(string str) {
        logger.info << "[SCHEME] Eval: " << str << logger.end;
        scheme_load_string(sc, str.c_str());
    }
    
    void Scheme::DefinePointer(string name, void* p) {
        scheme_define(sc, sc->global_env, mk_symbol(sc, name.c_str()), mk_pointer(sc, p));
    }

    
    void Scheme::AddFileToAutoLoad(string file) {
        
        string path = DirectoryManager::FindFileInPath(file);            
        FILE *fh = fopen(path.c_str(), "r");
        
        logger.info << "Loading: " << file << logger.end;
        
        scheme_load_file(sc, fh);
        fclose(fh);
        autoFiles[file] = last_write_time(path);
        fProcess = scheme_apply0(sc, "getp");

    }

    
    void Scheme::Handle(ProcessEventArg arg) {
        if (reloadTimer.GetElapsedIntervals(1000000)) {
            reloadTimer.Reset();
            
            //check files
            for(map<string,time_t>::iterator itr = autoFiles.begin();
                itr != autoFiles.end();
                itr++) {
                string file = (*itr).first;
                string path = DirectoryManager::FindFileInPath(file);
                if (last_write_time(path) != (*itr).second)
                    AddFileToAutoLoad(file);
            }
        }
        
        
        int dt = time.GetElapsedIntervals(1);
        if (time.GetElapsedIntervals(100000)) {
            time.Reset();
            scheme_call(sc, fProcess, _cons(sc, mk_integer(sc, dt), sc->NIL, 0));
                    
            //scheme_apply0(sc, "process");
        }
    }
    
    pointer Scheme::CbQuit(scheme* s, pointer args) {
        exit(0);
        return s->NIL;
    }
    
    pointer Scheme::CbTnGetPos(scheme *s, pointer args) {
        // TODO: make less evail (typecheck and stuff)    
        TransformationNode *tn = (TransformationNode*)        pointervalue(pair_car(args));
        Vector<3,float> pos = tn->GetPosition(); // Valid as long as tn is in memory
        return mk_pointer(s, &pos);
    }
    pointer Scheme::CbVecToList(scheme *s, pointer args) {
        Vector<3,float> *vec = (Vector<3,float>*)pointervalue(pair_car(args));
        
        logger.info << *vec << logger.end;
        
        return s->NIL;
    }

    
}
}