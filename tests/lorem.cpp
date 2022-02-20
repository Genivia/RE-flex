#include <reflex/matcher.h>
#include <reflex/pcre2matcher.h>
#include <reflex/boostmatcher.h>
#include <reflex/stdmatcher.h>
#include <reflex/timer.h>
#include <fstream>
#include <sstream>

#ifndef DEBUG
#define RUNS 100
#else
#define RUNS 1
#endif

#include "test_lorem_tokenizer.cpp"  // reflex_code_tokenizer
#include "test_lorem_filter.cpp"     // reflex_code_filter
#include "test_lorem_splitter.cpp"   // reflex_code_splitter

using namespace reflex;

void banner(const char *title)
{
  int i;
  printf("\n\n/");
  for (i = 0; i < 78; i++)
    putchar('*');
  printf("\\\n *%76s*\n * %-75s*\n *%76s*\n\\", "", title, "");
  for (i = 0; i < 78; i++)
    putchar('*');
  printf("/\n\n");
}

void timer(const char *text = NULL)
{
  static timer_type t;
  if (text)
  {
    float ms = timer_elapsed(t);
    printf("%s %g micro seconds\n", text, 1000*ms/RUNS);
  }
  else
  {
    timer_start(t);
  }
}

// http://www.lipsum.com

