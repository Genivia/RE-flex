// cvt2utf.cpp
// Convert files to UTF-8, UTF-16, or UTF-32
// Demonstrates the use of the reflex::Input class for UTF-8 normalization
//
// cvt2utf [-b] [-f encoding] [-t utf-encoding] [file]
//
// Example to convert ISO-8859-1 file.txt to UTF-8:
//   cvt2utf -f ISO-8859-1 file.txt
//
// Example to convert file.txt (encoded in UTF-8 or has a UTF BOM) to UTF-16:
//   cvt2utf -t UTF-16 file.txt
//
// Example to convert file.txt encoded in CP-1250 to UTF-16:
//   cvt2utf -f CP-1250 -t UTF-16 file.txt

#include <reflex/input.h>
#include <reflex/utf8.h>

// check if we are on a windows OS
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__BORLANDC__)
# define OS_WIN
#endif

// command-line options
bool flag_with_bom         = false;
const char *flag_from_code = NULL;
const char *flag_to_code   = NULL;

// table of file formats for cvt2utf option -f
const struct { const char *format; reflex::Input::file_encoding_type encoding; } format_table[] = {
  { "binary",      reflex::Input::file_encoding::plain      },
  { "ASCII",       reflex::Input::file_encoding::utf8       },
  { "UTF-8",       reflex::Input::file_encoding::utf8       },
  { "UTF-16",      reflex::Input::file_encoding::utf16be    },
  { "UTF-16BE",    reflex::Input::file_encoding::utf16be    },
  { "UTF-16LE",    reflex::Input::file_encoding::utf16le    },
  { "UTF-32",      reflex::Input::file_encoding::utf32be    },
  { "UTF-32BE",    reflex::Input::file_encoding::utf32be    },
  { "UTF-32LE",    reflex::Input::file_encoding::utf32le    },
  { "ISO-8859-1",  reflex::Input::file_encoding::latin      },
  { "ISO-8869-2",  reflex::Input::file_encoding::iso8859_2  },
  { "ISO-8869-3",  reflex::Input::file_encoding::iso8859_3  },
  { "ISO-8869-4",  reflex::Input::file_encoding::iso8859_4  },
  { "ISO-8869-5",  reflex::Input::file_encoding::iso8859_5  },
  { "ISO-8869-6",  reflex::Input::file_encoding::iso8859_6  },
  { "ISO-8869-7",  reflex::Input::file_encoding::iso8859_7  },
  { "ISO-8869-8",  reflex::Input::file_encoding::iso8859_8  },
  { "ISO-8869-9",  reflex::Input::file_encoding::iso8859_9  },
  { "ISO-8869-10", reflex::Input::file_encoding::iso8859_10 },
  { "ISO-8869-11", reflex::Input::file_encoding::iso8859_11 },
  { "ISO-8869-13", reflex::Input::file_encoding::iso8859_13 },
  { "ISO-8869-14", reflex::Input::file_encoding::iso8859_14 },
  { "ISO-8869-15", reflex::Input::file_encoding::iso8859_15 },
  { "ISO-8869-16", reflex::Input::file_encoding::iso8859_16 },
  { "MAC",         reflex::Input::file_encoding::macroman   },
  { "MACROMAN",    reflex::Input::file_encoding::macroman   },
  { "EBCDIC",      reflex::Input::file_encoding::ebcdic     },
  { "CP437",       reflex::Input::file_encoding::cp437      },
  { "CP850",       reflex::Input::file_encoding::cp850      },
  { "CP858",       reflex::Input::file_encoding::cp858      },
  { "CP1250",      reflex::Input::file_encoding::cp1250     },
  { "CP1251",      reflex::Input::file_encoding::cp1251     },
  { "CP1252",      reflex::Input::file_encoding::cp1252     },
  { "CP1253",      reflex::Input::file_encoding::cp1253     },
  { "CP1254",      reflex::Input::file_encoding::cp1254     },
  { "CP1255",      reflex::Input::file_encoding::cp1255     },
  { "CP1256",      reflex::Input::file_encoding::cp1256     },
  { "CP1257",      reflex::Input::file_encoding::cp1257     },
  { "CP1258",      reflex::Input::file_encoding::cp1258     },
  { "KOI8-R",      reflex::Input::file_encoding::koi8_r     },
  { "KOI8-U",      reflex::Input::file_encoding::koi8_u     },
  { "KOI8-RU",     reflex::Input::file_encoding::koi8_ru    },
  { NULL, 0 }
};

