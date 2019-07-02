#include <reflex/input.h>

int main()
{
  // open a UTF-16 file with UTF-16 BOM
  FILE *fd = fopen("utf16lorem.txt", "r");
  if (fd == NULL)
    exit(EXIT_FAILURE);
  // create an Input object for the file, to convert UTF-16 to UTF-8
  reflex::Input input(fd);
  // input.file_encoding(...); // optionally specify the encoding of the input file here
  // create a streambuf object for the input file
  reflex::Input::streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // copy the istream to standard output
  int ch;
  while ((ch = is.get()) != EOF)
    std::cout << (char)ch;
  //
  std::cout << "\nFile converted from UTF-16 to UTF-8 by std::istream(reflex::Input::streambuf*)" << std::endl;
  // done
  fclose(fd);
  exit(EXIT_SUCCESS);
}
