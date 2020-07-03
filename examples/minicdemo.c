// Mini C feature demo by Robert van Engelen
//
// compile this file with:
//   ./minic minicdemo.c
//
// to view the contents of the generated minicdemo.class file:
//   javap -c minicdemo
//
// to run the code:
//   java minicdemo

// functions must be declared (with a prototype) before referenced
void init_statics();
int fac(int n);

// static variables are declared outside of function scopes and must be declared before referenced
float pi; // a static variable; variables are not implicitly initialized

// the main function may return int or void and takes no arguments (its arguments are accessed with $1, $2, ...)
int main()
{
  // locals are declared at the top of a function, not in blocks, and may be int, float or string
  int i;    // integers are signed 32 bit
  float x;  // floats are 64 bit
  string s; // strings are immutable and contain 16 bit wide chars

  // variables are not implicitly initialized, should do this explicitly
  i = 0;
  x = 0.0;
  s = "";

  // the usual operators may be used with integers and floats (but no bit-ops for floats):
  // ?: || && == != <= < >= > << >> & | ^ + = * / % ! ~ = <<= >>= &= |= ^= += -= *= /= %= ++ --
  // the following operators may be used with string operands:
  // == != <= < >= > + = +=
  // mismatching operand types are coerced, while mismatching function arguments are not

  // the print statement takes one or more expressions to print
  print "command line arguments:";

  // #$ is the number of arguments of main(), $i is the i'th string argument of main()
  for (i = 1; i <= #$; ++i)
    print " ", $i;

  // strings may contain \a, \b, \t, \n, \v, \f, \r, \\, and \"
  print "\n\n";

  // besides the for loop we also have while and do-while loops, if-else and switch control flow
  while (true) // 'true' is essentially the same as 1 but optimized for logic, same for 'false'
  {
    if (--i <= 0)
      break; // 'break' and 'continue' work as expected
  }

  // functions may or may not return values; functions that return void have no return value
  init_statics();                                       // first initialize static variables
  print "static variable pi = ", pi, "\n\n";            // print static variable value
  print "recursive function fac(7) = ", fac(7), "\n\n"; // print return value of recursive function

  // operations on strings
  print "#\"ab\"                  \t= ", #"ab",                 "\n";
  print "length(\"ab\")           \t= ", length("ab"),          " (same as #)\n";
  print "empty(\"\")              \t= ", empty(""),             "\n";
  print "\"ab\" == \"cd\"         \t= ", "ab" == "cd",          "\n";
  print "\"ab\" < \"cd\"          \t= ", "ab" < "cd",           "\n";
  print "compare(\"ab\", \"cd\")  \t= ", compare("ab", "cd"),   "\n";
  print "\"ab\" + \"cd\"          \t= ", "ab" + "cd",           "\n";
  print "concat(\"ab\", \"cd\")   \t= ", concat("ab", "cd"),    " (same as +)\n";
  print "at(\"abb\", 1)           \t= ", at("abb", 1),          "\n";
  print "find(\"ab\", 98)         \t= ", find("abb", 98),       "\n";
  print "find(\"abb\", 98, 1)     \t= ", find("abb", 98, 1),    "\n";
  print "find(\"abb\", \"b\")     \t= ", find("abb", "b"),      "\n";
  print "find(\"abb\", \"b\", 1)  \t= ", find("abb", "b", 1),   "\n";
  print "rfind(\"abb\", 98)       \t= ", rfind("abb", 98),      "\n";
  print "rfind(\"abb\", 98, 1)    \t= ", rfind("abb", 98, 1),   "\n";
  print "rfind(\"abb\", \"b\")    \t= ", rfind("abb", "b"),     "\n";
  print "rfind(\"abb\", \"b\", 1) \t= ", rfind("abb", "b", 1),  "\n";
  print "lower(\"Abc\")           \t= ", lower("Abc"),          "\n";
  print "upper(\"Abc\")           \t= ", upper("Abc"),          "\n";
  print "substr(\"abc\", 1)       \t= ", substr("abc", 1),      "\n";
  print "substr(\"abc\", 0, 1)    \t= ", substr("abc", 0, 1),   "\n";
  print "trim(\" abc \")          \t= ", trim(" abc "),         "\n";
  print "matches(\"ab\", \"\\w+\")\t= ", matches("ab", "\\w+"), "\n";
  print "strtoi(\"123\")          \t= ", strtoi("123"),         "\n";
  print "strtoi(\"ff\", 16)       \t= ", strtoi("ff", 16),      "\n";
  print "strtof(\"1.3\")          \t= ", strtof("1.3"),         "\n";
  print "\n";

  // operations on ints and floats
  print "abs(-123)     \t= ", abs(-123),      "\n";
  print "abs(-0.5)     \t= ", abs(-0.5),      "\n";
  print "sgn(-0.5)     \t= ", sgn(-0.5),      "\n";
  print "floor(0.5)    \t= ", floor(0.5),     "\n";
  print "ceil(0.5)     \t= ", ceil(0.5),      "\n";
  print "sin(0.5)      \t= ", sin(0.5),       "\n";
  print "cos(0.5)      \t= ", cos(0.5),       "\n";
  print "tan(0.5)      \t= ", tan(0.5),       "\n";
  print "sinh(0.5)     \t= ", sinh(0.5),      "\n";
  print "cosh(0.5)     \t= ", cosh(0.5),      "\n";
  print "tanh(0.5)     \t= ", tanh(0.5),      "\n";
  print "asin(0.5)     \t= ", asin(0.5),      "\n";
  print "acos(0.5)     \t= ", acos(0.5),      "\n";
  print "atan(0.5)     \t= ", atan(0.5),      "\n";
  print "deg(3.14)     \t= ", deg(3.14),      "\n";
  print "rad(180.0)    \t= ", rad(180.0),     "\n";
  print "log(0.5)      \t= ", log(0.5),       "\n";
  print "log10(0.5)    \t= ", log10(0.5),     "\n";
  print "exp(0.5)      \t= ", exp(0.5),       "\n";
  print "pow(2.0, 0.5) \t= ", pow(2.0, 0.5),  "\n";
  print "sqrt(2.0)     \t= ", sqrt(2.0),      "\n";
  print "min(1, 2)     \t= ", min(1, 2),      "\n";
  print "min(0.5, -0.5)\t= ", min(0.5, -0.5), "\n";
  print "max(1, 2)     \t= ", max(1, 2),      "\n";
  print "max(0.5, -0.5)\t= ", max(0.5, -0.5), "\n";
  print "str(0.5)      \t= ", str(0.5),       "\n";
  print "bin(123)      \t= ", bin(123),       "\n";
  print "hex(123)      \t= ", hex(123),       "\n";
  print "oct(123)      \t= ", oct(123),       "\n";
  print "\n";

  // coercing int to float and back is lossless
  print "max int = ", i = x = i = 2147483647, "\n\n";

  // we have two system functions
  print "getenv(\"HOME\")\t= ", getenv("HOME"), "\n";
  print "exit(0)\n";

  exit(0);
}

// functions must be declared before being referenced
void init_statics()
{
  pi = 3.141592653589793; // initialize a static variable
}

// even recursive functions must be declared before self-referenced (this begs for compiler improvements)
int fac(int n)
{
  return n == 0 ? 1 : n * fac(n - 1);
}