// file formats for cvt2utf option -t
enum UTF { UTF8 = 0, UTF16 = 1, UTF32 = 2 };

// function protos
void help(const char *message = NULL, const char *arg = NULL);

// cvt2utf main
int main(int argc, char **argv)
{
  const char *infile = NULL;

  // parse command-line options and arguments
  for (int i = 1; i < argc; ++i)
  {
    const char *arg = argv[i];

    if ((*arg == '-' && arg[1])
#ifdef OS_WIN
     || *arg == '/'
#endif
     )
    {
      bool is_grouped = true;

      // parse a command-line option
      while (is_grouped && *++arg)
      {
        switch (*arg)
        {
          case '-':
            ++arg;
            if (strncmp(arg, "from=", 5) == 0)
              flag_from_code = arg + 5;
            else if (strncmp(arg, "to=", 3) == 0)
              flag_to_code = arg + 3;
            else if (strcmp(arg, "with-bom") == 0)
              flag_with_bom = true;
            else if (strcmp(arg, "help") == 0)
              help();
            else
              help("invalid option --", arg);
            is_grouped = false;
            break;

          case 'b':
            flag_with_bom = true;
            break;

          case 'f':
            ++arg;
            if (*arg)
              flag_from_code = &arg[*arg == '='];
            else if (++i < argc)
              flag_from_code = argv[i];
            else
              help("missing code for option -f");
            is_grouped = false;
            break;

          case 't':
            ++arg;
            if (*arg)
              flag_to_code = &arg[*arg == '='];
            else if (++i < argc)
              flag_to_code = argv[i];
            else
              help("missing code for option -t");
            is_grouped = false;
            break;

          case '?':
          case 'h':
            help();
            break;

          default:
            help("invalid option -", arg);
        }
      }
    }
    else
    {
      // parse a command-line argument
      if (infile == NULL)
        infile = argv[i];
      else
        help("one input file argument can be specified, also found ", argv[i]);
    }
  }

  // parse option -f

  reflex::Input::file_encoding_type from_encoding = reflex::Input::file_encoding::plain;

  if (flag_from_code)
  {
    int i;

    // scan the format_table[] for a matching format
    for (i = 0; format_table[i].format != NULL; ++i)
      if (strcmp(flag_from_code, format_table[i].format) == 0)
        break;

    if (format_table[i].format == NULL)
      help("invalid --from value");

    // encoding is the file format used by all input files, if no BOM is present
    from_encoding = format_table[i].encoding;
  }

  // parse option -t

  UTF to_encoding = UTF8;

  if (flag_to_code)
  {
    if (strcmp(flag_to_code, "UTF-8") == 0)
    {
      to_encoding = UTF8;
    }
    else if (strcmp(flag_to_code, "UTF-16") == 0)
    {
      flag_with_bom = true;
      to_encoding = UTF16;
    }
    else if (strcmp(flag_to_code, "UTF-32") == 0)
    {
      flag_with_bom = true;
      to_encoding = UTF32;
    }
    else
    {
      help("invalid --to value");
    }
  }

  // open file or use stdin

  FILE *file;
  
  if (infile == NULL || strcmp(infile, "-") == 0)
  {
    file = stdin;
  }
  else
  {
#ifdef OS_WIN
    file = fopen(infile, "rb");
#else
    file = fopen(infile, "r");
#endif

    if (file == NULL)
    {
      perror("Cannot open file for reading");
      exit(EXIT_FAILURE);
    }
  }

  // create an input object to read the file (or stdin) using the given file encoding

  reflex::Input input(file, from_encoding);

  // -b option or target is UTF-16/32 (big endian): emit UTF BOM

  if (flag_with_bom)
  {
    const char *BOM[] = { "\xEF\xBB\xBF", "\xFE\xFF", "\x00\x00\xFE\xFF" };

    if (fwrite(BOM[to_encoding], to_encoding == 0 ? 3 : to_encoding == 1 ? 2 : 4, 1, stdout) == 0)
    {
      perror("Cannot write");
      exit(EXIT_FAILURE);
    }
  }

  // convert

  if (to_encoding == UTF8)
  {
    // convert input to UTF-8 using reflex::Input::get(buf, size) by 8K blocks
    while (input)
    {
      char buf[16384];

      size_t size = input.get(buf, sizeof(buf));

      if (size == 0)
        break;

      if (fwrite(buf, size, 1, stdout) == 0)
      {
        perror("Cannot write");
        exit(EXIT_FAILURE);
      }
    }
  }
  else
  {
    int ch = input.get();

    // convert input to UTF-16/32 using reflex::Input::get()
    while (ch != EOF)
    {
      size_t size = 1;
      char buf[6];
      buf[0] = ch;

      // read UTF-8 octet sequence
      if ((ch & 0xC0) == 0xC0)
      {
         while (true)
         {
           ch = input.get();
           if ((ch & 0xC0) == 0xC0 || ch == EOF)
             break;
           if (size < 6)
             buf[size++] = ch;
         }
      }
      else
      {
        ch = input.get();
      }

      // convert UTF-8 to UCS4
      int wch = reflex::utf8(buf, NULL);

      if (to_encoding == UTF16)
      {
        // convert UCS4 to UTF-16BE
        if (wch <= 0xFFFF)
        {
          // write as UTF-16BE
          buf[0] = (wch >> 8) & 0xFF;
          buf[1] = wch & 0xFF;
          size = 2;
        }
        else if (wch <= 0x10FFFF)
        {
          // write as UTF-16BE surrogate pair
          wch -= 0x10000;
          buf[0] = 0xD8 | ((wch >> 18) & 0x03);
          buf[1] = (wch >> 10) & 0xFF;
          buf[2] = 0xDC | ((wch >> 8) & 0x03);
          buf[3] = wch & 0xFF;
          size = 4;
        }
        else
        {
          // invalid UCS4, write U+FFFD
          buf[0] = 0xFF;
          buf[1] = 0xFD;
          size = 2;
        }
      }
      else
      {
        // convert UCS4 to UTF-32BE
        buf[0] = (wch >> 24) & 0xFF;
        buf[1] = (wch >> 16) & 0xFF;
        buf[2] = (wch >> 8) & 0xFF;
        buf[3] = wch & 0xFF;
        size = 4;
      }

      if (fwrite(buf, size, 1, stdout) == 0)
      {
        perror("Cannot write");
        exit(EXIT_FAILURE);
      }
    }
  }

  // done

  if (file != stdin)
    fclose(file);

  exit(EXIT_SUCCESS);
}

// Display help information with an optional diagnostic message and exit
void help(const char *message, const char *arg)
{
  if (message)
    std::cout << "cvt2utf: " << message << (arg != NULL ? arg : "") << std::endl;
  std::cout <<
"Usage: cvt2utf [-b] [-f ENCODING] [-t ENCODING] [FILE]\n\
\n\
    -b, --with-bom\n\
            Include the UTF BOM in the output.  This is the default for options\n\
            -t UTF-16 and -t UTF-32.\n\
    -f ENCODING, --from=ENCODING\n\
            The input file encoding.  The possible values of ENCODING can be:";
  for (int i = 0; format_table[i].format != NULL; ++i)
    std::cout << (i == 0 ? "" : ",") << (i % 4 ? " " : "\n            ") << "`" << format_table[i].format << "'";
  std::cout << ".\n\
    -t ENCODING, --to=ENCODING\n\
            The output file encoding.  The possible values of ENCODING can be:\n\
            `UTF-8', `UTF-16', `UTF-32'.\n\
    -?, -h, --help\n\
            Print a help message.\n\
" << std::endl;
  exit(EXIT_FAILURE);
}
