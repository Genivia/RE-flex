// Super fast search with fixed regex patterns compiled into FSM code
// This search uses a Bloom filter with SIMD (SSE2/AVX2/AVX512/AArch64)
//
// Build steps:
//
// 1. change PATTERN below as desired
// 2. build fastsearch by 'make fastsearch' or 'make -f Make fastsearch'
// 3. run 'fastsearch' to generate fastsearch_fsm.cpp
// 4. build fastsearch by 'make fastsearch' or 'make -f Make fastsearch'
//
// Usage:
//
// run 'fastsearch FILE' to search FILE fast, for example:
// ./fastsearch ../tests/lorem.txt
//
// How does this work?
//
// The first build step generates FSM code for PATTERN and hash data to speed
// up search with the following:
// - memchr() or AVX/SSE2/NEON-AArch64 optimized code
// - hash-based match predictor to search multiple words (works with any regex)
// - FSM direct code in C++ runs efficiently when a possible match is detected
//
// The FSM code is generated with reflex::Pattern option "o" (optimize).
// The predictor data is generated with reflex::Pattern option "p" (predict).
// The FSM code and predictor are saved to fastsearch_fsm.cpp with
// reflex::Pattern option "f=fastsearch_fsm.cpp;o;p".
//
// Suggestions:
//
// Use mmap(2) for large files, at least larger than 16K.  To use mmap call
// matcher.buffer(base, size + 1), where base is the mmap base address and
// size is the size of the file.
//
// See also:
//
// fastfind.l and mmap.l
//
// Output timing results:
//
// Compile with -DWARM_START_RUNS=100 to run the search 100 times on the same
// file, to display the average search time and number of matches found.

#include <reflex/matcher.h>
#include <reflex/timer.h>

// find words "elit" and "eleifend"
#define PATTERN "\\<(elit|eleifend)\\>"

// uncomment for performance testing
// #define WARM_START_RUNS 100

// externals, defined in fastsearch_fsm.cpp
extern void reflex_code_FSM(reflex::Matcher& m);
extern const reflex::Pattern::Pred reflex_pred_FSM[];

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    // without arguments, generate FSM and predictor, save to fastsearch_fsm.cpp
    reflex::Pattern(PATTERN, "f=fastsearch_fsm.cpp;o;p");
    std::cout << "Generated fastsearch_fsm.cpp for pattern \"" PATTERN "\"" << std::endl;
  }
  else
  {
    // with FILE argument, search FILE fast
    FILE *file = fopen(argv[1], "r");

    if (file == NULL)
    {
      perror("Cannot open file for reading");
      exit(EXIT_FAILURE);
    }

    // construct a pattern from the FSM code and predictor in fastsearch_fsm.cpp
    reflex::Pattern pattern(reflex_code_FSM, reflex_pred_FSM);
    reflex::Matcher matcher(pattern, file);

#ifdef WARM_START_RUNS

    reflex::timer_type t;
    reflex::timer_start(t);

    size_t hit = 0;

    for (int run = 0; run < WARM_START_RUNS; ++run)
    {
      rewind(file);
      matcher.input(file);
      while (matcher.find())
        ++hit;
    }

    printf("Search took %.3g ms for %zu matches\n", reflex::timer_elapsed(t)/WARM_START_RUNS, hit/WARM_START_RUNS);

#else

    while (matcher.find())
    {
      // display line and column numbers
      printf("%zu,%zu: ", matcher.lineno(), matcher.columno());

      // we use matcher.begin() and matcher.size() to extract the match
      fwrite(matcher.begin(), 1, matcher.size(), stdout);
      putchar('\n');
    }

#endif

    fclose(file);
  }
}
