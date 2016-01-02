#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define ARRAY_SIZE(ar) (sizeof (ar) / sizeof ((ar)[0]))

#define BZERO(buf, sz) ((void) memset(buf, 0, sz))

#ifdef __GNUC__
#define PRINTFLIKE(arg_no)	__attribute__ ((format (printf, arg_no, arg_no + 1)))
#define NORETURN		__attribute__ ((noreturn))
#else
#define PRINTFLIKE(arg_no)
#define NORETURN
#endif

#endif