static const std::string lorem("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque at accumsan turpis, non feugiat magna. Quisque sed fringilla felis. Ut at ullamcorper metus, eu ornare lorem. Etiam id leo in elit varius condimentum vel ut sapien. Morbi at rhoncus urna. Nunc hendrerit at metus sit amet aliquam. Donec lectus sapien, euismod ut urna id, eleifend mattis dolor. Nunc sit amet orci lacus. Proin mattis consectetur iaculis. Nam hendrerit nisi felis, vitae consectetur urna euismod nec. Cras ultricies lorem vel commodo tempus.\
\
Vestibulum mollis magna et leo dictum, vitae dictum libero sagittis. Vestibulum eget est laoreet, consequat ex et, cursus justo. Duis quis est ut diam pellentesque tristique. Mauris condimentum erat id leo molestie, vitae dignissim nisl lacinia. Sed ultrices, velit id semper tristique, augue ante scelerisque nisi, a pretium ante ipsum at justo. Vivamus aliquam nulla vel tortor cursus, blandit egestas orci condimentum. Phasellus pellentesque lobortis neque non faucibus. Suspendisse ut laoreet ex. Fusce augue massa, ultricies eu orci sed, venenatis blandit sapien. Sed interdum nibh pulvinar, malesuada libero non, vulputate turpis.\
\
Fusce enim nibh, sollicitudin id laoreet in, aliquam ut justo. Nulla id diam ac mi volutpat consectetur. Nam eu odio a odio vulputate iaculis eu eu ligula. Etiam bibendum, sapien vitae hendrerit gravida, neque metus gravida augue, a posuere nisi est sed felis. In quam enim, sodales vitae felis ac, congue imperdiet massa. Nunc suscipit, lectus at volutpat fermentum, sem mi vestibulum lectus, vel consectetur ante augue et nisl. In vel gravida ligula, ut malesuada nulla. Nulla ut blandit massa, quis egestas risus. Duis tincidunt eros dui, vel elementum nunc gravida id. Pellentesque a mattis ligula, eget feugiat purus. Donec eu est porttitor, vulputate tortor scelerisque, convallis nulla. Integer ornare turpis a quam ullamcorper, et euismod sem semper. Ut vulputate sit amet elit et condimentum. In hac habitasse platea dictumst. Vestibulum congue sem et nisi lacinia pellentesque. Aenean tincidunt, augue sit amet blandit molestie, velit lacus ullamcorper sem, a egestas est elit eu mi.\
\
Nam ut nunc nunc. Curabitur posuere vulputate arcu nec rhoncus. Morbi elementum commodo enim. Nullam faucibus bibendum diam. Integer facilisis facilisis massa. Curabitur vitae mauris ac orci pharetra sodales. Aliquam massa eros, commodo et dolor sit amet, porttitor dictum eros. In lectus magna, ullamcorper eu massa eu, iaculis vulputate orci. Integer auctor, lorem sit amet dignissim ullamcorper, ex nunc ornare odio, sit amet vulputate arcu dui non velit. Proin sollicitudin mi justo, eu vestibulum enim ultrices et. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Integer sit amet ipsum mauris. Etiam vel dui id metus fermentum pharetra. Sed hendrerit, ante ac volutpat sodales, purus dolor consectetur justo, vel malesuada augue neque in mi. Suspendisse egestas lacus id sapien vehicula posuere sed bibendum metus.\
\
Donec auctor bibendum neque aliquam consequat. In tempus tempus dignissim. Suspendisse eu diam est. Pellentesque ultrices mauris odio, id ullamcorper ante condimentum in. Sed semper laoreet urna sit amet venenatis. Mauris eget fringilla diam. Nullam eu fringilla libero, feugiat commodo sapien. Mauris cursus elit augue, vel suscipit sem molestie eget. Aliquam at lacinia leo. Fusce consequat odio arcu, iaculis ultricies diam viverra sit amet. Maecenas congue neque ut bibendum euismod. Nam efficitur, ligula ac iaculis facilisis, ligula ligula facilisis ipsum, ac ultricies neque est at augue. Etiam risus enim, cursus eu vehicula nec, commodo vel dui. Quisque sit amet massa non erat fermentum varius. Vivamus ut euismod nunc. Duis eu fringilla nunc.\
\
Praesent venenatis nisi nec velit aliquam, et consectetur risus dapibus. Etiam nisl arcu, porta at sapien gravida, posuere lacinia metus. Aliquam tincidunt dolor at pellentesque eleifend. Praesent a enim non quam aliquam venenatis vel id ante. Nam accumsan ultrices sagittis. Nulla elementum rutrum massa, id semper velit scelerisque non. Donec sagittis consequat tortor, eget venenatis ex cursus nec. Aenean ultrices imperdiet lorem in convallis. Nulla pretium eros eu consequat congue. Donec posuere ligula sit amet sem vulputate, vitae elementum enim porttitor. In aliquam, metus quis vehicula maximus, mi ante malesuada velit, et consectetur ligula diam eu ligula. Curabitur lacinia feugiat sodales. Etiam in nulla placerat, imperdiet ligula et, finibus nisl. Nullam posuere enim at felis aliquam venenatis.\
\
Proin elit tortor, ultrices sed nunc in, tincidunt cursus turpis. Donec lectus eros, laoreet sed suscipit ac, rhoncus ut augue. Nunc vitae semper metus. Pellentesque lorem nibh, imperdiet sit amet rhoncus quis, ultrices a turpis. Cras eu aliquam libero, ac sollicitudin ligula. Proin tincidunt accumsan malesuada. Curabitur luctus dui a nunc dapibus, nec consequat magna vestibulum. Ut quis arcu tincidunt, viverra massa et, venenatis lacus. Ut non nisl at enim laoreet semper ac nec erat. Vestibulum fringilla vulputate nulla, sed congue arcu hendrerit ut. Sed eleifend urna ac enim condimentum, sit amet sollicitudin sem faucibus. Suspendisse facilisis volutpat tristique. Morbi fringilla urna ut erat iaculis vestibulum.\
\
Donec efficitur tristique felis non ultricies. Aliquam magna lorem, posuere in porta et, facilisis ut massa. Mauris ac blandit lacus, a laoreet odio. Suspendisse potenti. Donec venenatis rhoncus dolor, eu ornare augue mattis et. Nunc ornare porta mollis. Phasellus interdum eros eget feugiat rutrum. Nullam a magna id lorem mattis aliquam. Phasellus non posuere libero. Suspendisse finibus, nulla et viverra semper, velit metus finibus nunc, non tempus dolor augue non nisl. Phasellus ultricies id nisi vel sollicitudin. Pellentesque ut neque eu eros mattis interdum ac vel est.\
\
Proin id dui in justo pulvinar tempus eu ut lorem. Vivamus tincidunt felis vitae malesuada iaculis. Donec pellentesque metus a tristique ultrices. Nullam vel dui sit amet dui dignissim suscipit quis id libero. Donec blandit et ex ut dignissim. Integer est magna, gravida a mauris id, euismod commodo metus. Proin vel tellus volutpat, lobortis nibh non, rutrum lacus. Donec nunc erat, tincidunt vitae tellus ut, condimentum scelerisque ligula. Pellentesque pharetra lectus et enim dignissim finibus.\
\
In mollis odio et luctus hendrerit. Aenean non enim enim. Donec maximus mi vitae finibus vehicula. Sed convallis commodo porttitor. In auctor nec enim at luctus. Suspendisse urna sapien, vulputate blandit auctor et, placerat ac lorem. Proin et euismod leo. Nunc non orci vulputate, dignissim arcu rhoncus, feugiat urna. Curabitur ut augue turpis. Aliquam auctor vestibulum tempus. Curabitur hendrerit efficitur ullamcorper. Duis tellus tortor, volutpat in augue id, iaculis ultrices arcu. Nullam ut pretium erat. Aliquam iaculis feugiat velit, a tincidunt turpis. In malesuada facilisis mauris vitae sollicitudin. Curabitur ultricies porttitor mattis.\
\
Etiam interdum porta libero, sed lobortis sapien porta sed. Curabitur ullamcorper mauris vitae viverra mattis. Nulla ac semper ante. Phasellus maximus aliquam lorem, vitae molestie lectus efficitur ut. Donec tempor fringilla nulla id pretium. Etiam euismod est vitae dui auctor, id imperdiet ex cursus. Donec congue elit nec felis cursus, non fermentum urna fringilla. Donec posuere quis purus vel eleifend. Nulla sed dui dignissim, vehicula urna non, facilisis ligula. Proin eu molestie ligula, non dapibus nibh. Donec lobortis arcu sit amet lacus semper, quis mattis ipsum tincidunt. Nam pellentesque justo eleifend, facilisis metus at, tincidunt diam. Suspendisse ac mi luctus, maximus sapien sit amet, interdum lectus. Quisque tempor eget dolor vel pellentesque. Maecenas placerat at lectus sed scelerisque. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.\
\
Mauris dignissim mattis dui, feugiat lobortis diam mattis non. Curabitur neque mi, scelerisque in convallis quis, imperdiet consectetur lorem. Vestibulum non mi ac justo facilisis consequat in sit amet ligula. Quisque urna sem, tristique non ligula consectetur amet.\n");

