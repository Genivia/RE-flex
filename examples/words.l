// words.l search for words in a file to display info

%top{
#include <cstdlib>
}

// we search for Unicode words with option find, not scanning the input as a lexer
%option fast find unicode

// a Words class with search() lexer to find words
%option class=Words lex=search

%{
  class Words : public Lexer {
    public:
      Words() : words(0), lines(0), sum(0) { }
      ~Words()
      {
        std::cout << words << " words in " << lines << " lines, " <<
          static_cast<float>(sum)/words << " characters per word, " <<
          static_cast<float>(words)/lines << " words per line" << std::endl;
      }
      using Lexer::search; // to set lexing input/output, then calls search()
      int search(); // the generated lexer/searcher
    private:
      void analyze()
      {
        first[reflex::utf8(wchr(), first)] = '\0'; // convert first wide char
        last[reflex::utf8(wchr_last(), last)] = '\0'; // convert last wide char
        size_t length = wsize();
        std::cout << lineno() << "," << columno() + 1 << "-" << columno_end() << ": " <<
          text() << " [" << first << "," << last << "] <" << wchr_next() << ">\n";
        ++words;
        lines = lineno();
        sum += length;
      }
      size_t words;
      size_t lines;
      size_t sum;
      char first[8];
      char last[8];
  };
%}

word    \w+

%%

{word}  analyze();

%%

int main(int argc, char **argv)
{
  FILE *fd = stdin;
  if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
  {
    std::cerr << "Cannot open " << argv[1] << " for reading" << std::endl;
    exit(EXIT_FAILURE);
  }
  Words analyzer;
  analyzer.search(fd);
  if (fd != stdin)
    fclose(fd);
}
