#include "simple_string.h"

#define SMPSTR_MAX_LEN 256

// copy with length arg (-1 == until null terminator, no bounds check)
void str_copy(char* dst, const char * src, s8 len) {
  u8 i=0;
  if(len < 0) {
      while(1) { // (i < SMPSTR_MAX_LEN) {
      if(src[i] == 0) {
	break;
      }
      dst[i] = src[i];
      i++;
    }
  } else {
    while(i < len) {
      dst[i] = src[i];
      i++;
    }
  }
}
