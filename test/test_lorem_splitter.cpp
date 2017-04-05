#include <reflex/matcher.h>

void reflex_code_splitter(reflex::Matcher& m)
{
  int c0 = 0, c1 = c0;
  m.FSM_INIT(c1);

S0:
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == ' ') goto S3;
  if ('\t' <= c1 && c1 <= '\r') goto S3;
  return m.FSM_HALT(c1);

S3:
  m.FSM_TAKE(1);
  c0 = c1, c1 = m.FSM_CHAR();
  if (c1 == ' ') goto S3;
  if ('\t' <= c1 && c1 <= '\r') goto S3;
  return m.FSM_HALT(c1);
}

