#ifndef REFLEX_CODE_DECL
#include "pattern.h"
#define REFLEX_CODE_DECL const reflex::Pattern::Opcode
#endif

REFLEX_CODE_DECL reflex_code_splitter[9] =
{
  0x85850004, // 0: GOTO 4 ON \x85
  0x20200004, // 1: GOTO 4 ON \x20
  0x090D0004, // 2: GOTO 4 ON \t-\r
  0x00FFFFFF, // 3: HALT
  0xFF000001, // 4: TAKE 1
  0x85850004, // 5: GOTO 4 ON \x85
  0x20200004, // 6: GOTO 4 ON \x20
  0x090D0004, // 7: GOTO 4 ON \t-\r
  0x00FFFFFF, // 8: HALT
};

