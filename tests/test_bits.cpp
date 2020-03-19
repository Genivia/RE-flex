// test bits.h

#include <reflex/bits.h>
#include <iostream>
#include <cassert>

using namespace reflex;

int main()
{
  static Bits digit('0', '9'); // bits '0' (48th bit) to '9' (57th bit)
  static Bits upper('A', 'Z'); // bits 'A' (65th bit) to 'Z' (92th bit)
  static Bits lower('a', 'z'); // bits 'a' (97th bit) to 'z' (122th bit)
  static Bits alnum(digit | upper | lower);
  if (upper.intersects(lower) == false)
    std::cout << "upper and lower are disjoint\n";
  // reflex::Bits alnum = digit | upper | lower;
  if (alnum.contains(digit) == true)
    std::cout << "digit is a subset of alnum\n";
  if (alnum['_'] == false)
    std::cout << "_ is not in alnum\n";
  alnum['_'] = true;
  if (alnum['_'] == true)
    std::cout << "_ is in updated alnum\n";
  alnum.lshift().rshift();
  std::cout << alnum.count() << " bits in alnum\n";
  for (size_t i = alnum.find_first(); i != reflex::Bits::npos; i = alnum.find_next(i))
    std::cout << (char)i;
  std::cout << std::endl;

  assert(digit.count() == 10);
  assert(alnum.count() == 63);

  Bits alpha(lower);
  alpha |= upper;

  assert(alpha.contains(lower) == true && alpha.contains(upper) == true);
  assert(alpha.intersects(lower) == true);
  assert(lower.intersects(upper) == false);

  Bits bucket;
  bucket.swap(alpha);
  bucket -= upper;
  bucket -= digit;

  assert(bucket == lower);
  assert(alpha.any() == false);
  assert(bucket.any() == true);
  assert(bucket.all() == false);

  for (size_t i = bucket.find_first(); i != Bits::npos; i = bucket.find_next(i))
    std::cout << (char)i;
  std::cout << std::endl;

  return 0;
}
