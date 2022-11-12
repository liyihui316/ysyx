#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  //panic("Not implemented");
  size_t i = 0;
  while(s[i]!='\0'){
  	i++;
  }
  return i;
}

char *strcpy(char *dst, const char *src) {
  //panic("Not implemented");
  size_t len = strlen(src);
  size_t i;
  for (i=0;i<len;i++){
  	dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  //panic("Not implemented");
  size_t i;
  for (i=0;i<n;i++){
  	if (src[i] == '\0'){
		break;
	}
	dst[i] = src[i];
  }
  for (;i<n;i++)
	  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
  size_t len = strlen(dst);
  size_t i;
  for (i=0;src[i]!='\0';i++){
  	dst[len+i] = src[i];
  }
  dst[len+i] = '\0';
  return dst;
}

//int strcmp(const char *s1, const char *s2) {
//  //panic("Not implemented");
//  size_t i;
//  for(i=0;s1[i]!='\0'||s2[i]!='\0';i++){
//  	if (s1[i] < s2[i])
//		return -1;
//	if (s1[i] > s2[i])
//		return 1;
//  }
//  if(s1[i]==s2[i])
//	  return 0;
//  else if(s1[i]=='\0')
//	  return -1;
//  else
//	  return 1;
//}
//
int strcmp(const char* s1, const char *s2){
	size_t i;
	for (i=0;s1[i]!='\0'&&s2[i]!='\0';i++)
		if (s1[i]-s2[i]!=0)
			return	(s1[i] - s2[i]);
	return (s1[i] - s2[i]);
}
int strncmp(const char* s1, const char* s2, size_t n){
	size_t i;
	for (i=0;i<n;i++){
		if (s1[i] - s2[i] != 0 || s1[i]=='\0' || s2[i]=='\0')
			return s1[i] - s2[i];
	}
	return 0;
}
int memcmp(const void* s1, const void* s2, size_t n){
	size_t i;
	char *cs1 = (char *)s1;
	char *cs2 = (char *)s2;
	for (i=0;i<n;i++){
		if(cs1[i] - cs2[i] !=0 || cs1[i]=='\0' || cs2[i]=='\0')
			return cs1[i] - cs2[i];
	}
	return 0;
}
//int strncmp(const char *s1, const char *s2, size_t n) {
//  //panic("Not implemented");
//  size_t i;
//  for(i=0;(s1[i]!='\0'||s2[i]!='\0')&&i<n;i++){
//  	if (s1[i] < s2[i])
//		return -1;
//	if (s1[i] > s2[i])
//		return 1;
//  }
//  if(i<n){
//  	if(s1[i] == s2[i])
//		return 0;
//	else if(s1[i] == '\0')
//		return -1;
//	else
//		return 1;
//  }
//  else
//	  return 0;
//}

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
  size_t i;
  for (i=0;i<n;i++){
  	*((unsigned char*)s+i) = c;//unsigned char not char to avoid sign extension.
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  size_t i;
  char *cdst = (char*)dst;
  char *csrc = (char*)src;
  if (cdst<=csrc || cdst >=csrc+n)
	  for (i=0;i<n;i++)
		  cdst[i] = csrc[i];
  else
	  for (i=n-1;i>=0;i--)
		  cdst[i] = csrc[i];
  return cdst;
}

void *memcpy(void *out, const void *in, size_t n) {
  //panic("Not implemented");
  size_t i;
  char *cin = (char*)in;
  char *cout = (char*)out;
  for(i=0;i<n;i++){
  	cout[i] = cin[i];
  }
  return cout;
}

//int memcmp(const void *s1, const void *s2, size_t n) {
//  //panic("Not implemented");
//  size_t i;
//  char *cs1 = (char*)s1;
//  char *cs2 = (char*)s2;
//  for(i=0;(cs1[i]!='\0'||cs2[i]!='\0')&&i<n;i++){
//  	if (cs1[i] < cs2[i])
//		return -1;
//	if (cs1[i] > cs2[i])
//		return 1;
//  }
//  if(i<n){
//  	if(cs1[i] == cs2[i])
//		return 0;
//	else if(cs1[i] == '\0')
//		return -1;
//	else
//		return 1;
//  }
//  else
//	  return -1;
//}

#endif
