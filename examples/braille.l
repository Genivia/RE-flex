/* Braille translator, inspired by the re2c example but expanded to be useful.
   To learn more:
   https://en.wikipedia.org/wiki/English_Braille
   New mathematical symbols and new punctuation are not translated.

   Article 1 of the Universal Declaration of Human Rights:

./braille << END
⠠⠁⠇⠇⠀⠓⠥⠍⠁⠝⠀⠆⠬⠎⠀⠜⠑⠀⠃⠕⠗⠝⠀⠋⠗⠑⠑⠀⠯⠀⠑⠟⠥⠁⠇⠀⠔⠀⠙⠊⠛⠝⠰⠽⠀⠯⠀⠐⠗⠎⠲
⠠⠮⠽⠀⠜⠑⠀⠢⠙⠪⠫⠀⠾⠀⠗⠂⠎⠕⠝⠀⠯⠀⠒⠎⠉⠊⠰⠑⠀⠯⠀⠩⠙⠀⠁⠉⠞⠀⠞⠪⠜⠙⠎⠀⠐⠕⠀⠁⠝⠕⠤
⠮⠗⠀⠔⠀⠁⠀⠸⠎⠀⠷⠀⠃⠗⠕⠮⠗⠓⠕⠕⠙⠲
END
All human beings are born free and equal in dignity and rights.
They are endowed with reason and conscience and should act towards one ano-
ther in a spirit of brotherhood.

*/

%top{
  #include <cstdio>   // fopen/fclose
  #include <cctype>   // upper
  #include <iostream>
}

// we declare a Braille class with translate() lexer (this is purely cosmetic):
%o class=Braille lex=translate

// define the Braille class
%{
class Braille : public Lexer {
 public:
  Braille() : caps(false) { }
  using Lexer::translate; // to set Braille input/output
  int translate();
 private:
  void emit(const char *t) {
    if (caps)
      out() << (char)toupper(*t) << t + 1;
    else
      out() << t;
    caps = false;
  }
  bool caps;
};
%}

// enable unicode and disable the default lexer rule
%o unicode nodefault

// exclusive start conditions
%x letters numbers

// lookaheads for contractions to be valid, spacing and punctuation !?("'-
la [\u{2800} \t\n\r⠖⠦⠶⠴⠄⠤]

%%

<*>⠰            start(INITIAL);

<INITIAL,letters>{
⠁               emit("a"); start(letters);
⠃               emit("b"); start(letters);
⠉               emit("c"); start(letters);
⠙               emit("d"); start(letters);
⠑               emit("e"); start(letters);
⠋               emit("f"); start(letters);
⠛               emit("g"); start(letters);
⠓               emit("h"); start(letters);
⠊               emit("i"); start(letters);
⠚               emit("j"); start(letters);
⠅               emit("k"); start(letters);
⠇               emit("l"); start(letters);
⠍               emit("m"); start(letters);
⠝               emit("n"); start(letters);
⠕               emit("o"); start(letters);
⠏               emit("p"); start(letters);
⠟               emit("q"); start(letters);
⠗               emit("r"); start(letters);
⠎               emit("s"); start(letters);
⠞               emit("t"); start(letters);
⠥               emit("u"); start(letters);
⠧               emit("v"); start(letters);
⠺               emit("w"); start(letters);
⠭               emit("x"); start(letters);
⠽               emit("y"); start(letters);
⠵               emit("z"); start(letters);
⠯               emit("and"); start(letters);
⠿               emit("for"); start(letters);
⠷               emit("of"); start(letters);
⠮               emit("the"); start(letters);
⠾               emit("with"); start(letters);
⠡               emit("ch"); start(letters);
⠣               emit("gh"); start(letters);
⠩               emit("sh"); start(letters);
⠹               emit("th"); start(letters);
⠱               emit("wh"); start(letters);
⠫               emit("ed"); start(letters);
⠻               emit("er"); start(letters);
⠳               emit("ou"); start(letters);
⠪               emit("ow"); start(letters);
⠢               emit("en"); start(letters);
⠔               emit("in"); start(letters);
⠌               emit("st"); start(letters);
⠜               emit("ar"); start(letters);
⠬               emit("ing"); start(letters);
// INITIAL ABBREVIATIONS
⠐⠓              emit("here"); start(letters);
⠐⠮              emit("there"); start(letters);
⠐⠱              emit("where"); start(letters);
⠐⠑              emit("ever"); start(letters);
⠐⠳              emit("ought"); start(letters);
⠐⠋              emit("father"); start(letters);
⠐⠍              emit("mother"); start(letters);
⠐⠝              emit("name"); start(letters);
⠐⠡              emit("character"); start(letters);
⠐⠟              emit("question"); start(letters);
⠐⠅              emit("know"); start(letters);
⠐⠇              emit("lord"); start(letters);
⠐⠕              emit("one"); start(letters);
⠐⠙              emit("day"); start(letters);
⠐⠎              emit("some"); start(letters);
⠐⠏              emit("part"); start(letters);
⠐⠞              emit("time"); start(letters);
⠐⠗              emit("right"); start(letters);
⠐⠹              emit("through"); start(letters);
⠐⠥              emit("under"); start(letters);
⠐⠺              emit("work"); start(letters);
⠐⠽              emit("young"); start(letters);
⠘⠮              emit("these"); start(letters);
⠘⠹              emit("those"); start(letters);
⠘⠥              emit("upon"); start(letters);
⠘⠱              emit("whose"); start(letters);
⠘⠺              emit("word"); start(letters);
⠸⠉              emit("cannot"); start(letters);
⠸⠍              emit("many"); start(letters);
⠸⠓              emit("had"); start(letters);
⠸⠮              emit("their"); start(letters);
⠸⠎              emit("spirit"); start(letters);
⠸⠺              emit("world"); start(letters);
}

