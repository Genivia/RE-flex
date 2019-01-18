/* Example taken from the Flex documentation on multiple input buffers

   reflex --main reflexexample10.l
   c++ -o reflexexample10 lex.yy.cpp
   ./reflexexample10 < flexexample10.test
*/

/* the "incl" state is used for picking up the name of an include file */
%x incl

%%

include             start(incl);
[a-z]+              echo();
[^a-z\n]*\n?        echo();
<incl>[ \t]*
<incl>[^ \t\r\n]+   {
        FILE *fd = fopen(text(), "r");
        if (!fd)
        {
          fprintf(stderr, "Cannot open file %s\n", text());
          exit(1);
        }
        push_matcher(new_matcher(fd));
        start(INITIAL);
}

<<EOF>>             {
        fclose(in());
        pop_matcher();
        if (!has_matcher())
          return 0;
}

%%
