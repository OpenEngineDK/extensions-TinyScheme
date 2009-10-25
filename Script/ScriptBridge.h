/*
 *  ScriptBridge.h
 *  OpenEngine
 *
 *  Created by Peter Kristensen on 25/10/09.
 *  Copyright 2009 Lucky Software. All rights reserved.
 *
 */

#ifndef _TINY_SCHEME_SCRIPT_BRIDGE_H_
#define _TINY_SCHEME_SCRIPT_BRIDGE_H_

#include "sbo.h"
#include <typeinfo>
#include <string>
#include <map>
#include <vector>

namespace OpenEngine {
    namespace Script {

        using namespace std;
        
        struct SBO;
        
        class SBOHandler {
        public:
            //SBOHandler() =0;
            virtual SBO* Call(SBO* obj, string fun, vector<SBO*>args) =0;
            virtual void Release(void* p) =0;
        };
        
        struct SBO {
            enum type {
                pointerType,
                objectType
            };
            type _type;
            SBOHandler *_handler;
            void* _pointer;
        };
        
    class ScriptBridge {
        static map<string, SBOHandler*> handlers;
        
    public:
        
        template <class C> static void AddHandler(SBOHandler *h) {
            handlers[string(typeid(C).name())] = h;
        }
        
        template <class C> static SBO* CreateSboPointer(C* p) {
            SBOHandler *h = handlers[string(typeid(C).name())];
            if (h) {
            
                SBO *s = new SBO();
                s->_type = SBO::pointerType;
                s->_pointer = p;
                s->_handler = h;
                return s;                        
            }
            return 0;
        }
        template <class C> static SBO* CreateSboObject(C p) {
            SBOHandler *h = handlers[string(typeid(C).name())];
            if (h) {

                SBO *s = new SBO();
                s->_type = SBO::objectType;
                C *clone = new C(p);
                s->_pointer = clone;
                s->_handler = h;
                return s;
            }
            return 0;
        }
        
        static sbo CallSBO(sbo s, string fun, vector<sbo> args);
        
    };
        
        class TransformationNodeHandler : public SBOHandler {
        public:
            TransformationNodeHandler();
            SBO* Call(SBO* obj, string fun, vector<SBO*> args);
            void Release(void*);
        };

        class VectorHandler : public SBOHandler {
        public:
            VectorHandler();
            SBO* Call(SBO* obj, string fun, vector<SBO*>args);
            void Release(void*);
        };
        
        
}
}

#endif