<INITIAL>{
// CONTRACTIONS
⠃/{la}          emit("but");
⠉/{la}          emit("can");
⠙/{la}          emit("do");
⠑/{la}          emit("every");
⠋/{la}          emit("from and -self");
⠛/{la}          emit("go");
⠓/{la}          emit("have");
⠚/{la}          emit("just");
⠅/{la}          emit("knowledge");
⠇/{la}          emit("like");
⠍/{la}          emit("more");
⠝/{la}          emit("not");
⠏/{la}          emit("people");
⠟/{la}          emit("quite");
⠗/{la}          emit("rather");
⠎/{la}          emit("so");
⠞/{la}          emit("that");
⠌/{la}          emit("still");
⠥/{la}          emit("us");
⠧/{la}          emit("very");
⠭/{la}          emit("it");
⠽/{la}          emit("you");
⠵/{la}          emit("as");
⠡/{la}          emit("child");
⠩/{la}          emit("shall");
⠹/{la}          emit("this");
⠱/{la}          emit("which");
⠳/{la}          emit("out");
⠺/{la}          emit("will");
⠆               emit("be");
⠒               emit("con");
⠲               emit("dis");
⠢/{la}          emit("enough");
⠖/{la}          emit("to");
⠶/{la}          emit("were");
⠦/{la}          emit("his");
⠔/{la}          emit("in");
⠴/{la}          emit("and was");
⠤               emit("com");
// LONGER CONTRACTIONS
⠁⠃/{la}         emit("about");
⠁⠃⠧/{la}        emit("above");
⠁⠉/{la}         emit("according");
⠁⠉⠗/{la}        emit("across");
⠁⠋/{la}         emit("after");
⠁⠋⠝/{la}        emit("afternoon");
⠁⠋⠺/{la}        emit("afterward");
⠁⠛/{la}         emit("again");
⠁⠛⠌/{la}        emit("against");
⠁⠇/{la}         emit("also");
⠁⠇⠍/{la}        emit("almost");
⠁⠇⠗/{la}        emit("already");
⠁⠇⠞/{la}        emit("altogether");
⠁⠇⠹/{la}        emit("although");
⠁⠇⠺/{la}        emit("always");
⠆⠉/{la}         emit("because");
⠆⠋/{la}         emit("before");
⠆⠓/{la}         emit("behind");
⠆⠇/{la}         emit("below");
⠆⠝/{la}         emit("beneath");
⠆⠎/{la}         emit("beside");
⠆⠞/{la}         emit("between");
⠆⠽/{la}         emit("beyond");
⠃⠇/{la}         emit("blind");
⠃⠗⠇/{la}        emit("Braille");
⠉⠙/{la}         emit("could");
⠉⠧/{la}         emit("ceive");
⠉⠧⠛/{la}        emit("ceiving");
⠡⠝/{la}         emit("children");
⠙⠉⠇/{la}        emit("declare");
⠙⠉⠇⠛/{la}       emit("declaring");
⠑⠊/{la}         emit("either");
⠋⠌/{la}         emit("first");
⠋⠗/{la}         emit("friend");
⠛⠙/{la}         emit("good");
⠛⠗⠞/{la}        emit("great");
⠓⠻⠋/{la}        emit("herself");
⠓⠍/{la}         emit("him");
⠓⠍⠋/{la}        emit("himself");
⠊⠍⠍/{la}        emit("immediate");
⠇⠇/{la}         emit("little");
⠇⠗/{la}         emit("letter");
⠍⠡/{la}         emit("much");
⠍⠌/{la}         emit("must");
⠍⠽⠋/{la}        emit("myself");
⠝⠑⠉/{la}        emit("necessary");
⠝⠑⠊/{la}        emit("neither");
⠕⠄⠉/{la}        emit("o'clock");
⠳⠗⠧⠎/{la}       emit("ourselves");
⠏⠙/{la}         emit("paid");
⠏⠻⠓/{la}        emit("perhaps");
⠟⠅/{la}         emit("quick");
⠗⠚⠉/{la}        emit("rejoice");
⠗⠚⠉⠛/{la}       emit("rejoicing");
⠎⠡/{la}         emit("such");
⠎⠙/{la}         emit("said");
⠩⠙/{la}         emit("should");
⠞⠙/{la}         emit("today");
⠞⠛⠗/{la}        emit("together");
⠞⠍/{la}         emit("tomorrow");
⠞⠝/{la}         emit("tonight");
⠮⠍⠧⠎/{la}       emit("themselves");
⠺⠙/{la}         emit("would");
⠭⠎/{la}         emit("its");
⠭⠋/{la}         emit("itself");
⠽⠗/{la}         emit("your");
⠽⠗⠋/{la}        emit("yourself");
⠽⠗⠧⠎/{la}       emit("yourselves");
}

