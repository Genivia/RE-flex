#ifndef REFLEX_CODE_DECL
#include "pattern.h"
#define REFLEX_CODE_DECL const reflex::Pattern::Opcode
#endif

REFLEX_CODE_DECL reflex_code_filter[11] =
{
  0x617A0005, // 0: GOTO 5 ON a-z
  0x5F5F0005, // 1: GOTO 5 ON _
  0x415A0005, // 2: GOTO 5 ON A-Z
  0x30390005, // 3: GOTO 5 ON 0-9
  0x00FFFFFF, // 4: HALT
  0xFF000001, // 5: TAKE 1
  0x617A0005, // 6: GOTO 5 ON a-z
  0x5F5F0005, // 7: GOTO 5 ON _
  0x415A0005, // 8: GOTO 5 ON A-Z
  0x30390005, // 9: GOTO 5 ON 0-9
  0x00FFFFFF, // 10: HALT
};

