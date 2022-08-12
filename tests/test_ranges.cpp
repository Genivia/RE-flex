// test ranges.h

#include <reflex/bits.h>
#include <reflex/ranges.h>
#include <reflex/timer.h>
#include <bitset>
#include <map>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstdio>

enum Color { RED, GREEN, BLUE };

struct Pos {
  typedef int32_t type;
  type n;
  Pos(type n) : n(n) { }
  operator int32_t() const { return n; }
  bool is_lazy() const { return n & 0x100; }
  Pos& mk_lazy() { n |= 0x100; return *this; }
};

using namespace reflex;

int main()
{

  ORanges<Pos> positions(0, 3);
  
  typedef ORanges<int> ints;

  ints A(0, 3);   // A = 0 - - 3 
  ints B(0, 4);   // B = 0 - - - 4
  ints C(0, 5);   // C = 0 - - - - 5
  ints D(2, 6);   // D = - - 2 - - - 6
  ints E(4, 9);   // E = - - - - 4 - - - - 9
  ints F(5, 9);   // F = - - - - - 5 - - - 9
  ints G(0, 3);
  G.insert(5, 9); // G = 0 - - 3 - 5 - - - 9
  ints H(0, 1);
  H.insert(7, 9); // H = 0 1 - - - - - 7 - 9
  ints I(2, 3);
  I.insert(5, 6); // I = - - 2 3 - 5 6 - - -

  assert((C | F) == ints(0, 9));
  assert((C - F) == ints(0, 4));
  assert((C & F) == ints(5));

  assert((F | C) == ints(0, 9));
  assert((F - C) == ints(6, 9));
  assert((F & C) == ints(5));

  assert((B | F) == ints(0, 9));
  assert((B - F) == ints(0, 4));
  assert((B & F) == ints());

  assert((F | B) == ints(0, 9));
  assert((F - B) == ints(5, 9));
  assert((F & B) == ints());

  assert((C | E) == ints(0, 9));
  assert((C - E) == ints(0, 3));
  assert((C & E) == ints(4, 5));

  assert((E | C) == ints(0, 9));
  assert((E - C) == ints(6, 9));
  assert((E & C) == ints(4, 5));

  assert((D | G) == ints(0, 9));
  assert((D - G) == ints(4));
  assert((D & G) == I);

  assert((G | D) == ints(0, 9));
  assert((G - D) == H);
  assert((G & D) == I);

  assert((H | D) == ints(0, 9));
  assert((H - D) == H);
  assert((H & D) == ints());

  assert((I | D) == D);
  assert((I - D) == ints());
  assert((I & D) == I);

  assert(B.contains(A)   == true);
  assert(G.contains(H)   == true);
  assert(G.contains(I)   == true);
  assert(D.intersects(A) == true);
  assert(D.intersects(E) == true);
  assert(D.intersects(G) == true);
  assert(H.intersects(I) == false);

  std::map<ints,ints> map;

  map[A] = B;
  map[B] = B;
  map[C] = D;
  std::cout << "Map of " << map.size() << " ints to ints:" << std::endl;
  for (std::map<ints,ints>::iterator i = map.begin(); i != map.end(); ++i)
  {
    for (ints::iterator j = i->first.begin(); j != i->first.end(); ++j)
      std::cout << "[" << j->first << "," << j->second << ")";
    std::cout << " -> ";
    for (ints::iterator j = i->second.begin(); j != i->second.end(); ++j)
      std::cout << "[" << j->first << "," << j->second << ")";
    std::cout << std::endl;
  }

  Ranges<char> chars;
  chars.insert('0', '9');
  chars.insert('a', 'z');
  chars.insert('A', 'Z');
  chars.insert('a', 'c');
  chars.insert('-', '-');
  std::cout << "Set of " << chars.size() << " ranges:" << std::endl;
  for (Ranges<char>::const_iterator i = chars.begin(); i != chars.end(); ++i)
    std::cout << "[" << i->first << "," << i->second << "]" << std::endl;
  if (chars.find('R') != chars.end())
    std::cout << "'R' is in the set" << std::endl;
  for (Ranges<char>::const_iterator i = chars.find('0', 'Z'); i != chars.end() && !Ranges<char>::key_compare()(std::pair<char,char>('0', 'Z'), *i) /* i->first <= 'Z' */; ++i)
    std::cout << "[" << i->first << "," << i->second << "] overlaps [0,Z]" << std::endl;
  assert(chars.size() == 4);
  assert(chars.lo() == '-');
  assert(chars.hi() == 'z');

/* displays:
Set of 4 ranges:
[-,-]
[0,9]
[A,Z]
[a,z]
'R' is in the set
[0,9] overlaps [0,Z]
[A,Z] overlaps [0,Z]
*/

  ORanges<char> bytes;
  bytes.insert(32, 126);
  std::cout << "Set of " << bytes.size() << " open-ended byte ranges up to max 126:" << std::endl;
  for (ORanges<char>::const_iterator i = bytes.begin(); i != bytes.end(); ++i)
    std::cout << "[" << (int)i->first << "," << (int)i->second << ")" << std::endl;
  assert(bytes.size() == 1 && bytes.find(126) != bytes.end());

  Ranges<Color> colors1;
  colors1.insert(GREEN, BLUE);

  ORanges<Color> colors2;
  colors2.insert(GREEN, BLUE);

  Ranges<float> intervals;
  intervals.insert(1.0, 2.0);
  intervals.insert(2.0, 3.0);
  intervals.insert(-1.0, 0.0);
  std::cout << "Set of " << intervals.size() << " intervals:" << std::endl;
  for (Ranges<float>::const_iterator i = intervals.begin(); i != intervals.end(); ++i)
    std::cout << "[" << i->first << "," << i->second << "]" << std::endl;
  if (intervals.find(2.5) != intervals.end())
    std::cout << "2.5 is in intervals" << std::endl;
  for (Ranges<float>::const_iterator i = intervals.find(0.0, 1.0); i != intervals.end() && i->first <= 1.0; ++i)
    std::cout <<  "[" << i->first << "," << i->second << "] overlaps with [0.0,1.0]" << std::endl;
  if (intervals.intersects(Ranges<float>(2.5, 10.0)))
    std::cout << "intersects [2.5,10.0]" << std::endl;
  if (intervals.contains(Ranges<float>(1.0, 2.5)))
    std::cout << "contains [1.0,2.5]" << std::endl;
  assert(intervals.size() == 2);

  // example to use open-ended ranges over ordinal types to compress the set
  ORanges<int> ivals(0);
  ivals.insert(100, 200);
  ivals.insert(300, 400);
  ivals.insert(200, 300);
  std::cout << "Set of " << ivals.size() << " open-ended int ranges:" << std::endl;
  for (Ranges<int>::const_iterator i = ivals.begin(); i != ivals.end(); ++i)
    std::cout << "[" << i->first << "," << i->second << ")" << std::endl;
  if (ivals.find(200) != ivals.end())
    std::cout << "200 is in the set" << std::endl;
  if (ivals.find(99) == ivals.end())
    std::cout << "99 is not in the set" << std::endl;
  if (ivals.find(400) != ivals.end())
    std::cout << "400 is in the set" << std::endl;
  if (ivals.find(401) == ivals.end())
    std::cout << "401 is not in the set" << std::endl;

/*displays:
Set of 1 open-ended ranges:
[100,401)
200 is in the set
99 is not in the set
401 is not in the set
*/

  ivals.erase(250, 350);
  for (Ranges<int>::const_iterator i = ivals.begin(); i != ivals.end(); ++i)
    std::cout << "[" << i->first << "," << i->second << ")" << std::endl;

  ORanges<int> isec;
  isec.insert(300, 700);
  isec.insert(800, 900);
  assert(ivals.intersects(isec) == true);
  ivals.insert(500, 800);
  ivals &= isec;
  for (Ranges<int>::const_iterator i = ivals.begin(); i != ivals.end(); ++i)
    std::cout << "[" << i->first << "," << i->second << ")" << std::endl;
  assert(ivals.contains(ivals) == true);
  assert(isec.contains(ivals) == true);
  assert(ivals.contains(isec) == false);
  ivals -= isec;
  assert(ivals.empty() == true);

  ORanges<int> ichar1, ichar2;
  ichar1.insert('0','@');
  ichar1.insert('^','`');
  ichar2.insert('0','9');
  ichar2.insert('A','Z');
  ichar2.insert('_','_');
  ichar2.insert('a','z');
  ichar1 += ichar2;
  for (Ranges<int>::const_iterator i = ivals.begin(); i != ivals.end(); ++i)
    std::cout << "[" << (char)i->first << "," << (char)i->second << ")" << std::endl;
  assert(ichar1.contains(ichar2) == true);
  assert(ichar1.lo() == '0');
  assert(ichar1.hi() == 'z');

  unsigned int seed = 1; // time(0);
  int len1, len2;
  int sum = 0;
  timer_type t;

  std::cerr << "Random 256 range insertions timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 5000; ++run)
  {
    Ranges<int> ints1, ints2;
    srand(seed);
    for (int i = 0; i < 256; ++i)
    {
      int n = rand() % 1024;
      ints1.insert(n, n + 3);
      int m = rand() % 1024;
      ints2.insert(m, m + 3);
    }
    len1 = ints1.size();
    len2 = ints2.size();
  }
  float dt = timer_elapsed(t);
  fprintf(stderr, "%d+%d ranges, elapsed real time = %g ms\n", len1, len2, dt);

  std::cerr << "Random 256 o-range insertions timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 5000; ++run)
  {
    ORanges<int> ints1, ints2;
    srand(seed);
    for (int i = 0; i < 256; ++i)
    {
      int n = rand() % 1024;
      ints1.insert(n, n + 3);
      int m = rand() % 1024;
      ints2.insert(m, m + 3);
      sum += ints1.intersects(ints2);
      // sum += (ints1 & ints2).size();
    }
    len1 = ints1.size();
    len2 = ints2.size();
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "%d+%d ranges sum=%d, elapsed real time = %g ms\n", len1, len2, sum, dt);

  std::cerr << "Raw 0..255 range insertion timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 10000; ++run)
  {
    Ranges<int> ints;
    for (int i = 0; i < 256; ++i)
      ints.insert(i);
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "elapsed real time = %g ms\n", dt);

  std::cerr << "Raw 0..255 o-range insertion timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 10000; ++run)
  {
    ORanges<int> ints;
    for (int i = 0; i < 256; ++i)
      ints.insert(i);
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "elapsed real time = %g ms\n", dt);

  std::cerr << "Raw 0..255 bits insertion timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 10000; ++run)
  {
    Bits bits;
    for (int i = 0; i < 256; ++i)
      bits[i] = true;
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "elapsed real time = %g ms\n", dt);

  std::cerr << "Raw 0..255 std::set insertion timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 10000; ++run)
  {
    std::set<int> ints;
    for (int i = 0; i < 256; ++i)
      ints.insert(i);
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "elapsed real time = %g ms\n", dt);

  std::cerr << "Raw 0..255 std::bitset<256> insertion timings" << std::endl;
  timer_start(t);
  for (int run = 0; run < 10000; ++run)
  {
    std::bitset<256> ints;
    for (int i = 0; i < 256; ++i)
      ints[i] = true;
  }
  dt = timer_elapsed(t);
  fprintf(stderr, "elapsed real time = %g ms\n", dt);

  return 0;
}
