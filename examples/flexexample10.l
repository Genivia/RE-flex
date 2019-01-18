/* Example taken from the Flex documentation on multiple input buffers

   Fixed the example from the documentation that assigns yyin, which is bad
   practice because it changes the file stream we are currently reading
   and that we should use again after the file(s) are included.  Assigning
   yyin should only be done when changing input without the intent to return
   back to the old input.

   - with global yytext, yyleng, yylineno, and yylex() using --bison:

   reflex --flex --main --bison --noyywrap flexexample10.l
   c++ -o flexexample10 lex.yy.cpp
   ./flexexample10 < flexexample10.test

   - or with yyFlexLexer class:

   reflex --flex --main --noyywrap flexexample10.l
   c++ -o flexexample10 lex.yy.cpp
   ./flexexample10 < flexexample10.test

   Actually, this example code from the Flex documentation can be simplified by
   using Flex and reflex yypush_buffer_state() and yypop_buffer_state(), though
   you may want to add some logic to that code to put a limit on the number of
   files opened to prevent infinite loops on cyclic file includes:

include             BEGIN(incl);
[a-z]+              ECHO;
[^a-z\n]*\n?        ECHO;
<incl>[ \t]*
<incl>[^ \t\r\n]+   {
        FILE *fd = fopen( yytext, "r" );
        if ( ! fd )
            {
            fprintf( stderr, "Cannot open file %s\n", yytext );
            exit( 1 );
            }
        yypush_buffer_state( yy_create_buffer( fd, YY_BUF_SIZE ) );
        BEGIN(INITIAL);
        }

<<EOF>> {
        fclose(yyin);
        yypop_buffer_state();
        if ( ! YY_CURRENT_BUFFER )
            {
            yyterminate();
            }
        }
*/

/* the "incl" state is used for picking up the name
 * of an include file
 */
%x incl

%{
#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;
%}

%%
include             BEGIN(incl);

[a-z]+              ECHO;
[^a-z\n]*\n?        ECHO;

<incl>[ \t]*        /* eat the whitespace */
<incl>[^ \t\r\n]+   { /* got the include file name */
        if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
            {
            fprintf( stderr, "Includes nested too deeply\n" );
            exit( 1 );
            }

        include_stack[include_stack_ptr++] =
            YY_CURRENT_BUFFER;

        /* we should not assign yyin, because this changes the file stream we
           are currently reading that we want to continue after the include! 
             yyin = fopen( yytext, "r" );
           removed and replaced by:
             FILE *fd = fopen( yytext, "r" );
         */
        FILE *fd = fopen( yytext, "r" );

        if ( ! fd )
            {
            fprintf( stderr, "Cannot open file %s\n", yytext );
            exit( 1 );
            }

        yy_switch_to_buffer(
            yy_create_buffer( fd, YY_BUF_SIZE ) );

        BEGIN(INITIAL);
        }

<<EOF>> {
        if ( --include_stack_ptr < 0 )
            {
            yyterminate();
            }

        else
            {
            fclose(yyin);
            yy_delete_buffer( YY_CURRENT_BUFFER );
            yy_switch_to_buffer(
                 include_stack[include_stack_ptr] );
            }
        }
%%
