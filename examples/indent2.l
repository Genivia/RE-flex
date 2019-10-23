// This example demonstrates indent \i and dedent \j matching
// with nested multi-line /*-comments
%{
  int level;   // a variable to track the /*-comment nesting level
%}
%o main tabs=8
%x COMMENT
%%
^\h+           out() << "| ";    // nodent, text is aligned to current margin column
^\h+\i         out() << "> ";    // indent
^\h*\j         out() << "< ";    // dedent
\j             out() << "< ";    // dedent, triggered for each extra level dedented
(?^^\h*\n)     // eat empty lines without affecting indent stops
\h*"/*"\k?     level = 1;        // /*-comment after spacing, \k matches indent stop changes and then restores them
               start(COMMENT);   // continue w/o indent matching
(?^\\\n\h*)    // lines ending in \ continue on the next line
.|\n           echo();           // ECHO character
<COMMENT>{
"/*"           ++level;          // allow nested /*-comments 
"*/"           if (--level == 0)
                 start(INITIAL); // back to initial state
.|\n           // ignore all content in comments
<<EOF>>        out() << "/* not closed";
}
%%
