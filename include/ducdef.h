#ifndef DUC_DEFINITIONS_H
#define DUC_DEFINITIONS_H

#define BZERO(buf, sz) ((void) memset(buf, 0, sz))

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifdef __unix__
#define PRINTFLIKE(arg_no) __attribute__((format(printf, arg_no, arg_no + 1)))
#else
#define PRINTFLIKE(arg_no)
#endif

#ifdef __unix__
#define PTR_ARGS_NONNULL __attribute__((nonnull))
#else
#define PTR_ARGS_NONNULL
#endif

#ifdef __cplusplus
#define __DUC_BEGIN_DECLS extern "C" {
#define __DUC_END_DECLS }
#else
#define __DUC_BEGIN_DECLS
#define __DUC_END_DECLS
#endif

#define addrof(x) (&(x))

#ifndef nitems
#define nitems(_a) (sizeof((_a)) / sizeof((_a)[0]))
#endif

#ifndef __dead
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#    define __dead _Noreturn
#  elif defined(__unix__)
#    define __dead __attribute__((noreturn))
#  elif defined(_WIN32)
#    define __dead __declspec(noreturn)
#  else
#    define __dead
#  endif
#endif /* ifndef __dead */

#endif
