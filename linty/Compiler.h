#pragma once

#ifdef __GNUC__
    #define COMPILER_NAME "g++"
    #define COMPILER_VERSION __VERSION__
#else
    #define COMPILER_NAME "Unknown compiler"
    #define COMPILER_VERSION "Unknown compiler version"
#endif
