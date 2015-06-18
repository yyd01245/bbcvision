#ifndef GLOBAL_MACROS_H
#define GLOBAL_MACROS_H

#define BIT_ENABLED(WORD, BIT) (((WORD) & (BIT)) != 0)
#define BIT_DISABLED(WORD, BIT) (((WORD) & (BIT)) == 0)

//定义是否编译日志
//#define LOG_NOCOMPILE

typedef short           int16;
#if defined(DIGITAL) || defined(HPUX) || defined(AIX) || defined(SOLARIS) || defined(LINUX)
typedef signed int      int32;
#else
typedef signed long     int32;
#endif
typedef long long       int64;


#endif
