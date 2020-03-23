// A fast JSON parser and JSON writer
// Written by Robert van Engelen
//
// Passes tests suggested by <http://seriot.ch/parsing_json.php>
//
// This example demonstrates:
// 1.  %o flex compatible actions yytext and BEGIN
// 2.  %o freespace to space regular expressions between concat and alternations
// 3.  %o dotall mode: . matches \n
// 4.  %o unicode mode: . mathes any Unicode character
// 5.  %x STRING state with rules to convert a JSON string to std::wstring
// 6.  \p{Non_ASCII_Unicode} matches any non-ASCII Unicode character
// 7.  JSON value types are indicated with tokens '0', 't', 'f', '#', '$'
// 8.  C++ yyFlexLexer class members to store lexer values (MT-safe)
// 9.  JSONParser is a recursive descent parser for JSON
// 10. JSONParser inherits yyFlexLexer, for yyFlexLexer::yylex() and lexer state
// 11. JSONParser recurses 100 levels deep max (MAXLEVEL)
// 12. JSONParser accepts 1000 items per array/object max (MAXSIZE)
// 13. main() reads a FILE* for automatic UTF-16/32 normalization to UTF-8
// 14. JSON class stores parsed JSON data
// 15. JSON class instances are writable to std::ostream
// 16. wstr() to get the wide string matched, i.e. converted from yytext UTF-8

%top{
  #include <stdlib.h> // strtoul(), strtod()
  #include <iostream> // std::cout etc.
  #include <iomanip>  // std::setw
  #include <vector>   // to store JSON arrays
  #include <map>      // to store JSON objects
}

// define yyFlexLexer class variables to collect values in the lexer rules
%class{
 protected:
  double       number; // token value for token '#' (number)
  std::wstring string; // token value for token '$' (string)
}

%o fast flex freespace dotall unicode

digit   [0-9]
digit1  [1-9]
digits  {digit}+
int     -? {digit} | -? {digit1} {digits}
frac    \. {digits}
exp     [eE] [-+]? {digits}
number  {int} {frac}? {exp}?

%x STRING

%%

[ \t\n\r]+      { /* ignore white space */ }
[\]\[}{,:]      { return yytext[0]; }
null            { return '0'; }
true            { return 't'; }
false           { return 'f'; }
{number}        { number = strtod(yytext, NULL); return '#'; }
\"              { string.clear(); BEGIN STRING; }

