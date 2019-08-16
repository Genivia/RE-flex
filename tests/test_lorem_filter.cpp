#include <reflex/matcher.h>

#ifdef OS_WIN
#pragma warning(disable:4102)
#else
#pragma clang diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wunused-label"
#endif

void reflex_code_filter(reflex::Matcher& m)
{
  int c0 = 0, c1 = c0;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S5;
  if (c1 == '_') goto S5;
  if ('A' <= c1 && c1 <= 'Z') goto S5;
  if ('0' <= c1 && c1 <= '9') goto S5;
  return m.FSM_HALT(c1);

S5:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if ('a' <= c1 && c1 <= 'z') goto S5;
  if (c1 == '_') goto S5;
  if ('A' <= c1 && c1 <= 'Z') goto S5;
  if ('0' <= c1 && c1 <= '9') goto S5;
  return m.FSM_HALT(c1);
}

