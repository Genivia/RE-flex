// A lexer to normalize HTML by adding closing tags and quoting non-quoted attribute values.
// Limitations: does no match HTML mixed case tags, cannot handle DTDs.

  #include <stdio.h>
  #include <deque>
  #include <algorithm>

  class Tidy : public Lexer {
   public:
    virtual int lex(void);
   private:
    std::deque<std::string> tags;
  };

%o dotall main unicode class=Tidy

dot                     \p{Unicode}
pi                      <\?
comment                 <!--{dot}*?-->
open                    <[^!/>\s]+
close                   <\/{dot}+?>
string                  \"{dot}*?\"|'{dot}*?'
value                   =\s*[^/>\s'"]+

%x ATTRIBUTES

%%

{comment}               ;

{pi}                    echo();

{open}                  {
                          std::string tag(text() + 1);
                          if (!tags.empty() && tags.back() == tag)
                            out() << "</" << tag << ">";
                          else
                            tags.push_back(tag);
                          echo();
                          start(ATTRIBUTES);
                        }

{close}                 {
                          std::string tag(text() + 2, size() - 3);
                          if (std::find(tags.begin(), tags.end(), tag) != tags.end())
                          {
                            while (!tags.empty() && tags.back() != tag)
                            {
                              out() << "</" << tags.back() << ">" << std::endl;
                              tags.pop_back();
                            }
                            tags.pop_back();
                            echo();
                          }
                        }

{dot}                   echo();

<ATTRIBUTES>"/>"        echo();
                        tags.pop_back();
                        start(INITIAL);

<ATTRIBUTES>"?>"        |
<ATTRIBUTES>">"         echo();
                        start(INITIAL);

<ATTRIBUTES>{value}     {
                          const char *t = text() + 1;
                          while (isspace(*t))
                            ++t;
                          out() << "=\"" << t << "\"";
                        }

<ATTRIBUTES>{string}    echo();

<ATTRIBUTES>\s+         out() << " ";

<ATTRIBUTES>[^'"]       echo();

<*>.                    fprintf(stderr, "Invalid XML encoding\n");
                        return 0;
%%
