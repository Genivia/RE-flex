#include <reflex/input.h>

void make_streambuf(reflex::Input& input);

int main()
{
  // create an Input object
  reflex::Input input;
  // test string
  input = "chars\n";
  make_streambuf(input);
  // test wide string
  input = L"wchars\n";
  make_streambuf(input);
  // test std::string
  std::string s("string\n");
  input = s;
  make_streambuf(input);
  // test std::wstring
  std::wstring w(L"wstring\n");
  input = w;
  make_streambuf(input);
  // test file, open a UTF-16 file with UTF-16 BOM
  FILE *fd = fopen("utf16lorem.txt", "r");
  if (fd == NULL)
    exit(EXIT_FAILURE);
  input = fd;
  // input.file_encoding(...); // optionally specify the encoding of the input file here
  make_streambuf(input);
  std::cout << "\nFile converted from UTF-16 to UTF-8 by std::istream(reflex::Input::streambuf*)" << std::endl;
  // done
  fclose(fd);
  exit(EXIT_SUCCESS);
}

void make_streambuf(reflex::Input& input)
{
  // create a streambuf object for the input file
  reflex::Input::streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // copy the istream to standard output
  int ch;
  while ((ch = is.get()) != EOF)
    std::cout << (char)ch;
}
