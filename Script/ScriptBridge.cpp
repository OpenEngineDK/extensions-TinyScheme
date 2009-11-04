/*
 *  ScriptBridge.c
 *  OpenEngine
 *
 *  Created by Peter Kristensen on 25/10/09.
 *  Copyright 2009 Lucky Software. All rights reserved.
 *
 */

#include "ScriptBridge.h"
#include <Logging/Logger.h>


#include <Scene/TransformationNode.h>


using namespace OpenEngine::Script;

char *sbo_desc(sbo s) {    
    char* buf = (char*)malloc(sizeof(char)*256);
    SBO* obj = (SBO*)s;
    if (s == 0)
        snprintf(buf, 256, "NULL");
    else
        snprintf(buf, 256, "%s",typeid(*(obj->_handler)).name());
    return buf;
}

void sbo_release(sbo s) {
    SBO* obj = (SBO*)s;
    if (obj)  {
        if (obj->_type == SBO::objectType) {
            obj->_handler->Release(obj->_pointer);
        }
        delete obj;
    }
}

namespace OpenEngine {
    namespace Script {

        
        using namespace OpenEngine::Scene;

        map<string,SBOHandler*> ScriptBridge::handlers = map<string,SBOHandler*>();

        
        sbo ScriptBridge::CallSBO(sbo s, string fun, vector<sbo> args) {
            SBO *sObj = (SBO*)s;
            SBOHandler *h = sObj->_handler;
            vector<SBO*> *argP = (vector<SBO*>*)(&args);
            
            return (sbo)(h->Call(sObj, fun, *argP));
        }
        
        TransformationNodeHandler::TransformationNodeHandler() {
        }
        
        SBO* TransformationNodeHandler::Call(SBO* obj, string fun, vector<SBO*> args) {
            
            TransformationNode *node = (TransformationNode*)(obj->_pointer);
            
            if (fun == "get-position") {
                //logger.info << "getpos!" << logger.end;
                Vector<3,float> pos = node->GetPosition();
                return ScriptBridge::CreateSboObject<Vector<3,float> >(pos);
            } else if (fun == "set-position") {                
                Vector<3,float>* pos = (Vector<3,float>*)(args[0])->_pointer;
                node->SetPosition(*pos);
            } else if (fun == "get-rotation") {
                Vector<3,float> euler = node->GetRotation().GetEulerAngles();
                return ScriptBridge::CreateSboObject<Vector<3,float> >(euler);
            } else if (fun == "set-rotation") {
                Vector<3,float>* eulerP = (Vector<3,float>*)(args[0])->_pointer;
                Vector<3,float> euler = *eulerP;
                float roll=euler[0];
                float pitch=euler[1];
                float jaw = euler[2];
                
                node->SetRotation(Quaternion<float>(roll,pitch,jaw));
            } else {
                logger.info << "called " << fun << logger.end;
            }
            
            return 0;
        }
        void TransformationNodeHandler::Release(void*p) {
            delete (TransformationNode *)p;
        }
        
        
        VectorHandler::VectorHandler() {}
        SBO* VectorHandler::Call(SBO* obj, string fun, vector<SBO*> args) {
            return 0;
        }
        void VectorHandler::Release(void*p) {
            delete (Vector<3,float> *)p;
        }
    }
}

