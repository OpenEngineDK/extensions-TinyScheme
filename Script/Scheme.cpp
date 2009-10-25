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
#include <Script/ScriptBridge.h>

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
                //logger.info << "context: " << context << logger.end;
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
        
        
        scheme_define(sc,sc->global_env,mk_symbol(sc,"quit"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbQuit)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"tn-get-pos"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbTnGetPos)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"vec-to-list"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbVecToList)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"list-to-vec"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbListToVec)), Callback<Scheme>::scheme_ff)); 
        scheme_define(sc,sc->global_env,mk_symbol(sc,"oe-call"),
                      mk_foreign_func(sc, (void*)(new Callback<Scheme>(this,&Scheme::CbOECall)), Callback<Scheme>::scheme_ff)); 


        
        fProcess = scheme_apply0(sc, "getp");

        logger.info << "pointer: " << fProcess << logger.end;
        
    }
    
    void Scheme::EvalAndPrint(string str) {
        logger.info << "[SCHEME] Eval: " << str << logger.end;
        scheme_load_string(sc, str.c_str());
    }
    
    void Scheme::DefineSbo(string name, sbo s) {
        logger.info << "define sbo: " << s << logger.end;
        //mk_sbo(sc, s);
        //logger.info << "define sbo: " << s << logger.end;
        scheme_define(sc, sc->global_env, mk_symbol(sc, name.c_str()), mk_sbo(sc, s));
    }
    
    void Scheme::DefinePointer(string name, void* p) {
        
        //sbo s = createSboFromObject(p);
        
        //scheme_define(sc, sc->global_env, mk_symbol(sc, name.c_str()), mk_sbo(sc, p));
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
        TransformationNode *tn = (TransformationNode*)        sbovalue(pair_car(args));
        Vector<3,float> pos = tn->GetPosition(); // Valid as long as tn is in memory
        return mk_sbo(s, &pos);
    }
    pointer Scheme::CbVecToList(scheme *s, pointer args) {
        
        
        SBO* sboVec = (SBO*)sbovalue(pair_car(args));
        // We assume that it's a 3 float
        Vector<3,float> *vec = (Vector<3,float>*)(sboVec->_pointer);
        
        pointer z = _cons(s, mk_real(s, (*vec)[2]),s->NIL, 1);
        pointer yz = _cons(s, mk_real(s, (*vec)[1]), z, 1);
        pointer xyz = _cons(s, mk_real(s, (*vec)[0]), yz, 1);
        
        //logger.info << *vec << logger.end;
        
        return xyz;
    }
    
    pointer Scheme::CbListToVec(scheme *s, pointer args) {
        pointer list = pair_car(args);
        
        pointer p = list;
        Vector<3,float> vec;
        for (int i=0;i<3;i++) {
            vec[i] = rvalue(pair_car(p));
            p = pair_cdr(p);
        }
        return mk_sbo(s,ScriptBridge::CreateSboObject<Vector<3,float> >(vec));
    }

    pointer Scheme::CbOECall(scheme *s, pointer args) {

        //logger.info << "oe-call" << logger.end;
        
        sbo sb = sbovalue(pair_car(args));        
        string fun = string(symname(pair_car(pair_cdr(args))));
        
        vector<sbo> sboArgs;
        
        int len = list_length(s,args);
        pointer p = pair_cdr(args);
        for (int i=2;i<len;i++) {
            p = pair_cdr(p);
            sboArgs.push_back(sbovalue(pair_car(p)));
        }
        
        
        
        return mk_sbo(s,ScriptBridge::CallSBO(sb,fun,sboArgs));
        
        
        //return s->NIL;
    }
    
}
}