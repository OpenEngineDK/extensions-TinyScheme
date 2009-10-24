
# Create the extension library
ADD_LIBRARY(Extensions_TinyScheme
  Hello/World.cpp
  tinyscheme/dynload.c
  tinyscheme/dynload.h
  tinyscheme/opdefines.h
  tinyscheme/scheme.c
  tinyscheme/scheme.h
  tinyscheme/scheme-private.h
)
