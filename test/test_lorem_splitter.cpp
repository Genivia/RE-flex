#include <reflex/matcher.h>

void reflex_code_splitter(reflex::Matcher& m)
{
  int c0, c1;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 133) goto S4;
  if (c1 == 32) goto S4;
  if (9 <= c1 && c1 <= 13) goto S4;
  return m.FSM_HALT(c1);

S4:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == 133) goto S4;
  if (c1 == 32) goto S4;
  if (9 <= c1 && c1 <= 13) goto S4;
  return m.FSM_HALT(c1);
}

