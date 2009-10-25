/*
 *  sbo.h
 *  OpenEngine
 *
 *  Created by Peter Kristensen on 25/10/09.
 *  Copyright 2009 Lucky Software. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef void* sbo;
    
    char *sbo_desc(sbo s);
    void sbo_release(sbo s);
    
//    sbo createSboFromObject(void* obj);
    
    
#ifdef __cplusplus
}
#endif