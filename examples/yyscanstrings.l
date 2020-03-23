// Example Flex yyin, yyrestart, yy_scan_string, yy_scan_bytes, yy_scan_buffer
// Flex reentrant allows us to pass the lexer to these functions, otherwise a
// global YY_SCANNER is expected, which can be generated with option --bison
//
// Some notes on Flex reentrant use:
// - yyscanner can only be used in lexer methods and in rules, in fact
//   yyscanner is the same as the 'this' pointer;
// - yyin can only be used in lexer methods and in rules, use yyget_in(&lexer)
//   with a lexer object;
// - YY_CURRENT_BUFFER can only be used in lexer methods and in rules, use
//   yyget_current_buffer(&lexer) with a lexer object.

%top{
#include <algorithm>
}

%class{
  public:
    void do_lex();
    std::vector<std::string> words;
}

%option flex reentrant

%%
\w+     words.push_back(str());
.|\n    // ignore
%%

int main()
{
  yyFlexLexer lexer;

  lexer.do_lex();

  std::copy(lexer.words.begin(), lexer.words.end(), std::ostream_iterator<std::string>(std::cout, " "));
  std::cout << std::endl;
}

void yyFlexLexer::do_lex()
{
  // scan a string using reentrant yyget_in() as no input is set yet
  std::string string("Lorem ipsum dolor sit amet,");
  yyin = string;
  yylex();

  // restart scanning with new string input using reentrant yyrestart()
  // note: yyget_in() won't work here because it does not reset the scanner
  yyrestart("consectetur adipiscing elit.", yyscanner);
  yylex();

  // scan character string using yy_scan_string()
  yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
  yy_scan_string("Quisque at accumsan turpis,", yyscanner);
  yylex();

  // scan bytes up to specified length using yy_scan_bytes()
  yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
  yy_scan_bytes("non feugiat magna.", 18, yyscanner);
  yylex();

  // scan bytes in place using yy_scan_buffer(), buffer content is changed!!
  // note: string must be \0-terminated and the length includes final \0
  yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
  char scan_in_place[29] = "Quisque sed fringilla felis.";
  yy_scan_buffer(scan_in_place, 29, yyscanner);
  yylex();

  // scan wide std::wstring
  yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
  std::wstring wstring(L"Ut at ullamcorper metus,");
  yy_scan_wstring(wstring, yyscanner);
  yylex();

  // scan wide string
  yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
  yy_scan_wstring(L"eu ornare lorem.", yyscanner);
  yylex();
}