<letters>{
// DIGRAPHS
⠂               emit("ea");
⠆               emit("bb");
⠒               emit("cc");
⠲               emit("dd");
// FINAL ABBREVIATIONS
⠨⠙              emit("ound");
⠨⠞              emit("ount");
⠨⠑              emit("ance");
⠨⠎              emit("less");
⠨⠝              emit("sion");
⠰⠛              emit("ong");
⠰⠇              emit("ful");
⠰⠞              emit("ment");
⠰⠑              emit("ence");
⠰⠎              emit("ness");
⠰⠝              emit("tion");
⠰⠽              emit("ity");
⠠⠽              emit("ally");
⠠⠝              emit("ation");
}

<*>⠼            start(numbers);

<numbers>{
⠁               emit("0");
⠃               emit("1");
⠉               emit("2");
⠙               emit("3");
⠑               emit("4");
⠋               emit("5");
⠛               emit("6");
⠓               emit("7");
⠊               emit("8");
⠚               emit("9");
}

<*>{
// PUNCTUATION
⠂/{la}          emit(",");
⠆/{la}          emit(";");
⠒/{la}          emit(":");
⠲/{la}          emit(".");
⠤/{la}          emit("-");
⠖               emit("!");
⠦               emit("?");
⠶               emit("(");
⠴               emit("\"");
⠄               emit("'");
// MODES
⠈               // accent
⠨               // emph.
⠠               caps = true;
// SPACING
\u{2800}        emit(" "); start(INITIAL);
[ \t\n\r]       emit(text()); start(INITIAL);
// OTHER BRAILLE LIGATURES
\p{Braille}     emit(text());
}

<*>.            std::cerr << "Error in input at " << lineno() << std::endl;

%%

// The main program parses braille from file or stdin and translates it
int main(int argc, char **argv)
{
  FILE *fd = stdin;
  if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
  {
    std::cerr << "Cannot open " << argv[1] << " for reading" << std::endl;
    exit(EXIT_FAILURE);
  }
  Braille translator;
  // assign input to read and output to write to, then translate
  translator.translate(fd, &std::cout);
  if (fd != stdin)
    fclose(fd);
  return 0;
}
