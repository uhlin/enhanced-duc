#ifndef DUC_DEFINITIONS_H
#define DUC_DEFINITIONS_H
/* Copyright (c) 2021-2023 Markus Uhlin <markus.uhlin@icloud.com>
   All rights reserved.

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
   AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
   PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE. */

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
