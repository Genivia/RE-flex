#include <reflex/matcher.h>

void reflex_code_tokenizer(reflex::Matcher& m)
{
  int c0, c1;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if (97 <= c1 && c1 <= 122) goto S7;
  if (c1 == 95) goto S7;
  if (65 <= c1 && c1 <= 90) goto S7;
  if (48 <= c1 && c1 <= 57) goto S7;
  if (0 <= c1) goto S5;
  return m.FSM_HALT(c1);

S5:
  m.FSM_TAKE(2);
  return m.FSM_HALT(c1);

S7:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if (97 <= c1 && c1 <= 122) goto S7;
  if (c1 == 95) goto S7;
  if (65 <= c1 && c1 <= 90) goto S7;
  if (48 <= c1 && c1 <= 57) goto S7;
  return m.FSM_HALT(c1);
}

