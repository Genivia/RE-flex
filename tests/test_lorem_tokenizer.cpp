#include <reflex/matcher.h>

#ifdef OS_WIN
#pragma warning(disable:4102)
#else
#pragma clang diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wunused-label"
#endif

void reflex_code_tokenizer(reflex::Matcher& m)
{
  int c0 = 0, c1 = c0;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S7;
  if (c1 == '_') goto S7;
  if ('A' <= c1 && c1 <= 'Z') goto S7;
  if ('0' <= c1 && c1 <= '9') goto S7;
  goto S5;

S5:
  m.FSM_TAKE(2);
  c0 = c1, c1 = m.FSM_CHAR();
  return m.FSM_HALT(c1);

S7:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S7;
  if (c1 == '_') goto S7;
  if ('A' <= c1 && c1 <= 'Z') goto S7;
  if ('0' <= c1 && c1 <= '9') goto S7;
  return m.FSM_HALT(c1);
}

