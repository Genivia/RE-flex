#include <reflex/input.h>

void make_streambuf1(reflex::Input& input, size_t size);
void make_streambuf2(reflex::Input& input, size_t size);
void make_dos_streambuf1(reflex::Input& input, size_t size);
void make_dos_streambuf2(reflex::Input& input, size_t size);
void make_buffered_streambuf1(reflex::Input& input, size_t size);
void make_buffered_streambuf2(reflex::Input& input, size_t size);
void make_buffered_dos_streambuf1(reflex::Input& input, size_t size);
void make_buffered_dos_streambuf2(reflex::Input& input, size_t size);

int main()
{
  // create an Input object
  reflex::Input input;

  // test string
  input = "chars\n";
  make_streambuf1(input, 6);
  make_streambuf2(input, 6);
  make_dos_streambuf1(input, 6);
  make_dos_streambuf2(input, 6);
  make_buffered_streambuf1(input, 6);
  make_buffered_streambuf2(input, 6);
  make_buffered_dos_streambuf1(input, 6);
  make_buffered_dos_streambuf2(input, 6);

  // test wide string
  input = L"wchars\n";
  make_streambuf1(input, 7);
  make_streambuf2(input, 7);
  make_dos_streambuf1(input, 7);
  make_dos_streambuf2(input, 7);
  make_buffered_streambuf1(input, 7);
  make_buffered_streambuf1(input, 7);
  make_buffered_dos_streambuf1(input, 7);
  make_buffered_dos_streambuf1(input, 7);

  // test std::string
  std::string s("string\n");
  input = s;
  make_streambuf1(input, 7);
  make_streambuf2(input, 7);
  make_dos_streambuf1(input, 7);
  make_dos_streambuf2(input, 7);
  make_buffered_streambuf1(input, 7);
  make_buffered_streambuf2(input, 7);
  make_buffered_dos_streambuf1(input, 7);
  make_buffered_dos_streambuf2(input, 7);

  // test std::wstring
  std::wstring w(L"wstring\n");
  input = w;
  make_streambuf1(input, 8);
  make_streambuf2(input, 8);
  make_dos_streambuf1(input, 8);
  make_dos_streambuf2(input, 8);
  make_buffered_streambuf1(input, 8);
  make_buffered_streambuf2(input, 8);
  make_buffered_dos_streambuf1(input, 8);
  make_buffered_dos_streambuf2(input, 8);

  // test file, open a UTF-16 file with UTF-16 BOM
  FILE *fd = fopen("utf16lorem.txt", "r");
  if (fd == NULL)
    exit(EXIT_FAILURE);
  input = fd;
  // input.file_encoding(...); // optionally specify the encoding of the input file here
  make_streambuf1(input, 8193);
  rewind(fd);
  input = fd;
  make_streambuf2(input, 8193);
  rewind(fd);
  input = fd;
  make_dos_streambuf1(input, 8193);
  rewind(fd);
  input = fd;
  make_dos_streambuf2(input, 8193);
  std::cout << "\nFile converted from UTF-16 to UTF-8 by std::istream(reflex::Input::streambuf*)" << std::endl;
  rewind(fd);
  input = fd;
  // input.file_encoding(...); // optionally specify the encoding of the input file here
  make_buffered_streambuf1(input, 8193);
  rewind(fd);
  input = fd;
  make_buffered_streambuf2(input, 8193);
  rewind(fd);
  input = fd;
  make_buffered_dos_streambuf1(input, 8193);
  rewind(fd);
  input = fd;
  make_buffered_dos_streambuf2(input, 8193);
  std::cout << "\nFile converted from UTF-16 to UTF-8 by std::istream(reflex::BufferedInput::streambuf*)" << std::endl;
  fclose(fd);

  // done
  exit(EXIT_SUCCESS);
}

void make_streambuf1(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::Input::streambuf sb(input);
  // create an istream object for the streambuf
  std::istream is(&sb);
  // copy the istream to a std::string
  std::istreambuf_iterator<char> eos;
  std::string s(std::istreambuf_iterator<char>(is), eos);
  if (s.size() != size)
  {
    std::cerr << s << std::endl << "Failed reflex::Input::streambuf iterator size=" << s.size() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void make_streambuf2(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::Input::streambuf sb(input);
  // create a istream object for the streambuf
  std::istream is(&sb);
  // read a string
  char *cs = new char[size + 1];
  is.read(cs, size);
  cs[size] = '\0';
  if (is.gcount() != static_cast<std::streamsize>(size))
  {
    std::cerr << cs << std::endl << "Failed reflex::Input::streambuf read() size=" << is.gcount() << std::endl;
    exit(EXIT_FAILURE);
  }
  delete[] cs;
}

void make_dos_streambuf1(reflex::Input& input, size_t size)
{
  // create a dos_streambuf object for the input file
  reflex::Input::dos_streambuf sb(input);
  // create an istream object for the streambuf
  std::istream is(&sb);
  // copy the istream to a std::string
  std::istreambuf_iterator<char> eos;
  std::string s(std::istreambuf_iterator<char>(is), eos);
  if (s.size() != size)
  {
    std::cerr << s << std::endl << "Failed reflex::Input::streambuf DOS iterator size=" << s.size() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void make_dos_streambuf2(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::Input::dos_streambuf sb(input);
  // create a istream object for the streambuf
  std::istream is(&sb);
  // read a string
  char *cs = new char[size + 1];
  is.read(cs, size);
  cs[size] = '\0';
  if (is.gcount() != static_cast<std::streamsize>(size))
  {
    std::cerr << cs << std::endl << "Failed reflex::Input::streambuf DOS read() size=" << is.gcount() << std::endl;
    exit(EXIT_FAILURE);
  }
  delete[] cs;
}

void make_buffered_streambuf1(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::BufferedInput::streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // copy the istream to a std::string
  std::istreambuf_iterator<char> eos;
  std::string s(std::istreambuf_iterator<char>(is), eos);
  if (s.size() != size)
  {
    std::cerr << s << std::endl << "Failed reflex::Input::streambuf iterator size=" << s.size() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void make_buffered_streambuf2(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::BufferedInput::streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // read a string
  char *cs = new char[size + 1];
  is.read(cs, size);
  cs[size] = '\0';
  if (is.gcount() != static_cast<std::streamsize>(size))
  {
    std::cerr << cs << std::endl << "Failed reflex::Input::streambuf read() size=" << is.gcount() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void make_buffered_dos_streambuf1(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::BufferedInput::dos_streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // copy the istream to a std::string
  std::istreambuf_iterator<char> eos;
  std::string s(std::istreambuf_iterator<char>(is), eos);
  if (s.size() != size)
  {
    std::cerr << s << std::endl << "Failed reflex::Input::streambuf DOS iterator size=" << s.size() << std::endl;
    exit(EXIT_FAILURE);
  }
}

void make_buffered_dos_streambuf2(reflex::Input& input, size_t size)
{
  // create a streambuf object for the input file
  reflex::BufferedInput::dos_streambuf streambuf(input);
  // create an istream object for the streambuf
  std::istream is(&streambuf);
  // read a string
  char *cs = new char[size + 1];
  is.read(cs, size);
  cs[size] = '\0';
  if (is.gcount() != static_cast<std::streamsize>(size))
  {
    std::cerr << cs << std::endl << "Failed reflex::Input::streambuf DOS read() size=" << is.gcount() << std::endl;
    exit(EXIT_FAILURE);
  }
}

