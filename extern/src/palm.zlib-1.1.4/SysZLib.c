#include <PalmOS.h>

Err zlib_open(UInt16 ref) {
  UInt32 val=0;
  FtrGet('ZLib', ref, &val);
  FtrSet('ZLib', ref, ++val);
  return 0;
}

Err zlib_close(UInt16 ref, UInt16 *numop) {
  UInt32 val = 0;
  *numop = 0;

  FtrGet('ZLib', ref, &val);
  if( val <= 0 )
    return -1;

  FtrSet('ZLib', ref, --val);
  *numop = val;
  return 0;
}

Err zlib_null(UInt16 ref) {
  return 0;
}