static const std::wstring wlorem(L"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque at accumsan turpis, non feugiat magna. Quisque sed fringilla felis. Ut at ullamcorper metus, eu ornare lorem. Etiam id leo in elit varius condimentum vel ut sapien. Morbi at rhoncus urna. Nunc hendrerit at metus sit amet aliquam. Donec lectus sapien, euismod ut urna id, eleifend mattis dolor. Nunc sit amet orci lacus. Proin mattis consectetur iaculis. Nam hendrerit nisi felis, vitae consectetur urna euismod nec. Cras ultricies lorem vel commodo tempus.\
\
Vestibulum mollis magna et leo dictum, vitae dictum libero sagittis. Vestibulum eget est laoreet, consequat ex et, cursus justo. Duis quis est ut diam pellentesque tristique. Mauris condimentum erat id leo molestie, vitae dignissim nisl lacinia. Sed ultrices, velit id semper tristique, augue ante scelerisque nisi, a pretium ante ipsum at justo. Vivamus aliquam nulla vel tortor cursus, blandit egestas orci condimentum. Phasellus pellentesque lobortis neque non faucibus. Suspendisse ut laoreet ex. Fusce augue massa, ultricies eu orci sed, venenatis blandit sapien. Sed interdum nibh pulvinar, malesuada libero non, vulputate turpis.\
\
Fusce enim nibh, sollicitudin id laoreet in, aliquam ut justo. Nulla id diam ac mi volutpat consectetur. Nam eu odio a odio vulputate iaculis eu eu ligula. Etiam bibendum, sapien vitae hendrerit gravida, neque metus gravida augue, a posuere nisi est sed felis. In quam enim, sodales vitae felis ac, congue imperdiet massa. Nunc suscipit, lectus at volutpat fermentum, sem mi vestibulum lectus, vel consectetur ante augue et nisl. In vel gravida ligula, ut malesuada nulla. Nulla ut blandit massa, quis egestas risus. Duis tincidunt eros dui, vel elementum nunc gravida id. Pellentesque a mattis ligula, eget feugiat purus. Donec eu est porttitor, vulputate tortor scelerisque, convallis nulla. Integer ornare turpis a quam ullamcorper, et euismod sem semper. Ut vulputate sit amet elit et condimentum. In hac habitasse platea dictumst. Vestibulum congue sem et nisi lacinia pellentesque. Aenean tincidunt, augue sit amet blandit molestie, velit lacus ullamcorper sem, a egestas est elit eu mi.\
\
Nam ut nunc nunc. Curabitur posuere vulputate arcu nec rhoncus. Morbi elementum commodo enim. Nullam faucibus bibendum diam. Integer facilisis facilisis massa. Curabitur vitae mauris ac orci pharetra sodales. Aliquam massa eros, commodo et dolor sit amet, porttitor dictum eros. In lectus magna, ullamcorper eu massa eu, iaculis vulputate orci. Integer auctor, lorem sit amet dignissim ullamcorper, ex nunc ornare odio, sit amet vulputate arcu dui non velit. Proin sollicitudin mi justo, eu vestibulum enim ultrices et. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Integer sit amet ipsum mauris. Etiam vel dui id metus fermentum pharetra. Sed hendrerit, ante ac volutpat sodales, purus dolor consectetur justo, vel malesuada augue neque in mi. Suspendisse egestas lacus id sapien vehicula posuere sed bibendum metus.\
\
Donec auctor bibendum neque aliquam consequat. In tempus tempus dignissim. Suspendisse eu diam est. Pellentesque ultrices mauris odio, id ullamcorper ante condimentum in. Sed semper laoreet urna sit amet venenatis. Mauris eget fringilla diam. Nullam eu fringilla libero, feugiat commodo sapien. Mauris cursus elit augue, vel suscipit sem molestie eget. Aliquam at lacinia leo. Fusce consequat odio arcu, iaculis ultricies diam viverra sit amet. Maecenas congue neque ut bibendum euismod. Nam efficitur, ligula ac iaculis facilisis, ligula ligula facilisis ipsum, ac ultricies neque est at augue. Etiam risus enim, cursus eu vehicula nec, commodo vel dui. Quisque sit amet massa non erat fermentum varius. Vivamus ut euismod nunc. Duis eu fringilla nunc.\
\
Praesent venenatis nisi nec velit aliquam, et consectetur risus dapibus. Etiam nisl arcu, porta at sapien gravida, posuere lacinia metus. Aliquam tincidunt dolor at pellentesque eleifend. Praesent a enim non quam aliquam venenatis vel id ante. Nam accumsan ultrices sagittis. Nulla elementum rutrum massa, id semper velit scelerisque non. Donec sagittis consequat tortor, eget venenatis ex cursus nec. Aenean ultrices imperdiet lorem in convallis. Nulla pretium eros eu consequat congue. Donec posuere ligula sit amet sem vulputate, vitae elementum enim porttitor. In aliquam, metus quis vehicula maximus, mi ante malesuada velit, et consectetur ligula diam eu ligula. Curabitur lacinia feugiat sodales. Etiam in nulla placerat, imperdiet ligula et, finibus nisl. Nullam posuere enim at felis aliquam venenatis.\
\
Proin elit tortor, ultrices sed nunc in, tincidunt cursus turpis. Donec lectus eros, laoreet sed suscipit ac, rhoncus ut augue. Nunc vitae semper metus. Pellentesque lorem nibh, imperdiet sit amet rhoncus quis, ultrices a turpis. Cras eu aliquam libero, ac sollicitudin ligula. Proin tincidunt accumsan malesuada. Curabitur luctus dui a nunc dapibus, nec consequat magna vestibulum. Ut quis arcu tincidunt, viverra massa et, venenatis lacus. Ut non nisl at enim laoreet semper ac nec erat. Vestibulum fringilla vulputate nulla, sed congue arcu hendrerit ut. Sed eleifend urna ac enim condimentum, sit amet sollicitudin sem faucibus. Suspendisse facilisis volutpat tristique. Morbi fringilla urna ut erat iaculis vestibulum.\
\
Donec efficitur tristique felis non ultricies. Aliquam magna lorem, posuere in porta et, facilisis ut massa. Mauris ac blandit lacus, a laoreet odio. Suspendisse potenti. Donec venenatis rhoncus dolor, eu ornare augue mattis et. Nunc ornare porta mollis. Phasellus interdum eros eget feugiat rutrum. Nullam a magna id lorem mattis aliquam. Phasellus non posuere libero. Suspendisse finibus, nulla et viverra semper, velit metus finibus nunc, non tempus dolor augue non nisl. Phasellus ultricies id nisi vel sollicitudin. Pellentesque ut neque eu eros mattis interdum ac vel est.\
\
Proin id dui in justo pulvinar tempus eu ut lorem. Vivamus tincidunt felis vitae malesuada iaculis. Donec pellentesque metus a tristique ultrices. Nullam vel dui sit amet dui dignissim suscipit quis id libero. Donec blandit et ex ut dignissim. Integer est magna, gravida a mauris id, euismod commodo metus. Proin vel tellus volutpat, lobortis nibh non, rutrum lacus. Donec nunc erat, tincidunt vitae tellus ut, condimentum scelerisque ligula. Pellentesque pharetra lectus et enim dignissim finibus.\
\
In mollis odio et luctus hendrerit. Aenean non enim enim. Donec maximus mi vitae finibus vehicula. Sed convallis commodo porttitor. In auctor nec enim at luctus. Suspendisse urna sapien, vulputate blandit auctor et, placerat ac lorem. Proin et euismod leo. Nunc non orci vulputate, dignissim arcu rhoncus, feugiat urna. Curabitur ut augue turpis. Aliquam auctor vestibulum tempus. Curabitur hendrerit efficitur ullamcorper. Duis tellus tortor, volutpat in augue id, iaculis ultrices arcu. Nullam ut pretium erat. Aliquam iaculis feugiat velit, a tincidunt turpis. In malesuada facilisis mauris vitae sollicitudin. Curabitur ultricies porttitor mattis.\
\
Etiam interdum porta libero, sed lobortis sapien porta sed. Curabitur ullamcorper mauris vitae viverra mattis. Nulla ac semper ante. Phasellus maximus aliquam lorem, vitae molestie lectus efficitur ut. Donec tempor fringilla nulla id pretium. Etiam euismod est vitae dui auctor, id imperdiet ex cursus. Donec congue elit nec felis cursus, non fermentum urna fringilla. Donec posuere quis purus vel eleifend. Nulla sed dui dignissim, vehicula urna non, facilisis ligula. Proin eu molestie ligula, non dapibus nibh. Donec lobortis arcu sit amet lacus semper, quis mattis ipsum tincidunt. Nam pellentesque justo eleifend, facilisis metus at, tincidunt diam. Suspendisse ac mi luctus, maximus sapien sit amet, interdum lectus. Quisque tempor eget dolor vel pellentesque. Maecenas placerat at lectus sed scelerisque. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.\
\
Mauris dignissim mattis dui, feugiat lobortis diam mattis non. Curabitur neque mi, scelerisque in convallis quis, imperdiet consectetur lorem. Vestibulum non mi ac justo facilisis consequat in sit amet ligula. Quisque urna sem, tristique non ligula consectetur amet.\n");

