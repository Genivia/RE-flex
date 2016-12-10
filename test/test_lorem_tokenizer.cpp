#ifndef REFLEX_CODE_DECL
#include "pattern.h"
#define REFLEX_CODE_DECL const reflex::Pattern::Opcode
#endif

REFLEX_CODE_DECL reflex_code_tokenizer[13] =
{
  0x617A0007, // 0: GOTO 7 ON a-z
  0x5F5F0007, // 1: GOTO 7 ON _
  0x415A0007, // 2: GOTO 7 ON A-Z
  0x30390007, // 3: GOTO 7 ON 0-9
  0x00FF0005, // 4: GOTO 5 ON \0-\xff
  0xFF000002, // 5: TAKE 2
  0x00FFFFFF, // 6: HALT
  0xFF000001, // 7: TAKE 1
  0x617A0007, // 8: GOTO 7 ON a-z
  0x5F5F0007, // 9: GOTO 7 ON _
  0x415A0007, // 10: GOTO 7 ON A-Z
  0x30390007, // 11: GOTO 7 ON 0-9
  0x00FFFFFF, // 12: HALT
};

