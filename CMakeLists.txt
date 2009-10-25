
# Create the extension library
ADD_LIBRARY(Extensions_TinyScheme
  Script/Scheme.cpp
  Script/Scheme.h
  Script/ScriptBridge.cpp
  Script/ScriptBridge.h
  tinyscheme/dynload.c
  tinyscheme/dynload.h
  tinyscheme/opdefines.h
  tinyscheme/scheme.c
  tinyscheme/scheme.h
  tinyscheme/scheme-private.h
)
