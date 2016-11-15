
namespace reflex {

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
typedef unsigned __int16 uint16_t
typedef unsigned __int32 uint32_t
typedef unsigned __int64 uint64_t
#else
#include <stdint.h>
#endif

}