<STRING>{
\"                      { BEGIN INITIAL; return '$'; }
\\ ["\\/]               { string.push_back(yytext[1]); }
\\ b                    { string.push_back('\b'); }
\\ t                    { string.push_back('\t'); }
\\ n                    { string.push_back('\n'); }
\\ f                    { string.push_back('\f'); }
\\ r                    { string.push_back('\r'); }
\\ u [[:xdigit:]]{4}    { string.push_back(strtoul(yytext + 2, NULL, 16)); }
[\]-\x7f\x20-\[]        { string.push_back(yytext[0]); }
\p{Non_ASCII_Unicode}   { string.append(wstr()); }
}

<*>.            { return '!'; /* error */ }

%%

// JSON value
class JSON {
 public:
  JSON() : tag(UND) { }
  // one of: undefined, null, boolean, number, string, array, object, or error
  enum Tag { UND, NUL, BOO, NUM, STR, ARR, OBJ, ERR } tag;
  bool                        boolean;
  double                      number;
  std::wstring                string;
  std::vector<JSON>           array;
  std::map<std::wstring,JSON> object;
  // convert to bool, double, or std::wstring
  operator bool()         const { return this->boolean; }
  operator double()       const { return this->number; }
  operator std::wstring() const { return this->string; }
  // assign bool, double, std::wstring
  JSON& operator=(bool b)
  {
    this->tag = BOO;
    this->boolean = b;
    return *this;
  }
  JSON& operator=(double n)
  {
    this->tag = NUM;
    this->number = n;
    return *this;
  }
  JSON& operator=(const std::wstring& s)
  {
    this->tag = STR;
    this->string = s;
    return *this;
  }
  // array accessor
  JSON& operator[](size_t i)
  {
    this->tag = ARR;
    if (i >= array.size())
      this->array.resize(i + 1);
    return this->array[i];
  }
  // object accessor
  JSON& operator[](const wchar_t *p)
  {
    this->tag = OBJ;
    return this->object[p];
  }
  JSON& operator[](const std::wstring& p)
  {
    this->tag = OBJ;
    return this->object[p];
  }
};

// JSON parser inherits yyFlexLexer scanner to invoke yylex()
class JSONParser : public yyFlexLexer {
 public:
  JSONParser(FILE *fd = NULL) : yyFlexLexer(fd), level(0) { }
  JSON::Tag parse(JSON& data)
  {
    int token = yylex();
    switch (token) {
      case '0':                       return data.tag = JSON::NUL;
      case 't': data.boolean = true;  return data.tag = JSON::BOO;
      case 'f': data.boolean = false; return data.tag = JSON::BOO;
      case '#': data.number = number; return data.tag = JSON::NUM;
      case '$': data.string = string; return data.tag = JSON::STR;
      case '[': return parse_array(data);
      case '{': return parse_object(data);
      default : return error(token, data);
    }
  }
 protected:
  JSON::Tag parse_array(JSON& data)
  {
    if (++level > MAXLEVEL)
      return JSON::ERR;
    for (size_t len = 0; len < MAXSIZE; ++len) {
      JSON item;
      switch (parse(item)) {
        case JSON::NUL:
        case JSON::BOO:
        case JSON::NUM:
        case JSON::STR:
        case JSON::ARR:
        case JSON::OBJ: data.array.push_back(item); break;
        case JSON::UND: --level; return len == 0 ? data.tag = JSON::ARR : JSON::ERR;
        default       : return JSON::ERR;
      }
      int token = yylex();
      if (token == ']') {
        --level;
        return data.tag = JSON::ARR;
      }
      if (token != ',')
        return JSON::ERR;
    }
    return JSON::ERR;
  }
  JSON::Tag parse_object(JSON& data)
  {
    if (++level > MAXLEVEL)
      return JSON::ERR;
    for (size_t len = 0; len < MAXSIZE; ++len) {
      int token = yylex();
      if (len == 0 && token == '}') {
        --level;
        return data.tag = JSON::OBJ;
      }
      if (token != '$')
        return JSON::ERR;
      JSON& item = data.object[string];
      if (yylex() != ':')
        return JSON::ERR;
      switch (parse(item)) {
        case JSON::NUL:
        case JSON::BOO:
        case JSON::NUM:
        case JSON::STR:
        case JSON::ARR:
        case JSON::OBJ: break;
        default       : return JSON::ERR;
      }
      token = yylex();
      if (token == '}') {
        --level;
        return data.tag = JSON::OBJ;
      }
      if (token != ',')
        return JSON::ERR;
    }
    return JSON::ERR;
  }
  JSON::Tag error(int token, JSON& data)
  {
    data.tag = JSON::ERR;
    switch (token) {
      case ']': return level > 0 ? JSON::UND : JSON::ERR;
      default : return JSON::ERR;
    }
  }
  static const size_t MAXLEVEL = 100; // to keep things secure: max JSON nesting depth
  static const size_t MAXSIZE = 1000; // to keep things secure: max JSON array and object size
  size_t level;
};

// Write wide string as JSON string
void write_string(std::ostream& os, const std::wstring& s)
{
  os << '"';
  for (std::wstring::const_iterator i = s.begin(); i != s.end(); ++i) {
    switch (*i) {
      case '"' :
      case '\\': os << '\\' << static_cast<char>(*i); break;
      case '\b': os << "\\b"; break;
      case '\f': os << "\\f"; break;
      case '\n': os << "\\n"; break;
      case '\r': os << "\\r"; break;
      case '\t': os << "\\t"; break;
      default  : if (*i >= '\x20' && *i <= '\x7f') { // emit printable char
                   os << static_cast<char>(*i);
                 } else if (*i < 0x20) { // emit \u00xx for control codes
                   os << "\\u" << std::internal << std::setw(4) << std::setfill('0') << std::hex << *i << std::dec;
                 } else if (*i >= 0xD800 && *i < 0xE000) { // UTF-16 surrogates
                   char buf[8];
                   int c = 0x010000 + ((*i - 0xD800) << 10);
                   c += *++i - 0xDC00;
                   buf[reflex::utf8(c, buf)] = '\0'; // convert to UTF-8 and make \0-terminated
                   os << buf;
                 } else { // else emit UTF-8
                   char buf[8];
                   buf[reflex::utf8(*i, buf)] = '\0'; // convert to UTF-8 and make \0-terminated
                   os << buf;
                 }
    }
  }
  os << '"';
}

// Write JSON value
std::ostream& operator<<(std::ostream& os, const JSON& data)
{
  switch (data.tag) {
    case JSON::NUL: os << "null"; break;
    case JSON::BOO: os << (data.boolean ? "true" : "false"); break;
    case JSON::NUM: os << data.number; break;
    case JSON::STR: write_string(os, data.string); break;
    case JSON::ARR: os << '[';
                    for (std::vector<JSON>::const_iterator i = data.array.begin(); i != data.array.end(); ++i) {
                      if (i != data.array.begin())
                        os << ',';
                      os << *i;
                    }
                    os << ']';
                    break;
    case JSON::OBJ: os << '{';
                    for (std::map<std::wstring,JSON>::const_iterator i = data.object.begin(); i != data.object.end(); ++i) {
                      if (i != data.object.begin())
                        os << ',';
                      write_string(os, i->first);
                      os << ':' << i->second;
                    }
                    os << '}';
                    break;
    default       : os << "undefined"; break;
  }
  return os;
}

// The main program parses JSON from a file or from stdin and writes it
int main(int argc, char **argv)
{
  FILE *fd = stdin;
  // open file if a file name is given on the command line
  if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
    exit(EXIT_FAILURE);
  JSON data;
  // parse JSON from stdin or a file provided as a command-line argument (FILE* permits UTF BOM detection)
  JSONParser parser(fd);
  if (parser.parse(data) == JSON::ERR) {
    std::cerr <<
      "Error at (" << parser.lineno() << "," << parser.columno() << ") when looking at " << parser.text() <<
      std::endl;
    exit(EXIT_FAILURE);
  }
  // see the user guide on reflex option -p
  parser.perf_report();
  // close the file, if not stdin
  if (fd != stdin)
    fclose(fd);
  // write JSON value
  std::cout << data << std::endl;
  return 0;
}
