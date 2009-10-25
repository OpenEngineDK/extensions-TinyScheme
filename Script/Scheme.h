/*
 *  Scheme.h
 *  OpenEngine
 *
 *  Created by Peter Kristensen on 24/10/09.
 *  Copyright 2009 Lucky Software. All rights reserved.
 *
 */

#ifndef _TINY_SCHEME_SCRIPT_SCHEME_H_
#define _TINY_SCHEME_SCRIPT_SCHEME_H_

#include <Core/EngineEvents.h>
#include <Core/IListener.h>
#include <Utils/Timer.h>

#include <string>
#include <map>

extern "C" {
#include "tinyscheme/scheme.h"
#include "tinyscheme/scheme-private.h"
}

namespace OpenEngine {
namespace Script {

    using namespace OpenEngine::Core;
    using namespace OpenEngine::Utils;

    using namespace std;
    
    class Scheme : public IListener<ProcessEventArg> {
        scheme *sc;
        pointer fProcess;
        Timer time;
        Timer reloadTimer;
        
        map<string,time_t> autoFiles;
        
    public:
        Scheme();
        
        void EvalAndPrint(std::string str);        
        void Handle(ProcessEventArg arg);
        void AddFileToAutoLoad(string file);
        
        void DefinePointer(string name, void* p);
        
    private:
        pointer CbQuit(scheme *s, pointer args);
        pointer CbTnGetPos(scheme *s, pointer args);
        pointer CbVecToList(scheme *s, pointer args);
        
    };
        
}
}

#endif