void test_lorem(const char *title, AbstractMatcher &tokenizer, AbstractMatcher &filter, AbstractMatcher &splitter)
{
  FILE *file;
  size_t hits;

  banner(title);

  printf("Number of runs per timed test = %d\n\n", RUNS);

  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    tokenizer.input(lorem);
    hits = 0;
    while (tokenizer.input() != EOF)
      ++hits;
  }
  timer("Reading lorem string with input() took");
  if (hits != lorem.size())
    printf("FAIL hits=%zu\n", hits);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    tokenizer.input(wlorem);
    hits = 0;
    while (tokenizer.input() != EOF)
      ++hits;
  }
  timer("Reading lorem wide string with input() took");
  if (hits != wlorem.size())
    printf("FAIL hits=%zu\n", hits);

  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    tokenizer.input(lorem);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem string took");
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    tokenizer.input(wlorem);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem wide string took");
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    filter.input(lorem);
    hits = 0;
    while (filter.find())
      ++hits;
  }
  timer("Searching lorem string took");
  if (hits != 1211)
    printf("FAIL hits=%zu\n", hits);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    filter.input(wlorem);
    hits = 0;
    while (filter.find())
      ++hits;
  }
  timer("Searching lorem wide string took");
  if (hits != 1211)
    printf("FAIL hits=%zu\n", hits);

  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    splitter.input(lorem);
    hits = 0;
    while (splitter.split())
      ++hits;
  }
  timer("Splitting lorem string took");
  if (hits != 1201)
    printf("FAIL hits=%zu\n", hits);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    splitter.input(wlorem);
    hits = 0;
    while (splitter.split())
      ++hits;
  }
  timer("Splitting lorem wide string took");
  if (hits != 1201)
    printf("FAIL hits=%zu\n", hits);

  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    filter.input(lorem);
    std::vector<std::string> words(filter.find.begin(), filter.find.end());
    hits = words.size();
  }
  timer("Populating a vector of lorem string words took");
  if (hits != 1211)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("lorem.txt", "r");
  tokenizer.input(file);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(file);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("utf8lorem.txt", "r");
  tokenizer.input(file);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(file);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning utf8lorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("utf16lorem.txt", "r");
  tokenizer.input(file);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(file);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning utf16lorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("utf16lelorem.txt", "r");
  tokenizer.input(file);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(file);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning utf16lelorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("utf32lorem.txt", "r");
  tokenizer.input(file);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(file);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning utf32lorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("latin1lorem.txt", "r");
  tokenizer.input(Input(file, Input::file_encoding::latin));
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(Input(file, Input::file_encoding::latin));
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning latin1lorem.txt took");
  fclose(file);
  if (hits != 6183)
    printf("FAIL hits=%zu\n", hits);

  file = fopen("ebcdiclorem.txt", "r");
  tokenizer.input(Input(file, Input::file_encoding::ebcdic));
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    rewind(file);
    tokenizer.input(Input(file, Input::file_encoding::ebcdic));
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning ebcdiclorem.txt took");
  fclose(file);
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  std::istringstream stream(lorem);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    stream.clear(); // always do this before seekg()
    stream.seekg(0);
    tokenizer.input(stream);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem string stream took");
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);

  std::ifstream ifs("lorem.txt", std::ifstream::in);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    ifs.clear(); // always do this before seekg()
    ifs.seekg(0);
    tokenizer.input(ifs);
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem file stream took");
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);
  timer();
  for (size_t run = 0; run < RUNS; ++run)
  {
    ifs.clear(); // always do this before seekg()
    ifs.seekg(0);
    ifs >> tokenizer; // read and buffer the whole thing
    hits = 0;
    while (tokenizer.scan())
      ++hits;
  }
  timer("Scanning lorem file stream with >> took");
  ifs.close();
  if (hits != 2682)
    printf("FAIL hits=%zu\n", hits);
}

