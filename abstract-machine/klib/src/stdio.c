#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
  //object va_list
  /*va_list ap;
  va_start(ap, fmt);
  char *buf = fmt;
  while(*fmt){
  	char ret = *fmt;
	if(ret=='%'){
		switch(*++fmt){
			case 'd':{
				 int i = va_arg(ap, int);
				 for (int j=i;j!=0;j=j/10){
				 	*buf = (char)(j%10+96);
					buf++;
				 }
				 *buf = char
				 }
		}
	}
	*buf = ret;
	buf++;
	fmt++;
  }*/
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
