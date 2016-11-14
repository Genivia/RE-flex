// This example demonstrates indent \i and dedent \j matching
%o main
%%
^\h+      out() << "| "; // undent, text is aligned to current margin column
^\h*\i    out() << "> "; // indent
^\h*\j    out() << "< "; // dedent
\j        out() << "< "; // dedent, triggered for each extra level dedented
(?^\\\n\h+)              // lines ending in \ will continue on the next line
%%
