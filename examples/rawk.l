/* rawk: how to use the RE/flex search engine to implement AWK-like rules

   This demo implements an AWK class and a set of example AWK-like rules.
   This demo application runs many times faster than awk and gawk.

   One limitation is that the RS (record separator) is always a newline \n,
   which allows lineno() to be used to count records i.e. count lines.

   AWK                  Equivalent
   ---                  --------
   RS                   RS (but fixed to \n)
   FS                   FS, set_FS()
   NF                   num_fields()
   NR                   num_records()
   $0                   record, field(0)
   $1,$2,...            field(1), field(2), ...

   Pattern and action rules are the same as lexer rules, with the important
   difference that option '%o find' changes the lexer into a search engine.

   Each AWK pattern range is implemented as a state transition to increase
   efficiency, in this demo we use one state S1 (just add more as needed).

   For case-insensitive matching:
     %o case-insensitive

   For Unicode pattern matching:
     %o unicode

   Example:
     ./rawk < rawk.l

*/

%top{
#include <iostream>
#include <vector>
#include <string>
}

// rename our "Lexer" search engine "AWK"
%option lexer=AWK

// AWK class
%class{
  // record separator is \n
  static const char RS = '\n';
  // an empty string is returned for undefined fields
  static const std::string empty;
  // user-definable field separator regex
  const char *FS;
  // each record is a line
  std::string record;
  // fields are cached in a string vector for speed
  std::vector<std::string> field_cache;
  // number of fields parsed
  int nfields;
  // the Matcher to split a record into fields
  reflex::Matcher field_matcher;

  // advance to next record, assuming we did not match a RS
  void next()
  {
    matcher().skip(RS);
  };

  // fetch a record
  void fetch()
  {
    if (chr() == RS)
    {
      record.assign(matcher().bol(), matcher().border());
      matcher().less(0); // make match length zero to not include the \n
    }
    else
    {
      record.assign(matcher().line());
    }
    next();
    nfields = -1;
  }

  // extract n fields from a record and cache them
  void get_fields(int n)
  {
    while (nfields < n && field_matcher.split())
    {
      if (field_matcher.size() > 0)
      {
        if (nfields >= static_cast<int>(field_cache.size()))
          field_cache.push_back(field_matcher.text());
        else
          field_cache[nfields] = field_matcher.text();
        ++nfields;
      }
    }
  }

  // the number of fields extracted from a record (cached)
  int num_fields()
  {
    if (nfields == -1)
    {
      field_matcher.input(record);
      nfields = 0;
      get_fields(65536);
    }
    return nfields;
  }

  // return the i'th (cached) field of a record or the empty string when undefined
  const std::string& field(int i)
  {
    if (i == 0)
      return record;
    if (nfields == -1)
    {
      field_matcher.input(record);
      nfields = 0;
    }
    if (nfields < i && !field_matcher.at_end())
      get_fields(i);
    return i <= nfields ? field_cache.at(i - 1) : empty;
  }

  // the number of records read from a file so far
  int num_records()
  {
    return matcher().lineno() - 1;
  }

  // assign a field separator regex pattern
  void set_FS(const char *pattern = NULL)
  {
    FS = (pattern != NULL ? pattern : "\\h");
    field_matcher.pattern(FS);
  }

  void begin()
  {
    // BEGIN code goes here
  }

  void end()
  {
    // END code goes here
    std::cout << "num records = " << num_records() << '\n';
  }
}

// AWK class contructor (initialization)
%init{
  set_FS();
  nfields = -1;
  begin();
}

// AWK class statics must be defined out-of-line
%{
  const std::string AWK::empty;
%}

%o fast find main

// we define a start condition state for each pattern range
%x S1

%%

/* an example AWK rule with one pattern and an action to display */

Pattern {
  // find and display a line that matches Pattern
  fetch();
  std::cout << num_records() << ": /Pattern/\nrecord=\"" << record << "\"\n";
  for (int i = 1; i <= num_fields(); ++i)
    std::cout << "field[" << i << "]=\"" << field(i) << "\"\n";
}

/* an example AWK || rule with two patterns and an action to display */

Pattern1|Pattern2 {
  // find and display a line that matches Pattern1 or Pattern2
  fetch();
  std::cout << num_records() << ": /Pattern1/ || /Pattern2/\nrecord=\"" << record << "\"\n";
  for (int i = 1; i <= num_fields(); ++i)
    std::cout << "field[" << i << "]=\"" << field(i) << "\"\n";
}

/* an example AWK pattern range rule with two patterns and actions to display */

FromPatternX {
  // find and display all recors between FromPattern and ToPattern
  std::cout << num_records() << ": /FromPattern/,/ToPattern\n";
  // jump to S1 to match all records up to the ending pattern
  start(S1);
}
<S1> {
\n {
  // a \n matches every input line, here to display lines between two patterns
  fetch();
  std::cout << num_records() << ": " << record << '\n';
}
ToPatternY {
  // display the final record of a pattern range
  fetch();
  std::cout << num_records() << ": " << record << '\n';
  // back to the initial state
  start(INITIAL);
}
}

/* an example AWK empty pattern rule to match all other records (not enabled)

\n {
  // a \n matches every input line
  fetch();
  // do something with the record...
}
*/

<<EOF>> { end(); return 0; }

%%
