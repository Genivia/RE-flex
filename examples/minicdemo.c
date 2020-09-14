// Mini C demo by Robert van Engelen
// The minic compiler is built from minic.l, minic.y, and minic.hpp with:
//   $ bison -d minic.y
//   $ reflex minic.l
//   $ c++ -o minic parser.cpp scanner.cpp -lreflex
// the last step without RE/flex install: c++ -I. -I../include -o minic parser.cpp scanner.cpp ../lib/libreflex.a
//
// compile this file with the minic compiler to a minicdemo.class file:
//   $ ./minic minicdemo.c
//
// to view the contents of the generated minicdemo.class file:
//   $ javap -c minicdemo
//
// to run the code in a JVM:
//   $ java minicdemo

// #include "file.h" // #include files is supported, but #define is not

// because minic is a classic one-pass compiler, functions should be declared as a "function prototype" when not defined before used
void init_statics();

// a recursive function with conditional expression
int fac(int n)
{
  return n <= 0 ? 1 : n * fac(n - 1);
}

// static variables are declared outside of function scopes and must be declared before used
float pi; // a static variable; variables are not implicitly initialized, using them uninitialized is a runtime error

// the main function may return int or void and takes no arguments, command line arguments are $1, $2, ..., $n with n = #$
int main()
{
  // locals are declared at the top of a function, not in blocks, and may be int, float or string
  int i;    // integers are signed 32 bit
  float x;  // floats are 64 bit (double precision)
  string s; // strings are immutable and contain 16 bit wide chars, they are assigned and passed to functions by reference

  // arrays of ints, floats, strings, and arrays-of-arrays can be declared
  string[] words;
  float[][] matrix;

  // variables are not implicitly initialized, we should do this explicitly
  i = 0;
  x = 0.0;
  s = "";

  // arrays are assigned and passed to functions by reference, they should be allocated with 'new' and are garbage collected
  words = new string[2];    // array of two strings
  words[0] = "hello";
  words[1] = "world";
  matrix = new float[][2];  // array of two arrays of floats
  matrix[0] = new float[3]; // array of three floats
  matrix[1] = new float[3];

  // # returns the length of an array or string
  i = #words;    // = 2
  i = #words[0]; // = 5

  // the usual operators may be used with integers and floats (but not the bit-ops for floats):
  // ?: || && == != <= < >= > << >> & | ^ + = * / % ! ~ = <<= >>= &= |= ^= += -= *= /= %= ++ --
  // the following operators may be used with string operands:
  // == != <= < >= > + = +=
  // mismatching operand types are coerced, while mismatching function arguments are not

  // the println statement takes one or more expressions and prints each value on a separate line
  println "MINIC DEMO", "----------";

  // the print statement takes one or more expressions to print without inserting newlines
  print "command line arguments:";

  // #$ is the number of arguments of main(), $i is the i'th string argument of main()
  for (i = 1; i <= #$; ++i)
    print " ", $i;

  // strings may contain escapes: \a, \b, \t, \n, \v, \f, \r, \\, and \"
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
  print "length(\"ab\")           \t= ", length("ab"),          " (same as #\"ab\")\n";
  print "empty(\"\")              \t= ", empty(""),             "\n";
  print "\"ab\" == \"cd\"         \t= ", "ab" == "cd",          "\n";
  print "\"ab\" < \"cd\"          \t= ", "ab" < "cd",           "\n";
  print "compare(\"ab\", \"cd\")  \t= ", compare("ab", "cd"),   "\n";
  print "\"ab\" + \"cd\"          \t= ", "ab" + "cd",           "\n";
  print "concat(\"ab\", \"cd\")   \t= ", concat("ab", "cd"),    " (same as \"ab\"+\"cd\")\n";
  print "at(\"abb\", 1)           \t= ", at("abb", 1),          " (same as \"abb\"[1])\n";
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
  print "substr(\"abc\", 1, 2)    \t= ", substr("abc", 1, 2),   "\n";
  print "trim(\" abc \")          \t= ", trim(" abc "),         "\n";
  print "matches(\"ab\", \"\\w+\")\t= ", matches("ab", "\\w+"), "\n";
  print "\n";

  // converting string to int or float
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
  print "\n";

  // converting ints and floats to string
  print "bin(123)      \t= ", "\"", bin(123), "\"\n";
  print "hex(123)      \t= ", "\"", hex(123), "\"\n";
  print "oct(123)      \t= ", "\"", oct(123), "\"\n";
  print "dec(123)      \t= ", "\"", dec(123), "\"\n";
  print "dec(0.5)      \t= ", "\"", dec(0.5), "\"\n";
  print "\n";

  // casting
  print "(int)1.5      \t= ", (int)1.5, "\n";
  print "(float)1      \t= ", (float)1, "\n";
  print "(string)97    \t= ", "\"", (string)'a', "\"\n";
  print "(string)'a'   \t= ", "\"", (string)'a', "\"\n";
  print "\"ab\" + 99   \t= ", "\"", "ab" + 99,   "\"\n";
  print "\"ab\" + 'c'  \t= ", "\"", "ab" + 'c',  "\"\n";
  print "\n";

  // converting int (32 bit) to float (64 bit) and back is lossless
  print "max int = ", i = x = i = 2147483647, "\n\n";

  // we have two system functions
  print "getenv(\"HOME\")\t= ", getenv("HOME"), "\n";
  print "exit(0)\n";

  exit(0);
}

// function to initialize statics
void init_statics()
{
  pi = 3.141592653589793; // initialize a static variable
}
