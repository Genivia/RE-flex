// example to scan (un)compressed C/C++ files using zlib and std:istream
// streams do not support UTF-16/32 normalization to UTF-8 though!!
//
// usage:
// $ gzip somefile.c
// $ ./gz somefile.c.gz

%top{
#include <cstdio>
#include <streambuf>
#include <cstring>
#include <unistd.h>
#include <zlib.h>

#ifndef Z_BUF_LEN
#define Z_BUF_LEN (65536)
#endif
}

%include "cdefs.l"
%option nodefault
%option fast

%%

{WHITESPACE}
{ILCOMMENT}
{MLCOMMENT}
{DIRECTIVE}	out() << "DIRECTIVE " << str() << std::endl;
{NAME}		out() << "NAME      " << str() << std::endl;
{UFLT}		out() << "FLOAT     " << str() << std::endl;
{UINT}		out() << "INT       " << str() << std::endl;
{CHAR}		out() << "CHAR      " << str() << std::endl;
{STRING}	out() << "STRING    " << str() << std::endl;
"{"|"<%"	|
"}"|"%>"	|
"["|"<:"	|
"]"|":>"	|
"("		|
")"		|
"+="		|
"++"		|
"+"		|
"-="		|
"--"		|
"->*"		|
"->"		|
"-"		|
"=="		|
"="		|
"<="		|
"<<="		|
"<<"		|
"<"		|
">="		|
">>="		|
">>"		|
">"		|
"!="		|
"!"		|
","		|
";"		|
"..."		|
".*"		|
"."		|
"^="		|
"^"		|
"~"		|
"*="		|
"*"		|
"/="		|
"/"		|
"%="		|
"%"		|
"&="		|
"&&"		|
"&"		|
"|="		|
"||"		|
"|"		|
"::"		|
":"		|
"?"		out() << "PUNCT     " << str() << std::endl;
.		out() << "*** ERROR at line " << lineno() << std::endl;

%%

// a std::streambuf class to produce a decompressed stream from a compressed
// file or uncompressed file, using Zlib's gzdopen, gzread, gzclose
class zstreambuf : public std::streambuf {
 public:
  zstreambuf(FILE *file)
    :
      cur_(),
      len_()
  {
    int fd = dup(fileno(file));
    if (fd >= 0 && (gzfile_ = gzdopen(fd, "r")) != Z_NULL)
      gzbuffer(gzfile_, Z_BUF_LEN);
    else if (fd >= 0)
      close(fd);
  }
  virtual ~zstreambuf()
  {
    if (gzfile_ != Z_NULL)
      gzclose_r(gzfile_);
  }
 protected:
  int_type underflow()
  {
    return cur_ < len_ ? buf_[cur_] : peek();
  }
  int_type uflow()
  {
    return cur_ < len_ ? buf_[cur_++] : read();
  }
  std::streamsize showmanyc()
  {
    return gzfile_ == Z_NULL ? -1 : 0;
  }
  std::streamsize xsgetn(char *s, std::streamsize n)
  {
    if (gzfile_ == Z_NULL)
      return 0;
    std::streamsize k = n;
    while (k > 0)
    {
      if (cur_ >= len_)
        if (peek() == traits_type::eof())
          return n - k;
      if (k <= len_ - cur_)
      {
        memcpy(s, buf_ + cur_, k);
        cur_ += k;
        return n;
      }
      memcpy(s, buf_ + cur_, len_ - cur_);
      s += len_ - cur_;
      k -= len_ - cur_;
      cur_ = len_;
    }
    return n;
  }
  int_type peek()
  {
    if (gzfile_ == Z_NULL)
      return traits_type::eof();
    cur_ = 0;
    len_ = gzread(gzfile_, buf_, Z_BUF_LEN);
    if (len_ <= 0)
    {
      finish();
      return traits_type::eof();
    }
    return traits_type::to_int_type(buf_[cur_]);
  }
  int_type read()
  {
    if (gzfile_ == Z_NULL)
      return traits_type::eof();
    cur_ = 0;
    len_ = gzread(gzfile_, buf_, Z_BUF_LEN);
    if (len_ <= 0)
    {
      finish();
      return traits_type::eof();
    }
    return traits_type::to_int_type(buf_[cur_++]);
  }
  void finish()
  {
    if (!gzeof(gzfile_))
    {
      int err;
      gzerror(gzfile_, &err);
      if (err == Z_ERRNO)
        perror("zlib error");
      else
        fprintf(stderr, "zlib decompression error\n");
    }
    gzclose_r(gzfile_);
    gzfile_ = Z_NULL;
    len_ = 0;
  }
  gzFile gzfile_;
  unsigned char buf_[Z_BUF_LEN];
  std::streamsize cur_;
  std::streamsize len_;
};

int main(int argc, char **argv)
{
  FILE *file = stdin;
  if (argc > 1)
  {
    file = fopen(argv[1], "rb");
    if (!file)
    {
      perror("Cannot open file for reading\n");
      exit(EXIT_FAILURE);
    }
  }
  zstreambuf streambuf(file);
  std::istream stream(&streambuf);
  Lexer lexer(&stream);
  lexer.lex();
  if (file != stdin)
    fclose(file);
}
