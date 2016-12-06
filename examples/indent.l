// This example demonstrates indent \i and dedent \j matching
%o main tabs=8
%x CONTINUE
%%
^\h+            out() << "| "; // nodent, text is aligned to current margin column
^\h*\i          out() << "> "; // indent
^\h*\j          out() << "< "; // dedent
\j              out() << "< "; // dedent, triggered for each extra level dedented
(?^\\\n\h*)     // lines ending in \ will continue on the next line
"/*"            matcher().push_stops(); // save the indent margin/tab stops
                start(CONTINUE);        // continue w/o indent matching
<CONTINUE>{
"*/"            matcher().pop_stops();  // restore the indent margin/tab stops
                start(INITIAL);
.|\n            /* ignore comments */
}
%%
