// Example RE/flex string scanning with in() and matcher().buffer()

%top{
#include <algorithm>
}

%class{
  public:
    void do_lex();
    std::vector<std::string> words;
}

%%
\w+     words.push_back(str());
.|\n    // ignore
%%

int main()
{
  Lexer lexer;

  lexer.do_lex();

  std::copy(lexer.words.begin(), lexer.words.end(), std::ostream_iterator<std::string>(std::cout, " "));
  std::cout << std::endl;
}

void Lexer::do_lex()
{
  // scan std::string
  std::string string("Lorem ipsum dolor sit amet,");
  in(string);
  lex();

  // scan char* string
  in("consectetur adipiscing elit.");
  lex();

  // scan wchar_t* string
  in(L"Quisque at accumsan turpis,");
  lex();

  // scan bytes up to specified length
  in("non feugiat magna.", 18);
  lex();

  // scan bytes in place using matcher().buffer(), buffer content is changed!!
  // note: string must be \0-terminated and the length includes final \0
  char scan_in_place[29] = "Quisque sed fringilla felis.";
  buffer(scan_in_place, 29);
  lex();

  // scan wide std::wstring
  std::wstring wstring(L"Ut at ullamcorper metus,");
  in(wstring);
  lex();

  // scan wide string
  in(L"eu ornare lorem.");
  lex();
}