int main()
{
  Matcher tokenizer("(\\w+)|(\\W)");
  Matcher filter("\\w+");
  Matcher splitter("\\s+");
  test_lorem("RE/flex DFA/POSIX matching with FSM opcode table (default)", tokenizer, filter, splitter);

#if 0
  // 1. generate code with options n;o;f below:
  static const Pattern tokenizer_pattern("(\\w+)|(\\W)", "n=tokenizer;o;f=test_lorem_tokenizer.cpp");
  static const Pattern filter_pattern("\\w+", "n=filter;o;f=test_lorem_filter.cpp");
  static const Pattern splitter_pattern("\\s+", "n=splitter;o;f=test_lorem_splitter.cpp");
#else
  // 2. set the patterns and compile the generated code with this test code
  static const Pattern tokenizer_pattern(reflex_code_tokenizer);
  static const Pattern filter_pattern(reflex_code_filter);
  static const Pattern splitter_pattern(reflex_code_splitter);
#endif
  // 3. set the matchers
  Matcher fast_tokenizer(tokenizer_pattern);
  Matcher fast_filter(filter_pattern);
  Matcher fast_splitter(splitter_pattern);
  test_lorem("RE/flex DFA/POSIX matching with FSM code (--fast and pattern option \"o\")", fast_tokenizer, fast_filter, fast_splitter);

  PCRE2Matcher pcre2_tokenizer("(\\w+)|(\\W)");
  PCRE2Matcher pcre2_filter("\\w+");
  PCRE2Matcher pcre2_splitter("\\s+");
  test_lorem("PCRE2 JIT-optimized Perl matching", pcre2_tokenizer, pcre2_filter, pcre2_splitter);

  BoostMatcher boost_tokenizer("(\\w+)|(\\W)");
  BoostMatcher boost_filter("\\w+");
  BoostMatcher boost_splitter("\\s+");
  test_lorem("Boost.Regex Perl matching", boost_tokenizer, boost_filter, boost_splitter);

  BoostPosixMatcher boost_posix_tokenizer("(\\w+)|(\\W)");
  BoostPosixMatcher boost_posix_filter("\\w+");
  BoostPosixMatcher boost_posix_splitter("\\s+");
  test_lorem("Boost.Regex POSIX matching", boost_posix_tokenizer, boost_posix_filter, boost_posix_splitter);

  StdMatcher std_tokenizer("(\\w+)|(\\W)");
  StdMatcher std_filter("\\w+");
  StdMatcher std_splitter("\\s+");
  test_lorem("std::regex Perl matching", std_tokenizer, std_filter, std_splitter);

  return 0;
}
