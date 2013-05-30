/* zpipe.h: example of proper use of zlib's inflate() and deflate()
Not copyrighted -- provided to the public domain
Version 1.4  11 December 2005  Mark Adler */

#ifndef ZPIP_H_
#define ZPIP_H_

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

extern "C" int def(FILE *source, FILE *dest, int level);
extern "C" int inf(FILE *source, FILE *dest, double* progress = NULL);
extern "C" void zerr(int ret);

#endif /* ZPIP_H_ */
