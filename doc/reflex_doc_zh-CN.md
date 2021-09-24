RE/flex中文文档                                                      {#mainpage}
                                                                @tableofcontents

# RE/flex简介                                                           {#intro}

RE/flex是一个基于正则表达式的，快速词法分析器生成器，具有完整的Unicode支持，
支持缩进匹配，惰性匹配，单词边界匹配和很多其他的现代特性。

RE/flex还包括一个用C++编写的快速正则表达式引擎，可以选择生成有限状态机表或者
直接代码，以便于更有效的匹配输入。

RE/flex包括一个智能的输入类，用于规范处理来自文件、流、字符串和内存的输入。

RE/flex与Bison/Yacc兼容，并且可以接受Flex词法分析器规则脚本作为输入。

## RE/flex特点

+ 对于解析标记等典型应用，比Flex++更快；
+ 支持把Flex/Lex词法分析器规则脚本作为输入，并且扩展支持Unicode；
+ 符合IEEE POSIX P1003.2标准（像Lex和Flex一样）；
+ 完全支持Unicode，自动检测UTF-8/16/32编码，智能处理输入；
+ 支持旧文件编码格式，例如CP 1250，EBCDIC；
+ 提供用于lex或语法错误报告和恢复的方法；
+ 易于与Bison可重入、C++、桥接、位置解析器集成；
+ 生成的代码易于理解；
+ 生成线程安全的扫描器；
+ 为有限状态机可视化生成可视化图形（graphviz）文件；
+ 用于词法分析器源代码输出的直观输出的自定义选项；
+ 在直接代码或者有限状态机表中的高效匹配；
+ 可选的“自由空间模式”以提高分析器语法规则的可读性；
+ 正则表达式可以包含惰性匹配；
+ 正则表达式可以包含单词边界匹配；
+ 正则表达式可以包含缩进匹配；
+ 可以选择其他正则表达式引擎，比如PCRE2和Boost.Regex；
+ 在一个宽松的开源许可证（BSD-3）下发布。

RE/flex提供很多对Flex++的实用改进，比如：

+ 没有输入缓冲区长度限制（Flex缓冲区有16KB的限制）；
+ `yypush_buffer_state`保存扫描器的状态（行，列，以及缩进位置），不仅仅保存
输入缓冲区；
+ 提供新方法分析ASCII和Unicode输入，比如`str()`和`wstr()`获得（宽）字符串匹配，
`line()`和`wline()`获得当前（宽）行号以便于进行错误报告。

词法分析器规则脚本中的模式由`reflex`工具转换为高效确定有限状态机，放在直接代码
（--fast选项）或者操作码表（--full选项）中。其他的正则表达式引擎包括PCRE2和
Boost.Regex，可以用于Perl和POSIX匹配模式。UTF-8/16/32文件输入规范化，可以
自动的为Unicode模式匹配执行。其他编码可以通过用最少的代码编程指定。因此，
RE/flex扫描器可以处理任意类型编码输入。

RE/flex结合适当的面向对象设计原则，并且不像Flex那样依赖于宏和全局变量。
当设置`--flex`选项时，宏和全局变量由`reflex`生成添加到源代码中。一个
RE/flex扫描器是一个从词法分析基类模板继承而来的生成类，其匹配引擎
定义为一个模板参数。

RE/flex与Lex/Flex和Bison/Yacc兼容，分别通过`--flex`和`--bison`选项使用。
选项`−−yy`强制符合基本的、简洁的POSIX Lex要求。RE/flex还提供了与Bison无缝
集成的特定选项，包括Bison桥接、Bison定位、Bison C++、完整Bison以及
可重入解析器。

本文中用*regex*作为正则表达式的简称。但是“正则表达式”指*正则语言*的形式概念，
而*regex*通常指Perl中所介绍的基于回溯的正则匹配。两个概念都适用于RE/flex模式。

总而言之，RE/flex实际上是多个东西组成的一个包：

- 一个更快的，功能丰富的[Flex++ and Flex](dinosaur.compilertools.net/#flex)
替代品；
- 一个独立的正则表达式库，用于在C++中进行快速正则匹配；
- 一个[Boost.Regex](www.boost.org/libs/regex)和[PCRE2](www.pcre.org)
库的C++正则表达式API增强，用于匹配、搜索、分隔和扫描输入，输入可以来自
（宽）字符串、文件、以及长度可能无限的流。

本文采用的排版约定是：

+ `Courier` 表示C或C++源代码

+ <i>`Courier`</i>表示词法分析器规则脚本和文件名称

+ <b>`Courier`</b>表示在终端窗口显示的命令或者程序输出

@note 一个澄清技术问题的说明。

@warning 注意警告！

🔝 [Back to table of contents](#)

## 又是另一个扫描器生成器                                             {#intro1}

Lex，Flex和变体是强大的*扫描器生成器*（也称为*词法分析器*），它可以根据词法
分析规则生成扫描器。词法分析器规则脚本定义带有用户自定义动作的模式，在输入流中匹配
到模式时，对应的动作将被执行。扫描器重复的匹配模式和触发对应的动作直到输入流
结束。

Lex和Flex都是流行的用于开发*分词器（tokenizers）*的工具，当匹配到对应的模式时，
分词器会执行用户定义的动作或者返回一个*标记（token）*。这些分词器通常被用于
为编程语言的编译器或解释器实现扫描源代码并分割成标记。分词器中的正则表达式
用于定义扫描的源代码中的标识符、常量、关键字、符号的组成，以及需要跳过的空白。

例如，下述考虑在词法分析器规则脚本中定义的模式以及对应的动作：

<div class="alt">
~~~{.cpp}
    %%
    /* 模式 */              /* 动作 */
    "if"                    return KEYWORD_IF;
    "*"                     return OP_MUL;
    "="                     return OP_ASSIGN;
    [a-zA-Z_][a-zA-Z0-9_]*  return ASCII_IDENTIFIER;
    [0-9]+                  return CONST_NUMBER;
    \"([^\\"]|\\")*\"       return CONST_STRING;
    [ \t\r\n]               /* 没有动作: 忽略所有的空白 */
    %%
~~~
</div>

当分词器匹配一个模式时，对应的动作将被执行。上述示例返回标记给编译器的解析
器，编译器的解析器重复调用分词器获得更多的标记，直到达到输入的尾部。当达到
输入数据的尾部的时候，分词器返回0。

Lex和Flex工具一直保持相对稳定，但是对Unicode文本编码分词的需求却在增加，比如
UTF-8、UCS/UTF-16、和UTF-32等常用宽字符格式。还有，Flex/Lex中的正则表达式语
法与现代正则表达式语法相比也是受到限制的。FLex不支持Unicode模式、没有惰性重
复，没有单词边界匹配，没有缩进匹配，还有一个限制非常大的可供选择的元数据转义
集合。为了让事情变得更有趣，写一个可以避免POSIX匹配的“贪婪陷阱”的词法分析器
规则是一个挑战。

Flex/Lex扫描器使用POSIX模式匹配，意味着对同一个输入匹配时，在一组模式中，从
左侧匹配最长的模式将被返回。因为POSIX匹配器对任意给定的输入文本生成最长的
匹配，在使用“贪婪”重复的模式（`X*`、`X+`等）我们应该小心，因为我们的模式可能
会吞噬比预期更多的输入。我们最终陷入“贪婪陷阱”。

为了说明这个陷阱，考虑匹配HTML注释`<!−− ... −−>`使用模式`<!−−.*−−>`。问题是
重复模式`X*`是贪婪的，而`.*−−>`模式匹配所有的内容，直到最后一个`−−>`，并且跳
过`<!−−`和最后一个`−−>`之间的`−−>`。

@note 在Flex/Lex模式里，点`.`通常不匹配换行符号`\n`，除非我们使用*dot-all mode*。

我们可以使用更多复杂的模式，比如：`<!−−([^−]|−[^−]|−−+[^−>])*−*−−>`仅仅用于
匹配HTML中的注释，通过确定模式在第一个匹配到`−−>`时结束。

RE/flex用一个正则表达式`<!−−.*?−−>`足以匹配HTML注释，使用*惰性重复* `X*?`构
造， 也被称为*非贪婪重复*。问号`?`是一个*惰性记号*，`X*?`模式的重复方式被修改
为：当这个模式的后边部分（-->）不匹配时，仅重复`X`。因此，正则表达式
`<!−−.*?−−>`在RE/flex中就可以匹配HTML注释，而不会匹配更多的内容。
但是Flex/Lex不允许我们使用惰性。

所以，毫不奇怪，即使Flex手册也使用临时代码而不是模式扫描C/C++源代码输入以
匹配在`/*`和`*/`之间的多行注释。Flex手册推荐：

<div class="alt">
~~~{.cpp}
    "/*"    {  /* 跳过多行注释 */
      int c;
      while ((c = yyinput()) != 0)
      {
        if (c == '\n')
          ++mylineno;
        else if (c == '*')
        {
          if ((c = yyinput()) == '/')
            break;
          unput(c);
        }
      }
    }
~~~
</div>

使用Flex代码的另一个争论是Flex内部缓冲区被限制为16KB。相比之下，RE/flex的缓冲
区是动态变化的并且将永远不会用完缓冲空间，以接收更长的匹配。

在RE/flex中不需要诸如此类的解决方法。RE/flex扫描器使用具有表达模式语法的正则
表达式库。我们可以使用惰性重复为多行注释编写正则表达式模式，如下：

<div class="alt">
~~~{.cpp}
    "/*"(.|\n)*?"*/"  /* 无动作: 忽略多行注释 */
~~~
</div>

大多数正则表达式库都支持被模式匹配所依赖的语法和功能。拥有惰性标记的正则表达
式可以更加容易阅读和理解。大多数支持惰性标记的正则表达式库在Perl模式下运行，
在输入上使用回溯。扫描器使用POSIX模式匹配，意味着找到左侧最长匹配。正如我们
之前看到的，差异非常重要，当我们考虑在Perl模式下匹配指定模式去解析输入的问题
更是如此，我们接下来会解释。

考虑之前展示的词法分析器规则脚本示例。假定输入的文本是`iflag = 1`。在POSIX模式下，
我们为`iflag`返回`ASCII_IDENTIFIER`，为`=`返回`OP_ASSIGN`，并且为`1`返回
`NUMBER`。在Perl模式下，我们发现`iflag`匹配`if`，单词的后半部分没有被匹配，
`if`被返回`KEYWORD_IF`，`lag`被返回`ASCII_IDENTIFIER`，`=`被返回`OP_ASSIGN`，
并且`1`被返回`NUMBER`。Perl模式贪婪的匹配，返回左侧最长匹配。

在一个扫描器中使用Perl模式需要在词法分析器规则脚本中定义所有的重复的模式，以便
首先定义最长的匹配模式以确保最长匹配。相比之下，POSIX模式时*声明式的*，并且
允许在规则定义中用任意顺序定义模式。也许唯一的顺序限制是匹配相同的输入，
比如：匹配关键字`if`，`KEYWORD_IF`必须在`ASCII_IDENTIFIER`之前被匹配。

为此原因，RE/flex扫描器使用正则表达式库默认在POSIX模式。

总之：

- RE/flex规则和使用与Lex/Flex和Bison/Yacc是兼容的。

- RE/flex扫描器生成器接受[Flex](dinosaur.compilertools.net/#flex)兼容的规则，
并有额外的选项用于选择匹配引擎，以及用于指定C++类生成器的名称和选项
  
- RE/flex扫描器生成器选项`−−bison`生成与[Bison](dinosaur.compilertools.net/#bison)
兼容的扫描器。RE/flex还提供和Bison桥接、Bison定位、Bison完整、以及重定位解析
器集成的选项。

- Flex和Lex规则中的正则表达式语法被限制为POSIX ERE。相比而言，RE/flex规则语法
是正则表达式中心的，被设计并提供一个更加丰富的模式语法，包括惰性标记。

- 词法分析类的匹配引擎有一个通用的API接口，被声明为抽象的基础匹配类模板。

- RE/flex扫描器没有被实现为一组全局函数和表。RE/flex扫描器是生成词法分析器类的
实例，它确保扫描器是线程安全的（相比之下，Flex和Flex++不是线程安全的）。
一个词法分析器类是继承自一个抽象基词法分析器类模板的，它实例化时，带着一个
正则表达式匹配引擎作为模板参数。

- Lex和Flex扫描器被限在ASCII编码或者8位字符集，没有对Unicode的充分支持。
RE/flex规则被扩展为支持Unicode，比如RE/flex扫描器生成器生成的扫描器可以
自动的处理UTF-8/16/32编码输入文件。

- 匹配引擎和扫描器的输入被实现为一个类，它支持流输入，并且自动的解码UTF编码
文件。

- RE/flex扫描器可以是独立的应用，类似于Flex扫描器。

\ref reflex章节有RE/flex扫描器生成工具的更多细节。

在本手册的下一个章节，我们将快速的浏览一下RE/flex正则表达式API，它可以被作为
一个独立的库，用于常规C++应用中来自字符串、文件和流输入的匹配、搜索、扫描
和拆分。 （即应用程序不一定是编译器的分词器）。

🔝 [Back to table of contents](#)                                       {#intro2}

## 还是一个灵活的正则表达式库

RE/flex正则表达式模式匹配类，包括Boost.Regex的两个类、PCRE2的两个类、
C++11 std::regex的两个类，以及一个RE/flex类：

  引擎        | 需要include的头文件  | C++ 匹配器类
  ------------- | ----------------------- | -----------------------------------
  RE/flex regex | `reflex/matcher.h`      | `Matcher`
  Boost.Regex   | `reflex/boostmatcher.h` | `BoostMatcher`, `BoostPosixMatcher`
  PCRE2         | `reflex/pcre2matcher.h` | `PCRE2Matcher`, `PCRE2UTFMatcher`
  std::regex    | `reflex/stdmatcher.h`   | `StdMatcher`, `StdPosixMatcher`

RE/flex的`reflex::Matcher`类实例化时将正则表达式模式编译为有效的有限状态机
(FSMs)。这些确定的自动机相当程度的加速了匹配速度，以初始化FSM构架为代价 
（如何避免运行时开销，请参考下文）。RE/flex匹配器仅支持POSIX模式匹配，见
\ref reflex-posix-perl。

`reflex::BoostMatcher`和`reflex::BoostPosixMatcher`类分别是用于Perl模式和POSIX
模式匹配的。

`reflex::PCRE2Matcher`和`reflex::PCRE2UTFMatcher`类仅用于Perl模式匹配，其中，
后者通过`PCRE2_UTF+PCRE2_UCP`使用本机的PCRE2 Unicode编码。PCRE2匹配
器用JIT优化加速匹配，这是在匹配器实例化时，以额外处理为代价的。 当许多匹配
项需要处理时，收益是大于代价的。

C++11的std::regex支持ECMAScript和AWK的POSIX语法，可以分别通过`StdMatcher`和
`reflex::StdPosixMatcher`类调用。因此，std::regex语法与Boost.Regex，PCRE2，
以及RE/flex相比，有更多的限制。

RE/flex正则表达式公共接口API是在一个抽象基类模板`reflex::AbstractMatcher`中实现
的，正则表达式匹配器从该类派生。这个正则表达式API提供一个公共接口，接口用于
生成的扫描器。你也可以在你的C++程序中，用这个API用于模式匹配。

RE/flex抽象匹配器提供四个用于正则表达式匹配的动作，匹配使用的正则表达式引擎
继承自抽象基类：

  方法        | 结果
  ----------- | ---------------------------------------------------------------
  `matches()` | 返回非零值，如果输入文本从开始到结尾匹配
  `find()`    | 搜索输入文本，并且返回非零值，如果匹配到了结果
  `scan()`    | 返回非零值，如果当前位置匹配到部分内容
  `split()`   | 返回非零值，用于在下一个匹配分割输入文本

这些方法为一个匹配返回非零值，即`size_t accept()`值，并且是可重复的，其中，
最后三个返回额外的匹配。

例如，检查一个字符串是否是合法的日期，使用Boost.Regex表示如下：

~~~{.cpp}
    #include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

    // 使用BoostMatcher检查生日字符串是否合法的日期
    if (reflex::BoostMatcher("\\d{4}-\\d{2}-\\d{2}", birthdate).matches())
      std::cout << "Valid date!" << std::endl;
~~~

我们可以用PCRE2代替Boost.Regex执行完全相同的检查。但是，JIT优化过的PCRE2匹
配器更适合当很多匹配需要处理时，而不仅仅像上边那样匹配一个。

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 使用PCRE2Matcher检查生日字符串是否合法的日期
    if (reflex::PCRE2Matcher("\\d{4}-\\d{2}-\\d{2}", birthdate).matches())
      std::cout << "Valid date!" << std::endl;
~~~

交换正则表达式库非常简单。有时我们需要正则表达式转换器，当我们用一个正则表
达式库不支持的正则表达式特性时。

搜索字符串，查找与所有单词匹配的模式`\w+`:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // use a PCRE2Matcher to search for words in a sentence
    reflex::PCRE2Matcher matcher("\\w+", "How now brown cow.");
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

我们执行这个代码，输出：

    Found How
    Found now
    Found brown
    Found cow

如果我们想匹配Unicode单词，`\w+`需要转换为Unicode模式，这里我们使用Boost.Regex：

~~~{.cpp}
    #include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

    // convert \w+ for Unicode matching
    static const std::string pattern = reflex::BoostMatcher::convert("\\w+", reflex::convert_flag::unicode);

    // use a BoostMatcher to search for words in a sentence
    reflex::BoostMatcher matcher(pattern, "Höw nöw bröwn cöw.");
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

所有的匹配器都需要转换为Unicode模式，除了`reflex::PCRE2UTFMatcher`，因为默认
情况下匹配器在非Unicode模式下工作，以匹配字节而不是宽字符。


我们执行这个代码，输出：

    Found Höw
    Found nöw
    Found bröwn
    Found cöw

使用`reflex/pcre2matcher.h`中定义的`reflex::PCRE2Matcher`代码相同，产生相同的
结果。对于以下示例，我们将使用Boost.Regex或PCRE2，两者可以互换使用。

`scan`方法与`find`相似，但是`scan`仅从输入的当前位置匹配。在当前位置不可能部分
匹配时会失败。在输入源文件中重复扫描必须是连续的，否则`scan`返回零（不匹配）。

`split`方法大致与`find`方法相反，返回匹配项之间的文本。例如，使用非单词匹配
`\W+`：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 使用PCRE2Matcher搜索句子里的单词
    reflex::PCRE2Matcher matcher("\\W+", "How now brown cow.");
    while (matcher.split() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

运行这段代码输出：

    Found How
    Found now
    Found brown
    Found cow
    Found

注意split还返回最后一次匹配后的剩余文本（可能为空），正如上面输出所看到的：
使用`\W+`的最后一次分割返回了一个空字符串，这是句子中句号之后的剩余输入。

`find()`，`scan()`和`split()`方法返回一个非零*"accept"*值，它对应于捕获的正则
表达式组，或者返回零，当没有找到匹配时。方法返回1，用于没有使用组的匹配。
`split()`方法有一个特例。它返回`reflex::PCRE2Matcher::Const::EMPTY`（并且其他
匹配器也是如此） ，当在输入的结尾执行一个匹配，并且分割达到一个空字符串，
像上述`split()`匹配示例一样。

另一个示例：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 使用PCRE2Matcher将字符串标记化
    reflex::PCRE2Matcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
    size_t accept;
    while ((accept = matcher.scan()) != 0)
    {
      switch (accept)
      {
        case 1: std::cout << "word\n";  break;
        case 2: std::cout << "space\n"; break;
        case 3: std::cout << "other\n"; break;
      }
    }
~~~

执行以上代码输出：

    word
    space
    word
    space
    word
    space
    word
    other

当前，`reflex`命名空间中可用的正则表达式引擎类有：

  类                  | 模式  | 引擎          | 性能
  ------------------- | ----- |-------------- | ---------------------------------
  `Matcher`           | POSIX | RE/flex lib   | 确定有限自动机，无回溯
  `BoostMatcher`      | Perl  | Boost.Regex   | 回溯
  `BoostPerlMatcher`  | Perl  | Boost.Regex   | 回溯
  `BoostPosixMatcher` | POSIX | Boost.Regex   | 回溯
  `PCRE2Matcher`      | Perl  | PCRE2         | JIT优化的回溯
  `PCRE2UTFMatcher`   | Perl  | PCRE2 UTF+UPC | JIT优化的回溯
  `StdMatcher`        | ECMA  | std::regex    | 回溯
  `StdEcmaMatcher`    | ECMA  | std::regex    | 回溯
  `StdPosixMatcher`   | POSIX | std::regex    | 回溯

RE/flex正则表达式引擎使用一个确定的有限状态机(FSM)以获得更好的匹配性能。但是，
构建一个FSM增加开销。这个匹配器更加适合搜索长文本。FSM构造器开销可以被消除，
通过将正则表达式预转换为C++代码表，我们将很快就看到。

Boost.Regex引擎通常使用Perl模式匹配。我们为RE/flex扫描器生成器添加了一个POSIX
模式的Boost.Regex引擎类。扫描器通常使用POSIX模式匹配。见 \ref reflex-posix-perl
获取更多信息。

Boost.Regex引擎都被初始化使用`match_not_dot_newline`选项，这将禁止dotall匹配
作为默认设置。Dotall可以用`(?s)`正则表达式修饰符重新启用。这样做是为了和扫描器
兼容。

PCRE2引擎使用Perl模式匹配。PCRE2还提供POSIX模式匹配，通过`pcre2_dfa_match()`。
但是，组捕获在这个模式下不被支持。所以，没有PCRE2 POSIX模式类。JIT优化加速
匹配。但是，这是在当PCRE2匹配器实例化时，付出额外处理代价的。

匹配器可以被用于字符串和款字符串，比如`std::string`和`std::wstring`，`char*`和
 `wchar_t*`。款字符串被转换为UTF-8，以便于可以匹配包含Unicode模式的正则表达式
 匹配。

使用基于8位或者不支持Unicode的正则表达式库匹配Unicode模式，我们希望先转换
正则表达式字符串，然后我们在正则表达式匹配引擎中使用它：

~~~{.cpp}
    #include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern

    // 使用Unicdoe字符类转换正则表达式，以创建一个模式FSM:
    static const std::string regex = reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+", reflex::convert_flag::unicode);
    static const reflex::Pattern pattern(regex);

    // 使用匹配器检查句子是否是希腊语：
    if (reflex::Matcher(pattern, sentence).matches())
      std::cout << "This is Greek" << std::endl;
~~~


这会将Unicode字符类转换为UTF-8，以便于用一个8位的正则表达式引擎匹配。静态
`convert`方法在每个匹配器中有差异。如果不幸转换失败，或者正则表达式语法不正确，
一个`reflex::regex_error`错误将作为异常被抛出。

转换非常快（运行时间与正则表达式大小线性相关），但是并非没有代价。我们应该
尽可能的`static`的转换正则表达式模式，如上所述，以消除重复转换和模式构件的成本。

一个`reflex::Pattern`对象时不可改变的（它存储一个常量表）并且可能在线程间共享。

使用`convert`，用选项`reflex::convert_flag::unicode`改变 `.` (点), `\w`, `\s`, `\l`, `\u`, 
`\W`, `\S`, `\L`, `\U`字符类的涵义。


文件内容通过流的方式输入匹配器，使用部分匹配算法立即匹配。这意味着输入不需要
被完整的加载到内存。这支持交互式匹配，比如，从控制台输入匹配：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 使用PCRE2Matcher从控制台输入搜索和显示单词
    reflex::PCRE2Matcher matcher("\\w+", std::cin);
    matcher.interactive();
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

交互式输入消耗相对较慢，因为没有缓存输入。

我们还可以从`FILE`描述符匹配文本。使用`FILE`描述符的额外好处是：管理输入源及其
状态的`reflex::Input`类将UTF-16/32输入自动解码为UTF-8。

例如,模式匹配<i>`"cows.txt"`</i>的内容可能使用UTF-8，16，或者32编码：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 使用一个PCRE2Matcher搜索和显示FILE中的单词
    FILE *fd = fopen("cows.txt", "r");
    if (fd == NULL)
      exit(EXIT_FAILURE);
    reflex::PCRE2Matcher matcher("\\w+", fd);
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

方法`find`, `scan`,和`split`还被实现为输入迭代器，迭代器应用过滤标记化和拆分：

  迭代范围                           | 作为      | 迭代对象
  ------------------------------ | --------- | -------------------------------
  `find.begin()`...`find.end()`    | filter       | all matches
  `scan.begin()`...`scan.end()`  | tokenizer| continuous matches
  `split.begin()`...`split.end()`    | splitter    | text between matches

类型`reflex::AbstractMatcher::Operation`是一个定义`find`、`scan`、以及`split`的
仿函数。仿函数操作在成功后返回true。迭代器的使用，是简单的通过调用仿函数的
 `begin()`和`end()` 方法来实现的，并且返回`reflex::AbstractMatcher::iterator`。
 同样，还有`cbegin()`和`cend()`方法，返回`const_iterator`。

我们可以在很多C++任务上使用RE/flex迭代器，包括将迭代器的文本匹配塞入容器中
填充容器：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input
    #include <vector>                // std::vector

    // 使用一个PCRE2Matcher转换句子里的单词到到一个字符串向量
    reflex::PCRE2Matcher matcher("\\w+", "How now brown cow.");
    std::vector<std::string> words(matcher.find.begin(), matcher.find.end());
~~~

结果中，`words`向量包含"How", "now", "brown", "cow"。

把一个匹配器对象强制转换为`std::string`，与把`text()`转换为`std::string(text(), size())`
是相同的，正如上边示例中做的，创建`words`向量。把一个匹配器对象强制转换为
 `std::wstring`相似，但是还将UTF-8`text()`匹配转换为一个宽字符串。

RE/flex迭代器在C++11基于范围的循环中是很有用的。例如：

~~~{.cpp}
    // 需要 C++11，编译选项: cc -std=c++11
    #include <reflex/stdmatcher.h> // reflex::StdMatcher, reflex::Input, std::regex

    // 使用一个StdMatcher通过迭代器搜索句子中的单词
    reflex::StdMatcher matcher("\\w+", "How now brown cow.");
    for (auto& match : matcher.find)
      std::cout << "Found " << match.text() << std::endl;
~~~

执行上述代码输出：

    Found How
    Found now
    Found brown
    Found cow

并且RE/flex迭代器对STL算法和lambda匿名函数也是有用的。例如，计算单词频次的
直方图：

~~~{.cpp}
    // 需要 C++11，编译选项: cc -std=c++11
    #include <reflex/stdmatcher.h> // reflex::StdMatcher, reflex::Input, std::regex
    #include <algorithm>           // std::for_each

    // 使用一个StdMatcher创建组捕获的频次直方图
    reflex::StdMatcher matcher("(now)|(cow)|(ow)", "How now brown cow.");
    size_t freq[4] = { 0, 0, 0, 0 };
    std::for_each(matcher.find.begin(), matcher.find.end(), [&](size_t n){ ++freq[n]; });
~~~

结果为：`freq`数组包含 0, 1, 1, 和 2。

把一个匹配器对象强制转换为`size_t`返回组捕获的序号，正如上述示例所示。我们还
在下边例子中使用，将所有的正则表达式模式组捕获到一个向量：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input
    #include <vector>                // std::vector

    // 使用一个PCRE2Matcher转换捕获的组到一个数字向量
    reflex::PCRE2Matcher matcher("(now)|(cow)|(ow)", "How now brown cow.");
    std::vector<size_t> captures(matcher.find.begin(), matcher.find.end());
~~~

结果为：向量包含组捕获 3, 1, 3, 和 2.

把一个匹配器对象强制转换为`size_t`与调用`accept()`是相同的。

用于获得一个匹配的详细信息的方法：

  方法              | 结果
  --------------- | ---------------------------------------------------------------
  `accept()`      | 返回组捕获的序号（或者零，档没有捕获/匹配）
  `text()`        | 返回`const char*`到0结束的匹配（以`\0`结束）
  `str()`         | 返回`std::string`文本匹配（保留`\0`）
  `wstr()`        | 返回 `std::wstring`宽字符串匹配（从UTF-8转换）
  `chr()`         | 返回匹配的文本的第一个8位字符（`str()[0]`）
  `wchr()`        | 返回匹配文本的第一个宽字符（`wstr()[0]`）
  `pair()`        | 返回`std::pair<size_t,std::string>(accept(),str())`
  `wpair()`       | 返回`std::pair<size_t,std::wstring>(accept(),wstr())`
  `size()`        | 返回匹配文本的字节数
  `wsize()`       | 返回匹配文本的宽字符数
  `lines()`       | 返回匹配文本的行数 (>=1)
  `columns()`     | 返回匹配文本的列数 (>=0)
  `begin()`       | returns `const char*` to non-0-terminated text match begin
  `end()`         | returns `const char*` to non-0-terminated text match end
  `rest()`        | returns `const char*` to 0-terminated rest of input
  `span()`        | returns `const char*` to 0-terminated match enlarged to span the line
  `line()`        | returns `std::string` line with the matched text as a substring
  `wline()`       | returns `std::wstring` line with the matched text as a substring
  `more()`        | tells the matcher to append the next match (when using `scan()`)
  `less(n)`       | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`      | returns line number of the match, starting at line 1
  `columno()`     | returns column number of the match in characters, starting at 0
  `lineno_end()`  | returns ending line number of the match, starting at line 1
  `columno_end()` | returns ending column number of the match, starting at 0
  `border()`      | returns the byte offset from the start of the line of the match
  `first()`       | returns input position of the first character of the match
  `last()`        | returns input position + 1 of the last character of the match
  `at_bol()`      | true if matcher reached the begin of a new line `\n`
  `at_bob()`      | true if matcher is at the begin of input and no input consumed
  `at_end()`      | true if matcher is at the end of input
  `[0]`           | operator returns `std::pair<const char*,size_t>(begin(),size())`
  `[n]`           | operator returns n'th capture `std::pair<const char*,size_t>`

这些方法的详细解释，见 \ref regex-methods-props。

`operator[n]`通过组号`n`返回第n个捕获的组匹配，形式为一个组，包括一个
`const char*`指针指向组匹配的文本和匹配到的文本的字节数。因为指针指向的字符串
不是0结尾的，使用字节数确定匹配到的文本。

当组捕获没有匹配时，指针为`NULL`。

例如：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 一个PCRE2Matcher用于捕获名字和数字：
    reflex::PCRE2Matcher matcher("(\\w+)\\s+(\\d+)");

    // 在一个字符串上使用匹配器：
    if (matcher.input("cow 123").matches())
      std::cout <<
        "name: " << std::string(matcher[1].first, matcher[1].second) <<
        ", number: " << std::string(matcher[2].first, matcher[2].second) <<
        std::endl;
~~~

执行以上代码输出：

    name: cow, number: 123

@warning 方法`text()`返回的匹配是指向存储在内部缓冲区的`const char*`字符串。
这个指针应该在连续匹配之后，匹配器对象释放前使用。要保留`text()`的值，使用
`str()`方法，它返回`text()`的副本。

@warning 方法`operator[]`返回一个值对，包含第n个组的匹配信息（非0结束的
`const char*`指针或者NULL）和捕获的匹配的字节数。

@note 当使用`reflex::Matcher`类时，`accept()`方法返回仅在顶层正则表达式中指定的
接受模式。例如正则表达式`"(a(b)c)|([A-Z])"`有两个组，仅识别外部顶层的组。因为仅
接受顶层组，所以最外层的括号是可选的。我们可以简化这个正则表达式为
`"a(b)c|[A-Z]"`，仍然捕获定义的两个模式。

下列方法可以用于直接操作输入流：

  方法     | 结果
  ---------- | ----------------------------------------------------------------
  `input()`  | 从输入返回下一个8位字符匹配器跳过它
  `winput()` | 从输入返回下一个宽字符，匹配器跳过它
  `unput(c)` | 把8位字符`c`放回流中，匹配器然后接受它
  `wunput(c)`| 把宽字符`c`放回流中，匹配器然后接受它
  `peek()`   | 从输入返回下一个8位字符，但不消费它
  `skip(c)`  | 跳过输入直到字符`c` (`char` or `wchar_t`)被消费掉
  `skip(s)`  | 跳过输入，直到UTF-8字符串`s`被消费掉
  `rest()`   | 返回剩余的输入，以 0 结束的字符串（`char*`）
  
方法`input()`, `winput()`, 和`peek()` 返回一个非负的字符码，当输入到尾部时
返回EOF（-1）。

初始化一个匹配器用于交互使用，要分配一个新的输入源或者修改模式，使用如下
方法：

  方法                | 结果
  --------------- | -----------------------------------------------------------
  `input(i)`          | 输入设置为 `reflex::Input i` (string, stream, or `FILE*`)
  `pattern(p)`      | 设置模式 `reflex::Pattern`, `boost::regex`，或者字符串`p`
  `has_pattern()` | true，如果匹配器被设置为一个模式
  `own_pattern()` | true，如果匹配器有一个模式取管理和删除
  `pattern()`      | 模式对象的引用
  `buffer()`        | 一次缓存所有输入，成功则返回true
  `buffer(n)`      | 为输入缓冲区设置大小为`n`字节
  `buffer(b, n)`  | 使用`n`字节大小的缓冲区，在地址`b`，用一个`n`-1字节字符串(零拷贝)
  `interactive()` | 为控制台（TTY）输入设置缓冲区大小为1
  `flush()`         | 刷新内部缓冲区的剩余输入
  `reset()`         | 重置匹配器，从剩余的输入重启
  `reset(o)`       | 使用新的选项`o` ("A?N?T?")重置匹配器

`reflex::Input`对象代表一个匹配器的输入，它可以是一个文件`FILE*`，或者一个字符串
（用UTF-8字符数据），`const char*`或者`std::string`类型，或者一个流指针
 `std::istream*`。`reflex::Input`对象被隐式的从输入源创建，例如：
 
~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 设置输入源为一个字符串 (or a stream or a FILE*)
    reflex::Input source = "How now brown cow.";

    reflex::PCRE2Matcher matcher("\\w+", source);

    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;

    // 用一个新的源使用同一个匹配器（一个输入对象）：
    std::ifstream ifs("cows.txt", std::ios::in | std::ios::binary);
    source = ifs;           // 输入源被重新设置了
    matcher.input(source);  // 把ifs作为参数

    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;

    ifs.close();
~~~

整个输入被缓存在一个匹配器中，用`buffer()`，或者使用`buffer(n)`零碎的读，或者
使用`interactive()`交互的读。这些方法应该在设置输入源之后使用。读一个流，一次
缓存所有输入数据，可以用`>>`操作符作为快捷方式：

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    // 读和缓存cows.txt文件
    reflex::PCRE2Matcher matcher("\<cow\>");
    std::ifstream cows("cows.txt", std::ios::in | std::ios::binary);
    cows >> matcher;     // 与matcher.input(cows).buffer()相同;
    cows.close();        // 已经可以关闭了，因为流内容已经保存了

    // count number of 'cow' words:
    std::out << std::distance(matcher.find.begin(), matcher.find.end()) << " 'cow' in cows.txt\n";
~~~

内部匹配通过使用`buffer(b, n)`读`n`-1字节，地址在`b`的内容，可以实现零拷贝开销，
其中字节`b[0...n]`可能被匹配器修改：

~~~{.cpp}
    // 读一个0结尾的缓冲区，缓冲区内容改变了!!
    char *base = ...;  // 指向一个0结尾的缓冲区
    size_t size = ...; // 缓冲区长度包含最后的字节\0
    matcher.buffer(base, size);
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

@note 实际上，指定的字符串可能以任意字节值结束。当使用`text()`或`rest()`时，
字符串的最后一个字符被设置为0。仅仅`unput(c)`, `wunput()`, `text()`, `rest()`,
以及`span()`修改缓冲区的内容，因为这些函数在缓冲区的尾部需要一个额外的字节
使得返回的字符串是以0结束的。这意味着，我们可以使用`buffer(b, n+1)`指定只读的
内存(`n`字节，位于地址`b`），只要我们不用`unput()`, `unput()`, `text()`, 
`rest()`, 以及`span()`就是安全的。例如，搜索只读的mmap(2) `PROT_READ`内存。

到目前位置，我们解释了如何使用`reflex::PCRE2Matcher`和`reflex::BoostMatcher`进行
模式匹配。我们还可以使用RE/flex的`reflex::Matcher`类用于模式匹配。API是完全一样
的。`reflex::Matcher`类使用`reflex::Pattern`，它在内部表示为从正则表达式编译的
有效有限状态机。 这些状态机用于快速匹配。

确定有限状态机（FSMs）的构造是被优化的，但是需要一定时间，并且因此在匹配
执行前增加开销。FSM的构造不应被重复执行，如果可以避免的话。所以，我们建议
构造静态的模式对象，以仅创建FSMs一次。

~~~{.cpp}
    #include <reflex/matcher.h> // reflex::Matcher, reflex::Pattern, reflex::Input

    // 静态的分配和构造模式，即一劳永逸
    static reflex::Pattern word_pattern("\\w+");

    // 使用RE/flex POSIX匹配器在一个句子中搜索单词
    reflex::Matcher matcher(word_pattern, "How now brown cow.");
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

`reflex::Pattern`对象是不可变的（它保存一个常量表）可以在线程间共享。

RE/flex匹配器仅支持POSIX模式匹配，不支持Perl模式匹配。见 \ref reflex-posix-perl
获取更多信息。

RE/flex `reflex::Pattern`类多个选项可以用于控制正则表达式。正则表达式的选项和
模式被设置为字符串，例如：

~~~{.cpp}
    static reflex::Pattern word_pattern("\\w+", "f=graph.gv;f=machine.cpp");
~~~

选项`f=graph.gv`选项发出图形可视化 <i>`.gv`</i> 文件，文件可以用开源的工具
[Graphviz dot tool](http://www.graphviz.org) 可视化显示，以把确定有限状态机
FSM转换为PDF, PNG, 或其他格式：

@dot
digraph FSM {
                size="3,3";
                rankdir=LR;
                concentrate=true;
                node [fontname="ArialNarrow"];
                edge [fontname="Courier"];

                init [root=true,peripheries=0,label="",fontname="Courier"];
                init -> N0x7fff57feb0d0;

/*START*/       N0x7fff57feb0d0 [label=""];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="0-9"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="A-Z"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="_"];
                N0x7fff57feb0d0 -> N0x7fef78c03cb0 [label="a-z"];

/*ACCEPT 1*/    N0x7fef78c03cb0 [label="[1]",peripheries=2];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="0-9"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="A-Z"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="_"];
                N0x7fef78c03cb0 -> N0x7fef78c03cb0 [label="a-z"];
}
@enddot

选项`f=machine.cpp`为有限状态机表发出操作表，这种情况下，它是11个码，见下表：

~~~{.cpp}
    REFLEX_CODE_DECL reflex_code_FSM[11] =
    {
      0x617A0005, // 0: GOTO 5 ON a-z
      0x5F5F0005, // 1: GOTO 5 ON _
      0x415A0005, // 2: GOTO 5 ON A-Z
      0x30390005, // 3: GOTO 5 ON 0-9
      0x00FFFFFF, // 4: HALT
      0xFF000001, // 5: TAKE 1
      0x617A0005, // 6: GOTO 5 ON a-z
      0x5F5F0005, // 7: GOTO 5 ON _
      0x415A0005, // 8: GOTO 5 ON A-Z
      0x30390005, // 9: GOTO 5 ON 0-9
      0x00FFFFFF, // 10: HALT
    };
~~~

选项`o`可以和`f=machine.cpp`一起用，为有限状态机发出优化的本机C++代码：

~~~{.cpp}
    void reflex_code_FSM(reflex::Matcher& m)
    {
      int c0 = 0, c1 = 0;
      m.FSM_INIT(c1);

    S0:
      c1 = m.FSM_CHAR();
      if (97 <= c1 && c1 <= 122) goto S5;
      if (c1 == 95) goto S5;
      if (65 <= c1 && c1 <= 90) goto S5;
      if (48 <= c1 && c1 <= 57) goto S5;
      return m.FSM_HALT(c1);

    S5:
      m.FSM_TAKE(1);
      c1 = m.FSM_CHAR();
      if (97 <= c1 && c1 <= 122) goto S5;
      if (c1 == 95) goto S5;
      if (65 <= c1 && c1 <= 90) goto S5;
      if (48 <= c1 && c1 <= 57) goto S5;
      return m.FSM_HALT(c1);
    }
~~~

紧凑的FSM操作代码表或者优化的较大FSM码可以直接在你的代码中使用。这在运行时，省略
了构造开销。只需将生成的文件包含到你的代码中，然后将其传递给`reflex::Pattern`
构造器：

~~~{.cpp}
    #include <reflex/matcher.h>   // reflex::Matcher, reflex::Pattern, reflex::Input
    #include "machine.cpp" // reflex_code_FSM[]

    // 使用FSM (操作码表或者C++代码)模式以快速搜索
    static reflex::Pattern pattern(reflex_code_FSM);

    // 使用RE/flex POSIX匹配器在句子中搜索单词
    reflex::Matcher matcher(pattern, "How now brown cow.");
    while (matcher.find() != 0)
      std::cout << "Found " << matcher.text() << std::endl;
~~~

RE/flex `reflex::Pattern`构造器选项以字符串形式给出：

  选项          | 作用
  ------------- | -------------------------------------------------------------
  `b`           | 解析括号列表，不转换转义字符
  `e=c;`        | 重定义转义字符
  `f=file.cpp;` | 把有限状态机码保存到`file.cpp`
  `f=file.gv;`  | 把确定有限状态机保存到`file.gv`
  `i`           | 不区分大小写的匹配，与`(?i)X`相同
  `m`           | 多行模式，与`(?m)X`相同
  `n=name;`     | 为机器使用`reflex_code_name`（代替`FSM`）
  `o`           | 只有选项`f`：生成优化的FSM本机C++代码
  `q`           | Flex/Lex-形式引用，"..."与`\Q...\E`相同，与`(?q)X`相同
  `r`           | 抛出正则表达式语法错误异常，否则忽略错误
  `s`           | 点匹配所有内容（即单行模式），与`(?s)X`相同
  `x`           | 带内嵌注释的自由空间模式，与`(?x)X`相同
  `w`           | 显示正则表达式语法错误，在将它们当做异常抛出前

例如：`reflex::Pattern pattern(pattern, "isr")`启用不分大小写的全点匹配模式，
语法错误被抛出为`reflex::Pattern::Error`类型异常。默认情况下，`reflex::Pattern`
构造器只抛出`reflex::regex_error::exceeds_length`和
`reflex::regex_error::exceeds_limits`异常，并且忽略语法错误，见
\ref regex-pattern.

总之：

- RE/flex定义一个可扩展的抽象类接口，它提供正则表达式引擎的标准API。这个API被
用于生成扫描器。API支持UTF-8/16/32编码文件，宽字符串和流数据。

- RE/flex包括一个正则表达式匹配器类和一个正则表达式匹配类，通过确定有限状态
机(FSMs)实现快速匹配。FSM图可以用Graphviz dot工具可视化。更远的，FSM还可以
被导出然后作为源码导入加快模式匹配。

\ref 正则表达式段有更多的关于RE/flex正则表达式库的信息。

🔝 [Back to table of contents](#)




# RE/flex扫描器生成器                                                 {#reflex}

RE/flex扫描器生成器 <b>`reflex`</b> 接受一个词法分析器规则，然后生成一个基于
正则表达式的C++词法分析器类，并保存在 <i>`lex.yy.cpp`</i>代码文件中，也可以
用`-o`命令选项指定保存的代码文件。源文件被编译后，并且用`-lreflex`选项链接。如
果我们使用Boost.Regex作为正则表达式引擎，链接选项为`-lboost_regex`
 (或者 `-lboost_regex-mt`)，如果我们使用PCRE2，链接选项为`-lpcre2-8`。

    reflex lexerspec.l
    c++ lex.yy.cpp -lreflex

使用选项 `−−header-file` 生产头文件 <i>`lex.yy.h`</i>，可以在你的词法分析器程序
中引用：

    reflex −−header-file lexerspec.l
    c++ mylexer.cpp lex.yy.cpp -lreflex

如果没有安装`libreflex`，使用选项`-lreflex`链接会识别。详见\ref link-errors 看
如何解决这个问题。

扫描器可以是一个独立的应用程序，基于 <i>`lex.yy.cpp`</i>，也可以是一个较大
程序的一部分，比如一个编译器。

@dot
digraph build {
  ranksep=.25;
  node     [ shape=box, fontname=Helvetica, fontsize=11 ];
  spec     [ label="lexer specification\n(.l)", peripheries=0 ];
  reflex   [ label="reflex scanner generator" ];
  lexyycpp [ label="lexer class\n(lex.yy.cpp)", peripheries=0 ];
  cpp      [ label="C++ compiler & linker" ];
  scanner  [ label="scanner\n(a.out)", peripheries=0 ];
  libs     [ label="libraries\n(libreflex, libboost_regex, libpcre2-8)", peripheries=0 ];
  incs     [ label="option --header-file\n(lex.yy.h)", peripheries=0 ];
  spec -> reflex -> lexyycpp -> cpp -> scanner;
  incs -> cpp;
  libs -> cpp;
}
@enddot

RE/flex生成的扫描器使用RE/flex正则表达式库API匹配模式。RE/flex正则表达式库API
由抽象类`reflex::AbstractMatcher`定义。

有3个正则表达式匹配引擎可以用于生成扫描器：Boost.Regex库，PCRE2库，以及内
置的RE/flex POSIX匹配器引擎。在任何情况下，都需要链接 `libreflex` 库。
`libboost_regex`库或者`libpcre2-8`库仅需要在使用Boost.Regex或者PCRE2引擎时链接。

 `libreflex`库的输入类`reflex::Input`管理从字符串、宽字符串、流、以及`FILE`文件描述
符来的数据输入。文件数据编码可以是ASCII，二进制或者UTF-8/16/32。UTF-16/32会
自动被解码，并转换为UTF-8编码，以便于基于UTF-8的正则表达式匹配：

@dot
digraph execute {
  ranksep=.25;
  node     [ shape=box, fontname=Helvetica, fontsize=11 ];
  string   [ label="string/buffer input\n(ASCII, binary, UTF-8)", peripheries=0 ];
  wstring  [ label="wide string input\n(UCS-4)", peripheries=0 ];
  file     [ label="FILE input\n(ASCII, binary, UTF-8/16/32)", peripheries=0 ];
  istream  [ label="istream input\n(cin, fstream, etc.)", peripheries=0 ];
  aout     [ label="scanner\n(a.out)" ];
  actions  [ label="actions & tokens", peripheries=0 ];
  string  -> aout -> actions;
  wstring -> aout;
  file    -> aout;
  istream -> aout;
}
@enddot

生成的扫描器执行动作（通常是为解析器生成标记）。这些动作是通过将模式与输入
匹配来触发的。

🔝 [Back to table of contents](#)


## reflex命令行工具                                          {#reflex-command}

<b>`reflex`</b> 命令从标准输入或者词法规则文件接收词法分析器规则脚本（词法规则文件
通常后缀名为<i>`.l`</i>，<i>`.ll`</i>，<i>`.l++`</i>，<i>`.lxx`</i>或者
<i>`.lpp`</i>），生成一个C++扫描器类，并保存到源文件<i>`lex.yy.cpp`</i>中。

输入的源文件<i>`lex.yy.cpp`</i>由多个段组成，清洁、可读并且可重用。

使用选项 `−−header-file` 生产头文件 <i>`lex.yy.h`</i>，可以在你的词法分析器程序
中引用：

    reflex −−header-file lexerspec.l

<b>`reflex`</b>命令中的`−−flex`和`−−bison`选项，分别表示与Flex和Bison/Yacc兼容。
这些选项使得<b>`reflex`</b>可以作为经典Flex和Lex工具的代替品：

    reflex −−flex −−bison lexerspec.l

第一个选项`−−flex`指<i>`lexerspec.l`</i>是一个经典的Flex/Lex规则，带有`yytext`
或者`YYText()`以及常用的"yy"变量和函数。

第二个选项`−−bison`生成一个扫描器类，包含常用的全局"yy"变量和函数，比如`yytext`，
`yyleng`，`yylineno`，和`yylex()`，兼容重定向[Bison](dinosaur.compilertools.net/#bison)
解析器。关于使用Bison重定向、Bison桥接、Bison定位选项的解析器的更多信息见
 \ref reflex-bison。对于Bison 3.0 C++解析器，使用`−−bison-cc`和可选的选项
`−−bison-locations`.

选项`−−yy`同时启用`−−flex`和`−−bison`，通过生成全局变量`yyin`和`yyout`以及全局
`yy`函数最大兼容Lex/Flex。除此之外，`yyin`指向一个`reflex::Input`对象，可以进行
更高级的操作，比传统的`FILE*`类型`yyin`功能更强大。

🔝 [Back to table of contents](#)


## 选项                                                     {#reflex-options}

使用命令行参数选项，可以控制<b>`reflex`</b>扫描器生成器的输出，将在下一小节中
讲述。这些选项也可以在词法分析器规则脚本文件中，用<i>`%%option`</i>控制，或者简写
为 <i>`%%o`</i>，示例如下：

<div class="alt">
~~~{.cpp}
    %option flex
    %option bison
    %option graphs-file=mygraph.gv
~~~
</div>

上述代码与命令行选项 `−−flex`, `−−bison`, `−−graphs-file=mygraph.gv` 等价。

多个选项可以在一行中定义：

<div class="alt">
~~~{.cpp}
    %o flex bison graphs-file=mygraph.gv
~~~
</div>

选项参数名字可以包含短横线(-), 点 (.), 以及双冒号(::)。Flex通常需要把选项放到引号
内，但是RE/flex不需要引号，除非使用的特殊符号，比如：

<div class="alt">
~~~{.cpp}
    %o flex bison graphs-file="dev/output/mygraph.gv"
~~~
</div>

选项参数中的引号 (") 和反斜杠 (\\) 需要用转义字符：

<div class="alt">
~~~{.cpp}
    %o flex bison graphs-file="D:\\dev\\output\\mygraph.gv"
~~~
</div>

缩写模式<i>`%%o`</i>可以省略option，需要每个选项独立写在一行：

<div class="alt">
~~~{.cpp}
    %flex
    %bison
    %graphs-file="dev/output/mygraph.gv"
~~~
</div>

影响正则表达式的选项，比如：<i>`%%option unicode`</i>和
<i>`%%option dotall`</i>应该在任意正则表达式定义和使用前指定。

<b>`reflex`</b>生成的扫描器代码将选项用宏名的形式定义：
`REFLEX_OPTION_name`如果选项启用，则值为`true`或者值为指定的值。这允许你的
程序代码检测和使用RE/flex选项。比如：词法分析器类名字是`REFLEX_OPTION_lexer` ，
当词法分析器类名字在命令行选项`−−lexer=NAME`进行了重定义，或者在词法规则
文件中用<i>`%%option lexer=NAME`</i>进行了定义。

🔝 [Back to table of contents](#)

### 扫描器选项                                    {#reflex-options-scanner}

#### `−+`, `−−flex`

真个选项生成一个与FLex兼容的`yyFlexLexer`扫描器类（假设Flex用C++，并代有`−+`
选项）。生成的`yyFlexLexer`类有通常的 `yytext`和其他"yy"变量及函数，和Flex规则
标准定义的一样。如果没有这个选项，RE/flex的动作需要使用词法分析器类的方法，
比如：`text()`，`echo()`以及词法分析器匹配器的方法，比如`matcher().more()`，更多
信息见\ref reflex-spec-rules。

#### `-a`, `−−dotall`

这个选项使点 (`.`) 可以匹配换行符。通常点匹配除换行符（`\n` ）以外的任意一个字符。

#### `-B`, `−−batch`

这个选项生成一个批处理输入扫描器，它在可能的情况下一次性读入所有输入。扫描
器速度很快，但是根据输入数据的大小，消耗更多的内存。

#### `-f`, `−−full`

（仅RE/flex匹配器）。这个选项把FSM以静态操作码表的形式添加到生成的代码中，
这样生成的扫描器就是完整的。在扫描器初始化时没有FSM构造开销，因此扫描器启动时，立即扫描输入。当使用`−−fast`选项时，-f这个选项无效。

#### `-F`, `−−fast`

（仅RE/flex匹配器）。这个选项把FSM作为优化的本机C++代码添加到生成的代码。
在扫描器初始化时没有FSM构造开销，因此扫描器启动时，立即扫描输入。
生成的代码消耗更多的空间，相比`−−full`选项。
option.

#### `-S`, `−−find`

这个选项生成一个搜索引擎，用以插值模式匹配，以调用与匹配模式相对应的动作。
不匹配的输入将被忽略。相比之下，选项`-s`（或`−−nodefault`）在输入没有匹配时
产生一个错误。

#### `-i`, `−−case-insensitive`

这个选项忽略模式中的大小写。仅仅ASCII范围内的模式匹配大小写。

#### `-I`, `−−interactive`, `−−always-interactive`

此选项生成一个交互式的扫描器，允许控制台输入牺牲一定速度。相比之下，默认的
缓冲区输入策略更加高效。

#### `−−indent` and `−−noindent`

此选项启用或禁用缩进匹配支持，缩进通过锚`\i`, `\j`, 和 `\k`的形式支持。缩进匹配
默认是开启的。可以通过禁用缩进匹配增加匹配速度，但是只有在模式中没有缩进
锚时才能禁用。

#### `-m reflex`, `−−matcher=reflex`

此选项生成一个扫描器，使用带有POSIX匹配器引擎的RE/flex `reflex::Matcher`类。
这是默认的扫描匹配器。这个选项对于Flex兼容性是最好的。这个匹配器支持惰性
匹配\ref reflex-pattern-unicode，\ref reflex-pattern-anchor，
\ref reflex-pattern-dents matching，还支持FSM输出通过Graphviz可视化。

#### `-m boost`, `−−matcher=boost`

此选项生成一个扫描器， 使用带有Boost.Regex POSIX匹配器引擎的 
`reflex::BoostPosixMatcher`类。这个匹配器支持Unicode，单词边界锚，
但是不支持惰性匹配。也不支持Graphviz输出。

#### `-m boost-perl`, `−−matcher=boost-perl`

此选项生成一个扫描器， 使用带有Boost.Regex标准(Perl)匹配器引擎的
 `reflex::BoostPerlMatcher`类。其匹配行为与POSIX *左侧最长匹配规则*不同，
它匹配第一个匹配到的规则，而不是生成最长的匹配。不支持Graphviz输出。

#### `-m pcre2-perl`, `−−matcher=pcre2-perl`

此选项生成一个扫描器， 使用带有PCRE2 (Perl)匹配器引擎的`reflex::PCRE2Matcher`
类。其匹配行为与POSIX *左侧最长匹配规则*不同，它匹配第一个匹配到的规则，
而不是生成最长的匹配。不支持Graphviz输出。

#### `−−pattern=NAME`

此选项为选项' -m '指定的自定义匹配器定义一个自定义模式类' NAME '。

#### `−−include=FILE`

此选项为选项`-m`指定的自定义匹配器定义一个自定义包含文件<i>`FILE.h`</i>。

#### `-T N`, `−−tabs=N`

此选项将默认的tab大小设置为`N`，`N`可以为1, 2, 4, 或者 8. tab 大小在内部被用于
确定\ref reflex-pattern-dents 匹配的列位置，以及由`columno()`, `columno_end()`
返回的列位置，以及由`columns()`返回的类数量。否则它无效。
此选项在运行时为`reflex::Matcher`构造器选项设定`T=N`值。这个值可以在运行时
通过`matcher().tabs(N)`设置，`N`为 1, 2, 4, 或 8。

#### `-u`, `−−unicode`

此选项使得 `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` 匹配Unicode。
并且在正则表达式中对UTF-8序列进行分组，这样正则表达式中的每一个UTF-8编码
字符都被匹配为一个宽字符。

#### `-x`, `−−freespace`

此选项把<b>`reflex`</b>扫描器切换到*自由空间模式*.  自由空间模式的正则表达式
可以包含空格还可以通过缩进提高可读性。正则表达式内、前边、后边的空格都将被
忽略。匹配空格用：`" "` 或者 `[ ]`，使用`\h`匹配一个空格或者水平制表符。
自由空间模式的动作必须放在 <i>`{ }`</i>内用户代码必须放在 <i>`%{ %}`</i> 内。
模式末尾的 `\` 表示在下一行继续。

🔝 [Back to table of contents](#)

### 输出文件选项                                 {#reflex-options-files}

#### `-o FILE`, `−−outfile=FILE`

此选项把扫描器保存到`FILE`，而不是<i>`lex.yy.cpp`</i>.

#### `-t`, `−−stdout`

此选项把扫描器写到标准输出stdout，而不是<i>`lex.yy.cpp`</i>.

#### `−−graphs-file[=FILE]`

（仅RE/flex匹配器）。这个选项生成一个Graphviz文件<i>`FILE.gv`</i>，其中，
`FILE`是可选的。当`FILE`被忽略时，<b>`reflex`</b>命令为每一个在词法分析器规则脚本中
定义的启动条件状态<i>`S`</i>生成一个<i>`reflex.S.gv`</i>文件。包括
为启动条件状态<i>`INITIAL`</i>生成的<i>`reflex.INITIAL.gv`</i>文件。
这个选项可以使用[Graphviz dot](http://www.graphviz.org) 工具显示RE/flex匹配器的
有限状态机。比如：

@dot
digraph INITIAL {
                size="7,7";
                rankdir=LR;
                concentrate=true;
                node [fontname="ArialNarrow"];
                edge [fontname="Courier"];

                init [root=true,peripheries=0,label="INITIAL",fontname="Courier"];
                init -> N0x7fff52a39300;

/*START*/       N0x7fff52a39300 [label=""];
                N0x7fff52a39300 -> N0x7f947940ebf0 [label="\\0-#"];
                N0x7fff52a39300 -> N0x7f947940ecb0 [label="$"];
                N0x7fff52a39300 -> N0x7f947940ebf0 [label="%-\\xff"];

/*ACCEPT 2*/    N0x7f947940ebf0 [label="[2]",peripheries=2];

/*ACCEPT 2*/    N0x7f947940ecb0 [label="[2]",peripheries=2];
                N0x7f947940ecb0 -> N0x7f947940ee30 [label="0-9"];

/*ACCEPT 1*/    N0x7f947940ee30 [label="[1]",peripheries=2];
                N0x7f947940ee30 -> N0x7f947940ef50 [label="."];
                N0x7f947940ee30 -> N0x7f947940ee30 [label="0-9"];

/*STATE*/       N0x7f947940ef50 [label=""];
                N0x7f947940ef50 -> N0x7f947940f070 [label="0-9"];

/*STATE*/       N0x7f947940f070 [label=""];
                N0x7f947940f070 -> N0x7f947940f150 [label="0-9"];

/*ACCEPT 1*/    N0x7f947940f150 [label="[1]",peripheries=2];
}
@enddot

如果你好奇的话：这种FSM图的规则有两种模式：[1]一个匹配美元符号数量的正则表达式
模式`\$\d+(\.\d{2})?`和[2]跳过一个字符然后进入下一个匹配的正则表达式`.|\n`。

#### `−−header-file[=FILE]`

此选项生成一个声明lexer类的C++头文件，除了生成的lexer类代码，`FILE`是可选的。
当`FILE`被忽略时，<b>`reflex`</b>命令生成<i>`lex.yy.h`</i>。

#### `−−regexp-file[=FILE]`

此选项生成一个文本文件<i>`FILE.txt`</i>，文件包含扫描器的正则表达式模式，`FILE`
是可选的。当忽略`FILE`时，<b>`reflex`</b>为每一个条件状态<i>`S`</i>生成
<i>`reflex.S.txt`</i>。词法规则被转换为正则表达式模式，然后转化为合法的C++字符
串。

#### `−−tables-file[=FILE]`

(仅RE/flex匹配器)。此选项生成一个C++文件<i>`FILE.cpp`</i>，它包含一个源代码形式
的有限状态机，`FILE`是可选的。当忽略`FILE`时，<b>`reflex`</b>命令为每一个启动
条件状态<i>`S`</i>生成<i>`reflex.S.cpp`。包括为启动条件状态<i>`INITIAL`</i>生成
的<i>`reflex.INITIAL.cpp`</i>。当与`−−full`或者`−−fast`选项同时使用时，
`reflex::Pattern`用文件中定义的代码表实例化。因此，当我们与`−−full`或`−−fast`
选项同时使用时，需要用扫描程序编译生成的表文件。`−−full`和`−−fast`选项可以在
扫描器初始化时消除FSM构建开销。

🔝 [Back to table of contents](#)

### 输出选项                                           {#reflex-options-code}

#### `−−namespace=NAME`

此选项把生成的类放在C++名称控件`NAME`范围内，生成的类为`NAME::Lexer`（使用
`−−flex`选项时，是`NAME::yyFlexLexer`）。`NAME`可以是一个嵌套的名称
`NAME1::NAME2::NAME3`... 或者使用点`NAME1.NAME2.NAME3` ...

#### `−−lexer=NAME`

此选项定义生成的扫描器类的名字`NAME`，取代默认的名字`Lexer`（当使用选项`−−flex`
时，取代`yyFlexLexer`）。扫描器类成员可以在块<i>`%%class{ }`</i>内声明。扫描器
类构造器代码可以定义在块<i>`%%init{ }`</i> 内。额外的构造参数可以在
<i>`%%option ctorarg="argument, argument, ...`</i>内声明。

#### `−−lex=NAME`

此选项定义生成的扫描器函数名称为`NAME`，取代`lex()` (当使用`−−flex`选项时的
`yylex()`）。

#### `−−params="TYPE NAME, ..."`

此选项为扫描器函数`lex()`定义额外的参数（当使用`−−flex`选项时的`yylex()`）。
函数签名被扩展为包含用逗号分隔的`TYPE NAME`参数。这种机制取代了Flex`YY_DECL`，
详见 \ref YYDECL。

#### `−−class=NAME`

此选项定义继承自生成的基类`Lexer`的用户自定义扫描器类名称为`NAME`。当你定义你
自己的扫描器类名称`NAME`时，使用这个选项。你可以在词法分析器规则的第一段中声明
你的自定义lexer类。因为自定义lexer类是用户定义的，<b>`reflex`</b>为这个类生成
扫描器函数`lex()`的实现。

#### `−−yyclass=NAME`

此选项相当于`−−flex`和`−−class=NAME`。

#### `−−main`

此选项生成一个`main`函数，以生成独立的可以从标准输入（`stdio`）扫描数据的扫描器。

#### `-L`, `−−noline`

此选项在生成的扫描器代码中禁用`#line`指令。

#### `-P NAME`, `−−prefix=NAME`

此选项把`NAME`定义为生成的了`yyFlexLexer`的前缀，替换默认的`yy`前缀。同时重命名
`yylex()`的前缀。生成<i>`lex.NAME.cpp`</i>文件，替代<i>`lex.yy.cpp`</i>，并且
在有`−−header-file`选项时，生成<i>`lex.NAME.h`</i>。

#### `−−nostdinit`

此选项初始化输入为`std::cin`，取代`stdin`，如果没有为扫描器指定输入。
此选项也可以避免在标准输入探查到UTF编码是，扫描器在任何其他输入被指定前自动读
`stdin`。注意，在`std::cin`中不支持UTF自动解码。使用`stdin`在标准输入流中自动
探查和解码UTF BOM和UTF，而不是`std::cin`。

#### `−−bison`

此选项生成一个可以与Bison解析器一起工作的扫描器，通过定义"yy"全局变量和函数
(即 线程不安全和非重入)，比如`yytext`, `yyleng`, `yylineno`, and `yylex()`。
详见 \ref reflex-bison。使用选项`−−noyywrap`移除对全局函数`yywrap()`的依赖。
使用选项`−−bison-locations`支持Bison<i>`%%locations`</i>特性。请参加`−−yy`选项。

#### `−−bison-bridge`

此选项生成一个扫描器，它与纯净的Bison解析器一起工作（可重入线程安全），它为一个
或多个扫描器对象使用Bison桥接。与`−−bison-locations`选项一起使用，支持Bison
<i>`%%locations`</i>特性。详见 \ref reflex-bison-bridge。

#### `−−bison-cc`

此选项生成一个扫描器，它与线程安全的Bison 3.0<i>`%%skeleton "lalr1.cc"`</i>
C++ 解析器一起工作。 与`−−bison-locations`选项一起使用，支持Bison
<i>`%%locations`</i>语法。详见 \ref reflex-bison-cc。

#### `−−bison-cc-namespace=NAME`

此选项为Bison3.0<i>`%%skeleton "lalr1.cc"`</i> C++ 解析器指定一个或多个`NAME`
命名空间，默认为`yy`。

#### `−−bison-cc-parser=NAME`

此选项为Bison3.0<i>`%%skeleton "lalr1.cc"`</i> C++ 解析器指定类`NAME`，默认为
`parser`。

#### `−−bison-complete`

此选项生成一个与Bison 3.2 C++ 完整的符号一起工作的扫描器，在一个Bison语法文件
用<i>`%%define api.value.type variant`</i>和<i>`%%define api.token.constructor`
指定。此选项还为解析器的`symbol_type`设置`−−bison-cc`和`−−token-type`选项，同时
设置`−−token-eof`为`0`（如果这个选项还没有设置的话）。此选项与
`−−bison-locations`一起使用，支持Bison <i>`%%locations`</i>特性。
详见 \ref reflex-bison-complete。

#### `−−bison-locations`

此选项生成一个扫描器，它与启用定位的Bison一起工作。
详见 \ref reflex-bison-locations。

#### `-R`, `−−reentrant`

此选项生成额外的与Flex兼容的`yylex()`可冲入扫描器函数。RE/flex扫描器通常是可
重入的，当使用变量<i>`%%class`</i>代替用户定义的全局变量时。这是一个Flex-兼容
的选项，并且仅可以与`−−flex`和`−−bison`选项一起使用。这个选项启用时，大多数Flex
函数把`yyscan_t`扫描器作为一个额外的最后参数。详见 \ref reflex-reentrant
以及 \ref reflex-bison。

#### `-y`, `−−yy`

此选项启用 `−−flex` 和 `−−bison`以生成一个可以接受Flex词法规则的扫描器，并且
与Bison解析器一起工作，它定义全局(即 非线程安全 和 非可重入) "yy" 变量和函数，
比如`yyin`, `yyout`, `yytext`, `yyleng`, `yylineno`, 和 `yylex()`。

#### `−−yywrap` and `−−noyywrap`

选项 `−−yywrap` 生成一个扫描器，当到达EOF时，它调用全局函数 `int yywrap()`。
此选项仅在`−−flex`和`−−bison`被同时启用，或者`−−yy`启用时，才可用。Wrapping
默认是被启用的。使用`−−noyywrap`禁用对这个全局函数的依赖。此选项对C++词法分析
器类无效，它有一个可能被重载的虚方法`int wrap()` (或者 `yywrap()` ，当使用
选项`−−flex`时)。

#### `−−exception=VALUE`

这个选项定义了当没有匹配输入的规则时，由生成的扫描器的默认规则抛出的异常。
此选项将使用动作`throw VALUE`生成默认值，替换标准默认规则，当没有规则匹配时，
回显所有不匹配的输入文本。当指定选项`-S`(或者`−−find`)时无效。请参加选项`-s`
(或者`−−nodefault`)。应该注意在异常处理程序中显式地处理输入，例如当
`lexer.size()`为0时，调用`lexer.matcher().winput()`。

#### `−−token-type=NAME`

此选项设置`lex()`和`yylex()`返回的标记类型值。`lex()`和`yylex()`返回的值类型默认
是`int`。此选项可以用于指定一个其他的标记类型。选项`−−bison-complete`自动定义
合适的标记类型`symbol_type`，取决于选项`−−bison-cc-namespace`和
`−−bison-cc-parser`指定的参数。

#### `−−token-eof=VALUE`

此选项指定当输入到达结尾或者没有`<<EOF>>`规则时，`lex()`和`yylex()`的返回值。
默认情况下，翻译个默认的构造标记类型值。对于`int`，这是`int()`，它是0。
比如，通过设置`−−token-type=EOF`，返回值`EOF`。

🔝 [Back to table of contents](#)

### 调试选项                                     {#reflex-options-debug}

#### `-d`, `−−debug`

此选项在生成的扫描器中启用调试模式。运行扫描器在标准错误`std::cerr`上产生调试
信息，并且`debug()`函数返回非零值。临时关闭调试信息的方法是：在动作中使用
`set_debug(0)` 。重新打开调试信息，使用`set_debug(1)`。`set_debug()`和`debug()`
方法是lexer类的虚方法，所以可以在一个继承的lexer类中重载它们的行为。此选项
还启用断言，用于内部错误检查。见 \ref reflex-debug。

#### `-p`, `−−perf-report`

此选项为生成的扫描器启用统计信息的收集和报告功能。当到达EOF时，扫描器在
`std::cerr`上报告统计性能。如果扫描器未到达EOF，那么在你的代码中可以显式的
调用lexer的`perf_report()`方法。调用这个方法还会重置统计信息和计时器，这意味
着这个方法将从上一次调用开始报告搜集的信息。详见 \ref reflex-debug。

#### `-s`, `−−nodefault`

此选项禁止默认的"当没有规则匹配时显示所有的未匹配输入文本"。使用`−−flex`选项，
当没有规则匹配时，通过调用`yyFlexLexer::LexerError("scanner jammed")`报告
"扫描器堵塞"。没有`−−flex`和`−−debug`选项时，通过调用
`AbstractLexer::lexer_error("scanner jammed")`，抛出`std::runtime`异常。
如果要抛出自定义异常，使用`−−exception`选项，或者在继承的lexer类中重载虚方法
`lexer_error`。虚方法`LexerError`和`lexer_error`可以通过在继承的lexer类中重定义，
详见 \ref reflex-inherit。没有`−−flex`选项，但是有`−−debug`选项时，默认的规则是
调用`lexer_error`提出异常。见选项`−−exception=VALUE`和`-S` (or `−−find`)。

#### `-v`, `−−verbose`

显示扫描器统计信息的摘要。

#### `-w`, `−−nowarn`

禁止警告。

🔝 [Back to table of contents](#)

### 其他选项                         {#reflex-options-miscellaneous}

#### `-h`, `−−help`

显示<b>`reflex`</b>的帮助信息。

#### `-V`, `−−version`

显示<b>`reflex`</b>的当前发布版本。

#### `−−yylineno`, `−−yymore`

此选项默认开启，但是没有影响。

🔝 [Back to table of contents](#)


## 词法分析器规则脚本                                             {#reflex-spec}

词法分析器规则脚本包括三段，它由<i>`%%`</i>分隔符分开，分隔符需要放在独立
的一行。

<div class="alt">
~~~{.cpp}
    Definitions
    %%
    Rules
    %%
    User code
~~~
</div>

\ref reflex-spec-defs 用于定义命名的正则表达式，为扫描器设置选项，以及和包含
C++声明。

\ref reflex-spec-rules 是扫描器的主体，由模式和动作组成，模式可以用在
\ref reflex-spec-defs 中定义的命名正则表达式。当模式匹配时，动作会被执行。
例如：下面的词法分析器规则脚本把输入中所有的`cow`替换为`chick`：

<div class="alt">
~~~{.cpp}
    %%
    cow      out() << "chick";
    %%
~~~
</div>

默认的规则是：输入流中遇到 \ref reflex-spec-rules 内没有定义对应规则匹配的文本
时，显示文本。所以这个简单的扫描器示例会如实的复制所有其他的文本。

因为模式`cow`还可以匹配单词的部分内容，`cows`将会获得`chicks`。
但是我们也会为一些单词获得混乱的糟糕输出，比如`coward`，并且我们
会跳过首字母大写的Cows。我们可以通过字编辑标记以及大小写匹配改进这个
模式：

<div class="alt">
~~~{.cpp}
    cow      \<[Cc]ow\>
    %%
    {cow}    out() << text()[0] << "hick";
    %%
~~~
</div>

注意，我们定义了一个命名的模式`cow`，通过正则表达式`\<[Cc]ow\>`匹配
"cow"或者"Cow"，以及它们的起始边界。我们在匹配规则中使用通过`{cow}`
使用命名正则表达式。匹配到文本后，取第一个字母`text()[0]`，然后我们直接
添加`"hick"`完成单词chick。

注意，正则表达式中用括号括起来的组，也就是子正则表达式匹配，在扫描器
生成器中不支持，所以我们只能用整个字符串匹配`text()`。

Flex和Lex不支持单词编辑标记 `\<`, `\>`, `\b`, 和 `\B`，所以这个示例仅在RE/flex
中可以使用。

如果你对我们示例中“动作代码与Flex期望的C代码不同”有疑问，那么请放心，
RE/flex支持经典的flex和Lex动作，比如' yytext '而不是' text() '和' *yyout '而不是
' out() '。很简单，使用`−−flex`选项，可以返回到C-样式的 Flex 名字和动作。
使用`−−flex`和`−−bison`选项 (或者`−−yy`)，进一步回归生成一个全局函数
 `yylex()` 和全局 "yy" 变量。

创建独立的扫描器应用，我们可以在用户代码段添加 `main`函数：

<div class="alt">
~~~{.cpp}
    cow      \<[Cc]ow\>
    %%
    {cow}    out() << text()[0] << "hick";
    %%
    int main() { return Lexer().lex(); }
~~~
</div>

main函数实例化lexer类，然后调用扫描器，它将会在整个输入处理完之后返回。
事实上，可以通过 `−−main`选项让<b>`reflex`</b>生成main函数。

接下来将介绍这三个词法分析器脚本段的更多细节。  

🔝 [Back to table of contents](#)

### 定义段                                      {#reflex-spec-defs}

定义段包括名字-模式对，定义模式的名字。在正则表达式模式中可以可以使用大括号引用
引用命名模式，形式为：<i>`{命名模式名称}`</i>。

下述示例定义了两个模式的名字，其中，第二个正则表达式使用了前一个命名正则表达式：

<div class="alt">
~~~{.cpp}
    digit     [0-9]
    number    {digit}+
~~~
</div>

模式使用反斜杠 `\` 结束时，表示在下一行连续，下一行可以选择用空格缩进。
这允许你为一个长模式进行排版布局。请参考 \ref reflex-pattern-freespace
改进模式的可读性。 

名字在使用之前必须先定义。名字在正则表达式模式中被展开为宏。例如： `{digit}+`
被展开为`[0-9]+`。

@note 这是不完全正确的：当一个名称被展开到它的右手边模式`φ`时，展开模式`φ被
放置在一个非捕获组`(?:φ)`中，以保持其结构。例如，`{number}`扩大到`(?:{digit}+)`
进而扩大到`(?:(?:[0-9])+)`。

要把代码注入到生成的扫描器中，缩进代码或者把代码放到<i>`%{ %}`</i>块中。
<i>`%{`</i>和匹配的<i>`%}`</i>需要放在新行的开始。
要把代码注入到生成的扫描器的最前端，需要把代码放到<i>`%%top{ }`</i>块中：

<div class="alt">
~~~{.cpp}
    %top{
      #include <iostream>    // std::cout etc.
    }
~~~
</div>

\ref reflex-spec-defs 还可以通过<i>`%%option`</i> (或者简写 <i>`%%o`</i>)
包含一个或者多个选项。例如：

<div class="alt">
~~~{.cpp}
    %option dotall main
    %o matcher=boost
~~~
</div>

多个选项可以组合在同一行中，如上述示例所示。可用选项列表见 \ref reflex-options 。

@note 选项 `freespace`, `case-insensitive`, `dotall`, 和 `unicode` 会影响在
\ref reflex-spec-defs 中定义的名字。因此，我们需要把这些选项放在所有命名模式
定义之前。如果正则表达式模式特别需要这些选项中的一个或多个，使用`(?isux:φ)`
修改器，详见\ref reflex-patterns。

考虑下述示例。假设我们想计算某些文本中出现"cow"的次数。我们生命一个全局的计数器，
当我们遇到"cow"时，增加计数器，最后，当我们到达`<<EOF>>`规则标记的输入结束时，
报告总数。

<div class="alt">
~~~{.cpp}
    %option dotall main

    %top{
      #include <iostream>    // std::cout etc.
    }

    %{
      static int herd = 0;   // a global static variable to count cows
    %}

    cow        \<[Cc]ow\>

    %%

    {cow}      herd++;       // found a cow, bump count by one
    .          // do nothing
    <<EOF>>    out() << herd << " cows!" << std::endl; return 0;

    %%
~~~
</div>

上面的方法可以很好地工作，但是使用全局计数器不是最佳实践，也不是线程安全的:多个
Lexer类实例可能会相互碰撞计数器。另一个问题是Lexer只能被使用一次，在新的输入上
重新启动Lexer不能够正确的初始化。

RE/flex允许您在生成的Lexer类中注入代码，这意味着可以添加类成员和构造函数代码
来管理Lexer类的状态。所有的Lexer类成员在动作中都是可见的，即使是私有的。给定
一些要扫描的输入，可以实例化新的词法分析器。Lexers可以并行在多个线程中运行，
而不需要同步，当他们的状态是实例的一部分而不是由全局变量管理时。 

要注入Lexer类成员声明，如变量和方法，请将声明放在<i>`%%class{ }`</i>块中。
<i>`%%class{`</i>和对应的<i>`}`</i>每一个都应该放在新行的开始。  

同样，要注入Lexer类构造函数代码(例如初始化成员)，需要将代码放入
<i>`%%init{ }`</i> 块中。<i>`%%init{`</i>和对应的<i>`}`</i>每一个都应该放在
新行的开始。
选项<i>`%%option ctorarg="argument, argument, ..."`</i>可用于声明Lexer类构造
函数的构造函数参数。  

对于类`Lexer`(或`yyFlexLexer`)，除非类被选项`−−Lexer =NAME`
(<i>`%%option lexer=NAME`</i>)重命名，额外的构造函数和/或析构函数可以放在
<i>`%%class{ }`</i> 块中。

方便起见，你可以在你的代码中使用生成的`REFLEX_OPTION_lexer`宏，它被展开为类名。
使用<b>`reflex`</b>的选项`−- header-file`生成一个头文件，可以包括到您的代码。 

例如，我们使用这些代码注入器使我们的计数器“羊群”成为Lexer类状态的一部分:

<div class="alt">
~~~{.cpp}
    %option dotall main

    %top{
      #include <iostream>    // std::cout etc.
    }

    %class{
      int herd;  // lexer class member variable (private by default)
    }

    %init{
      herd = 0;  // initialize member variable in Lexer class constructor
    }

    cow        \<[Cc]ow\>

    %%

    {cow}      herd++;       // found a cow, bump count by one
    .          // do nothing
    <<EOF>>    out() << herd << " cows!" << std::endl; return 0;

    %%
~~~
</div>

注意，不需要更改其他内容，因为动作是生成的Lexer类的一部分，可以访问Lexer类成员，
在本例中成员变量`herd`。  

模块化lexer的规则脚本，使用<i>`%%include`</i> (or <i>`%%i`</i>)可以包含一个或
多个文件到一个规则脚本的 \ref reflex-spec-defs 。比如:

<div class="alt">
~~~{.cpp}
    %include "examples/jdefs.l"
~~~
</div>

它包含使用java模式的 <i>`examples/jdefs.l`</i> 到当前脚本文件，因此你可以匹配
jvava词法结构，比如将某些java源代码作为输入，输出Java标识符：

<div class="alt">
~~~{.cpp}
    %include "examples/jdefs.l"
    %%
    {Identifier}    echo();
    .|\n            // do nothing
    %%
~~~
</div>

可以使用<i>`%%include`</i>指定多个文件。Quotes may be
<i>`%%include`</i>参数可以省略引号，如果参数除了`.`和`-`没有其他标点字符。
例如<i>`%%include jdefs.l`</i>.

声明启动条件状态名，使用<i>`%%state`</i> (or <i>`%%s`</i>)声明包容性状态。
使用<i>`%%xstate`</i> (or <i>`%%x`</i>) 声明排斥性状态:

<div class="alt">
~~~{.cpp}
    %s INCLUSIVE
    %x EXCLUSIVE
~~~
</div>

更多关于状态的信息见 \ref reflex-states

🔝 [Back to table of contents](#)

### 规则段                                        {#reflex-spec-rules}

规则段中的每一个规则都由一个模式-动作对构成，模式-动作对由空格分开（除非启用了
自由空间模式）。例如，下述代码定义了一个模式对应的动作：

<div class="alt">
~~~{.cpp}
    [0-9]+      out() << "number " << text() << std::endl;
~~~
</div>

若要添加跨多行的动作代码，请缩进代码或把代码放到<i>`{ }`</i>块中。如果动作中
声明了局部变量，代码需要放在块中。

在自由空间模式，你必须把动作放到<i>`{ }`</i>块中，并且用户代码放到<i>`%{ %}`</i>
块中，代替缩进，见\ref reflex-pattern-freespace 。

规则段中的动作可以使用预定义的RE/flex变量和函数。<b>`reflex`</b>使用选项
`−−flex`时，变量和函数是典型的Flex动作，显示在该表的第二列:  

  RE/flex action           | Flex action          | Result
  ------------------------ | -------------------- | -------------------------------
  `text()`                 | `YYText()`, `yytext` | 0-terminated text match
  `str()`                  | *n/a*                | `std::string` text match
  `wstr()`                 | *n/a*                | `std::wstring` wide text match
  `chr()`                  | `yytext[0]`          | first 8-bit char of text match
  `wchr()`                 | *n/a*                | first wide char of text match
  `size()`                 | `YYLeng()`, `yyleng` | size of the match in bytes
  `wsize()`                | *n/a*                | number of wide chars matched
  `lines()`                | *n/a*                | number of lines matched (>=1)
  `columns()`              | *n/a*                | number of columns matched (>=0)
  `lineno(n)`              | `yylineno = n`       | set line number of the match to `n`
  `lineno()`               | `yylineno`           | line number of the match (>=1)
  `columno()`              | *n/a*                | column number of match (>=0)
  `lineno_end()`           | *n/a*                | ending line number of match (>=1)
  `columno_end()`          | *n/a*                | ending column number of match (>=0)
  `border()`               | *n/a*                | border of the match (>=0)
  `echo()`                 | `ECHO`               | `out().write(text(), size())`
  `in(i)`                  | `yyrestart(i)`       | set input to `reflex::Input i`
  `in()`, `in() = i`       | `*yyin`, `yyin = &i` | get/set `reflex::Input i`
  `out(o)`                 | `yyout = &o`         | set output to `std::ostream o`
  `out()`                  | `*yyout`             | get `std::ostream` object
  `out().write(s, n)`      | `LexerOutput(s, n)`  | output chars `s[0..n-1]`
  `out().put(c)`           | `output(c)`          | output char `c`
  `start(n)`               | `BEGIN n`            | set start condition to `n`
  `start()`                | `YY_START`           | get current start condition
  `push_state(n)`          | `yy_push_state(n)`   | push current state, start `n`
  `pop_state()`            | `yy_pop_state()`     | pop state and make it current
  `top_state()`            | `yy_top_state()`     | get top state start condition
  `states_empty()`         | *n/a*                | true if state stack is empty
  `matcher().accept()`     | `yy_act`             | number of the matched rule
  `matcher().text()`       | `YYText()`, `yytext` | same as `text()`
  `matcher().str()`        | *n/a*                | same as `str()`
  `matcher().wstr()`       | *n/a*                | same as `wstr()`
  `matcher().chr()`        | `yytext[0]`          | same as `chr()`
  `matcher().wchr()`       | *n/a*                | same as `wchr()`
  `matcher().size()`       | `YYLeng()`, `yyleng` | same as `size()`
  `matcher().wsize()`      | *n/a*                | same as `wsize()`
  `matcher().lines()`      | *n/a*                | same as `lines()`
  `matcher().columns()`    | *n/a*                | same as `columns()`
  `matcher().lineno(n)`    | `yylineno = n`       | same as `lineno(n)`
  `matcher().lineno()`     | `yylineno`           | same as `lineno()`
  `matcher().columno()`    | *n/a*                | same as `columno()`
  `matcher().lineno_end()` | `yylineno`           | same as `lineno_end()`
  `matcher().columno_end()`| *n/a*                | same as `columno_end()`
  `matcher().border()`     | *n/a*                | same as `border()`
  `matcher().begin()`      | *n/a*                | non-0-terminated text match begin
  `matcher().end()`        | *n/a*                | non-0-terminated text match end
  `matcher().input()`      | `yyinput()`          | get next 8-bit char from input
  `matcher().winput()`     | *n/a*                | get wide character from input
  `matcher().unput(c)`     | `unput(c)`           | put back 8-bit char `c`
  `matcher().wunput(c)`    | `unput(c)`           | put back (wide) char `c`
  `matcher().peek()`       | *n/a*                | peek at next 8-bit char on input
  `matcher().skip(c)`      | *n/a*                | skip input to char `c`
  `matcher().skip(s)`      | *n/a*                | skip input to UTF-8 string `s`
  `matcher().more()`       | `yymore()`           | append next match to this match
  `matcher().less(n)`      | `yyless(n)`          | shrink match length to `n`
  `matcher().first()`      | *n/a*                | first pos of match in input
  `matcher().last()`       | *n/a*                | last pos+1 of match in input
  `matcher().rest()`       | *n/a*                | get rest of input until end
  `matcher().span()`       | *n/a*                | enlarge match to span line
  `matcher().line()`       | *n/a*                | get line with the match
  `matcher().wline()`      | *n/a*                | get line with the match
  `matcher().at_bob()`     | *n/a*                | true if at the begin of input
  `matcher().at_end()`     | *n/a*                | true if at the end of input
  `matcher().at_bol()`     | `YY_AT_BOL()`        | true if at begin of a newline
  `set_debug(n)`           | `set_debug(n)`       | reflex option `-d` sets `n=1`
  `debug()`                | `debug()`            | nonzero when debugging

输入源`reflex::Input`，在表中用`i`表示，可以是一个`FILE*`描述符，`std::istream`，
一个字符串`std::string` 或者 `const char*`，或者一个宽字符串 `std::wstring` 或
`const wchar_t*`。输出 `o` 是一个 `std::ostream` 对象。

注意Flex中的 `switch_streams(i, o)` 与调用 `in(i)` 和 `out(o)` 方法相同。Flex `yyrestart(i)`与调用`in(i)`设置输入相同。调用`switch_streams(i, o)`和`in(i)`
还重置lexer的匹配器(内部方法为`matcher.reset()`)。
它清除行和列计数器，重置内部用于边界匹配的锚和标记，重置匹配器以使用缓冲输入。

还可以用`in() = i` (或者 `yyin = &i` 当使用选项`−−flex`时) 设置输入。然而，这并
不重置匹配器。这意味着，当到达输入的结尾  (EOF) 时，你应该先用
`matcher().set_end(false)` 清除EOF状态，或者用`matcher().reset()`清除匹配器
状态。重置匹配器状态还会刷新缓冲区中剩余的输入，否则这些输入仍然会被
使用。因此，使用 `in(i)` (或者 `yyrestart(i)` ，当使用选项`−−flex`时) 是更可取的。

方法 `matcher().input()`, `matcher().winput()`, 和 `matcher().peek()` 返回一个非
负的字符码，当输入到尾部时。这些方法保留当前的 `text()` 匹配 (当使用选项
`−−flex`时，为 `yytext`)，但是`text()` (和 `yytext`) 返回的指针在这些方法被调用
后可能会改变。无论如何，`yytext`指针不会被保留，当<b>`reflex`</b>使用
 选项 `−−flex` 和 `−−bison`时。

@warning Flex兼容的函数 `yyinput()` 返回 0，当到达输入的结尾时，它这使得
`\0` (NUL)和EOF不可能被区分。相比而言，`matcher().input()`返回EOF (-1) ，
当到达输入的结尾时。

@warning 不要在 `lex()`（or `yylex()` ，当使用选项`−−flex`时) 之前调用 
`matcher()` !  当lexer构造时，并未为其指定匹配器，这会导致`matcher()`
未定义。

方法`matcher().skip(c)`跳过输入，直到遇到`c`（字符`char`或者宽字符`wchar_t`）
并且返回`true`。这个方法会改变`text()` (`yytext`，当使用选项`−−flex`时)。这个
方法比重复调用`matcher().input()`更有效。同样的， `matcher().skip(s)`跳过输入，
直到遇到 UTF-8 字符串 `s`，并且返回`true`。

<b>`reflex`</b>使用选项 `−−flex` 和 `−−bison` (或者选项 `−−yy`) 开启全局
Flex 函数和变量。这将是Flex函数和变量可以在 \ref reflex-spec-rules 外部全局
可访问，除了`yy_push_state()`, `yy_pop_state()`, `yy_top_state()`。
\ref reflex-spec-rules 外部你必须使用全局函数 `yyinput()` 而不是`input()`，全局
函数 `yyunput()` 而不是 `unput()`，全局函数`yyoutput()` 而不是 `output()`。
因为 `yyin` 和 `yyout` 是宏，它们不能作为全局变量访问或声明，但它们可以当作
变量来使用。为避免编译错误，, 使用<b>`reflex`</b> 的选项 `−−header-file`
生成一个头文件 <i>`lex.yy.h`</i>，在你的代码中包含这个头文件可以使用全局
Flex函数和变量。更多的关于  `−−bison` 选项的使用信息，见 \ref reflex-bison。

当使用 <b>`reflex`</b> 的选项 `−−flex`, `−−bison` 和 `−−reentrant` 时，大多数
Flex 函数使用一个`yyscan_t`扫描器作为额外的最后一个参数。详见
\ref reflex-reentrant 。

从上面显示的表中的前两个条目中，您可能已经猜到了`text()`只是`matcher().text()`
的简写，`matcher()`是与生成的Lexer类关联的匹配器对象。同样的简写也适用于
`str()`, `wstr()`, `size()`, `wsize()`, `lineno()`, `columno()`, 和 `border()`。使用`text()` 
可以快速的访问匹配文本。方法`str()`返回匹配的字符串的拷贝，并且效率较低。
同样的，`wstr()`返回匹配的宽字符串的拷贝，宽字符串从UTF-8转换而来。

方法`lineno()`返回匹配的行号，从1开始。最后一行的行号是 `lineno_end()`，它的
值与`lineno()` + `lines()` - 1相同。

方法 `columno()` 返回当从行开头的列偏移，从0开始。这个方法考虑制表符间距
和宽字符。最后一列列号是`columno_end()`。

`lines()` 和 `columns()` 方法返回匹配的行和列的总数，其中`columns()`考虑制表符
间距和宽字符。如果匹配跨越多行，`columns()`统计所有行的列，但不统计换行符。

一行中的匹配的起始字节数是 `border()`，包含匹配结束的字节偏移量为
`border() + size() - 1`。

@note 宽字符被计算为1，因此`columno()`，`columno_end()`和`columns()`
不考虑全宽字符的字符宽度，也不考虑Unicode字符的组合。推荐使用
`wcwidth`函数或者[wcwidth.c](https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c)
确定Unicode字符宽度。

The `matcher().more()` method is used to create longer matches by stringing
together consecutive matches in the input after scanning the input with the
`scan()` method.  When this method is invoked, the next match with `scan()` has
its matched text prepended to it.  The `matcher().more()` operation is often
used in lexers and was introduced in Lex.

The `matcher().less(n)` method reduces the size of the matched text to `n`
bytes.  This method has no effect if `n` is larger than `size()`.  The value of
`n` should not be `0` to prevent infinite looping on the same input as no input
is consumed (or you could switch to another start condition state with
`start(n)` in the action that uses `less(0)`).  The `matcher().less(n)`
operation was introduced in Lex and is often used in lexers to place input back
into the input stream and as a means to perform sophisticated lookaheads.

The `matcher().first()` and `matcher().last()` methods return the position in
the input stream of the match, counting in bytes from the start of the input at
position 0.  If the input stream is a wide character sequence, the UTF-8
positions are returned as a result of the internally-converted UTF-8 wide
character input.

The `matcher().rest()` method returns the rest of the input character sequence
as a 0-terminated `char*` string.  This method buffers all remaining input to
return the string.

The `matcher().span()` method enlarges the text matched to span the entire line
and returns the matching line as a 0-terminated `char*` string without the `\n`.

The `matcher().line()` and `matcher().wline()` methods return the entire line
as a (wide) string with the matched text as a substring.  These methods can be
used to obtain the context of a match, for example to display the line where a
lexical error or syntax error occurred.

@warning The methods `matcher().span()`, `matcher().line()`, and
`matcher().wline()` invalidate the previous `text()`, `yytext`, `begin()`, and
`end()` string pointers.  Call these methods again to retrieve the updated
pointer or call `str()` or `wstr()` to obtain a string copy of the match:
~~~{.cpp}
    // INCORRECT, because t is invalid after line():
    const char *t = matcher().text();
    std::string s = matcher().line();
    std::cout << t << " in " << s << std::endl;
    // OK with line():
    std::string s = matcher().line();
    const char *t = matcher().text();
    std::cout << t << " in " << s << std::endl;
    // OK with span():
    std::string t = matcher().str();
    const char *s = matcher().span();
    std::cout << t << " in " << s << std::endl;
~~~
The start of a line is truncated when the line is too long.  The length of the
line's contents before the pattern match on the line is restricted to 8KB,
which is the size specified by `reflex::AbstractMatcher::Const::BLOCK`.  When
this length is exceeded, the line's length before the match is truncated to
8KB.  This ensures that pattern matching binary files or files with very long
lines cannot cause memory allocation exceptions.

Because `matcher()` returns the current matcher object, the following Flex-like
actions are also supported:

  RE/flex action            | Flex action             | Result
  ------------------------- | ----------------------- | -----------------------
  `matcher().buffer()`      | *n/a*                   | buffer entire input
  `matcher().buffer(n)`     | *n/a*                   | set buffer size to `n`
  `matcher().interactive()` | `yy_set_interactive(1)` | set interactive input
  `matcher().flush()`       | `YY_FLUSH_BUFFER`       | flush input buffer
  `matcher().get(s, n)`     | `LexerInput(s, n)`      | read `s[0..n-1]`
  `matcher().set_bol(b)`    | `yy_set_bol(b)`         | (re)set begin of line
  `matcher().set_bob(b)`    | *n/a*                   | (re)set begin of input
  `matcher().set_end(b)`    | *n/a*                   | (re)set end of input
  `matcher().reset()   `    | *n/a*                   | reset the state as new

You can switch to a new matcher while scanning input, and use operations to
create a new matcher, push/pop a matcher on/from a stack, and delete a matcher:

  RE/flex action    | Flex action              | Result
  ----------------- | ------------------------ | ------------------------------
  `matcher(m)`      | `yy_switch_to_buffer(m)` | use matcher `m`
  `new_matcher(i)`  | `yy_create_buffer(i, n)` | returns new matcher for `reflex::Input i`
  `del_matcher(m)`  | `yy_delete_buffer(m)`    | delete matcher `m`
  `push_matcher(m)` | `yypush_buffer_state(m)` | push current matcher, then use `m`
  `pop_matcher()`   | `yypop_buffer_state()`   | pop matcher and delete current
  `ptr_matcher()`   | `YY_CURRENT_BUFFER`      | pointer to current matcher
  `has_matcher()`   | `YY_CURRENT_BUFFER != 0` | current matcher is usable

The matcher type `m` is a Lexer class-specific `Matcher` type, which depends on
the underlying matcher used by the scanner.  Therefore, `new_matcher(i)`
instantiates a `reflex::Matcher` or the matcher specified with the `−−matcher`
option.

The `push_matcher()` and `pop_matcher()` functions can be used to temporarily
switch to another input source while preserving the original input source
associated with the matcher on the stack with `push_matcher()`.  The
`pop_matcher()` action returns `true` when successful and `false` otherwise,
when the stack is empty.  When `false`, `has_matcher()` returns `false` and
`ptr_matcher()` returns `NULL`.  See also \ref reflex-multiple-input.

The following Flex actions are also supported with <b>`reflex`</b> option
`−−flex`:

  RE/flex action   | Flex action              | Result
  ---------------- | ------------------------ | -------------------------
  `in(s)`          | `yy_scan_string(s)`      | reset and scan string `s` (`std::string` or `char*`)
  `in(s)`          | `yy_scan_wstring(s)`     | reset and scan wide string `s` (`std::wstring` or `wchar_t*`)
  `in(b, n)`       | `yy_scan_bytes(b, n)`    | reset and scan `n` bytes at address `b` (buffered)
  `buffer(b, n+1)` | `yy_scan_buffer(b, n+2)` | reset and scan `n` bytes at address `b` (zero copy)

These functions create a new buffer (i.e. a new matcher in RE/flex) to
incrementally buffer the input on demand, except for `yy_scan_buffer` that
scans a string in place (i.e. zero copy) that should end with two zero bytes,
which are included in the specified length.  A pointer to the new buffer is
returned, which becomes the `YY_CURRENT_BUFFER`.  You should delete the old
buffer with `yy_delete_buffer(YY_CURRENT_BUFFER)` before creating a new buffer
with one of these functions.  See \ref reflex-input for more details.

The generated scanner reads from the standard input by default or from an input
source specified as a `reflex::Input` object, such as a string, wide string,
file, or a stream.  See \ref reflex-input for more details on managing the
input to a scanner.

These functions take an extra last `yyscan_t` argument for reentrant scanners
generated with option `−−reentrant`.  This argument is a pointer to a lexer
object.  See \ref reflex-reentrant for more details.

🔝 [Back to table of contents](#)

### User code sections                                      {#reflex-spec-user}

To inject code at the end of the generated scanner, such as a `main`
function, we can use the third and final User code section.  All of the code in
the User code section is copied to the generated scanner.

Below is a User code section example with `main` that invokes the lexer to read
from standard input (the default input) and display all numbers found:

<div class="alt">
~~~{.cpp}
    %top{
      #include <iostream>
    }

    digit       [0-9]
    number      {digit}+

    %%

    {number}    out() << "number " << text() << std::endl;

    %%

    int main() { return Lexer().lex(); }
~~~
</div>

You can also automatically generate a `main` with the <b>`reflex`</b> `−−main`
option, which will produce the same `main` function shown in the example above.
This creates a stand-alone scanner that instantiates a Lexer that reads input
from standard input.

To scan from other input than standard input, such as from files, streams, and
strings, instantiate the Lexer class with the input source as the first
argument.  To set an alternative output stream than standard output, pass a
`std::ostream` object as the second argument to the Lexer class constructor:

<div class="alt">
~~~{.cpp}
    int main(int argc, char **argv)
    {
      FILE *fd = stdin;
      if (argc > 1 && (fd = fopen(argv[1], "r")) == NULL)
        exit(EXIT_FAILURE);
      std::ofstream of("output.txt", std::ofstream::out);
      if (!of)
        exit(EXIT_FAILURE);
      Lexer(fd, of).lex();
      of.close();
      if (fd != stdin)
        fclose(fd);
      return 0;
    }
~~~
</div>

The above uses a `FILE` descriptor to read input from, which has the advantage
of automatically decoding UTF-8/16/32 input.  Other permissible input sources
are `std::istream`, `std::string`, `std::wstring`, `char*`, and `wchar_t*`.

🔝 [Back to table of contents](#)


Patterns                                                     {#reflex-patterns}
--------

The regex pattern syntax you can use generally depends on the regex matcher
library that you use.  Fortunately, RE/flex accept a broad pattern syntax for
lexer specifications.  The <b>`reflex`</b> command internally converts the
regex patterns to regex forms that the underlying matcher engine library can
handle (except when specifically indicated in the tables that follow).  This
ensures that the same pattern syntax can be used with any matcher engine
library that RE/flex currently supports.

🔝 [Back to table of contents](#)

### Pattern syntax                                     {#reflex-pattern-syntax}

A pattern is an extended set of regular expressions, with nested sub-expression
patterns `φ` and `ψ`:

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `x`       | matches the character `x`, where `x` is not a special character
  `.`       | matches any single character except newline (unless in dotall mode)
  `\.`      | matches `.` (dot), special characters are escaped with a backslash
  `\n`      | matches a newline, others are `\a` (BEL), `\b` (BS), `\t` (HT), `\v` (VT), `\f` (FF), and `\r` (CR)
  `\N`      | matches any single character except newline
  `\0`      | matches the NUL character
  `\cX`     | matches the control character `X` mod 32 (e.g. `\cA` is `\x01`)
  `\0141`   | matches an 8-bit character with octal value `141` (use `\141` in lexer specifications instead, see below)
  `\x7f`    | matches an 8-bit character with hexadecimal value `7f`
  `\x{3B1}` | matches Unicode character U+03B1, i.e. `α`
  `\u{3B1}` | matches Unicode character U+03B1, i.e. `α`
  `\o{141}` | matches U+0061, i.e. `a`, in octal
  `\p{C}`   | matches a character in category C of \ref reflex-pattern-cat
  `\Q...\E` | matches the quoted content between `\Q` and `\E` literally
  `[abc]`   | matches one of `a`, `b`, or `c` as \ref reflex-pattern-class
  `[0-9]`   | matches a digit `0` to `9` as \ref reflex-pattern-class
  `[^0-9]`  | matches any character except a digit as \ref reflex-pattern-class
  `φ?`      | matches `φ` zero or one time (optional)
  `φ*`      | matches `φ` zero or more times (repetition)
  `φ+`      | matches `φ` one or more times (repetition)
  `φ{2,5}`  | matches `φ` two to five times (repetition)
  `φ{2,}`   | matches `φ` at least two times (repetition)
  `φ{2}`    | matches `φ` exactly two times (repetition)
  `φ??`     | matches `φ` zero or once as needed (lazy optional)
  `φ*?`     | matches `φ` a minimum number of times as needed (lazy repetition)
  `φ+?`     | matches `φ` a minimum number of times at least once as needed (lazy repetition)
  `φ{2,5}?` | matches `φ` two to five times as needed (lazy repetition)
  `φ{2,}?`  | matches `φ` at least two times or more as needed (lazy repetition)
  `φψ`      | matches `φ` then matches `ψ` (concatenation)
  `φ⎮ψ`     | matches `φ` or matches `ψ` (alternation)
  `(φ)`     | matches `φ` as a group to capture (this is non-capturing in lexer specifications)
  `(?:φ)`   | matches `φ` without group capture
  `(?=φ)`   | matches `φ` without consuming it (\ref reflex-pattern-lookahead)
  `(?<=φ)`  | matches `φ` to the left without consuming it (\ref reflex-pattern-lookbehind, not supported by the RE/flex matcher)
  `(?^φ)`   | matches `φ` and ignores it, marking everything as a non-match to continue matching (RE/flex matcher only)
  `^φ`      | matches `φ` at the begin of input or begin of a line (requires multi-line mode) (top-level `φ`, not nested in a sub-pattern)
  `φ$`      | matches `φ` at the end of input or end of a line (requires multi-line mode) (top-level `φ`, not nested in a sub-pattern)
  `\Aφ`     | matches `φ` at the begin of input (top-level `φ`, not nested in a sub-pattern)
  `φ\z`     | matches `φ` at the end of input (top-level `φ`, not nested in a sub-pattern)
  `\bφ`     | matches `φ` starting at a word boundary (top-level `φ`, not nested in a sub-pattern)
  `φ\b`     | matches `φ` ending at a word boundary (top-level `φ`, not nested in a sub-pattern)
  `\Bφ`     | matches `φ` starting at a non-word boundary (top-level `φ`, not nested in a sub-pattern)
  `φ\B`     | matches `φ` ending at a non-word boundary (top-level `φ`, not nested in a sub-pattern)
  `\<φ`     | matches `φ` that starts a word (top-level `φ`, not nested in a sub-pattern)
  `\>φ`     | matches `φ` that starts a non-word (top-level `φ`, not nested in a sub-pattern)
  `φ\<`     | matches `φ` that ends a non-word (top-level `φ`, not nested in a sub-pattern)
  `φ\>`     | matches `φ` that ends a word (top-level `φ`, not nested in a sub-pattern)
  `\i`      | matches an indent for \ref reflex-pattern-dents matching
  `\j`      | matches a dedent for \ref reflex-pattern-dents matching
  `\k`      | matches if indent depth changed, undoing this change to keep the current indent stops for \ref reflex-pattern-dents matching
  `(?i:φ)`  | \ref reflex-pattern-anycase matches `φ` ignoring case
  `(?m:φ)`  | \ref reflex-pattern-multiline `^` and `$` in `φ` match begin and end of a line (default in lexer specifications)
  `(?s:φ)`  | \ref reflex-pattern-dotall `.` (dot) in `φ` matches newline
  `(?u:φ)`  | \ref reflex-pattern-unicode `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode
  `(?x:φ)`  | \ref reflex-pattern-freespace ignore all whitespace and comments in `φ`
  `(?#:X)`  | all of `X` is skipped as a comment

@note The lazy quantifier `?` for optional patterns `φ??` and repetitions `φ*?`
`φ+?` is not supported by Boost.Regex in POSIX mode.  In general, POSIX
matchers do not support lazy quantifiers due to POSIX limitations that are
rooted in the theory of formal languages FSM of regular expressions.

The following patterns are available in RE/flex and adopt the same Flex/Lex
patterns syntax.  These pattern should only be used in lexer specifications:

  Pattern            | Matches
  ------------------ | --------------------------------------------------------
  `\177`             | matches an 8-bit character with octal value `177`
  `"..."`            | matches the quoted content literally
  `φ/ψ`              | matches `φ` if followed by `ψ` as a \ref reflex-pattern-trailing
  `<S>φ`             | matches `φ` only if state `S` is enabled in \ref reflex-states
  `<S1,S2,S3>φ`      | matches `φ` only if state `S1`, `S2`, or state `S3` is enabled in \ref reflex-states
  `<*>φ`             | matches `φ` in any state of the \ref reflex-states
  `<<EOF>>`          | matches EOF in any state of the \ref reflex-states
  `<S><<EOF>>`       | matches EOF only if state `S` is enabled in \ref reflex-states
  `[a-z￨￨[A-Z]]`     | matches a letter, see \ref reflex-pattern-class
  `[a-z&&[^aeiou]]`  | matches a consonant, see \ref reflex-pattern-class
  `[a-z−−[aeiou]]`   | matches a consonant, see \ref reflex-pattern-class
  `[a-z]{+}[A-Z]`    | matches a letter, same as `[a-z￨￨[A-Z]]`, see \ref reflex-pattern-class
  `[a-z]{￨}[A-Z]`    | matches a letter, same as `[a-z￨￨[A-Z]]`, see \ref reflex-pattern-class
  `[a-z]{&}[^aeiou]` | matches a consonant, same as `[a-z&&[^aeiou]]`, see \ref reflex-pattern-class
  `[a-z]{-}[aeiou]`  | matches a consonant, same as `[a-z−−[aeiou]]`, see \ref reflex-pattern-class

Note that the characters `.` (dot), `\`, `?`, `*`, `+`, `|`, `(`, `)`, `[`,
`]`, `{`, `}`, `^`, and `$` are meta-characters and should be escaped to match.
Lexer specifications also include the `"` and `/` as meta-characters and these
should be escaped to match.

Spaces and tabs cannot be matched in patterns in lexer specifications.  To
match the space character use `" "` or `[ ]` and to match the tab character use
`\t`.  Use `\h` to match a space or tab.

The order of precedence for composing larger patterns from sub-patterns is as
follows, from high to low precedence:

1. Characters, character classes (bracket expressions), escapes, quotation
2. Grouping `(φ)`, `(?:φ)`, `(?=φ)`, and inline modifiers `(?imsux-imsux:φ)`
3. Quantifiers `?`, `*`, `+`, `{n,m}`
4. Concatenation `φψ` (including trailing context `φ/ψ`)
5. Anchoring `^`, `$`, `\<`, `\>`, `\b`, `\B`, `\A`, `\z` 
6. Alternation `φ|ψ`
7. Global modifiers `(?imsux-imsux)φ`

@note When using regex patterns in C++ literal strings, make sure that "regex
escapes are escaped", meaning that an extra backslash is needed for every
backslash in the regex string.

@note Trigraphs in C/C++ strings are special tripple-character sequences,
beginning with two question marks and followed by a character that is
translated.  Avoid `??` in regex strings.  Instead, use at least one escaped
question mark, such as `?\?`, which the compiler will translate to `??`.  This
problem does not apply to lexer specifications that the <b>`reflex`</b> command
converts to regex strings.  Fortunately, most C++ compilers ignore trigraphs
unless in standard-conforming modes, such as `-ansi` and `-std=c++98`.

🔝 [Back to table of contents](#)

### Character classes                                   {#reflex-pattern-class}

Character classes in bracket lists represent sets of characters.  Sets can be
negated (or inverted), subtracted, intersected, and merged (except for the
`PCRE2Matcher`):

  Pattern           | Matches
  ----------------- | ---------------------------------------------------------
  `[a-zA-Z]`        | matches a letter
  `[^a-zA-Z]`       | matches a non-letter (character class negation)
  `[a-z￨￨[A-Z]]`    | matches a letter (character class union)
  `[a-z&&[^aeiou]]` | matches a consonant (character class intersection)
  `[a-z−−[aeiou]]`  | matches a consonant (character class subtraction)

Bracket lists cannot be empty, so `[]` and `[^]` are invalid.  In fact, the
first character after the bracket is always part of the list.  So `[][]` is a
list that matches a `]` and a `[`, `[^][]` is a list that matches anything but
`]` and `[`, and `[-^]` is a list that matches a `-` and a `^`.

It is an error to construct an empty character class by subtraction or by
intersection, for example `[a&&[b]]` is invalid.

Bracket lists may contain ASCII and Unicode \ref reflex-pattern-cat, for
example `[a-z\d]` contains the letters `a` to `z` and digits `0` to `9` (or
Unicode digits when Unicode is enabled).  To add Unicode character categories
and wide characters (encoded in UTF-8) to bracket lists
\ref reflex-pattern-unicode should be enabled.

An negated Unicode character class is constructed by subtracting the character
class from the Unicode range U+0000 to U+D7FF and U+E000 to U+10FFFF.

Character class operations can be chained together in a bracket list.  The
union `||`, intersection `&&`, and subtraction `--` operations are left
associative and have the same operator precedence.  For example,
`[a-z||[A-Z]--[aeiou]--[AEIOU]]`, `[a-z--[aeiou]||[A-Z]--[AEIUO]]`,
`[a-z&&[^aeiou]||[A-Z]&&[^AEIOU]]`, and `[B-DF-HJ-NP-TV-Zb-df-hj-np-tv-z]` are
the same character classes.

Character class operations may be nested.  For example, `[a-z||[A-Z||[0-9]]]`
is the same as `[a-zA-Z0-9]`.

Character class negation, when specified, is applied to the resulting character
class after the character class operations are applied.  For example,
`[^a-z||[A-Z]]` is the same as `[^||[a-z]||[A-Z]]`, which is the class
`[^a-zA-Z]`.

Note that negated character classes such as `[^a-zA-Z]` match newlines when
`\n` is not included in the class.  Include `\n` in the negated character class
to prevent matching newlines.  The `reflex::convert_flag::notnewline` removes
newlines from negated character classes when used with \ref regex-convert.

A lexer specification may use a defined name in place of the second operand of
a character class operation.  A defined name when used as an operand should
expand into a POSIX character class containing ASCII characters only.  For
example:

<div class="alt">
~~~{.cpp}
    lower     [a-z]
    upper     [A-Z]
    letter    [||{lower}||{upper}]
    alnum     [0-9||{letter}]
    name      {letter}{alnum}*
    %%
    {name}    std::cout << "name: " << text() << std::endl;
    .|\n      // skip everything else
    %%
~~~
</div>

@warning Defined names may only occur immediately after a `||`, `&&`, and a
`--` operator in a bracket list.  Do not place a defined name as the first
operand to a union, intersection, and subtraction operation, because the
definition is not expanded.  For example, `[{lower}||{upper}]` contains
`[A-Zelorw{}]`.  The name and the `{`, `}` characters are literally included in
the resulting character class.  Instead, this bracket list should be written as
`[||{lower}||{upper}]`.  Likewise, `[^{lower}||{upper}]` should be written as
`[^||{lower}||{upper}]`.

Alternatively, unions may be written as alternations.  That is,
`[||{name1}||{name2}||{name3}||...]` can be written as
`({name1}|{name2}|{name3}|...)`, where the latter form supports full Unicode
not restricted to ASCII.

The character class operators `{+}` (or `{|}`), `{&}`, and `{-}` may be used in
lexer specifications.  Note that Flex only supports the two operators `{+}` and
`{-}`:

  Pattern            | Matches
  ------------------ | --------------------------------------------------------
  `[a-z]{+}[A-Z]`    | matches a letter, same as `[a-z￨￨[A-Z]]`
  `[a-z]{￨}[A-Z]`    | matches a letter, same as `[a-z￨￨[A-Z]]`
  `[a-z]{&}[^aeiou]` | matches a consonant, same as `[a-z&&[^aeiou]]`
  `[a-z]{-}[aeiou]`  | matches a consonant, same as `[a-z−−[aeiou]]`

Multiple operators can be chained together.  Unlike Flex, defined names may be
used as operands.  For example `{lower}{+}{upper}` is the same as
`[a-z]{+}[A-Z]`, i.e. the character class `[A-Za-z]`.  A defined name when used
as an operand should expand into a POSIX character class containing ASCII
characters only.

🔝 [Back to table of contents](#)

### Character categories                                  {#reflex-pattern-cat}

The 7-bit ASCII POSIX character categories are:

  POSIX form   | POSIX category    | Matches
  ------------ | ----------------- | ---------------------------------------------
  `[:ascii:]`  | `\p{ASCII}`       | matches any ASCII character
  `[:space:]`  | `\p{Space}`       | matches a white space character `[ \t\n\v\f\r]`
  `[:xdigit:]` | `\p{Xdigit}`      | matches a hex digit `[0-9A-Fa-f]`
  `[:cntrl:]`  | `\p{Cntrl}`       | matches a control character `[\x00-\0x1f\x7f]`
  `[:print:]`  | `\p{Print}`       | matches a printable character `[\x20-\x7e]`
  `[:alnum:]`  | `\p{Alnum}`       | matches a alphanumeric character `[0-9A-Za-z]`
  `[:alpha:]`  | `\p{Alpha}`       | matches a letter `[A-Za-z]`
  `[:blank:]`  | `\p{Blank}`, `\h` | matches a blank `[ \t]`
  `[:digit:]`  | `\p{Digit}`       | matches a digit `[0-9]`
  `[:graph:]`  | `\p{Graph}`       | matches a visible character `[\x21-\x7e]`
  `[:lower:]`  | `\p{Lower}`       | matches a lower case letter `[a-z]`
  `[:punct:]`  | `\p{Punct}`       | matches a punctuation character `[\x21-\x2f\x3a-\x40\x5b-\x60\x7b-\x7e]`
  `[:upper:]`  | `\p{Upper}`       | matches an upper case letter `[A-Z]`
  `[:word:]`   | `\p{Word}`        | matches a word character `[0-9A-Za-z_]`
  `[:^blank:]` | `\P{Blank}`, `\H` | matches a non-blank character `[^ \t]`
  `[:^digit:]` | `\P{Digit}`       | matches a non-digit `[^0-9]`

The POSIX forms are used in bracket lists.  For example `[[:lower:][:digit:]]`
matches an ASCII lower case letter or a digit.  

You can also use the upper case `\P{C}` form that has the same meaning as
`\p{^C}`, which matches any character except characters in the class `C`.
For example, `\P{ASCII}` is the same as `\p{^ASCII}` which is the same as
`[^[:ascii:]]`.

When Unicode matching mode is enabled, `[^[:ascii]]` is a Unicode character
class that excludes the ASCII character category.  Unicode character classes
and categories require the <b>`reflex`</b> `−−unicode` option.

The following Unicode character categories are enabled with the <b>`reflex`</b>
`−−unicode` option or \ref reflex-pattern-unicode `(?u:φ)` and with the regex
matcher converter flag `reflex::convert_flag::unicode` when using a regex
library:

  Unicode category                       | Matches
  -------------------------------------- | ------------------------------------
  `.`                                    | matches any single Unicode character except newline (including \ref invalid-utf)
  `\a`                                   | matches BEL U+0007
  `\d`                                   | matches a digit `\p{Nd}`
  `\D`                                   | matches a non-digit
  `\e`                                   | matches ESC U+001b
  `\f`                                   | matches FF U+000c
  `\l`                                   | matches a lower case letter `\p{Ll}`
  `\n`                                   | matches LF U+000a
  `\N`                                   | matches any non-LF character
  `\r`                                   | matches CR U+000d
  `\R`                                   | matches a Unicode line break
  `\s`                                   | matches a white space character `[ \t\v\f\r\x85\p{Z}]` excluding `\n`
  `\S`                                   | matches a non-white space character
  `\t`                                   | matches TAB U+0009
  `\u`                                   | matches an upper case letter `\p{Lu}`
  `\v`                                   | matches VT U+000b
  `\w`                                   | matches a Unicode word character `[\p{L}\p{Nd}\p{Pc}]`
  `\W`                                   | matches a non-Unicode word character
  `\X`                                   | matches any ISO-8859-1 or Unicode character
  `\p{Space}`                            | matches a white space character `[ \t\n\v\f\r\x85\p{Z}]` including `\n`
  `\p{Unicode}`                          | matches any Unicode character U+0000 to U+10FFFF minus U+D800 to U+DFFF
  `\p{ASCII}`                            | matches an ASCII character U+0000 to U+007F
  `\p{ASCII}`                            | matches an ASCII character U+0000 to U+007F)
  `\p{Non_ASCII_Unicode}`                | matches a non-ASCII character U+0080 to U+10FFFF minus U+D800 to U+DFFF)
  `\p{L&}`                               | matches a character with Unicode property L& (i.e. property Ll, Lu, or Lt)
  `\p{Letter}`,`\p{L}`                   | matches a character with Unicode property Letter
  `\p{Mark}`,`\p{M}`                     | matches a character with Unicode property Mark
  `\p{Separator}`,`\p{Z}`                | matches a character with Unicode property Separator
  `\p{Symbol}`,`\p{S}`                   | matches a character with Unicode property Symbol
  `\p{Number}`,`\p{N}`                   | matches a character with Unicode property Number
  `\p{Punctuation}`,`\p{P}`              | matches a character with Unicode property Punctuation
  `\p{Other}`,`\p{C}`                    | matches a character with Unicode property Other
  `\p{Lowercase_Letter}`, `\p{Ll}`       | matches a character with Unicode sub-property Ll
  `\p{Uppercase_Letter}`, `\p{Lu}`       | matches a character with Unicode sub-property Lu
  `\p{Titlecase_Letter}`, `\p{Lt}`       | matches a character with Unicode sub-property Lt
  `\p{Modifier_Letter}`, `\p{Lm}`        | matches a character with Unicode sub-property Lm
  `\p{Other_Letter}`, `\p{Lo}`           | matches a character with Unicode sub-property Lo
  `\p{Non_Spacing_Mark}`, `\p{Mn}`       | matches a character with Unicode sub-property Mn
  `\p{Spacing_Combining_Mark}`, `\p{Mc}` | matches a character with Unicode sub-property Mc
  `\p{Enclosing_Mark}`, `\p{Me}`         | matches a character with Unicode sub-property Me
  `\p{Space_Separator}`, `\p{Zs}`        | matches a character with Unicode sub-property Zs
  `\p{Line_Separator}`, `\p{Zl}`         | matches a character with Unicode sub-property Zl
  `\p{Paragraph_Separator}`, `\p{Zp}`    | matches a character with Unicode sub-property Zp
  `\p{Math_Symbol}`, `\p{Sm}`            | matches a character with Unicode sub-property Sm
  `\p{Currency_Symbol}`, `\p{Sc}`        | matches a character with Unicode sub-property Sc
  `\p{Modifier_Symbol}`, `\p{Sk}`        | matches a character with Unicode sub-property Sk
  `\p{Other_Symbol}`, `\p{So}`           | matches a character with Unicode sub-property So
  `\p{Decimal_Digit_Number}`, `\p{Nd}`   | matches a character with Unicode sub-property Nd
  `\p{Letter_Number}`, `\p{Nl}`          | matches a character with Unicode sub-property Nl
  `\p{Other_Number}`, `\p{No}`           | matches a character with Unicode sub-property No
  `\p{Dash_Punctuation}`, `\p{Pd}`       | matches a character with Unicode sub-property Pd
  `\p{Open_Punctuation}`, `\p{Ps}`       | matches a character with Unicode sub-property Ps
  `\p{Close_Punctuation}`, `\p{Pe}`      | matches a character with Unicode sub-property Pe
  `\p{Initial_Punctuation}`, `\p{Pi}`    | matches a character with Unicode sub-property Pi
  `\p{Final_Punctuation}`, `\p{Pf}`      | matches a character with Unicode sub-property Pf
  `\p{Connector_Punctuation}`, `\p{Pc}`  | matches a character with Unicode sub-property Pc
  `\p{Other_Punctuation}`, `\p{Po}`      | matches a character with Unicode sub-property Po
  `\p{Control}`, `\p{Cc}`                | matches a character with Unicode sub-property Cc
  `\p{Format}`, `\p{Cf}`                 | matches a character with Unicode sub-property Cf
  `\p{UnicodeIdentifierStart}`           | matches a character in the Unicode IdentifierStart class
  `\p{UnicodeIdentifierPart}`            | matches a character in the Unicode IdentifierPart class
  `\p{IdentifierIgnorable}`              | matches a character in the IdentifierIgnorable class
  `\p{JavaIdentifierStart}`              | matches a character in the Java IdentifierStart class
  `\p{JavaIdentifierPart}`               | matches a character in the Java IdentifierPart class
  `\p{CsIdentifierStart}`                | matches a character in the C# IdentifierStart class
  `\p{CsIdentifierPart}`                 | matches a character in the C# IdentifierPart class
  `\p{PythonIdentifierStart}`            | matches a character in the Python IdentifierStart class
  `\p{PythonIdentifierPart}`             | matches a character in the Python IdentifierPart class

To specify a Unicode block as a category when using the `−−unicode` option, use
`\p{IsBlockName}`.  The table below lists the block categories up to U+FFFF,
However, all Unicode blocks up to U+10FFFF are available (not listed):

  IsBlockName                                        | Unicode character range
  -------------------------------------------------- | -----------------------
  `\p{IsBasicLatin}`                                 | U+0000 to U+007F 
  `\p{IsLatin-1Supplement}`                          | U+0080 to U+00FF 
  `\p{IsLatinExtended-A}`                            | U+0100 to U+017F 
  `\p{IsLatinExtended-B}`                            | U+0180 to U+024F 
  `\p{IsIPAExtensions}`                              | U+0250 to U+02AF 
  `\p{IsSpacingModifierLetters}`                     | U+02B0 to U+02FF 
  `\p{IsCombiningDiacriticalMarks}`                  | U+0300 to U+036F 
  `\p{IsGreekandCoptic}`                             | U+0370 to U+03FF 
  `\p{IsCyrillic}`                                   | U+0400 to U+04FF 
  `\p{IsCyrillicSupplement}`                         | U+0500 to U+052F 
  `\p{IsArmenian}`                                   | U+0530 to U+058F 
  `\p{IsHebrew}`                                     | U+0590 to U+05FF 
  `\p{IsArabic}`                                     | U+0600 to U+06FF 
  `\p{IsSyriac}`                                     | U+0700 to U+074F 
  `\p{IsArabicSupplement}`                           | U+0750 to U+077F 
  `\p{IsThaana}`                                     | U+0780 to U+07BF 
  `\p{IsNKo}`                                        | U+07C0 to U+07FF 
  `\p{IsSamaritan}`                                  | U+0800 to U+083F 
  `\p{IsMandaic}`                                    | U+0840 to U+085F 
  `\p{IsSyriacSupplement}`                           | U+0860 to U+086F 
  `\p{IsArabicExtended-A}`                           | U+08A0 to U+08FF 
  `\p{IsDevanagari}`                                 | U+0900 to U+097F 
  `\p{IsBengali}`                                    | U+0980 to U+09FF 
  `\p{IsGurmukhi}`                                   | U+0A00 to U+0A7F 
  `\p{IsGujarati}`                                   | U+0A80 to U+0AFF 
  `\p{IsOriya}`                                      | U+0B00 to U+0B7F 
  `\p{IsTamil}`                                      | U+0B80 to U+0BFF 
  `\p{IsTelugu}`                                     | U+0C00 to U+0C7F 
  `\p{IsKannada}`                                    | U+0C80 to U+0CFF 
  `\p{IsMalayalam}`                                  | U+0D00 to U+0D7F 
  `\p{IsSinhala}`                                    | U+0D80 to U+0DFF 
  `\p{IsThai}`                                       | U+0E00 to U+0E7F 
  `\p{IsLao}`                                        | U+0E80 to U+0EFF 
  `\p{IsTibetan}`                                    | U+0F00 to U+0FFF 
  `\p{IsMyanmar}`                                    | U+1000 to U+109F 
  `\p{IsGeorgian}`                                   | U+10A0 to U+10FF 
  `\p{IsHangulJamo}`                                 | U+1100 to U+11FF 
  `\p{IsEthiopic}`                                   | U+1200 to U+137F 
  `\p{IsEthiopicSupplement}`                         | U+1380 to U+139F 
  `\p{IsCherokee}`                                   | U+13A0 to U+13FF 
  `\p{IsUnifiedCanadianAboriginalSyllabics}`         | U+1400 to U+167F 
  `\p{IsOgham}`                                      | U+1680 to U+169F 
  `\p{IsRunic}`                                      | U+16A0 to U+16FF 
  `\p{IsTagalog}`                                    | U+1700 to U+171F 
  `\p{IsHanunoo}`                                    | U+1720 to U+173F 
  `\p{IsBuhid}`                                      | U+1740 to U+175F 
  `\p{IsTagbanwa}`                                   | U+1760 to U+177F 
  `\p{IsKhmer}`                                      | U+1780 to U+17FF 
  `\p{IsMongolian}`                                  | U+1800 to U+18AF 
  `\p{IsUnifiedCanadianAboriginalSyllabicsExtended}` | U+18B0 to U+18FF 
  `\p{IsLimbu}`                                      | U+1900 to U+194F 
  `\p{IsTaiLe}`                                      | U+1950 to U+197F 
  `\p{IsNewTaiLue}`                                  | U+1980 to U+19DF 
  `\p{IsKhmerSymbols}`                               | U+19E0 to U+19FF 
  `\p{IsBuginese}`                                   | U+1A00 to U+1A1F 
  `\p{IsTaiTham}`                                    | U+1A20 to U+1AAF 
  `\p{IsCombiningDiacriticalMarksExtended}`          | U+1AB0 to U+1AFF 
  `\p{IsBalinese}`                                   | U+1B00 to U+1B7F 
  `\p{IsSundanese}`                                  | U+1B80 to U+1BBF 
  `\p{IsBatak}`                                      | U+1BC0 to U+1BFF 
  `\p{IsLepcha}`                                     | U+1C00 to U+1C4F 
  `\p{IsOlChiki}`                                    | U+1C50 to U+1C7F 
  `\p{IsCyrillicExtended-C}`                         | U+1C80 to U+1C8F 
  `\p{IsGeorgianExtended}`                           | U+1C90 to U+1CBF 
  `\p{IsSundaneseSupplement}`                        | U+1CC0 to U+1CCF 
  `\p{IsVedicExtensions}`                            | U+1CD0 to U+1CFF 
  `\p{IsPhoneticExtensions}`                         | U+1D00 to U+1D7F 
  `\p{IsPhoneticExtensionsSupplement}`               | U+1D80 to U+1DBF 
  `\p{IsCombiningDiacriticalMarksSupplement}`        | U+1DC0 to U+1DFF 
  `\p{IsLatinExtendedAdditional}`                    | U+1E00 to U+1EFF 
  `\p{IsGreekExtended}`                              | U+1F00 to U+1FFF 
  `\p{IsGeneralPunctuation}`                         | U+2000 to U+206F 
  `\p{IsSuperscriptsandSubscripts}`                  | U+2070 to U+209F 
  `\p{IsCurrencySymbols}`                            | U+20A0 to U+20CF 
  `\p{IsCombiningDiacriticalMarksforSymbols}`        | U+20D0 to U+20FF 
  `\p{IsLetterlikeSymbols}`                          | U+2100 to U+214F 
  `\p{IsNumberForms}`                                | U+2150 to U+218F 
  `\p{IsArrows}`                                     | U+2190 to U+21FF 
  `\p{IsMathematicalOperators}`                      | U+2200 to U+22FF 
  `\p{IsMiscellaneousTechnical}`                     | U+2300 to U+23FF 
  `\p{IsControlPictures}`                            | U+2400 to U+243F 
  `\p{IsOpticalCharacterRecognition}`                | U+2440 to U+245F 
  `\p{IsEnclosedAlphanumerics}`                      | U+2460 to U+24FF 
  `\p{IsBoxDrawing}`                                 | U+2500 to U+257F 
  `\p{IsBlockElements}`                              | U+2580 to U+259F 
  `\p{IsGeometricShapes}`                            | U+25A0 to U+25FF 
  `\p{IsMiscellaneousSymbols}`                       | U+2600 to U+26FF 
  `\p{IsDingbats}`                                   | U+2700 to U+27BF 
  `\p{IsMiscellaneousMathematicalSymbols-A}`         | U+27C0 to U+27EF 
  `\p{IsSupplementalArrows-A}`                       | U+27F0 to U+27FF 
  `\p{IsBraillePatterns}`                            | U+2800 to U+28FF 
  `\p{IsSupplementalArrows-B}`                       | U+2900 to U+297F 
  `\p{IsMiscellaneousMathematicalSymbols-B}`         | U+2980 to U+29FF 
  `\p{IsSupplementalMathematicalOperators}`          | U+2A00 to U+2AFF 
  `\p{IsMiscellaneousSymbolsandArrows}`              | U+2B00 to U+2BFF 
  `\p{IsGlagolitic}`                                 | U+2C00 to U+2C5F 
  `\p{IsLatinExtended-C}`                            | U+2C60 to U+2C7F 
  `\p{IsCoptic}`                                     | U+2C80 to U+2CFF 
  `\p{IsGeorgianSupplement}`                         | U+2D00 to U+2D2F 
  `\p{IsTifinagh}`                                   | U+2D30 to U+2D7F 
  `\p{IsEthiopicExtended}`                           | U+2D80 to U+2DDF 
  `\p{IsCyrillicExtended-A}`                         | U+2DE0 to U+2DFF 
  `\p{IsSupplementalPunctuation}`                    | U+2E00 to U+2E7F 
  `\p{IsCJKRadicalsSupplement}`                      | U+2E80 to U+2EFF 
  `\p{IsKangxiRadicals}`                             | U+2F00 to U+2FDF 
  `\p{IsIdeographicDescriptionCharacters}`           | U+2FF0 to U+2FFF 
  `\p{IsCJKSymbolsandPunctuation}`                   | U+3000 to U+303F 
  `\p{IsHiragana}`                                   | U+3040 to U+309F 
  `\p{IsKatakana}`                                   | U+30A0 to U+30FF 
  `\p{IsBopomofo}`                                   | U+3100 to U+312F 
  `\p{IsHangulCompatibilityJamo}`                    | U+3130 to U+318F 
  `\p{IsKanbun}`                                     | U+3190 to U+319F 
  `\p{IsBopomofoExtended}`                           | U+31A0 to U+31BF 
  `\p{IsCJKStrokes}`                                 | U+31C0 to U+31EF 
  `\p{IsKatakanaPhoneticExtensions}`                 | U+31F0 to U+31FF 
  `\p{IsEnclosedCJKLettersandMonths}`                | U+3200 to U+32FF 
  `\p{IsCJKCompatibility}`                           | U+3300 to U+33FF 
  `\p{IsCJKUnifiedIdeographsExtensionA}`             | U+3400 to U+4DBF 
  `\p{IsYijingHexagramSymbols}`                      | U+4DC0 to U+4DFF 
  `\p{IsCJKUnifiedIdeographs}`                       | U+4E00 to U+9FFF 
  `\p{IsYiSyllables}`                                | U+A000 to U+A48F 
  `\p{IsYiRadicals}`                                 | U+A490 to U+A4CF 
  `\p{IsLisu}`                                       | U+A4D0 to U+A4FF 
  `\p{IsVai}`                                        | U+A500 to U+A63F 
  `\p{IsCyrillicExtended-B}`                         | U+A640 to U+A69F 
  `\p{IsBamum}`                                      | U+A6A0 to U+A6FF 
  `\p{IsModifierToneLetters}`                        | U+A700 to U+A71F 
  `\p{IsLatinExtended-D}`                            | U+A720 to U+A7FF 
  `\p{IsSylotiNagri}`                                | U+A800 to U+A82F 
  `\p{IsCommonIndicNumberForms}`                     | U+A830 to U+A83F 
  `\p{IsPhags-pa}`                                   | U+A840 to U+A87F 
  `\p{IsSaurashtra}`                                 | U+A880 to U+A8DF 
  `\p{IsDevanagariExtended}`                         | U+A8E0 to U+A8FF 
  `\p{IsKayahLi}`                                    | U+A900 to U+A92F 
  `\p{IsRejang}`                                     | U+A930 to U+A95F 
  `\p{IsHangulJamoExtended-A}`                       | U+A960 to U+A97F 
  `\p{IsJavanese}`                                   | U+A980 to U+A9DF 
  `\p{IsMyanmarExtended-B}`                          | U+A9E0 to U+A9FF 
  `\p{IsCham}`                                       | U+AA00 to U+AA5F 
  `\p{IsMyanmarExtended-A}`                          | U+AA60 to U+AA7F 
  `\p{IsTaiViet}`                                    | U+AA80 to U+AADF 
  `\p{IsMeeteiMayekExtensions}`                      | U+AAE0 to U+AAFF 
  `\p{IsEthiopicExtended-A}`                         | U+AB00 to U+AB2F 
  `\p{IsLatinExtended-E}`                            | U+AB30 to U+AB6F 
  `\p{IsCherokeeSupplement}`                         | U+AB70 to U+ABBF 
  `\p{IsMeeteiMayek}`                                | U+ABC0 to U+ABFF 
  `\p{IsHangulSyllables}`                            | U+AC00 to U+D7AF 
  `\p{IsHangulJamoExtended-B}`                       | U+D7B0 to U+D7FF 
  `\p{IsHighSurrogates}`                             | U+D800 to U+DB7F 
  `\p{IsHighPrivateUseSurrogates}`                   | U+DB80 to U+DBFF 
  `\p{IsLowSurrogates}`                              | U+DC00 to U+DFFF 
  `\p{IsPrivateUseArea}`                             | U+E000 to U+F8FF 
  `\p{IsCJKCompatibilityIdeographs}`                 | U+F900 to U+FAFF 
  `\p{IsAlphabeticPresentationForms}`                | U+FB00 to U+FB4F 
  `\p{IsArabicPresentationForms-A}`                  | U+FB50 to U+FDFF 
  `\p{IsVariationSelectors}`                         | U+FE00 to U+FE0F 
  `\p{IsVerticalForms}`                              | U+FE10 to U+FE1F 
  `\p{IsCombiningHalfMarks}`                         | U+FE20 to U+FE2F 
  `\p{IsCJKCompatibilityForms}`                      | U+FE30 to U+FE4F 
  `\p{IsSmallFormVariants}`                          | U+FE50 to U+FE6F 
  `\p{IsArabicPresentationForms-B}`                  | U+FE70 to U+FEFF 
  `\p{IsHalfwidthandFullwidthForms}`                 | U+FF00 to U+FFEF 
  `\p{IsSpecials}`                                   | U+FFF0 to U+FFFF 

In addition, the `−−unicode` option enables standard Unicode language scripts:

  `\p{Adlam}`, `\p{Ahom}`, `\p{Anatolian_Hieroglyphs}`, `\p{Arabic}`,
  `\p{Armenian}`, `\p{Avestan}`, `\p{Balinese}`, `\p{Bamum}`, `\p{Bassa_Vah}`,
  `\p{Batak}`, `\p{Bengali}`, `\p{Bhaiksuki}`, `\p{Bopomofo}`, `\p{Brahmi}`,
  `\p{Braille}`, `\p{Buginese}`, `\p{Buhid}`, `\p{Canadian_Aboriginal}`,
  `\p{Carian}`, `\p{Caucasian_Albanian}`, `\p{Chakma}`, `\p{Cham}`,
  `\p{Cherokee}`, `\p{Chorasmian}`, `\p{Common}`, `\p{Coptic}`,
  `\p{Cuneiform}`, `\p{Cypriot}`, `\p{Cyrillic}`, `\p{Deseret}`,
  `\p{Devanagari}`, `\p{Dives_Akuru}`, `\p{Dogra}`, `\p{Duployan}`,
  `\p{Egyptian_Hieroglyphs}`, `\p{Elbasan}`, `\p{Elymaic}`, `\p{Ethiopic}`,
  `\p{Georgian}`, `\p{Glagolitic}`, `\p{Gothic}`, `\p{Grantha}`, `\p{Greek}`,
  `\p{Gujarati}`, `\p{Gunjala_Gondi}`, `\p{Gurmukhi}`, `\p{Han}`, `\p{Hangul}`,
  `\p{Hanifi_Rohingya}`, `\p{Hanunoo}`, `\p{Hatran}`, `\p{Hebrew}`,
  `\p{Hiragana}`, `\p{Imperial_Aramaic}`, `\p{Inscriptional_Pahlavi}`,
  `\p{Inscriptional_Parthian}`, `\p{Javanese}`, `\p{Kaithi}`, `\p{Kannada}`,
  `\p{Katakana}`, `\p{Kayah_Li}`, `\p{Kharoshthi}`, `\p{Khitan_Small_Script}`,
  `\p{Khmer}`, `\p{Khojki}`, `\p{Khudawadi}`, `\p{Lao}`, `\p{Latin}`,
  `\p{Lepcha}`, `\p{Limbu}`, `\p{Linear_A}`, `\p{Linear_B}`, `\p{Lisu}`,
  `\p{Lycian}`, `\p{Lydian}`, `\p{Mahajani}`, `\p{Makasar}`, `\p{Malayalam}`,
  `\p{Mandaic}`, `\p{Manichaean}`, `\p{Marchen}`, `\p{Masaram_Gondi}`,
  `\p{Medefaidrin}`, `\p{Meetei_Mayek}`, `\p{Mende_Kikakui}`,
  `\p{Meroitic_Cursive}`, `\p{Meroitic_Hieroglyphs}`, `\p{Miao}`, `\p{Modi}`,
  `\p{Mongolian}`, `\p{Mro}`, `\p{Multani}`, `\p{Myanmar}`, `\p{Nabataean}`,
  `\p{Nandinagari}`, `\p{New_Tai_Lue}`, `\p{Newa}`, `\p{Nko}`, `\p{Nushu}`,
  `\p{Nyiakeng_Puachue_Hmong}`, `\p{Ogham}`, `\p{Ol_Chiki}`,
  `\p{Old_Hungarian}`, `\p{Old_Italic}`, `\p{Old_North_Arabian}`,
  `\p{Old_Permic}`, `\p{Old_Persian}`, `\p{Old_Sogdian}`,
  `\p{Old_South_Arabian}`, `\p{Old_Turkic}`, `\p{Oriya}`, `\p{Osage}`,
  `\p{Osmanya}`, `\p{Pahawh_Hmong}`, `\p{Palmyrene}`, `\p{Pau_Cin_Hau}`,
  `\p{Phags_Pa}`, `\p{Phoenician}`, `\p{Psalter_Pahlavi}`, `\p{Rejang}`,
  `\p{Runic}`, `\p{Samaritan}`, `\p{Saurashtra}`, `\p{Sharada}`, `\p{Shavian}`,
  `\p{Siddham}`, `\p{SignWriting}`, `\p{Sinhala}`, `\p{Sogdian}`,
  `\p{Sora_Sompeng}`, `\p{Soyombo}`, `\p{Sundanese}`, `\p{Syloti_Nagri}`,
  `\p{Syriac}`, `\p{Tagalog}`, `\p{Tagbanwa}`, `\p{Tai_Le}`, `\p{Tai_Tham}`,
  `\p{Tai_Viet}`, `\p{Takri}`, `\p{Tamil}`, `\p{Tangut}`, `\p{Telugu}`,
  `\p{Thaana}`, `\p{Thai}`, `\p{Tibetan}`, `\p{Tifinagh}`, `\p{Tirhuta}`,
  `\p{Ugaritic}`, `\p{Vai}`, `\p{Wancho}`, `\p{Warang_Citi}`, `\p{Yezidi}`,
  `\p{Yi}`, `\p{Zanabazar_Square}`,

@note Unicode language script character classes differ from the Unicode blocks
that have a similar name.  For example, the `\p{Greek}` class represents Greek
and Coptic letters and differs from the Unicode block `\p{IsGreek}` that spans
a specific Unicode block of Greek and Coptic characters only, which also
includes unassigned characters.

🔝 [Back to table of contents](#)

### Anchors and boundaries                             {#reflex-pattern-anchor}

Anchors are used to demarcate the start and end of input or the start and end
of a line:

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `^φ`      | matches `φ` at the start of input or start of a line (multi-line mode)
  `φ$`      | matches `φ` at the end of input or end of a line (multi-line mode)
  `\Aφ`     | matches `φ` at the start of input
  `φ\z`     | matches `φ` at the end of input

Anchors in lexer specifications require pattern context, meaning that `φ`
cannot be empty.

Note that `<<EOF>>` in lexer specifications match the end of input, which 
can be used in place of the pattern `\z`.

Actions for the start of input can be specified in an initial code block
preceding the rules, see \ref reflex-code-blocks.

Word boundaries demarcate words.  Word characters `\w` are letters, digits, and
the underscore.

  Pattern   | Matches
  --------- | -----------------------------------------------------------------
  `\bφ`     | matches `φ` starting at a word boundary
  `φ\b`     | matches `φ` ending at a word boundary
  `\Bφ`     | matches `φ` starting at a non-word boundary
  `φ\B`     | matches `φ` ending at a non-word boundary
  `\<φ`     | matches `φ` that starts as a word
  `\>φ`     | matches `φ` that starts as a non-word
  `φ\<`     | matches `φ` that ends as a non-word
  `φ\>`     | matches `φ` that ends as a word

@note The RE/flex regex library requires anchors and word boundaries to be
specified in patterns at the start or end of the pattern.  Boundaries are not
permitted in the middle of a pattern, see \ref reflex-limitations.

🔝 [Back to table of contents](#)

### Indent/nodent/dedent                                {#reflex-pattern-dents}

Automatic indent and dedent matching is a special feature of RE/flex and is
only available when the RE/flex matcher engine is used (the default matcher).
An indent and a dedent position is defined and matched with:

  Pattern | Matches
  ------- | -------------------------------------------------------------------
  `\i`    | indent: matches and adds a new indent stop position
  `\j`    | dedent: matches a previous indent position, removes one indent stop

The `\i` and `\j` anchors should be used in combination with the start of a
line anchor `^` followed by a pattern that represents left margin spacing for
indentations, followed by a `\i` or a `\j` at the end of the pattern.  The
margin spacing pattern may include any characters that are considered part of
the left margin, but should exclude `\n`.  For example:

<div class="alt">
~~~{.cpp}
    %o tabs=8
    %%
    ^\h+      out() << "| "; // nodent: text is aligned to current indent
    ^\h+\i    out() << "> "; // indent: matched and added with \i
    ^\h*\j    out() << "< "; // dedent: matched with \j
    \j        out() << "< "; // dedent: for each extra level dedented
    .|\n      echo();
    %%
~~~
</div>

The `\h` pattern matches space and tabs, where tabs advance to the next column
that is a multiple of 8.  The tab multiplier can be changed by setting the
`−−tabs=N` option where `N` must be 1, 2, 4, or 8.  The tabs value can be
changed at runtime with `matcher().tabs(N)`:

  RE/flex action      | Result
  ------------------- | -------------------------------------------------------
  `matcher().tabs()`  | returns the current tabs value 1, 2, 4, or 8
  `matcher().tabs(n)` | set the tabs value `n` where `n` is 1, 2, 4 or 8

Using negative patterns we can ignore empty lines and multi-line comments that
would otherwise affect indent stops:

<div class="alt">
~~~{.cpp}
    %o main tabs=8
    %%
    ^\h+                      out() << "| "; // nodent, text is aligned to current margin column
    ^\h+\i                    out() << "> "; // indent
    ^\h*\j                    out() << "< "; // dedent
    \j                        out() << "< "; // dedent, for each extra level dedented
    (?^^\h*\n)                // eat empty lines without affecting indent stops
    (?^^\h*"/*"(.|\n)*?"*/")  // eat /*-comments that start a line without affecting indent stops
    (?^\\\n\h*)               // lines ending in \ continue on the next line
    (?^"/*"(.|\n)*?"*/")      // eat /*-comments
    .|\n                      echo(); // ECHO character
    %%
~~~
</div>

Likewise, we can add rules to ignore inline `//`-comments to our lexer
specification.  To do so, we should add a rule with pattern `(?^^\h*"//".*)` to
ignore `//`-comments without affecting stop positions.

To scan input that continues on the next new line(s) (which may affect indent
stops) while preserving the current indent stop positions, use the RE/flex
matcher `matcher().push_stops()` and `matcher().pop_stops()`, or
`matcher().stops()` to directlye access the vector of indent stops to modify:

  RE/flex action             | Result
  -------------------------- | ------------------------------------------------
  `matcher().push_stops()`   | push indent stops on the stack then clear stops
  `matcher().pop_stops()`    | pop indent stops and make them current
  `matcher().clear_stops()`  | clear current indent stops
  `matcher().stops()`        | reference to current `std::vector<size_t>` stops
  `matcher().last_stop()`    | returns the last indent stop position or 0
  `matcher().insert_stop(n)` | inserts/appends an indent stop at position `n`
  `matcher().delete_stop(n)` | remove stop positions from position `n` and up

For example, to continue scanning after a `/*` for multiple lines without
indentation matching, allowing for possible nested `/*`-comments, up to a `*/`
you can save the current indent stop positions and transition to a new start
condition state to scan the content between `/*` and `*/`:

<div class="alt">
~~~{.cpp}
    %{
      int level;                 // a variable to track the /*-comment nesting level
      std::vector<size_t> stops; // a variable to save the stop positions after indent
    %}
    %o tabs=8
    %x COMMENT
    %%
    ^\h+           out() << "| ";               // nodent, text is aligned to current margin column
    ^\h+\i         out() << "> ";               // indent
                   stops = matcher().stops();   // save the stop positions
    ^"/*"\j        level = 1;                   // do not count dedent(s) to the first line that has a /*-comment
                   start(COMMENT);              // skip comment
    ^\h*\j         out() << "< ";               // dedent
    \j             out() << "< ";               // dedent, triggered for each extra level dedented
    (?^^\h*\n)     // a negative pattern to eat empty lines without affecting indent stops
    (?^^\h+/"/*")  // a negative pattern to eat white space before /*-comments without affecting indent stops
    "/*"           level = 1;
                   start(COMMENT);              // continue w/o indent matching
    (?^\\\n\h*)    // lines ending in \ continue on the next line
    .|\n           echo();                      // ECHO character
    <COMMENT>{
    "/*"           ++level;                     // allow nested /*-comments 
    "*/"           if (--level == 0)
                   {
                     matcher().stops() = stops; // restore the indent margin/tab stops
                     start(INITIAL);            // back to initial state
                   }
    .|\n           // ignore all content in comments
    <<EOF>>        out() << "/* not closed";
    }
    %%
~~~
</div>

The multi-line comments enclosed in `/*` `*/` are processed by the exclusive
`COMMENT` start condition rules.  The rules allow for `/*`-comment nesting.
We use `stops = matcher().stops()` and `matcher().stops() = stops` to save and
restore stops.

In this example we added rules so that comments on a line do not affect the
current indent stops.  This is done by using the negative pattern
`(?^^\h+/"/*")` with a trailing context `/"/*"`.  Here we used a negative
pattern to eat the margin spacing without affecting indent stops.  The trailing
context looks ahead for a `/*` but does not consume the `/*`.

However, when a `/*`-comment starts at the first column of a line, the pattern
`(?^^\h+/"/*")` does not match it, even when we change it to `(?^^\h*/"/*")`.
This is because the `\h*` cannot be an empty match since the trailing context
does not return a match, and matches cannot be empty.  Therefore, adding the
rule with pattern `^"/*"\j` adjusts for that, but accepting the dedents caused
by the `/*`-comment.  This is fine, because  the stop positions are restored
after scanning the `/*`-comment.

We added the negative pattern `(?^^\h*\n)` to ignore empty lines.  This allows
empty lines in the input without affecting indent stops.

@warning When using the `matcher().stops()` method to access the vector of
stops to modify, we must make sure to keep the stop positions in the vector
sorted.

In addition to the `\i` and `\j` indent and dedent anchors, the `\k` undent
anchor matches when the indent depth changed (before the position of `\k`),
undoing this change to keep the current indent stops ("undenting"):

  Pattern | Matches
  ------- | -------------------------------------------------------------------
  `\k`    | undent: matches when indent depth changed, keep current indent stops

The example shown above can be simplified with `\k`.  We no longer need to
explicitly save and restore indent stops in a variable:

<div class="alt">
~~~{.cpp}
    %{
      int level;   // a variable to track the /*-comment nesting level
    %}
    %o tabs=8
    %x COMMENT
    %%
    ^\h+           out() << "| ";    // nodent, text is aligned to current margin column
    ^\h+\i         out() << "> ";    // indent
    ^\h*\j         out() << "< ";    // dedent
    \j             out() << "< ";    // dedent, triggered for each extra level dedented
    (?^^\h*\n)     // a negative pattern to eat empty lines without affecting indent stops
    \h*"/*"\k?     level = 1;        // /*-comment after spacing, \k matches indent stop changes
                   start(COMMENT);   // continue w/o indent matching
    (?^\\\n\h*)    // lines ending in \ continue on the next line
    .|\n           echo();           // ECHO character
    <COMMENT>{
    "/*"           ++level;          // allow nested /*-comments 
    "*/"           if (--level == 0)
                     start(INITIAL); // back to initial state
    .|\n           // ignore all content in comments
    <<EOF>>        out() << "/* not closed";
    }
    %%
~~~
</div>

The pattern `\h*"/*"\k?` matches a `/*`-comment with leading white space.  The
`\k` anchor matches if the indent depth changed in the leading white space,
which is also matched by the first three patterns in the lexer specification
before their `\i` and `\j` indent and dedent anchors, respectively.  If the
indent depth changed, the `\k` anchor matches, while keeping the current indent
stops unchanged by undoing these changes.  Because we also want to match `\*`
when the indent depth does not change, we made `\k` optional in pattern
`\h*"/*"\k?`.  The anchor `^` is not used here either, since comments after any
spacing should be matched.  Alternatively, two patterns `^\h*"/*"\k` and
`\h*"/*"` may be used, where the first matches if and only if the indent stops
changed on a new line and were undone.

Note that the `COMMENT` rules do not use `\i` or `\j`.  This means that the
current indent stops are never matched or changed and remain the same as in the
`INITIAL` state, when returning to the `INITIAL` state.

Another use of `\k` is to ignore indents to only detect a closing dedent with
`\j`.  For example, when comments are allowed to span multiple lines when
indented below the start of the `#` comment:

<div class="alt">
~~~{.cpp}
    %x COMMENT MORECOM
    %%
    ^\h+\i         // indent
    ^\h*\j         // dedent
    \j             // dedent
    ^\h+           // nodent
    #.*\n          start(COMMENT);
    <COMMENT>{
    ^\h+\i         start(MORECOM);
    .|\n           matcher().less(0); start(INITIAL);
    }
    <MORECOM>{
    ^\h*\j         |
    \j             start(INITIAL);
    ^\h+\k         // undent, i.e. ignore all indents in comments
    .|\n           // ignore all content in comments
    }
    .|\n           echo();
    %%
~~~
</div>

The `COMMENT` state checks for an indent to switch to state `MORECOM`, which
eats the indented comment block.  When there is no indent `.|\n` is matched,
i.e. something must be matched.  This match is put back into the input with
`matcher().less(0)` (or `yyless(0)` with `−−flex`).

Alternatively, the indent level in the `COMMENT` rules could be tracked by
incrementing a variable when matching `\i` and decrementing the variable when
matching `\j` until the variable is zero at the final dedent.

@note Anchors `\i`, `\j`, and `\k` should appear at the end of a regex pattern.

See \ref reflex-states for more information about start condition states.  See
\ref reflex-pattern-negative for more information on negative patterns.

🔝 [Back to table of contents](#)

### Negative patterns                                {#reflex-pattern-negative}

When negative patterns of the form `(?^φ)` match, they are simply ignored by
the matcher and never returned as matches.  They are useful to return matches
for some given pattern except when this pattern is more specific.  For example,
to match any sequence of digits except digits starting with a zero the pattern
`\d+|(?^0\d+)` can be used instead of `[1-9]\d+`.  While these two patterns may
look similar at first glance, these two patterns differ in that the first
pattern (with the negative sub-pattern `(?^0\d+)`) ignores numbers with leading
zeros such as `012` while the second pattern will match the `12` in `012`.

As another example, say we are searching for a given word while ignoring
occurrences of the word in quoted strings.  We can use the pattern
`word|(?^".*?")` for this, where `(?^".*?")` matches all quoted strings that we
want to ignore (to skip C/C++ quoted strings in source code input files, use
the longer pattern `(?^"(\\.|\\\r?\n|[^\\\n"])*")`).

A negative pattern can also be used to consume line continuations without
affecting the indentation stops defined by indent marker `\i`.  Negative
patterns are a RE/flex feature.  For example:

<div class="alt">
~~~{.cpp}
    %o tabs=8
    %%
    ^\h+         out() << "| "; // nodent: text is aligned to current indent
    ^\h*\i       out() << "> "; // indent: matched and added with \i
    ^\h*\j       out() << "< "; // dedent: matched with \j
    \j           out() << "< "; // dedent: for each extra level dedented
    (?^\\\n\h+)  /* lines ending in \ will continue on the next line
                    without affecting the current \i stop positions */
    .|\n         echo();
    %%
~~~
</div>

The negative pattern `(?^\\\n\h+)` consumes input internally as if we are
repeately calling `input()` (or `yyinput()` with `−−flex`).  We used it here to
consume the line-ending `\` and the indent that followed it, as if this text
was not part of the input, which ensures that the current indent positions
defined by `\i` are not affected.  See \ref reflex-pattern-dents for more
details on indentation matching.

@note Negative patterns may be preceded or followed by any pattern, which
enlarges the negative pattern.  That is, the pattern `X(?^Y)` equals `(?^XY)`
and the pattern `(?^Y)Z` equals `(?^YZ)`.  At least one character should be
matched in a negative pattern for the pattern to be effective.  For example,
`X(?^Y)?` matches `X` but not `XY`, which is the same as `X|(?^XY)`.

@warning Actions corresponding to negative patterns in the lexer specification
are never executed, because negative pattern matches are never returned by the
matcher engine.

🔝 [Back to table of contents](#)

### Lookahead                                       {#reflex-pattern-lookahead}

A lookahead pattern `φ(?=ψ)` matches `φ` only when followed by pattern `ψ`.
The text matched by `ψ` is not consumed.

Boost.Regex and PCRE2 matchers support lookahead `φ(?=ψ)` and lookbehind
`φ(?<=ψ)` patterns that may appear anywhere in a regex.  The RE/flex matcher
supports lookahead at the end of a pattern, similar to \ref
reflex-pattern-trailing.

🔝 [Back to table of contents](#)

### Lookbehind                                     {#reflex-pattern-lookbehind}

A lookbehind pattern `φ(?<=ψ)` matches `φ` only when it also matches pattern
`ψ` at its end (that is, `.*(?<=ab)` matches anything that ends in `ab`).

The RE/flex matcher does not support lookbehind.  Lookbehind patterns should
not look too far behind, see \ref reflex-limitations.

🔝 [Back to table of contents](#)

### Trailing context                                 {#reflex-pattern-trailing}

Flex "trailing context" `φ/ψ` matches a pattern `φ` only when followed by the
lookahead pattern `ψ`.  A trailing context `φ/ψ` has the same meaning as the
lookahead `φ(?=ψ)`, see \ref reflex-pattern-lookahead.

A trailing context can only be used in lexer specifications and should only
occur at the end of a pattern, not in the middle of a pattern.  There are some
important \ref reflex-limitations to consider that are historical and related
to the construction of efficient FSMs for regular expressions.  The limitations
apply to trailing context and lookaheads that the RE/flex matcher implements.

🔝 [Back to table of contents](#)

### Unicode mode                                      {#reflex-pattern-unicode}

Use <b>`reflex`</b> option `−−unicode` (or <i>`%%option unicode`</i>) to
globally enable Unicode.  Use `(?u:φ)` to locally enable Unicode in a pattern
`φ`.  Use `(?-u:φ)` to locally disable Unicode in `φ`.  Unicode mode enables
the following patterns to be used:

  Pattern            | Matches
  ------------------ | --------------------------------------------------------
  `.`                | matches any Unicode character (beware of \ref invalid-utf)
  `€` (UTF-8)        | matches wide character `€`, encoded in UTF-8
  `[€¥£]` (UTF-8)    | matches wide character `€`, `¥` or `£`, encoded in UTF-8
  `\X`               | matches any ISO-8859-1 or Unicode character
  `\R`               | matches a Unicode line break `\r\n` or `[\u{000A}-\u{000D}u{U+0085}\u{2028}\u{2029}]`
  `\s`               | matches a white space character `[ \t\n\v\f\r\p{Z}]`
  `\l`               | matches a lower case letter with Unicode sub-property Ll
  `\u`               | matches an upper case letter with Unicode sub-property Lu
  `\w`               | matches a Unicode word character with property L, Nd, or Pc
  `\u{20AC}`         | matches Unicode character U+20AC
  `\p{C}`            | matches a character in category C
  `\p{^C}`,`\P{C}`   | matches any character except in category C

When converting regex patterns for use with a C++ regex library, use regex
matcher converter flag `reflex::convert_flag::unicode` to convert Unicode
patterns for use with the 8-bit based RE/flex, Boost.Regex, PCRE2, and
std::regex regex libraries, see \ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Free space mode                                 {#reflex-pattern-freespace}

Free space mode can be useful to improve readability of patterns.  Free space
mode permits spacing between concatenations and alternations in patterns.  To
to match a single space use `[ ]`, to match a tab use `[\t]`, to match either
use `\h`.  Long patterns may continue on the next line when the line ends with
an escape `\`.  Comments are ignored in patterns in free-space mode.  Comments
start with a `#` and end at the end of the line.  To specify a `#` use `[#]`.

In addition, `/*...*/` comments are permitted in lexer specifications in
free-space mode when the `−−matcher=reflex` option is specified (the default
matcher).

Free space mode requires lexer actions in \ref reflex-spec-rules of a lexer
specification to be placed in <i>`{ }`</i> blocks and user code to be placed in
<i>`%{ %}`</i> blocks instead of indented.

To enable free space mode in <b>`reflex`</b> use the `−−freespace` option (or
<i>`%%option freespace`</i>).

Prepend `(?x)` to the regex to specify free-space mode or use `(?x:φ)` to
locally enable free-space mode in the sub-pattern `φ`.  Use `(?-x:φ)` to
locally disable free-space mode in `φ`.  The regex pattern may require
conversion when the regex library does not support free-space mode modifiers,
see \ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Multi-line mode                                 {#reflex-pattern-multiline}

Multi-line mode makes the anchors `^` and `$` match the start and end of a
line, respectively.  Multi-line mode is the default mode in lexer
specifications.

Prepend `(?m)` to the regex to specify multi-line mode or use `(?m:φ)` to
locally enable multi-line mode in the sub-pattern `φ`.  Use `(?-m:φ)` to
locally disable multi-line mode in `φ`.

🔝 [Back to table of contents](#)

### Dotall mode                                        {#reflex-pattern-dotall}

To enable dotall mode in <b>`reflex`</b> use the `-a` or `−−dotall` option (or
<i>`%%option dotall`</i>).

Prepend `(?s)` to the regex to specify dotall mode or use `(?s:φ)` to locally
enable dotall mode in the sub-pattern `φ`.  Use `(?-s:φ)` to locally disable
dotall mode in `φ`.  The regex pattern may require conversion when the regex
library does not support dotall mode modifiers, see \ref regex-convert for more
details.

🔝 [Back to table of contents](#)

### Case-insensitive mode                             {#reflex-pattern-anycase}

To enable case-insensitive mode in <b>`reflex`</b> use the `-i` or
`−−case-insensitive` option (or <i>`%%option case-insensitive`</i>).

Prepend `(?i)` to the regex to specify case-insensitive mode or use `(?i:φ)` to
locally enable case-insensitive mode in the sub-pattern `φ`.  Use `(?-i:φ)` to
locally disable case-insensitive mode in `φ`.  The regex pattern may require
conversion when the regex library does not support case-insensitive mode
modifiers, see \ref regex-convert for more details.

🔝 [Back to table of contents](#)

### Multiple mode modifiers                         {#reflex-pattern-modifiers}

Multiple `(?i:φ)` \ref reflex-pattern-anycase, `(?m:φ)`
\ref reflex-pattern-multiline, `(?s:φ)` \ref reflex-pattern-dotall, `(?u:φ)`
\ref reflex-pattern-unicode, and `(?x:φ)` \ref reflex-pattern-freespace
modifiers may be applied to the same pattern `φ` by combining them in one
inline modifier `(?imsux-imsux:φ)`, where the mode modifiers before the dash
are enabled and the mode modifiers after the dash are disabled.

🔝 [Back to table of contents](#)


The Lexer/yyFlexLexer class                                     {#reflex-lexer}
---------------------------

By default, <b>`reflex`</b> produces a Lexer class with a virtual lex scanner
function.  The name of this function as well as the Lexer class name and the
namespace can be set with options:

  Option      | RE/flex default name | Flex default name
  ----------- | -------------------- | ----------------------------------------
  `namespace` | *n/a*                | *n/a* 
  `lexer`     | `Lexer` class        | `yyFlexLexer` class
  `lex`       | `lex()` function     | `yylex()` function

To customize the Lexer class use these options and code injection.

You can declare multiple nested namespace names by
`namespace=NAME1::NAME2::NAME3`, or by separating the names with a dot such as
`namespace=NAME1.NAME2.NAME3`, to declare the lexer in `NAME1::NAME2::NAME3`.

To understand the impact of these options, consider the following lex
specification template:

<div class="alt">
~~~{.cpp}
    %option namespace=NAMESPACE
    %option lexer=LEXER
    %option lex=LEX
    %option params=PARAMS

    %class{
      MEMBERS
    }

    %option ctorarg="CTORARGS"
    %init{
      INIT
    }

    %%

    %{
      CODE
    %}

    REGEX ACTION

    %%
~~~
</div>

This produces the following Lexer class with the template parts filled in:

~~~{.cpp}
    #include <reflex/abslexer.h>
    namespace NAMESPACE {
      class LEXER : public reflex::AbstractLexer<reflex::Matcher> {
        MEMBERS
       public:
        LEXER(
            CTORARGS,
            const reflex::Input& input = reflex::Input(),
            std::ostream&        os    = std::cout)
          :
            AbstractLexer(input, os)
        {
          INIT
        }
        static const int INITIAL = 0;
        virtual int LEX(PARAMS);
        int LEX(
            const reflex::Input& input,
            std::ostream        *os = NULL,
            PARAMS)
        {
          in(input);
          if (os)
            out(*os);
          return LEX(PARAMS);
        }
      };
      int NAMESPACE::LEXER::LEX(PARAMS)
      {
        static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
        if (!has_matcher())
        {
          matcher(new Matcher(PATTERN_INITIAL, stdinit(), this));
        }
        CODE
        while (true)
        {
          switch (matcher().scan())
          {
            case 0:
              if (matcher().at_end())
              {
                return 0;
              }
              else
              {
                out().put(matcher().input());
              }
              break;
            case 1:
              ACTION
              break;
          }
        }
      }
    }
~~~

The Lexer class produced with option `−−flex` is compatible with Flex (assuming
Flex with option `-+` for C++):

~~~{.cpp}
    #include <reflex/flexlexer.h>

    namespace NAMESPACE {

      typedef reflex::FlexLexer<reflex::Matcher> FlexLexer;

      class LEXER : public FlexLexer {
        MEMBERS
       public:
        LEXER(
            CTORARGS,
            const reflex::Input& input = reflex::Input(),
            std::ostream        *os    = NULL)
          :
            FlexLexer(input, os)
        {
          INIT
        }
        virtual int LEX(PARAMS);
        int LEX(
            const reflex::Input& input,
            std::ostream        *os = NULL,
            PARAMS)
        {
          in(input);
          if (os)
            out(*os);
          return LEX(PARAMS);
        }
      };

      int NAMESPACE::LEXER::LEX(PARAMS)
      {
        static const reflex::Pattern PATTERN_INITIAL("(?m)(REGEX)");
        if (!has_matcher())
        {
          matcher(new Matcher(PATTERN_INITIAL, stdinit(), this));
          YY_USER_INIT
        }
        CODE
        while (true)
        {
          switch (matcher().scan())
          {
            case 0:
              if (matcher().at_end())
              {
                return 0;
              }
              else
              {
                output(matcher().input());
              }
              YY_BREAK
            case 1:
              YY_USER_ACTION
              ACTION
              YY_BREAK
          }
        }
      }
    }
~~~

To use a custom lexer class that inherits the generated base Lexer class, use
option `−−class=NAME` to declare the name of your custom lexer class (or option
`−−yyclass=NAME` to also enable `−−flex` compatibility with the `yyFlexLexer`
class).  For details, see \ref reflex-inherit.

🔝 [Back to table of contents](#)

Inheriting Lexer/yyFlexLexer                                  {#reflex-inherit}
----------------------------

To define a custom lexer class that inherits the generated Lexer or the
yyFlexLexer class, use option `−−class=NAME` or option `−−yyclass=NAME`,
respectively.  Note that `−−yyclass=NAME` also enables option `−−flex` and
therefore enables Flex specification syntax.

When a `−−class=NAME` or `−−yyclass=NAME` option is specified with the name of
your custom lexer class, <b>`reflex`</b> generates the `lex()` (or `yylex()`) method
code for your custom lexer class.  The custom lexer class should declare a
public `int lex()` method (or `int yylex()` method with option
`−−yyclass=NAME`).  Otherwise, C++ compilation of your custom class will fail.

For example, the following bare-bones custom Lexer class definition simply
inherits Lexer and declares a public `int lex()` method:
  
<div class="alt">
~~~{.cpp}
    %option class=MyLexer

    %{
      class MyLexer : public Lexer {
       public:
        int lex();
      };
    %}
~~~
</div>

The `int MyLexer::lex()` method code is generated by <b>`reflex`</b> for this lexer
specification.

Options `−−lexer=NAME` and `−−lex=NAME` may be combined with `−−class=NAME` to
change the name of the inherited Lexer class and change the name of the `lex()`
method, respectively.

When using option `−−yyclass=NAME` the inherited lexer is `yyFlexLexer`.  The
custom lexer class should declare a public `yylex()` method similar to Flex.
For example:

<div class="alt">
~~~{.cpp}
    %option yyclass=MyLexer

    %{
      class MyLexer : public yyFlexLexer {
       public:
        int yylex();
      };
    %}
~~~
</div>

The `int MyLexer::yylex()` method code is generated by <b>`reflex`</b> for this
lexer specification.

🔝 [Back to table of contents](#)


Combining multiple lexers                                    {#reflex-multiple}
-------------------------

To combine multiple lexers in one application, use the `−−lexer=NAME` option.
This option renames the generated lexer class to avoid lexer class name
clashes.  Use this option in combination with option `−−header-file` to output
a header file with the lexer class declaration to include in your application
source code.

Use option `−−prefix=NAME` to output the generated code in file
<i>`lex.NAME.cpp`</i> instead of the standard <i>`lex.yy.cpp`</i> to avoid file
name clashes.  This option also affect the `−−flex` option by generating
`xxFlexLexer` with a `xxlex()` method when option `−−prefix=xx` is specified.
The generated `#define` names (some of which are added to support option
`−−flex`) are prefixed to avoid macro name clashes.

Alternatively to `−−lexer=NAME` you can use `−−namespace=NAME` to place the
generated lexer class in a C++ namespace to avoid lexer class name clashes.

🔝 [Back to table of contents](#)


Switching input sources                                         {#reflex-input}
-----------------------

To create a Lexer class instance that reads from a designated input source
instead of standard input, pass the input source as the first argument to its
constructor and use the second argument to optionally set an `std::ostream`
that is assigned to `out()` and is used by `echo()` (likewise, assigned to
`*yyout` and used by `ECHO` when option `−−flex` is specified):

~~~{.cpp}
    Lexer lexer(input, std::cout);
~~~

likewise, with option `−−flex`:

~~~{.cpp}
    yyFlexLexer lexer(input, std::cout);
~~~

where `input` is a `reflex::Input` object.  The `reflex::Input` constructor
takes a `FILE*` descriptor, `std::istream`, a string `std::string` or
`const char*`, or a wide string `std::wstring` or `const wchar_t*`.

The following methods are available to specify an input source:

  RE/flex action      | Flex action              | Result
  ------------------- | ------------------------ | ----------------------------
  `in()`              | `*yyin`                  | get pointer to current `reflex::Input i`
  `in() = i`          | `yyin = &i`              | set input `reflex::Input i`
  `in(i)`             | `yyrestart(i)`           | reset and scan input from `reflex::Input i`
  `in(s)`             | `yy_scan_string(s)`      | reset and scan string `s` (`std::string` or `char*`)
  `in(s)`             | `yy_scan_wstring(s)`     | reset and scan wide string `s` (`std::wstring` or `wchar_t*`)
  `in(b, n)`          | `yy_scan_bytes(b, n)`    | reset and scan `n` bytes at address `b` (buffered)
  `buffer(b, n+1)`    | `yy_scan_buffer(b, n+2)` | reset and scan `n` bytes at address `b` (zero copy)

For example, to switch input to another source while using the scanner, use
`in(i)` with `reflex::Input i` as an argument:

~~~{.cpp}
    // read from a file, this also decodes UTF-8/16/32 encodings automatically
    FILE *fd = fopen("cow.txt", "r");
    if (fd == NULL)
      ... // error, bail out
    lexer.in(fd);
    lexer.lex();

    // read from a stream (ASCII or UTF-8)
    std::istream i = std::ifstream("file", std::ios::in);
    lexer.in(i);
    lexer.lex();

    // read from a string (0-terminated, ASCII or UTF-8)
    lexer.in("How now brown cow.");
    lexer.lex();

    // read from a memory segment (raw bytes, ASCII, or UTF-8)
    const char *ptr = ...; // points to segment
    size_t len = ...;      // length of the memory segment
    lexer.in(ptr, len);
    lexer.lex();

    // read from a wide string, 0-terminated, encoding it to UTF-8 for matching
    lexer.in(L"How now brown cow.");
    lexer.lex();
~~~

You can assign new input with `in() = i`, which does not reset the lexer's
matcher.  This means that when the end of the input (EOF) is reached, and you
want to switch to new input, then you should clear the EOF state first with
`lexer.matcher().set_end(false)` to reset EOF.  Or use
`lexer.matcher().reset()` to clear the state.

Invoking `in(i)` resets the lexer's matcher (i.e. internally with
`matcher.reset()`).  This clears the line and column counters, resets the
internal anchor and boundary flags for anchor and word boundary matching, and
resets the matcher to consume buffered input.

These `in(i)` operations specify strings and bytes that are copied to an
internal buffer.  This is desirable, because the scanner uses a matcher that
initializes a buffer, block-wise copies more input to this internal buffer on
demand, and modifies this buffered content, e.g. to allow `text()` to return a
0-terminated `char` string.  Zero copy overhead is obtained with lexer method
`buffer(b, n)` to assign an external buffer:

~~~{.cpp}
    // read a 0-terminated buffer in place, buffer content is changed!!
    char *base = ...;  // points to 0-terminated buffer
    size_t size = ...; // length of the buffer including final \0 byte
    lexer.buffer(base, size);
    lexer.lex();
~~~

@warning Function `buffer(b, n)` scans `n`-1 bytes at address `b`.  The length
`n` should include the final zero byte at the end of the string.

With options `−−flex` and `−−bison` you can also use classic Flex functions:

~~~{.cpp}
    // read from a file, this also decodes UTF-8/16/32 encodings automatically
    FILE *fd = fopen("cow.txt", "r");
    if (fd == NULL)
      ... // error, bail out
    yyin = fd;
    yylex();

    // read from a stream (ASCII or UTF-8)
    std::istream i = std::ifstream("file", std::ios::in);
    yyin = &i;
    yylex();

    // read from a string (0-terminated, ASCII or UTF-8)
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_scan_string("How now brown cow."); // new buffer to scan a string
    // yyin = "How now brown cow.";       // alternative, does not create a new buffer
    yylex();

    // read from a memory segment (raw bytes, ASCII, or UTF-8)
    const char *ptr = ...; // points to memory segment
    size_t len = ...;      // length of the memory segment
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_scan_bytes(ptr, len); // new buffer to scan memory
    yylex();

    // read from a wide string, 0-terminated, encoding it to UTF-8 for matching
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_scan_wstring(L"How now brown cow."); // new buffer to scan a wide string
    // yyin = L"How now brown cow.";        // alternative, does not create a new buffer
    yylex();
~~~

The `yy_scan_string`, `yy_scan_bytes`, and `yy_scan_wstring` functions create
a new buffer (i.e. a new matcher in RE/flex) and replace the old buffer
without deleting it.  A pointer to the new buffer is returned, which becomes
the new `YY_CURRENT_BUFFER`.  You should delete the old buffer with
`yy_delete_buffer(YY_CURRENT_BUFFER)` before creating a new buffer.

Zero copy overhead is obtained with `yy_scan_buffer(b, n)`:

~~~{.cpp}
    // read a 0-terminated buffer in place, buffer content is changed!!
    char *base = ...;  // points to 0-terminated buffer
    size_t size = ...; // length of the buffer including two final \0 bytes
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_scan_buffer(base, size);
    yylex();
~~~

@warning The Flex-compatible `yy_scan_buffer(b, n)` (when option `−−flex` is
used) scans `n`-2 bytes at address `b`.  The length `n` should include *two
final zero bytes at the end!*

@note Function `yy_scan_buffer(b, n)` only touches the first final byte and not
the second byte, since this function is the same as calling `buffer(b, n-1)`.
In fact, the specified string may have any final byte value.  The final byte of
the string will be set to zero when `text()` (or `yytext`) or `rest()` are
used.  But otherwise the final byte remains completely untouched by the other
lexer functions, including `echo()` (and Flex-compatible `ECHO`).  Only
`unput(c)`, `wunput()`, `text()` (or `yytext`), `rest()`, and `span()` modify
the buffer contents, where `text()` and `rest()` require an extra byte at the
end of the buffer to make the strings returned by these functions 0-terminated.
This means that you can scan read-only memory of `n` bytes located at address
`b` by using `buffer(b, n+1)` safely, for example to read read-only mmap(2)
`PROT_READ` memory, as long as `unput(c)`,`wunput()`, `text()` (or `yytext`),
`rest()`, and `span()` are not used.

The Flex `yy_scan_string`, `yy_scan_bytes`, `yy_scan_wstring`, and
`yy_scan_buffer` functions take an extra last `yyscan_t` argument for reentrant
scanners generated with option `−−reentrant`, for example:

~~~{.cpp}
    // read from a file, this also decodes UTF-8/16/32 encodings automatically
    FILE *fd = fopen("cow.txt", "r");
    if (fd == NULL)
      ... // error, bail out
    yyget_in(yyscanner) = fd;
    yylex();

    // read from a stream (ASCII or UTF-8)
    std::istream i = std::ifstream("file", std::ios::in);
    yyget_in(yyscanner) = &i;
    yylex();

    // read from a string (0-terminated, ASCII or UTF-8)
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    yy_scan_string("How now brown cow.", yyscanner); // new buffer to scan a string
    yylex();

    // read from a memory segment (raw bytes, ASCII, or UTF-8)
    const char *ptr = ...; // points to memory segment
    size_t len = ...;      // length of the memory segment
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    yy_scan_bytes(ptr, len, yyscanner); // new buffer to scan memory
    yylex();

    // read from a wide string, 0-terminated, encoding it to UTF-8 for matching
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    yy_scan_wstring(L"How now brown cow.", yyscanner); // new buffer to scan a wide string
    yylex();

    // read a 0-terminated buffer in place, buffer content is changed!!
    char *base = ...;  // points to 0-terminated buffer
    size_t size = ...; // length of the buffer including final 0 byte
    yy_delete_buffer(YY_CURRENT_BUFFER, yyscanner);
    yy_scan_buffer(base, size, yyscanner);
    yylex();
~~~

The `yyscanner` macro is essentially the same is the `this` pointer
that can only be used in lexer methods and in lexer rules.
Outside the scope of lexer methods a pointer to your `yyFlexLexer lexer` object
should be used instead, for example `yyget_in(&lexer)`.  Also
`YY_CURRENT_BUFFER` should be replaced by `yyget_current_buffer(&lexer)`.
See also \ref reflex-reentrant.

Switching input before the end of the input source is reached discards all
remaining input from that source.  To switch input without affecting the
current input source, switch matchers instead.  The matchers buffer the input
and manage the input state, in addition to pattern matching the input.

The following methods are available to specify a matcher `Matcher m` (a Flex
"buffer") for a lexer:

  RE/flex action    | Flex action              | Result
  ----------------- | ------------------------ | ------------------------------
  `matcher(m)`      | `yy_switch_to_buffer(m)` | use matcher `m`
  `new_matcher(i)`  | `yy_create_buffer(i, n)` | returns new matcher for `reflex::Input i`
  `del_matcher(m)`  | `yy_delete_buffer(m)`    | delete matcher `m`
  `push_matcher(m)` | `yypush_buffer_state(m)` | push current matcher, then use `m`
  `pop_matcher()`   | `yypop_buffer_state()`   | pop matcher and delete current
  `ptr_matcher()`   | `YY_CURRENT_BUFFER`      | pointer to current matcher
  `has_matcher()`   | `YY_CURRENT_BUFFER != 0` | current matcher is usable

For example, to switch to a matcher that scans from a new input source, then
restores the old input source:

~~~{.cpp}
    ... // scanning etc.
    Matcher *oldmatcher = matcher();
    Matcher *newmatcher = new_matcher(input);
    matcher(newmatcher);
    ... // scan the new input
    del_matcher(newmatcher);
    matcher(oldmatcher);
    ... // continue scanning the old input
~~~

the same with the `−−flex` option becomes:

~~~{.cpp}
    ... // scanning etc.
    YY_BUFFER_STATE oldbuf = YY_CURRENT_BUFFER;
    YY_BUFFER_STATE newbuf = yy_create_buffer(input, YY_BUF_SIZE);
    yy_switch_to_buffer(newbuf);
    ... // scan the new input
    yy_delete_buffer(newbuf);
    yy_switch_to_buffer(oldbuf);
    ... // continue scanning the old input
~~~

This switches the scanner's input by switching to another matcher.  Note that
`matcher(m)` may be used by the virtual `wrap()` method (or `yywrap()` when
option `−−flex` is specified) if you use input wrapping after EOF to set things
up for continued scanning.

Switching input sources (via either `matcher(m)`, `in(i)`, or the Flex
functions) does not change the current start condition state.

When the scanner reaches the end of the input, it will check the `int wrap()`
method to detetermine if scanning should continue.  If `wrap()` returns one (1)
the scanner terminates and returns zero to its caller.  If `wrap()` returns
zero (0) then the scanner continues.  In this case `wrap()` should set up a new
input source to scan.

For example, continuing reading from `std:cin` after some other input source
reached EOF:

<div class="alt">
~~~{.cpp}
    %class{
      virtual int wrap() // note: yywrap() with option −−flex
      {
        in(std::cin);
        return in().good() ? 0 : 1;
      }
    }
~~~
</div>

To implement a `wrap()` (and `yywrap()` when option `−−flex` is specified) in a
derived lexer class with option `class=NAME` (or `yyclass=NAME`), override the
`wrap()` (or `yywrap()`) method as follows:

<div class="alt">
~~~{.cpp}
    %option class=Tokenizer

    %{
      class Tokenizer : Lexer { // note: yyFlexLexer with option −−flex
       public:
        virtual int wrap() // note: yywrap() with option −−flex
        {
          in(std::cin);
          return in().good() ? 0 : 1;
        }
      };
    %}
~~~
</div>

You can override the `wrap()` method to set up a new input source when the
current input is exhausted.  Do not use `matcher().input(i)` to set a new input
source `i`, because that resets the internal matcher state.

With the `−−flex` options your can override the `yyFlexLexer::yywrap()` method
that returns an integer 0 (more input available) or 1 (we're done).

With the `−−flex` and `−−bison` options you should define a global `yywrap()`
function that returns an integer 0 (more input available) or 1 (we're done).  

To set the current input as interactive, such as input from a console, use
`matcher().interactive()` (`yy_set_interactive(1)` with option `−−flex`).  This
disables buffering of the input and makes the scanner responsive to direct
input.

To read from the input without pattern matching, use `matcher().input()` to
read one character at a time (8-bit, ASCII or UTF-8).  This function returns
EOF if the end of the input was reached.  But be careful, the Flex `yyinput()`
and `input()` functions return 0 instead of an `EOF` (-1)!

To put back one character unto the input stream, use `matcher().unput(c)` (or
`unput(c)` with option `−−flex`) to put byte `c` back in the input or
`matcher().wunput(c)` to put a (wide) character `c` back in the input.

@warning Functions `unput()` and `wunput()` invalidate the previous `text()`
and `yytext` pointers.  Basically, `text()` and `yytext` cannot be used after
`unput()`.

For example, to crudily scan a C/C++ multiline comment we can use the rule:

<div class="alt">
~~~{.cpp}
    "/*"    {  /* skip multiline comments */
      int c;
      while ((c = yyinput()) != 0)
      {
        if (c == '\n')
          ++mylineno;
        else if (c == '*')
        {
          if ((c = yyinput()) == '/')
            break;
          unput(c);
        }
      }
    }
~~~
</div>

We actually do not need to keep track of line numbers explicitly, because
`yyinput()` with RE/flex implicitly updates line numbers, unlike Flex from
which this example originates.

Instead of the crude approach shown above, a better alternative is to use a
regex `/\*.*?\*/` or perhaps use start condition states, see
\ref reflex-states.

A simpler and faster approach is to use `skip("*/")` to skip comments:

<div class="alt">
~~~{.cpp}
    "/*"    skip("*/");
~~~
</div>

Using `skip()` is fast and flushes the internal buffer when searching, unlike
`yyinput()` that maintains the buffer contents to keep `text()` (and `yytext`)
unchanged.

To grab the rest of the input as a string, use `matcher().rest()` which returns
a `const char*` string that points to the internal buffer that is enlarged to
contain all remaining input.  Copy the string before using the matcher again.

To read a number of bytes `n` into a string buffer `s[0..n-1]`, use
the virtual `matcher().get(s, n)` method.  This method is the same as invoking
`matcher().in.get(s, n)` to directly read data from the `reflex::Input` source
`in`, but also handles interactive input when enabled with
`matcher().interactive()` to not read beyond the next newline character.  The
`gets, n)` matcher method can be overriden by a derived matcher class to
customize reading.

The Flex `YY_INPUT` macro is not supported by RE/flex.  It is recommended to
use `YY_BUFFER_STATE` (Flex), which is a `reflex::FlexLexer::Matcher` class in
RE/flex that holds the matcher state and the state of the current input,
including the line and column number positions (so unlike Flex, `yylineno` does
not have to be saved and restored when switching buffers).  See also section
\ref reflex-spec on the actions to use.

To implement a custom input handler you can use a proper object-oriented
approach: create a derived class of `reflex::Matcher` (or another matcher
class derived from `reflex::AbstractMatcher`) and in the derived class override
the `size_t reflex::Matcher::get(char *s, size_t n)` method for input handling.
This function is called with a string buffer `s` of size `n` bytes.  Fill the
string buffer `s` up to `n` bytes and return the number of bytes stored in `s`.
Return zero upon EOF.  Use <b>`reflex`</b> options `−−matcher=NAME` and
`−−pattern=reflex::Pattern` to use your new matcher class `NAME` (or leave out
`−−pattern` for Boost.Regex derived matchers).

The `FlexLexer` lexer class that is the base class of the `yyFlexLexer` lexer
class generated with <b>`reflex`</b> option `−−flex` defines a virtual `size_t
LexerInput(char*, size_t)` method.  This approach is compatible with Flex.  The
virtual method can be redefined in the generated `yyFlexLexer` lexer to consume
input from some source of text:

<div class="alt">
~~~{.cpp}
    %class{
      virtual size_t LexerInput(char *s, size_t n)
      {
        size_t k;
        // populate s[0..k-1] for some k with k <= n
        return k; // return number of bytes filled in s[]
      }
    }
~~~
</div>

The `LexerInput` method may be invoked multiple times by the matcher engine
and should eventually return zero to indicate the end of input is reached (e.g.
when at EOF).

To prevent the scanner from initializing the input to `stdin` before reading
input with `LexerInput()`, use option `−−nostdinit`.


🔝 [Back to table of contents](#)


Multiple input sources                                 {#reflex-multiple-input}
----------------------

A typical scenario for a compiler of a programming language is to process
`include` directives in the source input that should include the source of
another file before continuing with the current input.

For example, the following specification defines a lexer that processes
`#include` directives by switching matchers and using the stack of matchers to
permit nested `#include` directives up to a depth of as much as 99 files:

<div class="alt">
~~~{.cpp}
    %top{
      #include <stdio.h>
    }

    %class{

      int depth;

      void include_file()
      {
        depth++;
        if (depth > 99)
          exit(EXIT_FAILURE);           // max include depth exceeded
        char *q = strchr(text(), '"');  // get ..."filename"
        char *file = strdup(q + 1);     // get filename"
        file[strlen(file) - 1] = '\0';  // get filename
        FILE *fd = fopen(file, "r");
        free(file);
        if (!fd)
          exit(EXIT_FAILURE);           // cannot open file
        push_matcher(new_matcher(fd));  // push current matcher, use new matcher
      }

      bool end_of_file()
      {
        if (depth == 0)
          return true;                  // return true: no more input to read
        fclose(in());                   // close current input in() (a FILE*)
        pop_matcher();                  // delete current matcher, pop matcher
        depth--;
        return false;                   // return false: continue reading
      }

    }

    %init{
      depth = 0;
    }

    %%

    ^\h*#include\h*\".*?\"    include_file();
    .|\n                      echo();
    <<EOF>>                   if (end_of_file()) return 0;

    %%
~~~
</div>

With option `−−flex`, the statement `push_matcher(new_matcher(fd))` above
becomes `yypush_buffer_state(yy_create_buffer(fd, YY_BUF_SIZE))` and
`pop_matcher()` becomes `yypop_buffer_state()`.  For comparison, here is a
C-based classic Flex example specification that works with RE/flex too:

<div class="alt">
~~~{.cpp}
    %{
      #include <stdio.h>
      int depth = 0;
    %}

    %%

    ^[ \t]*#include[ \t]*\".*?\"    {
        depth++;
        if (depth > 99)
          exit(EXIT_FAILURE);           // max include depth exceeded
        char *q = strchr(yytext, '"');  // get ..."filename"
        char *file = strdup(q + 1);     // get filename"
        file[strlen(file) - 1] = '\0';  // get filename
        FILE *fd = fopen(file, "r");
        free(file);
        if (!fd)
          exit(EXIT_FAILURE);           // cannot open file
        YY_BUFFER_STATE buf = yy_create_buffer(fd, YY_BUF_SIZE);
        yypush_buffer_state(buf);       // push current buffer, use new buffer
    }
    .|\n                      ECHO;
    <<EOF>>                   {
        fclose(yyin);                   // close current input
        yypop_buffer_state();           // delete current buffer, pop buffer
        if (!YY_CURRENT_BUFFER)         // no buffer, we're done
          yyterminate();
        depth--;
    }

    %%
~~~
</div>


🔝 [Back to table of contents](#)


Start condition states                                         {#reflex-states}
----------------------

Start conditions are used to group rules and selectively activate rules when
the start condition state becomes active.

A rule with a pattern that is prefixed with one ore more start conditions will
only be active when the scanner is in one of these start condition states.

For example:

<div class="alt">
~~~{.cpp}
    <A,B>pattern1    action1
    <A>pattern2      action2
    <B>pattern3      action3
~~~
</div>

When the scanner is in state `A` rules 1 and 2 are active. When the scanner
is in state `B` rules 1 and 3 are active.

Start conditions are declared in \ref reflex-spec-defs (the first section) of
the lexer specification using <i>`%%state`</i> or <i>`%%xstate`</i> (or
<i>`%%s`</i> and <i>`%%x`</i> for short) followed by a space-separated list of
names called *start symbols*.  Start conditions declared with `%%s` are
*inclusive start conditions*.  Start conditions declared with <i>`%%x`</i> are
*exclusive start conditions*:

If a start condition is inclusive, then all rules without a start condition and
rules with the corresponding start condition will be active.

If a start condition is exclusive, only the rules with the corresponding start
condition will be active.

When declaring start symbol names it is recommended to use all upper case to
avoid name clashes with other Lexer class members.  For example, we cannot use
`text` as a start symbol name because `text()` is a Lexer method.  When option
`−−flex` is specified, start symbol names are macros for compatibility with
Lex/Flex.

The scanner is initially in the `INITIAL` start condition state.  The `INITIAL`
start condtion is inclusive: all rules without a start condition and those
prefixed with the `INITIAL` start condition are active when the scanner is in
the `INITIAL` start condition state.

The special start condition prefix `<*>` matches every start condition.
The prefix `<*>` is not needed for `<<EOF>>` rules, because unprefixed
`<<EOF>>` rules are always active as a special case.  The `<<EOF>>` pattern and
this exception were originally introduced by Flex.

For example:

<div class="alt">
~~~{.cpp}
    %s A
    %x X

    %%

    <A,X>pattern1    action1    // rule for states A and X
    <A>pattern2      action2    // rule for state A
    <X>pattern3      action3    // rule for state X
    <*>pattern4      action4    // rule for states INITIAL, A and X
    pattern5         action5    // rule for states INITIAL and A
    <<EOF>>          action6    // rule for states INITIAL, A and X

    %%
~~~
</div>

When the scanner is in state `INITIAL` rules 4, 5, and 6 are active.  When the
scanner is in state `A` rules 1, 2, 4, 5, and 6 are active.  When the scanner is
in state `X` rules 1, 3, 4, and 6 are active.  Note that `A` is inclusive
whereas `X` is exclusive.

To switch to a start condition state, use `start(START)` (or `BEGIN START` when
option `−−flex` is specified).  To get the current state use `start()` (or
`YY_START` when option `−−flex` is specified).  Switching start condition
states in your scanner allows you to create "mini-scanners" to scan portions of
the input that are syntactically different from the rest of the input, such as
comments:

<div class="alt">
~~~{.cpp}
    %x COMMENT

    %%

    "/*"    start(COMMENT);    // with −−flex use: BEGIN COMMENT;
    .|\n    echo();            // with −−flex use: ECHO;

    <COMMENT>[^*]*         // eat anything that is not a '*'
    <COMMENT>"*"+[^*/]*    // eat '*'s not followed by a '/'
    <COMMENT>"*"+"/"       start(INITIAL);
    <COMMENT><<EOF>>       std::cerr << "EOF in comment\n"; return 1;

    %%
~~~
</div>

Start symbols are actually integer values, where `INITIAL` is 0.  This means
that you can store a start symbol value in a variable.  You can also push the
current start condition on a stack and transition to start condition `START`
with `push_state(START)`.  To transition to a start condition that is on the
top of the stack and pop it use `pop_state()`.  The `top_state()` returns the
start condition that is on the top of the stack:

<div class="alt">
~~~{.cpp}
    %x COMMENT

    %%

    "/*"    push_state(COMMENT);
    .|\n    echo();

    <COMMENT>[^*]*         // eat anything that is not a '*'
    <COMMENT>"*"+[^*/]*    // eat '*'s not followed by a '/'
    <COMMENT>"*"+"/"       pop_state();
    <COMMENT><<EOF>>       std::cerr << "EOF in comment\n"; return 1;

    %%
~~~
</div>

When many rules are prefixed by the same start conditions, you can simplify
the rules by placing them in a *start condition scope*:

<div class="alt">
~~~{.cpp}
    <COMMENT>{
    [^*]*         // eat anything that is not a '*'
    "*"+[^*/]*    // eat '*'s not followed by a '/'
    "*"+"/"       start(INITIAL);
    <<EOF>>       std::cerr << "EOF in comment\n"; return 1;
    }
~~~
</div>

Start condition scopes may be nested.  A nested scope extends the scope of
start conditions that will be associated with the rules in the nested scope.

For example:

<div class="alt">
~~~{.cpp}
    %s A
    %x X

    %%

    pattern    action    // rule for states INITIAL and A

    <A>{
    pattern    action    // rule for state A
    <X>{
    pattern    action    // rule for states A and X
    }
    }

    <X>{
    pattern    action    // rule for state X
    <A>{
    pattern    action    // rule for states A and X
    }
    }

    %%
~~~
</div>

Designating a start condition as inclusive or exclusive is effective only for
rules that are not associated with a start condition scope.  That is, inclusive
start condition states are implicitly associated with rules unless a rule has a
start condition scope that explicitly associates start condition states with
the rule.

RE/flex extends the syntax of start conditions scopes beyond Flex syntax,
allowing the removal of start conditions from the current scope.  A start
condition name prefixed with the `^` operator is removed from the current
scope:

<div class="alt">
~~~{.cpp}
    %s A B C

    %%

    <B,C>{
    pattern    action    // rule for states B and C
    <A,^B>{
    pattern    action    // rule for states A and C
    }
    }

    <*,^A,^C>pattern    action    // rule for states INITIAL and B (all states except A and C)

    %%
~~~
</div>

Note that scopes should be read from outer to inner scope, and from left to
right in a `<...>` scope declaration.  This means that `<*,^A,^C>` first
extends the scope to include all start conditions and then removes `A` and `C`.

A start condition cannot be removed when it is not included in the current
scope.  For example, `<*,^A>` is correct but `<^A,*>` is incorrect when used as
a top-level scope.

Empty `<>` without start condition states cannot be specified because this is a
valid regex pattern.  To remove all states from a scope use `<^*>`.  This
construct is only useful when the empty scope is extended by start conditions
specified in sub-scopes.

@note Contrary to some Flex manuals, rules cannot be indented in a start
condition scope.  When a code block is indented at the begin of a start
condition scope it is considered an initial code block, see \ref
reflex-code-blocks.  All indented lines are considered actions.  Note that
<i>`%%option freespace`</i> allows patterns to be indented.  With this option
all action code blocks must be bracketed.

🔝 [Back to table of contents](#)


Initial code blocks                                       {#reflex-code-blocks}
-------------------

An initial code block may be placed at the start of the rules section or in a
condition scope.  This code block is executed each time the scanner is invoked
(i.e. when `lex()` or `yylex()` is called) before matching a pattern.  Initial
code blocks may be associated with start condition states as follows:

<div class="alt">
~~~{.cpp}
    %s A
    %x X

    %%

    %{
      // Code block for all inclusive states (INITIAL and A)
    %}
    pattern    action    // rule for states INITIAL and A
    ...
    pattern    action    // rule for states INITIAL and A

    <X>{
    %{
      // Code block for state X
    %}
    pattern    action    // rule for state X
    ...
    pattern    action    // rule for state X
    }

    <*>{
    %{
      // Code block for all inclusive and exclusive states (INITIAL, A, and X)
    %}
    pattern    action    // rule for states INITIAL, A, and X
    ...
    pattern    action    // rule for states INITIAL, A, and X
    }

    %%
~~~
</div>

Initial code blocks should be indented or should be placed within
<i>`%{ %}`</i> blocks.

An initial code block can be used to configure the lexer's matcher, since a
new matcher with the lexer patterns is created by the lexer just before the
rules are matched.  For example:

<div class="alt">
~~~{.cpp}
    %class{
      bool init_matcher;
    }

    %init{
      init_matcher = true;
    }

    %%

    %{
      if (init_matcher)            // init the new matcher?
      {
        init_matcher = false;      // init only once
        if (BUFFER_ALL)            // buffer all input at once?
          matcher().buffer();      // same as %option batch
        else if (INTERACTIVE)      // console-based (TTY) "interactive" input?
          matcher().interactive(); // same as %option interactive   
        matcher().tabs(4);         // same as %option tabs=4
      }
    %}
~~~
</div>

🔝 [Back to table of contents](#)


## 与Bison/Yacc的接口                                     {#reflex-bison}

[Bison](dinosaur.compilertools.net/#bison)工具生成解析器，它调用全局的C函数
`yylex()`获得下一个标记（token）。标记是`yylex()`返回的一个整数。

<b>`reflex`</b> 使用选项 `−−bison` 支持bison。此选项生成一个带有一个全局lexer
对象`YY_SCANNER`和一个全局函数`YY_EXTERN_C int yylex()`的扫描器。当Bison是用
C编译，但扫描器是用C++编译的时，必须在lex规则脚本中设置`YY_EXTERN_C`为
`extern "C"`，以启用C链接规则：

<div class="alt">
~~~{.cpp}
    %top{
      #include "y.tab.h"               /* include y.tab.h generated by bison */
      #define YY_EXTERN_C extern "C"   /* yylex() must use C linkage rules */
    }

    %option noyywrap bison

    %%

    [0-9]+               yylval.num = strtol(text(), NULL, 10);
                         return CONST_NUMBER;
    \"([^\\"]|\\")*\"    yylval.str = text();
                         return CONST_STRING;

    %%
~~~
</div>

注意选项`−−noyywrap`可以删除对全局函数`yywrap()`的依赖，这个函数并没有被定义。

这个示例设置全局变量`yylval.num`为扫描的整数或者`yylval.str`为扫描的字符串。
它假设Bison/Yacc语法文件定义了标记`CONST_NUMBER`和`CONST_STRING`以及`yylval`
的类型`YYSTYPE`。例如:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) */

    %{
      extern int yylex(void);
    %}

    %union {         // YYSTYPE yylval is a union:
      int num;       // yylval.num
      char* str;     // yylval.str
    }

    %token <num> CONST_NUMBER
    %token <str> CONST_STRING

    %%
    ...  // grammar rules
    %%
~~~
</div>

`YYSTYPE`是由Bison定义的一个联合，或者你可以在lexer规则脚本中把它作为一个选项
设置<i>`%%option YYSTYPE=type`</i>。

When option `−−flex` is specified with `−−bison`, the `yytext`, `yyleng`,
and `yylineno` globals are accessible to the Bison/Yacc parser.  In fact, all
Flex actions and variables are globally accessible (outside \ref reflex-spec-rules
of the lexer specification) with the exception of `yy_push_state`,
`yy_pop_state`, and `yy_top_state` that are class methods.  Furthermore, `yyin`
and `yyout` are macros and cannot be (re)declared or accessed as global
variables, but these can be used as if they are variables to assign a new input
source and to set the output stream.  To avoid compilation errors when using
globals such as `yyin`, use <b>`reflex`</b> option `−−header-file` to generate a
header file <i>`lex.yy.h`</i> to include in your code.  Finally, in code
outside of \ref reflex-spec-rules you must use `yyinput()` instead of
`input()`, use the global action `yyunput()` instead of `unput()`, and use the
global action `yyoutput()` instead of `output()`.

See the generated <i>`lex.yy.cpp`</i> "BISON" section, which contains
declarations specific to Bison when the `−−bison` option is specified.

### Bison解析器与reflex扫描器一起工作的方法

Bison解析器与reflex扫描器一起工作的方法有两种：第一种，Bison/Yacc语法文件应该
包括我们需要从扫描程序导入的extern：

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) assuming C with externs defined by the scanner using YY_EXTERN_C */

    %{
      extern int yylex(void);
      extern char *yytext;
      extern yy_size_t yyleng;
      extern int yylineno;
    %}

    %%
    ...  // grammar rules
    %%
~~~
</div>

更好的方法是：生成一个头文件 <i>`lex.yy.h`</i>，通过使用选项`−−header-file`，
然后在Bison/Yac语法文件中使用这个头文件：

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) for C++ */

    %{
      #include "lex.yy.h"
    %}

    %%
    ...  // grammar rules
    %%
~~~
</div>

第二种方法需要生成的解析器用C++编译，因为<i>`lex.yy.h`</i>包含C++声明。

@note The Flex macro `YY_DECL` is not supported by RE/flex.  The `YY_DECL`
macro is used or defined by Flex to redeclare the `yylex()` function signature,
See \ref YYDECL for more information.

🔝 [Back to table of contents](#)


### Bison and thread-safety                             {#reflex-bison-mt-safe}

Bison and Yacc are not thread-safe because the generated code uses and updates
global variables.  Yacc and Bison use the global variable `yylval` to exchange
token values.  By contrast, thread-safe reentrant Bison parsers pass the
`yylval` to the `yylex()` function as a parameter.  RE/flex supports all of
these Bison-specific features.

The following combinations of options are available to generate scanners for
Bison:

  Options                                         | Method                                                     | Global functions and variables
  ----------------------------------------------- | ---------------------------------------------------------- | ------------------------------
  &nbsp;                                          | `int Lexer::lex()`                                         | no global variables, but doesn't work with Bison
  `−−flex`                                        | `int yyFlexLexer::yylex()`                                 | no global variables, but doesn't work with Bison
  `−−bison`                                       | `int Lexer::lex()`                                         | `Lexer YY_SCANNER`, `int yylex()`, `YYSTYPE yylval`
  `−−flex` `−−bison`                              | `int yyFlexLexer::yylex()`                                 | `yyFlexLexer YY_SCANNER`, `int yylex()`, `YYSTYPE yylval`, `char *yytext`, `yy_size_t yyleng`, `int yylineno`
  `−−bison` `−−reentrant`                         | `int Lexer::lex()`                                         | `int yylex(yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−flex` `−−bison` `−−reentrant`                | `int yyFlexLexer::lex()`                                   | `int yylex(yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)`
  `−−bison-locations`                             | `int Lexer::lex(YYSTYPE& yylval)`                          | `Lexer YY_SCANNER`, `int yylex(YYSTYPE *yylval, YYLTYPE *yylloc)` 
  `−−flex` `−−bison-locations`                    | `int yyFlexLexer::yylex(YYSTYPE& yylval)`                  | `yyFlexLexer YY_SCANNER`, `int yylex(YYSTYPE *yylval, YYLTYPE *yylloc)` 
  `−−bison-bridge`                                | `int Lexer::lex(YYSTYPE& yylval)`                          | `int yylex(YYSTYPE *yylval, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)`
  `−−flex` `−−bison-bridge`                       | `int yyFlexLexer::yylex(YYSTYPE& yylval)`                  | `int yylex(YYSTYPE *yylval, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−bison-bridge` `−−bison-locations`            | `int Lexer::lex(YYSTYPE& yylval)`                          | `int yylex(YYSTYPE *yylval, YYLTYPE *yylloc, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−flex` `−−bison-bridge` `−−bison-locations`   | `int yyFlexLexer::yylex(YYSTYPE& yylval)`                  | `int yylex(YYSTYPE *yylval, YYLTYPE *yylloc, yyscan_t)`, `void yylex_init(yyscan_t*)`, `void yylex_destroy(yyscan_t)` 
  `−−bison-cc`                                    | `int Lexer::yylex(YYSTYPE *yylval)`                        | no global variables
  `−−flex` `−−bison-cc`                           | `int yyFlexLexer::yylex(YYSTYPE *yylval)`                  | no global variables
  `−−bison-cc` `−−bison-locations`                | `int Lexer::yylex(YYSTYPE *yylval, YYLTYPE *yylloc)`       | no global variables
  `−−flex` `−−bison-cc` `−−bison-locations`       | `int yyFlexLexer::yylex(YYSTYPE *yylval, YYLTYPE *yylloc)` | no global variables
  `−−bison-complete`                              | `PARSER::symbol_type Lexer::yylex()`                       | no global variables
  `−−flex` `−−bison-complete`                     | `PARSER::symbol_type yyFlexLexer::yylex()`                 | no global variables
  `−−bison-complete` `−−bison-locations`          | `PARSER::symbol_type Lexer::yylex()`                       | no global variables
  `−−flex` `−−bison-complete` `−−bison-locations` | `PARSER::symbol_type yyFlexLexer::yylex()`                 | no global variables

Option `−−prefix` may be used with option `−−flex` to change the prefix of the
generated `yyFlexLexer` and `yylex`.  This option may be combined with option
`−−bison` to also change the prefix of the generated `yytext`, `yyleng`, and
`yylineno`.

Furthermore, <b>`reflex`</b> options `−−namespace=NAME`, `−−lexer=LEXER` and
`−−lex=LEX` can be used to add a C++ namespace, to rename the lexer class
(`Lexer` or `yyFlexLexer` by default) and to rename the lexer function (`lex`
or `yylex` by default), respectively.

For option `−−bison-complete` the lexer function return type is the parser's
`symbol_type` as defined in the Bison grammar specification.  The parser class
is specified with option `−−bison-cc-parser=PARSER` and an optional namespace
may be specified with `−−bison-cc-namespace=NAME`.  The lexer function return
type may also be explicitly specified with option `−−token-type=TYPE`.

The following sections explain the `−−bison-cc`, `−−bison-complete`, `−−bison-bridge`,
`−−bison-locations`, and `−−reentrant` options for <b>`reflex`</b>.

Additional parameters may be passed to `lex()` and `yylex()` by declaring
<i>`%%option params="extra parameters"`</i> in the lexer specification.
See \ref YYDECL.

🔝 [Back to table of contents](#)


### Bison-cc                                                 {#reflex-bison-cc}

The <b>`reflex`</b> option `−−bison-cc` expects a Bison 3.0
<i>`%%skeleton "lalr1.cc"`</i> C++ parser that is declared as follows in a
Bison grammar file:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.yxx file) for C++ */

    %require  "3.0"
    %skeleton "lalr1.cc"

    %code requires{
      namespace yy {
        class Lexer;  // Generated by reflex with namespace=yy lexer=Lexer lex=yylex
      }
    }

    %defines

    %parse-param { yy::Lexer& lexer }  // Construct parser object with lexer

    %code{
      #include "lex.yy.h"  // header file generated with reflex --header-file
      #undef yylex
      #define yylex lexer.yylex  // Within bison's parse() we should invoke lexer.yylex(), not the global yylex()
    }

    %union {      // yy::parser::semantic_type yylval is a union:
      int num;    // type of yylval.num is int
      char* str;  // type of yylval.str is char*
    }

    %token <num> CONST_NUMBER  // This defines yy::parser::token::CONST_NUMBER
    %token <str> CONST_STRING  // This defines yy::parser::token::CONST_STRING

    %%
    ...  // grammar rules
    %%

    void yy::parser::error(const std::string& msg)
    {
      std::cerr << msg << std::endl;
    }
~~~
</div>

With the `−−bison-cc` option of <b>`reflex`</b>, the `yylex()` function takes a
`yy::parser::semantic_type yylval` argument that makes the `yylval` visible
in the lexer rules to assign semantic values to.

The scanner is generated with <b>`reflex`</b> options `−−bison-cc`,
`−−namespace=yy` and `−−lexer=Lexer`.  The lexer specification should
`#include` the Bison-generated header file to ensure that the
`yy::parser::token` enums `CONST_NUMBER` and `CONST_STRING` are defined.

Using the code above, we can now initialize a Bison parser.  We first should
create a scanner and pass it to the `parser` constructor as follows:

<div class="alt">
~~~{.cpp}
    yy::Lexer lexer(std::cin);  // read from stdin (or a stream, string or FILE)
    yy::parser parser(lexer);
    if (parser.parse() != 0)
      ... // error
~~~
</div>

We use options `−−bison-cc-namespace=NAME` and `−−bison-cc-parser=NAME` to
specify the namespace and parser class name of the Bison 3.0
<i>`%%skeleton "lalr1.cc"`</i> C++ parser you are generating with Bison.  These
are `yy` and `parser` by default, respectively. For option
`−−bison-cc-namespace=NAME` the `NAME` can be a list of nested namespaces of
the form `NAME1::NAME2::NAME3` or by separating the names by a dot as in
`NAME1.NAME2.NAME3`.

🔝 [Back to table of contents](#)


### Bison-cc & locations                           {#reflex-bison-cc-locations}

The <b>`reflex`</b> option `−−bison-cc` with `−−bison-locations` expects a
Bison 3.0 <i>`%%skeleton "lalr1.cc"`</i> C++ parser that is declared as follows
in a Bison grammar file:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.yxx file) for C++ */

    %require  "3.0"
    %skeleton "lalr1.cc"

    %code requires{
      namespace yy {
        class Lexer;  // Generated by reflex with namespace=yy lexer=Lexer
      }
    }

    %defines

    %locations

    %parse-param { yy::Lexer& lexer }  // Construct parser object with lexer

    %code{
      #include "lex.yy.h"  // header file generated with reflex --header-file
      #undef yylex
      #define yylex lexer.yylex  // Within bison's parse() we should invoke lexer.yylex(), not the global yylex()
    }

    %union {      // yy::parser::semantic_type yylval is a union:
      int num;    // type of yylval.num is int
      char* str;  // type of yylval.str is char*
    }

    %token <num> CONST_NUMBER  // This defines yy::parser::token::CONST_NUMBER
    %token <str> CONST_STRING  // This defines yy::parser::token::CONST_STRING

    %%
    ...  // grammar rules
    %%

    void yy::parser::error(const location_type& loc, const std::string& msg)
    {
      std::cerr << msg << " at " << loc << std::endl;
    }
~~~
</div>

With the `−−bison-cc` and `−−bison-locations` options of <b>`reflex`</b>, the
`yylex()` function takes `yy::parser::semantic_type yylval` as the first
argument that makes the `yylval` visible in the lexer rules to assign semantic
values to.   The second argument `yy::location yylloc` is set automatically by
by invoking the lexer's `yylloc_update()` in `yylex()` to update the line and
column of the match.  The auto-generated virtual `yylloc_update()` method can
be overriden by a user-defined lexer class that extends `Lexer` (or extends
`yyFlexLexer` when option `−−flex` is specified).

The scanner is generated with <b>`reflex`</b> options `−−bison-cc`,
`−−bison-locations`, `−−namespace=yy` and `−−lexer=Lexer`.  The lexer
specification should `#include` the Bison-generated header file to ensure that
the `yy::parser::token` enums `CONST_NUMBER` and `CONST_STRING` are defined.

Using the code above, we can now initialize a Bison parser.  We first should
create a scanner and pass it to the `parser` constructor as follows:

<div class="alt">
~~~{.cpp}
    yy::Lexer lexer(std::cin);  // read from stdin (or a stream, string or FILE)
    yy::parser parser(lexer);
    if (parser.parse() != 0)
      ... // error
~~~
</div>

🔝 [Back to table of contents](#)


### Bison-complete                                     {#reflex-bison-complete}

The <b>`reflex`</b> option `−−bison-complete` expects a Bison 3.2 C++ parser
which uses both <i>`%%define api.value.type variant`</i> and
<i>`%%define api.token.constructor`</i>.  This parser defines the type
`symbol_type` variant and the parser expects `yylex` to have the type
`yy::parser::symbol_type  yylex()`.  Here is an example Bison 3.2 C++ complete
symbols grammar file:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.yxx file) for C++ */

    %require "3.2"
    %language "c++"

    %define api.namespace {yy}
    %define api.parser.class {parser}
    %define api.value.type variant
    %define api.token.constructor

    %defines
    %output "parser.cpp"

    %code requires{
      namespace yy {
        class Lexer;  // Generated by reflex with namespace=yy lexer=Lexer lex=yylex
      }
    }

    %parse-param { yy::Lexer& lexer }  // Construct parser object with lexer

    %code{
      #include "lex.yy.h"  // header file generated with reflex --header-file
      #undef yylex
      #define yylex lexer.yylex  // Within bison's parse() we should invoke lexer.yylex(), not the global yylex()
    }

    %define api.token.prefix {TOK_}
    %token <std::string> IDENTIFIER "identifier"  // This defines TOK_IDENTIFIER
    %token <int> NUMBER "number"                  // This defines TOK_NUMBER
    %token EOF 0 "end of file"                    // This defines TOK_EOF with value 0

    %%
    ...  // grammar rules
    %%

    void yy::parser::error(const std::string& msg)
    {
      std::cerr << msg << std::endl;
      if (lexer.size() == 0)      // if token is unknown (no match)
        lexer.matcher().winput(); // skip character
    }
~~~
</div>

With the `−−bison-complete` option of <b>`reflex`</b>, the `yylex()` function
takes no arguments by default and returns a value of type
`yy::parser::symbol_type`.  This means that the lexer's action should return
values of this type, constructed with `yy::parser::symbol_type` or with
`make_TOKENNAME` as follows:

<div class="alt">
~~~{.cpp}
    %top{
    #include "parser.hpp"  /* Generated by bison. */
    }

    %option bison-complete
    %option bison-cc-namespace=yy
    %option bison-cc-parser=parser

    %option exception="yy::parser::syntax_error(\"Unknown token.\")"

    %option namespace=yy
    %option lexer=Lexer

    %%
    \s+      // skip space
    [a-z]+   return yy::parser::make_IDENTIFIER(str());
    [0-9]+   return yy::parser::make_NUMBER(atoi(text()));
    ":"      return yy::parser::symbol_type(':');
    <<EOF>>  return yy::parser::make_EOF();
    %%
~~~
</div>

The scanner is generated with <b>`reflex`</b> options `−−bison-complete`,
`−−namespace=yy` and `−−lexer=Lexer`.  Option `−−bison-complete` automatically
defines the appropriate token type `symbol_type` depending on
`−−bison-cc-namespace` and on `−−bison-cc-parser`.  We also used options
`−−bison-cc-namespace=NAME` and `−−bison-cc-parser=NAME` to specify the
namespace and parser class name of the Bison 3.2 C++ parser.  These are `yy`
and `parser` by default, respectively (<i>`%%define api.namespace {yy}`</i> and
<i>`%%define api.parser.class {parser}`</i> are actually superfluous in the
example grammer specification because their values are the defaults).  We use
option `−−exception` to specify that the scanner's default rule should
throw a `yy::parser::syntax_error("Unknown token.")`.  This exception is caught
by the parser which calls `yy::parser::error` with the string
`"Unknown token."` as argument.

We have to be careful with option `−−exception`.  Because no input is consumed,
the scanner should not be invoked again or we risk looping on the unmatched
input.  Alternatively, we can define a "catch all else" rule with pattern `.`
that consumes the offending input:

<div class="alt">
~~~{.cpp}
    %%
    \s+      // skip space
    [a-z]+   return yy::parser::make_IDENTIFIER(str());
    [0-9]+   return yy::parser::make_NUMBER(atoi(text()));
    ":"      return yy::parser::symbol_type(':');
    <<EOF>>  return yy::parser::make_EOF();
    .        throw yy::parser::syntax_error("Unknown token.");
    %%
~~~
</div>

For option `−−bison-cc-namespace=NAME` the `NAME` may be a list of nested
namespaces of the form `NAME1::NAME2::NAME3` or by separating the names by a
dot as in `NAME1.NAME2.NAME3`.

Using the code above, we can now initialize a Bison parser in our main program.
We first should create a scanner and pass it to the `parser` constructor as
follows:

<div class="alt">
~~~{.cpp}
    yy::Lexer lexer(std::cin);  // read from stdin (or a stream, string or FILE)
    yy::parser parser(lexer);
    if (parser.parse() != 0)
      ... // error
~~~
</div>

Note that when the end of input is reached, the lexer returns
`yy::parser::make_EOF()` upon matching `<<EOF>>`.  This rule is optional.
When omitted, the return value is `yy::parser::symbol_type(0)`.

🔝 [Back to table of contents](#)


### Bison-complete & locations               {#reflex-bison-complete-locations}

The <b>`reflex`</b> option `−−bison-complete` expects a Bison 3.2 C++ parser
which uses both <i>`%%define api.value.type variant`</i> and
<i>`%%define api.token.constructor`</i>.  This parser defines the type
`symbol_type` variant and the parser expects `yylex` to have the type
`parser::symbol_type::yylex()`.  Here is an example Bison 3.2 C++ complete
symbols grammar file with Bison <i>`%%locations`</i> enabled:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.yxx file) for C++ */

    %require "3.2"
    %language "c++"

    %define api.namespace {yy}
    %define api.parser.class {parser}
    %define api.value.type variant
    %define api.token.constructor

    %define parse.error verbose

    %defines
    %output "parser.cpp"

    %locations
    %define api.location.file "location.hpp"

    %code requires{
      namespace yy {
        class Lexer;  // Generated by reflex with namespace=yy lexer=Lexer lex=yylex
      }
    }

    %parse-param { yy::Lexer& lexer }  // Construct parser object with lexer

    %code{
      #include "lex.yy.h"  // header file generated with reflex --header-file
      #undef yylex
      #define yylex lexer.yylex  // Within bison's parse() we should invoke lexer.yylex(), not the global yylex()
    }

    %define api.token.prefix {TOK_}
    %token <std::string> IDENTIFIER "identifier"  // This defines TOK_IDENTIFIER
    %token <int> NUMBER "number"                  // This defines TOK_NUMBER
    %token EOF 0 "end of file"                    // This defines TOK_EOF with value 0

    %%
    ...  // grammar rules, note that we can use @1, @2 etc as locations, like this:
    type ID : {
                if (exists($2))
                  error(@2, "Redefined identifier");
              }
    %%

    void yy::parser::error(const location& loc, const std::string& msg)
    {
      std::cerr << loc << ": " << msg << std::endl;
      if (lexer.size() == 0)      // if token is unknown (no match)
        lexer.matcher().winput(); // skip character
    }
~~~
</div>

With the `−−bison-complete` option of <b>`reflex`</b>, the `yylex()` function
takes no arguments by default and returns a value of type
`yy::parser::symbol_type`.  This means that the lexer's action should return
values of this type, constructed with `yy::parser::symbol_type` or with
`make_TOKENNAME` as follows:

<div class="alt">
~~~{.cpp}
    %top{
    #include "parser.hpp"    /* Generated by bison. */
    #include "location.hpp"  /* Generated by bison %locations. */
    }

    %option bison-complete
    %option bison-cc-namespace=yy
    %option bison-cc-parser=parser
    %option bison-locations

    %option exception="yy::parser::syntax_error(location(), \"Unknown token.\")"

    %option namespace=yy
    %option lexer=Lexer

    %%
    \s+      // skip space
    [a-z]+   return yy::parser::make_IDENTIFIER(str(), location());
    [0-9]+   return yy::parser::make_NUMBER(atoi(text()), location());
    ":"      return yy::parser::symbol_type(':', location());
    <<EOF>>  return yy::parser::make_EOF(location());
    %%
~~~
</div>

The scanner is generated with <b>`reflex`</b> options `−−bison-complete`,
`−−bison-locations`, `−−namespace=yy` and `−−lexer=Lexer`.  Option
`−−bison-complete` automatically defines the appropriate token type
`symbol_type` depending on `−−bison-cc-namespace` and on `−−bison-cc-parser`.
We also used options `−−bison-cc-namespace=NAME` and `−−bison-cc-parser=NAME`
to specify the namespace and parser class name of the Bison 3.2 C++ parser.
These are `yy` and `parser` by default, respectively (i.e.
`%define api.namespace {yy}` and `%define api.parser.class {parser}` are
actually superfluous in the example grammer specification because their values
are the defaults).  We use option `−−exception` to specify that the scanner's
default rule should throw a
`yy::parser::syntax_error(location(), "Unknown token.")`.  This exception is
caught by the parser which calls `yy::parser::error` with the value of
`location()` and the string `"Unknown token."` as arguments.  The
auto-generated virtual lexer class method `location()` method may be overriden
by a user-defined lexer class that extends `Lexer` (or extends `yyFlexLexer`
when option `−−flex` is specified).

We have to be careful with option `−−exception`.  Because no input is consumed,
the scanner should not be invoked again or we risk looping on the unmatched
input.  Alternatively, we can define a "catch all else" rule with pattern `.`
that consumes the offending input:

<div class="alt">
~~~{.cpp}
    %%
    \s+      // skip space
    [a-z]+   return yy::parser::make_IDENTIFIER(str(), location());
    [0-9]+   return yy::parser::make_NUMBER(atoi(text()), location());
    ":"      return yy::parser::symbol_type(':', location());
    <<EOF>>  return yy::parser::make_EOF(location());
    .        throw yy::parser::syntax_error(location(), "Unknown token.");
    %%
~~~
</div>

For option `−−bison-cc-namespace=NAME` the `NAME` may be a list of nested
namespaces of the form `NAME1::NAME2::NAME3` or by separating the names by a
dot as in `NAME1.NAME2.NAME3`.

Using the code above, we can now initialize a Bison parser in our main program.
We first should create a scanner and pass it to the `parser` constructor as
follows:

<div class="alt">
~~~{.cpp}
    FILE *f = fopen(filename, "r");
    if (f != NULL)
    {
      yy::Lexer lexer(f);        // read file, decode UTF-8/16/32 format
      lexer.filename = filename; // the filename to display with error locations
      yy::parser parser(lexer);
      if (parser.parse() != 0)
        ... // error
~~~
</div>

Note that when the end of input is reached, the lexer returns
`yy::parser::make_EOF()` upon matching `<<EOF>>`.  This rule is optional.
When omitted, the return value is `yy::parser::symbol_type(0, location())`.

🔝 [Back to table of contents](#)


### Bison-bridge                                         {#reflex-bison-bridge}

The <b>`reflex`</b> option `−−bison-bridge` expects a Bison "pure parser" that
is declared as follows in a Bison grammar file:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) for C or C++ */

    %{
      #include "lex.yy.h"
      #define YYPARSE_PARAM scanner
      #define YYLEX_PARAM   scanner
    %}

    %pure-parser

    %%
    ...  // grammar rules
    %%
~~~
</div>

@note `%%pure-parser` is deprecated and replaced with `%%define api.pure`.

With the `−−bison-bridge` option of <b>`reflex`</b>, the `yyscan_t` argument
type of `yylex()` is a `void*` type that passes the scanner object to this
global function (as defined by `YYPARSE_PARAM` and `YYLEX_PARAM`).  The
function then invokes this scanner's lex function.  This option also passes the
`yylval` value to the lex function, which is a reference to an `YYSTYPE` value.

Wtih the `−−bison-bridge` option two additional functions are generated that
should be used to create a new scanner and delete the scanner in your program:

<div class="alt">
~~~{.cpp}
    yyscan_t scanner = nullptr;
    yylex_init(&scanner);                // create a new scanner
    ...
    int token = yylex(&yylval, scanner); // scan with bison-bridge
    ...
    yylex_destroy(scanner);              // delete a scanner
    scanner = nullptr;
~~~
</div>

🔝 [Back to table of contents](#)


### Bison-locations                                   {#reflex-bison-locations}

The option `−−bison-locations` expects a Bison parser with the locations
feature enabled.  This feature provides line and column numbers of the matched
text for error reporting.  For example:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) for C or C++ */

    %{
      #include "lex.yy.h"
      void yyerror(const char*);
    %}

    %locations

    %union {         // YYSTYPE yylval is a union:
      int num;       // type of yylval.num is int
      char* str;     // type of yylval.str is char*
    }

    %{
      /* reflex option −−bison-locations makes yylex() take yylval and yylloc */
      extern int yylex(YYSTYPE*, YYLTYPE*);
      #define YYLEX_PARAM &yylval, &yylloc
    %}

    /* add &yylval and &yyloc parameters to yylex() with a trick: use YYLEX_PARAM */
    %lex-param { void *YYLEX_PARAM }

    %token <num> CONST_NUMBER
    %token <str> CONST_STRING

    %%
    ...  // grammar rules
    %%

    void yyerror(const char *msg)
    {
      fprintf(stderr, "%s at %d,%d to line %d,%d\n",
          msg,
          yylloc.first_line,
          yylloc.first_column,
          yylloc.last_line,
          yylloc.last_column);
    }
~~~
</div>

The `yylval` value is passed to the lex function.  The `yylloc` structure is
automatically updated by the RE/flex scanner, so you do not need to define a
`YY_USER_ACTION` macro as you have to with Flex.  Instead, this is done
automatically in `yylex()` by invoking the lexer's `yylloc_update()` to update
the line and column of the match.  The auto-generated virtual `yylloc_update()`
method may be overriden by a user-defined lexer class that extends `Lexer` (or
extends `yyFlexLexer` when option `−−flex` is specified).

Note that with the `−−bison-location` option, `yylex()` takes an additional
`YYLTYPE` argument that a Bison parser provides.  You can set `YYLTYPE` as
an option <i>`%%option YYLTYPE=type`</i> in a lexer specification.

🔝 [Back to table of contents](#)


### Bison-bridge & locations                   {#reflex-bison-bridge-locations}

Here is a final example that combines options `−−bison-locations` and
`−−bison-bridge`,  The Bison parser should be a Bison pure-parser with
locations enabled:

<div class="alt">
~~~{.cpp}
    /* yacc grammar (.y file) for C or C++ */

    %{
      #include "lex.yy.h"
      void yyerror(YYLTYPE*, yyscan_t, const char*);
      #define YYPARSE_PARAM scanner
      #define YYLEX_PARAM   scanner
    %}

    %locations
    %pure-parser
    %lex-param { void *scanner }
    %parse-param { void *scanner }

    %union {         // YYSTYPE yylval is a union:
      int num;       // yylval.num
      char* str;     // yylval.str
    }

    %token <num> CONST_NUMBER
    %token <str> CONST_STRING

    %%
    ...  // grammar rules
    %%

    void yyerror(YYLTYPE *yylloc, yyscan_t scanner, const char *msg)
    {
      fprintf(stderr, "%s at %d,%d to line %d,%d\n",
        msg,
        yylloc->first_line,
        yylloc->first_column,
        yylloc->last_line,
        yylloc->last_column);
    }
~~~
</div>

@note `%%pure-parser` is deprecated and replaced with `%%define api.pure`.

@note When Bison <i>`%%locations`</i> with <i>`%%define api.pure full`</i> is
used, `yyerror` has the signature `void yyerror(YYLTYPE *locp, char const
*msg)`.  This function signature is required to obtain the location information
with Bison pure-parsers.

@note Argument `yylval` is not a pointer but is passed by reference and should
be used as such in the scanner's rules.

@note Because `YYSTYPE` is declared by the parser, do not forget to add a
`#include "y.tab.h"` to the top of the specification of your lexer:

<div class="alt">
~~~{.cpp}
    %top{
      #include "y.tab.h"    /* include y.tab.h generated by bison */
    }
~~~
</div>

With the `−−bison-bridge` and `−−bison-location` options two additional
functions are generated that should be used to create a new scanner and delete
the scanner in your program:

<div class="alt">
~~~{.cpp}
    yyscan_t scanner = nullptr;
    yylex_init(&scanner);      // create a new scanner
    ...
    int token = yylex(&yylval, &yylloc, scanner); // scan with bison-bridge and bison-locations
    ...
    yylex_destroy(scanner);    // delete a scanner
    scanner = nullptr;
~~~
</div>

🔝 [Back to table of contents](#)


### Reentrant scanners                                      {#reflex-reentrant}

Option `-R` or `−−reentrant` may be used to generate a reentrant scanner that
is compatible with reentrant Flex and Bison.  This is mainly useful when you
combine `−−reentrant` with `−−flex` and `−−bison`.  See also \ref reflex-bison.

When using Bison with reentrant scanners, your code should create a `yyscan_t`
scanner object with `yylex_init(&scanner)` and destroy it with
`yylex_destroy(scanner)`.  Reentrant Flex functions take the scanner object as
an extra last argument, for example `yylex(scanner)`:

<div class="alt">
~~~{.cpp}
    yyscan_t scanner = nullptr;
    yylex_init(&scanner);       // create a new scanner
    ...
    int token = yylex(scanner); // reentrant scan
    ...
    yylex_destroy(scanner);     // delete a scanner
    scanner = nullptr;
~~~
</div>

Within a rules section we refer to the scanner with macro `yyscanner`, for
example:

<div class="alt">
~~~{.cpp}
    %%
    {integer}    yylval.i = atoi(yyget_text(yyscanner));
                 return INTEGER;
    {decimal}    yylval.d = atof(yyget_text(yyscanner));
                 return DECIMAL;
    .            printf("unknown char at line %d\n", yyget_lineno(yyscanner));
    %%
~~~
</div>

The following functions are available in a reentrant Flex scanner generated
with options `−−flex` and `−−reentrant`.  These functions take an extra
argument `yyscan_t s` that is either `yyscanner` when the function is used in a
rule or in the scope of a lexer method, or is a pointer to the lexer object
when the function is used outside the scope of a lexer method:

  Reentrant Flex action       | Result
  --------------------------- | -----------------------------------------------
  `yyget_text(s)`             | 0-terminated text match
  `yyget_leng(s)`             | size of the match in bytes
  `yyget_lineno(s)`           | line number of the match (>=1)
  `yyset_lineno(n, s)`        | set line number of the match to `n`
  `yyget_in(s)`               | get `reflex::Input` object
  `yyset_in(i, s)`            | set `reflex::Input` object
  `yyget_out(s)`              | get `std::ostream` object
  `yyset_out(o, s)`           | set output to `std::ostream o`
  `yyget_debug(s)`            | reflex option `-d` sets `n=1`
  `yyset_debug(n, s)`         | reflex option `-d` sets `n=1`
  `yyget_extra(s)`            | get user-defined extra parameter
  `yyset_extra(x, s)`         | set user-defined extra parameter
  `yyget_current_buffer(s)`   | the current matcher
  `yyrestart(i, s)`           | set input to `reflex::Input i`
  `yyinput(s)`                | get next 8-bit char from input
  `yyunput(c, s)`             | put back 8-bit char `c`
  `yyoutput(c, s)`            | output char `c`
  `yy_create_buffer(i, n, s)` | new matcher `reflex::Input i`
  `yy_delete_buffer(m, s)`    | delete matcher `m`
  `yypush_buffer_state(m, s)` | push current matcher, use `m`
  `yypop_buffer_state(s)`     | pop matcher and delete current
  `yy_scan_string(s)`         | scan string `s`
  `yy_scan_wstring(s)`        | scan wide string `s`
  `yy_scan_bytes(b, n)`       | scan `n` bytes at `b` (buffered)
  `yy_scan_buffer(b, n)`      | scan `n`-1 bytes at `b` (zero copy)
  `yy_push_state(n, s)`       | push current state, go to state `n`
  `yy_pop_state(s)`           | pop state and make it current
  `yy_top_state(s)`           | get top state start condition

With respect to the `yyget_extra` functions, a scanner object has a
`YY_EXTRA_TYPE yyextra` value that is user-definable.  You can define the type
in a lexer specification with the `extra-type` option:

<div class="alt">
~~~{.cpp}
    %option flex bison reentrant
    %option extra-type="struct extra"
    struct extra { ... }; // type of the data to include in a FlexLexer
~~~
</div>

This mechanism is somewhat crude as it originates with Flex' C legacy to add
extra user-defined values to a scanner class.  Because <b>`reflex`</b> is C++,
it is recommended to define a derived class that extends the `Lexer` or
`FlexLexer` class, see \ref reflex-inherit.

Because scanners are C++ classes, the `yyscanner` macro is essentially the same
is the `this` pointer.  Outside the scope of lexer methods a pointer to your
`yyFlexLexer lexer` object should be used instead.

🔝 [Back to table of contents](#)


### YY_DECL alternatives                                              {#YYDECL}

The Flex macro `YY_DECL` is not supported by RE/flex.  The `YY_DECL` macro is
typically used with FLex to (re)declare the `yylex()` function signature, for
example to take an additional `yylval` parameter that must be passed through
from `yyparse()` to `yylex()`.  Note that the standard cases are already
covered by RE/flex using options such as `−−bison-cc`, `−−bison-bridge` and
`−−bison-locations` that pass additional parameters to the scanner function
invoked by the Bison parser.

There are two better alternatives to `YY_DECL`:

1. Option <i>`%%option params="TYPE NAME, ..."`</i> can be defined in the lexer
   specification to pass additional parameters to the lexer function `lex()`
   and `yylex()`.  One or more corresponding Bison <i>%%param</i> declarations
   should be included in the grammar specification, to pass the extra
   parameters via `yyparse()` to `lex()`/`yylex()`.  Also `yyerror()` is
   expected to take the extra parameters.

2. The generated lexer class defined with <i>`%%class{ }`</i> can be extended
   with member declarations to hold state information, such as token-related
   values.  These values can then be exchanged with the parser by accessing
   them within the parser.

The first alternative matches the `YY_DECL` use cases.  The comma-separated
list of additional parameters <i>`"TYPE NAME, ..."`</i> are added to the end of
the parameter list of the generated `lex()`/`yylex()` function.  The second
alternative uses a stateful approach.  The values are stored in the scanner
object and can be made accessible beyond the scopes of the scanners's rule
actions and the parser's semantic actions.

See also \ref reflex-lexer.

🔝 [Back to table of contents](#)


Searching versus scanning                                      {#reflex-search}
-------------------------

RE/flex generates an efficient search engine with option `-S` (or `−−find`).
The generated search engine finds all matches while ignoring unmatched input
silently, which is different from scanning, which matches all input piece-wise.

Searching with this option is more efficient than scanning with a "catch all
else" dot-rule to ignore unmatched input, as in:

<div class="alt">
~~~{.cpp}
    .    // no action, ignore unmatched input
~~~
</div>

The problem with this rule is that it is invoked for every single unmatched
character on the input, which is inefficient and slows down searching for
matching patterns significantly when more than a few unmatched characters are
encountered in the input.  Note that we cannot use `.+` to match longer
patterns because this overlaps with other patterns and is also likely longer
than the other patterns, i.e. the rule subsumes those patterns.

Unless the input contains relatively few unmatched characters or bytes to
ignore, option `-S` (or `−−find`) speeds up searching and matching
significantly.  This option applies the following optimizations to the RE/flex
FSM matcher:

- Hashing is used to match multiple strings, which is faster than multi-string
  matching with Aho-Corasick, Commentz-Walter, Wu-Manber, and other.

- Single short strings are searched with `memchr()`.  Single long strings are
  searched with Boyer-Moore-Horspool.  Also regex patterns with common prefixes
  are searched efficiently, e.g. the regex `reflex|regex|regular` has common
  prefix string `"re"` that is searched in the input first, then hashing is
  used to predict a match for the part after `"re"`, followed by regex matching
  with the FSM.

With option `-S` (or `−−find`), a "catch all else" dot-rule should not be
defined, since unmatched input is already ignored with this option and
defining a "catch all else" dot-rule actually slows down the search.

@note By contrast to option `-S` (or `−−find`), option `-s` (or `−−nodefault`)
cannot be used to ignore unmatched input.  Option `-s` produces runtime errors
and exceptions for unmatched input.

This option only applies to the RE/flex matcher and can be combined with
options `-f` (or `−−full`) and `-F` (or `−−fast`) to further increase
performance.

🔝 [Back to table of contents](#)


POSIX versus Perl matching                                 {#reflex-posix-perl}
--------------------------

The <b>`reflex`</b> scanner generator gives you a choice of matchers to use in
the generated scanner, where the default is the POSIX RE/flex matcher engine.
Other options are the Boost.Regex matcher in POSIX mode or in Perl mode.

To use a matcher for the generated scanner, use one of these three choices:

  Option          | Matcher class used  | Mode  | Engine      
  --------------- | ------------------- | ----- | ----------------------------- 
  `-m reflex`     | `Matcher`           | POSIX | RE/flex lib (default choice)
  `-m boost`      | `BoostPosixMatcher` | POSIX | Boost.Regex 
  `-m boost-perl` | `BoostPerlMatcher`  | Perl  | Boost.Regex 
  `-m pcre2-perl` | `PCRE2Matcher`      | Perl  | PCRE2

The POSIX matchers look for the *longest possible match* among the given set of
alternative patterns.  Perl matchers look for the *first match* among the given
set of alternative patterns.

POSIX is generally preferred for scanners, since it is easier to arrange rules
that may have partially overlapping patterns.  Since we are looking for the
longest match anyway, it does not matter which rule comes first.  The order
does not matter as long as the length of the matches differ.  When matches are
of the same length because multiple patterns match, then the first rule is
selected.

Consider for example the following `lexer.l` specification if a lexer with
rules that are intended to match keywords and identifiers in some input text:

<div class="alt">
~~~{.cpp}
    %%

    int                     out() << "=> int keyword\n;
    interface               out() << "=> interface keyword\n;
    float                   out() << "=> float keyword\n;
    [A-Za-z][A-Za-z0-9]*    out() << "=> identifier\n";

    %%
~~~
</div>

When the input to the scanner is the text `integer`, a POSIX matcher selects
the last rule that matches it by *leftmost longest matching* policy.  This
matching policy selects the rule that matches the longest text.  If more than
one pattern matches the same length of text then the first pattern that matches
takes precedence.  This is what we want because it is an identifier in our
example programming language:

    reflex -m reflex −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex
    echo "integer" | ./lexer
    => identifier

By contrast, a Perl matcher uses a *greedy matching* policy, which selects the
first rule that matches.  In this case it matches the first part `int` of the
text `integer` and leaves `erface` to be matched next as an identifier:

    reflex -m boost-perl −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex -lboost_regex
    echo "integer" | ./lexer
    => int keyword
    => identifier

Note that the same greedy matching happens when the text `interface` is
encountered on the input, which we want to recognize as a separate keyword and
not match against `int`:

    reflex -m boost-perl −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex -lboost_regex
    echo "interface" | ./lexer
    => int keyword
    => identifier

Switching the rules for `int` and `interface` fixes that specific problem.

<div class="alt">
~~~{.cpp}
    %%

    interface               out() << "=> interface keyword\n;
    int                     out() << "=> int keyword\n;
    float                   out() << "=> float keyword\n;
    [A-Za-z][A-Za-z0-9]*    out() << "=> identifier\n";

    %%
~~~
</div>

    reflex -m boost-perl −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex -lboost_regex
    echo "interface" | ./lexer
    => interface keyword

But we cannot do the same to fix matching `integer` as an identifier: when
moving the last rule up to the top we cannot match `int` and `interface` any
longer!

<div class="alt">
~~~{.cpp}
    %%

    [A-Za-z][A-Za-z0-9]*    out() << "=> identifier\n";
    interface               out() << "=> interface keyword\n;
    int                     out() << "=> int keyword\n;
    float                   out() << "=> float keyword\n;

    %%
~~~
</div>

    reflex -m boost-perl −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex -lboost_regex
    echo "int" | ./lexer
    => identifier
    echo "interface" | ./lexer
    => identifier

Basically, a Perl matcher works in an *operational* mode by working the regex
pattern as a sequence of *operations* for matching, usually using backtracking
to find a matching pattern.

Perl matchers generally support lazy quantifiers and group captures, while most
POSIX matchers do not (e.g. Boost.Regex in POSIX mode does not support lazy
quantifiers).  The RE/flex POSIX matcher supports lazy quantifiers, but not
group captures.  The added support for lazy quantifiers and word boundary
anchors in RE/flex matching offers a reasonably new and useful feature for
scanners that require POSIX mode matching.

To prevent a Perl matcher from matching a keyword when an identifier starts
with the name of that keyword, we could use a lookahead pattern such as
`int(?=[^A-Za-z0-9_])` which is written in a lexer specification with a
trailing context `int/[^A-Za-z0-9_]` with the `/` lookahead meta symbol.

A POSIX matcher on the other hand is *declarative* with a deeper foundation
in formal language theory.  An advantage of POSIX matchers is that a regular
expression can always be compiled to a deterministic finite state machine for
efficient matching.

POSIX matching still requires the `int` matching rule before the identifier
matching rule, as in the original lexer specification shown in this section.
Otherwise an `int` on the input will be matched by the identifier rule.

Lookaheads can also be used with POSIX matchers to prioratize rules.  Adding
a lookahead lengthens the pattern while keeping only the part that matches
before the lookahead.  For example, the following lexer specification
attempts to remove leading `0` from numbers:

<div class="alt">
~~~{.cpp}
    %%

    0                       // no action
    [0-9]+                  out() << text() << std::endl;

    %%
~~~
</div>

However, in POSIX mode the first rule only matches if the text is exactly one
`0` because the second rule matches longer texts.  The trick here is to use a
trailing context with the first rule as follows:

<div class="alt">
~~~{.cpp}
    %%

    0/[0-9]+                // no action
    [0-9]+                  out() << text() << std::endl;

    %%
~~~
</div>

    reflex -m reflex −−main lexer.l
    c++ -o lexer lex.yy.cpp -lreflex
    echo "00123" | ./lexer
    => 123
    echo "0" | ./lexer
    => 0

🔝 [Back to table of contents](#)


Debugging and profiling                                         {#reflex-debug}
-----------------------

There are several <b>`reflex`</b> options to debug a lexer and analyze its
performance given some input text to scan:

- Option `-d` (or `−−debug`) generates a scanner that prints the matched text,
  which allows you to debug your patterns.

- Option `-p` (or `−−perf-report`) generates a scanner that profiles the
  performance of your lexer and the lexer rules executed, which allows you to
  find hotspots and performance bottlenecks in your rules.

- Option `-s` (or `−−nodefault`) suppresses the default rule that echoes all
  unmatched text when no rule matches.  The scanner reports "scanner jammed"
  when no rule matches.  Without the `−−flex` option, a `std::runtime`
  exception is thrown.

- Option `-v` (or `−−verbose`) displays a summary of scanner statistics.

🔝 [Back to table of contents](#)

### Debugging

Option `-d` generates a scnner that prints the matched text while scanning
input.  The output displayed is of the form:

    −−accepting rule at line NNN ("TEXT")

where NNN is the line number of the pattern in the lexer specification and TEXT
is the matched text.

🔝 [Back to table of contents](#)

### Profiling

Option `-p` generates a scanner that profiles the performance of your lexer.
The performance report shows the performance statistics obtained for each
pattern defined in the lexer specification, i.e. the number of matches per
pattern, the total text length of the matches per pattern, and the total time
spent matching and executing the rule corresponding to the pattern.  The
performance profile statistics are collected when the scanner runs on some
given input.  Profiling allows you to effectively fine-tune the performance of
your lexer by focussing on patterns and rules that are frequently matched that
turn out to be computationally expensive.

This is perhaps best illustrated with an example.  The JSON parser
<i>`json.l`</i> located in the examples directory of the RE/flex download
package was built with reflex option `-p` and then run on some given JSON input
to analyze its performance:

    reflex 0.9.22 json.l performance report:
      INITIAL rules matched:
        rule at line 51 accepted 58 times matching 255 bytes total in 0.009 ms
        rule at line 52 accepted 58 times matching 58 bytes total in 0.824 ms
        rule at line 53 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 54 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 55 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 56 accepted 5 times matching 23 bytes total in 0.007 ms
        rule at line 57 accepted 38 times matching 38 bytes total in 0.006 ms
        rule at line 72 accepted 0 times matching 0 bytes total in 0 ms
        default rule accepted 0 times
      STRING rules matched:
        rule at line 60 accepted 38 times matching 38 bytes total in 0.021 ms
        rule at line 61 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 62 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 63 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 64 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 65 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 66 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 67 accepted 0 times matching 0 bytes total in 0 ms
        rule at line 68 accepted 314 times matching 314 bytes total in 0.04 ms
        rule at line 69 accepted 8 times matching 25 bytes total in 0.006 ms
        rule at line 72 accepted 0 times matching 0 bytes total in 0 ms
        default rule accepted 0 times
      WARNING: execution times are relative:
        1) includes caller's execution time between matches when yylex() returns
        2) perf-report instrumentation adds overhead and increases execution times

The timings shown include the time of the pattern match and the time of the
code executed by the rule.  If the rule returns to the caller than the time
spent by the caller is also included in this timing.  For this example, we have
two start condition states namely INITIAL and STRING. The rule at line 52 is:

<div class="alt">
~~~{.cpp}
    [][}{,:]        { return yytext[0]; }
~~~
</div>

This returns a [ or ] bracket, a { or } brace, a comma, or a colon to the
parser.  Since the pattern and rule are very simple, we do not expect these to
contribute much to the 0.824 ms time spent on this rule.  The parser code
executed when the scanner returns could be expensive.  Depending on the
character returned, the parser constructs a JSON array (bracket) or a JSON
object (brace), and populates arrays and objects with an item each time a comma
is matched.  But which is most expensive? To obtain timings of these events
separately, we can split the rule up into three similar rules:

<div class="alt">
~~~{.cpp}
    [][]        { return yytext[0]; }
    [}{]        { return yytext[0]; }
    [,:]        { return yytext[0]; }
~~~
</div>

Then we use reflex option `-p`, recompile the generated scanner
<i>`lex.yy.cpp`</i> and rerun our experiment to see these changes:

        rule at line 52 accepted 2 times matching 2 bytes total in 0.001 ms
        rule at line 53 accepted 14 times matching 14 bytes total in 0.798 ms
        rule at line 54 accepted 42 times matching 42 bytes total in 0.011 ms

So it turns out that the construction of a JSON object by the parser is
relatively speaking the most expensive part of our application, when { and }
are encountered on the input. We should focus our optimization effort there if
we want to improve the overall speed of our JSON parser.

🔝 [Back to table of contents](#)


Examples                                                     {#reflex-examples}
--------

Some lexer specification examples to generate scanners with RE/flex.

🔝 [Back to table of contents](#)

### Example 1

The following Flex specification counts the lines, words, and characters on the
input.  We use `yyleng` match text length to count 8-bit characters (bytes).

To build this example with RE/flex, use <b>`reflex`</b> option `−−flex` to
generate Flex-compatible "yy" variables and functions.  This generates a C++
scanner class `yyFlexLexer` that is compatible with the Flex scanner class
(assuming Flex with option `-+` for C++).

<div class="alt">
~~~{.cpp}
    %{
      #include <stdio.h>
      int ch = 0, wd = 0, nl = 0;
    %}

    %option noyywrap
    %option main

    nl      \r?\n
    wd      [^ \t\r\n]+

    %%

    {nl}    ch += yyleng; ++nl;
    {wd}    ch += yyleng; ++wd;
    .       ++ch;
    <<EOF>> printf("%8d%8d%8d\n", nl, wd, ch); yyterminate();

    %%
~~~
</div>

To generate a scanner with a global `yylex()` function similar to Flex in C
mode (i.e. without Flex option `-+`), use <b>`reflex`</b> option `−−bison` with
the specification shown above.  This option when combined with `−−flex`
produces the global "yy" functions and variables.  This means that you can use
RE/flex scanners with Bison (Yacc) and with any other C code, assuming
everything is compiled together with a C++ compiler.

🔝 [Back to table of contents](#)

### Example 2

An improved implementation drops the use of global variables in favor of Lexer
class member variables.  We also want to count Unicode letters with the `wd`
counter instead of ASCII letters, which are single bytes while Unicode UTF-8
encodings vary in size.  So we add the Unicode option and use `\w` to match
Unicode word characters.  Note that `.` (dot) matches Unicode, so the match
length may be longer than one character that must be counted.  We drop the
`−−flex` option and use RE/flex Lexer methods instead of the Flex "yy"
functions:

<div class="alt">
~~~{.cpp}
    %top{
      #include <iostream>
      #include <iomanip>
      using namespace std;
    }

    %class{
      int ch, wd, nl;
    }

    %init{
      ch = wd = nl = 0;
    }

    %option unicode
    %option main
    %option full

    nl      \r?\n
    wd      (\w|\p{Punctuation})+

    %%

    {nl}    ch += size(); ++nl;
    {wd}    ch += size(); ++wd;
    .       ch += size();
    <<EOF>> out() << setw(8) << nl << setw(8) << wd << setw(8) << ch << endl;
            return 0;

    %%
~~~
</div>

This simple word count program differs slightly from the Unix wc utility,
because the wc utility counts words delimited by wide character spaces
(`iswspace`) whereas this program counts words made up from word characters
combined with punctuation.

🔝 [Back to table of contents](#)

### Example 3

The following RE/flex specification filters tags from XML documents and verifies
whether or not the tags are properly balanced.  Note that this example uses the
lazy repetitions to keep the patterns simple.  The XML document scanned should
not include invalid XML characters such as `/`, `<`, or `>` in attributes
(otherwise the tags will not match properly).  The `dotall` option allows `.`
(dot) to match newline in all patterns, similar to the `(?s)` modifier in
regexes.

<div class="alt">
~~~{.cpp}
    %top{
      #include <stdio.h>
    }

    %class{
      int level;
    }

    %init{
      level = 0;
    }

    %o matcher=reflex dotall main
    %x ATTRIBUTES

    name                    [A-Za-z_:\x80-\xFF][-.0-9A-Za-z_:\x80-\xFF]*
    pi                      <\?{name}
    comment                 <!--.*?-->
    open                    <{name}
    close                   <\/{name}>
    cdata                   <!\[CDATA\[.*?]]>
    string                  \".*?\"|'.*?'

    %%

    {comment}               |
    {cdata}                 // skip comments and CDATA sections

    {pi}                    level++;
                            start(ATTRIBUTES);

    {open}                  printf("%*s%s\n", level++, "", text() + 1);
                            start(ATTRIBUTES);

    {close}                 matcher().less(size() - 1);
                            printf("%*s%s\n", --level, "", text() + 2);

    <<EOF>>                 printf("Tags are %sbalanced\n", level ? "im" : "");
                            return 0;

    <ATTRIBUTES>"/>"        --level;
                            start(INITIAL);

    <ATTRIBUTES>">"         start(INITIAL);

    <ATTRIBUTES>{name}      |
    <ATTRIBUTES>{string}    // skip attribute names and strings

    <*>.                    // skip anything else

    %%
~~~
</div>

Note thay we restrict XML tag names to valid characters, including all UTF-8
sequences that run in the range `\x80`-`\xFF` per 8-bit character.  This
matches all Unicode characters U+0080 to U+10FFFF.

The `ATTRIBUTES` state is used to scan attributes and their quoted values
separately from the `INITIAL` state.  The `INITIAL` state permits quotes to
freely occur in character data, whereas the `ATTRIBUTES` state matches quoted
attribute values.

We use `matcher().less(size() - 1)` to remove the ending `>` from the match in
`text()`.  The `>` will be matched again, this time by the `<*>.` rule that
ignores it.  We could also have used a lookahead pattern `"</"{name}/">"` where
`X/Y` means look ahead for `Y` after `X`.

🔝 [Back to table of contents](#)

### Example 4

This example Flex specification scans non-Unicode C/C++ source code.  It uses
free space mode to enhance readability.

<div class="alt">
~~~{.cpp}
    %{
      #include <stdio.h>
    %}

    %o flex freespace main

    directive       ^ \h* # (. | \\ \r? \n)+
    name            [\u\l_] \w*
    udec            0 | [1-9] [0-9]*
    uhex            0 [Xx] [[:xdigit:]]+
    uoct            0 [0-7]+
    int             [-+]? ({udec} | {uhex}) \
                      ([Ll]{0,2} [Uu]? | [Uu] [Ll]{0,2})
    float           [-+] [0-9]* ([0-9] | \.[0-9] | [0-9]\.) [0-9]* \
                      ([Ee][-+]? [0-9]+)? \
                      [FfLl]?
    char            L? ' (\\. | [^\\\n'])* '
    string          L? \" (\\. | \\\r?\n | [^\\\n"])* \"

    %%

    \s+
    "//" .*? \n
    "/*" (.|\n)*? "*/"
    {directive}     { printf("DIRECTIVE %s\n", yytext); }
    {name}          { printf("NAME      %s\n", yytext); }
    {int}           { printf("INT       %s\n", yytext); }
    {float}         { printf("FLOAT     %s\n", yytext); }
    {char}          { printf("CHAR      %s\n", yytext); }
    {string}        { printf("STRING    %s\n", yytext); }
    [[:punct:]]     { printf("PUNCT     %s\n", yytext); }
    .               { printf("*** ERROR '%s' at line %d\n", yytext, yylineno); }

    %%
~~~
</div>

Free space mode permits spacing between concatenations and alternations.  To
match a single space, use `" "` or `[ ]`.  Long patterns can continue on the
next line(s) when lines ends with an escape `\`.

In free space mode you MUST place actions in <i>`{ }`</i> blocks and user code
in <i>`%{ %}`</i> blocks instead of indented.

When used with option `unicode`, the scanner automatically recognizes and scans
Unicode identifier names.  Note that we can use `matcher().columno()` or
`matcher().border()` in the error message to indicate the location on a line of
the match.  The `matcher().columno()` method takes tab spacing and wide
characters into account.  To obtain the byte offset from the start of the line
use `matcher().border()`.  The `matcher()` object associated with the Lexer
offers several other methods that Flex does not support.

🔝 [Back to table of contents](#)

### Example 5

This example defines a search engine to find C/C++ directives, such as
`#define` and `#include`, in the input fast.

<div class="alt">
~~~{.cpp}
    %{
      #include <stdio.h>
    %}

    %o fast find main

    directive       ^\h*#(.|\\\r?\n)+

    %%

    {directive}     echo();

    %%
~~~
</div>

Option `%%o find` (`-S` or `−−find`) specifies that unmatched input text should
be ignored silently instead of being echoed to standard output, see
\ref reflex-search.  Option `%%fast` (`-F` or `−−fast`) generates an efficient
FSM in direct code.

🔝 [Back to table of contents](#)


Limitations                                               {#reflex-limitations}
-----------

The RE/flex matcher engine uses an efficient FSM.  There are known limitations
to FSM matching that apply to Flex/Lex and therefore also apply to the
<b>`reflex`</b> scanner generator and to the RE/flex matcher engine:

- Lookaheads (trailing contexts) must appear at the end of a pattern when using
  the RE/flex matcher, so `a/b` (which is the same as lookahead
  `a(?=b)`) is permitted, but `(a/b)c` and `a(?=b)c` are not.
- Lookaheads cannot be properly matched when the ending of the first part of
  the pattern matches the beginning of the second part, such as `zx*/xy*`,
  where the `x*` matches the `x` at the beginning of the lookahead pattern.
- Anchors and boundaries must appear at the start or at the end of a pattern.
  The begin of buffer/line anchors `\A` and `^`, end of buffer/line anchors
  `\z` and `$` and the word boundary anchors must start or end a pattern.  For
  example, `\<cow\>` is permitted, but `.*\Bboy` is not.
- The POSIX Lex `REJECT` action is not supported.
- The POSIX Lex table size parameters `%p`, `%n`, `%a`, `%e`, `%k`, and `%o`
  are not supported; `%o` may be used as a shorthand for `%option`.
- Flex translations <i>`%%T`</i> are not supported.

Some of these limitations may be removed in future versions of RE/flex.

Boost.Regex library limitations:

- Lookbehinds `φ(?<=ψ)` with the Boost.Regex matcher engines should not look
  too far behind.  Any input before the current line may be discarded and is no
  longer available when new input is shifted into the internal buffer.  Only
  input on the current line from the start of the line to the match is
  guaranteed.
- Boost.Regex partial matching may fail to find the longest match when greedy
  repetition patterns such as `.*` are used.  Under certain conditions greedy
  repetitions may behave as lazy repetitions.  For example, the Boost.Regex
  engine may return the short match `abc` when the regex `a.*c` is applied to
  `abc abc`, instead of returning the full match `abc abc`.  The problem is
  caused by the limitations of Boost.Regex `match_partial` matching algorithm.
  To work around this limitation, we suggest to make the repetition pattern as
  specific as possible and not overlap with the pattern that follows the
  repetition.  *The easiest solution is to read the entire input* using
  <b>`reflex`</b> option `-B` (batch input).  For a stand-alone `BoostMatcher`,
  use the `buffer()` method.  We consider this Boost.Regex partial match
  behavior a bug, not a restriction, because *as long as backtracking on a
  repetition pattern is possible given some partial text, Boost.Regex should
  flag the result as a partial match instead of a full match.*

PCRE2 library limitations:

- Lookbehinds `φ(?<=ψ)` with the PCRE2 matcher engines should not look too far
  behind.  Any input before the current line may be discarded and is no longer
  available when new input is shifted into the internal buffer.  Only input on
  the current line from the start of the line to the match is guaranteed.

🔝 [Back to table of contents](#)


The RE/flex regex library                                              {#regex}
=========================

The RE/flex regex library consists of a set of C++ templates and classes that
encapsulate regex engines in a standard API for scanning, tokenizing,
searching, and splitting of strings, wide strings, files, and streams.

The RE/flex regex library is a class hierarchy that has at the root an abstract
class `reflex::AbstractMatcher`.  Pattern types may differ between for matchers
so the `reflex::PatternMatcher` template class takes a pattern type and creates
a class that is complete except for the implementation of the `reflex::match()`
virtual method that requires a regex engine, such as Boost.Regex, PCRE2, or the
RE/flex engine.

To compile your application, simply include the applicable regex matcher of
your choice in your source code as we will explain in the next sections.  To
compile, link your application against the `libreflex` library:

    c++ myapp.cpp -lreflex

And optionally `-lboost_regex` (or `-lboost_regex-mt` depending on your Boost
installation) if you use Boost.Regex for matching:

    c++ myapp.cpp -lreflex -lboost_regex

or `-lpcre2-8` if you use PCRE2 for matching:

    c++ myapp.cpp -lreflex -lpcre2-8

If `libreflex` was not installed then linking with `-lreflex` fails.  See
\ref link-errors on how to resolve this.

🔝 [Back to table of contents](#)


Boost matcher classes                                            {#regex-boost}
---------------------

The `reflex::BoostMatcher` inherits `reflex::PatternMatcher<boost::regex>`, and
in turn the `reflex::BoostPerlMatcher` and `reflex::BoostPosixMatcher` are both
derived from `reflex::BoostMatcher`:

  ![](classreflex_1_1_boost_matcher__inherit__graph.png)

An instance of `reflex::BoostPerlMatcher` is initialized with flag `match_perl`
and the flag `match_not_dot_newline`, these are `boost::regex_constants` flags.
These flags are the only difference with the plain `reflex::BoostMatcher`.

An instance of `reflex::BoostPosixMatcher` creates a POSIX matcher.  This means
that lazy quantifiers are not supported and the *leftmost longest rule* applies
to pattern matching.  This instance is initialized with the flags `match_posix`
and `match_not_dot_newline`.

Boost.Regex is a powerful library.  The RE/flex regex API enhances this library
with operations to match, search, scan, and split data from a given input.  The
input may be a file, a string, or a stream.  Files that are UTF-8/16/32-encoded
are automatically decoded.  Further, streams can be potentially of unlimited
length because internal buffering is used by the RE/flex regex API enhancements
to efficiently apply Boost.Regex partial pattern matching to streaming data.
This enhancement permits pattern matching of interactive input from the
console, such that searching and scanning interactive input for matches will
return these matches immediately.

@note The `reflex::BoostMatcher` extends the capabilities of Boost.Regex, which
does not natively support streaming input:

@note The Boost.Regex library requires the target text to be loaded into memory
for pattern matching.  It is possible to match a target text incrementally with
the `match_partial` flag and `boost::regex_iterator`.  Incremental matching can
be used to support matching on (possibly infinite) streams.  To use this method
correctly, a buffer should be created that is large enough to hold the text for
each match.  The buffer must adjust with a growing size of the matched text, to
ensure that long matches that do not fit the buffer are not discared.

@note Boost.Regex `Boost.IOStreams` with `regex_filter` loads the entire stream
into memory which does not permit pattern matching of streaming and interactive
input data.

A `reflex::BoostMatcher` (or `reflex::BoostPerlMatcher`) engine is created from
a `boost::regex` object, or string regex, and some given input for normal (Perl
mode) matching:

~~~{.cpp}
    #include <reflex/boostmatcher.h>

    reflex::BoostMatcher matcher( boost::regex or string, reflex::Input [, "options"] )
~~~

Likewise, a `reflex::BoostPosixMatcher` engine is created from a `boost::regex`
object, or string regex, and some given input for POSIX mode matching:

~~~{.cpp}
    #include <reflex/boostmatcher.h>

    reflex::BoostPosixMatcher matcher( boost::regex or string, reflex::Input [, "options"] )
~~~

For input you can specify a string, a wide string, a file, or a stream object.

We use option `"N"` to permit empty matches when searching input with
`reflex::BoostMatcher::find`.

You can convert an expressive regex of the form defined in \ref reflex-patterns
to a regex that the boost::regex engine can handle:

~~~{.cpp}
    #include <reflex/boostmatcher.h>

    static const std::string regex = reflex::BoostMatcher::convert( string, [ flags ]);

    reflex::BoostMatcher matcher( regex, reflex::Input [, "options"] )
~~~

The converter is specific to the matcher selected, i.e.
`reflex::BoostMatcher::convert`, `reflex::BoostPerlMatcher::convert`, and
`reflex::BoostPosixMatcher::convert`.  The converters also translates Unicode
`\p` character classes to UTF-8 patterns, converts bracket character classes
containing Unicode, and groups UTF-8 multi-byte sequences in the regex string.

The converter throws a `reflex::regex_error` exception if conversion fails,
for example when the regex syntax is invalid.

To compile your application, link your application against the `libreflex`
library and `-lboost_regex` (or `-lboost_regex-mt` depending on your Boost
installation):

    c++ myapp.cpp -lreflex -lboost_regex

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


PCRE2 matcher classes                                            {#regex-pcre2}
---------------------

The `reflex::PCRE2Matcher` inherits `reflex::PatternMatcher<std::string>`.
The `reflex::PCRE2UTFMatcher` is derived from `reflex::PCRE2Matcher`:

  ![](classreflex_1_1_p_c_r_e2_matcher__inherit__graph.png)

An instance of `reflex::PCRE2Matcher` is initialized with a pattern that is
compiled with `pcre2_compile()` and `pcre2_jit_compile()` for optimal
performance with PCRE2 JIT-generated code.

An instance of `reflex::PCRE2UTFMatcher` creates a PCRE2 matcher with native
Unicode support, using PCRE2 options `PCRE2_UTF+PCRE2_UCP`.

PCRE2 is a powerful library.  The RE/flex regex API enhances this library with
operations to match, search, scan, and split data from a given input.  The
input may be a file, a string, or a stream.  Files that are UTF-8/16/32-encoded
are automatically decoded.  Further, streams can be of potentially unlimited
length because internal buffering is used by the RE/flex regex API enhancements
to efficiently apply PCRE2 partial pattern matching to streaming data.  This
enhancement permits pattern matching of interactive input from the console,
such that searching and scanning interactive input for matches will return
these matches immediately.

A `reflex::PCRE2Matcher` (or `reflex::PCRE2UTFMatcher`) engine is created from
a string regex and some given input:

~~~{.cpp}
    #include <reflex/pcre2matcher.h>

    reflex::PCRE2Matcher matcher( string, reflex::Input [, "options"] )
~~~

Likewise, a `reflex::PCRE2UTFMatcher` engine is created from a string regex and
some given input:

~~~{.cpp}
    #include <reflex/pcre2matcher.h>

    reflex::PCRE2UTFMatcher matcher( string, reflex::Input [, "options"] )
~~~

This matcher uses PCRE2 native Unicode matching.  Non-UTF input is not
supported, such as plain binary.  UTF encoding errors in the input will cause
the matcher to terminate.

For input you can specify a string, a wide string, a file, or a stream object.

We use option `"N"` to permit empty matches when searching input with
`reflex::PCRE2Matcher::find`.

You can convert an expressive regex of the form defined in \ref reflex-patterns
to a regex that the PCRE2 engine can handle:

~~~{.cpp}
    #include <reflex/pcre2matcher.h>

    static const std::string regex = reflex::PCRE2Matcher::convert( string, [ flags ]);

    reflex::PCRE2Matcher matcher( regex, reflex::Input [, "options"] )
~~~

The converter is specific to the matcher selected, i.e.
`reflex::PCRE2Matcher::convert` and `reflex::PCRE2UTFMatcher::convert`.
The former converter converts Unicode `\p` character classes to UTF-8 patterns,
converts bracket character classes containing Unicode, and groups UTF-8
multi-byte sequences in the regex string.  The latter converter does not
convert these regex constructs, which are matched by the PCRE2 engine
initialized with options `PCRE2_UTF+PCRE2_UCP`.

The converter throws a `reflex::regex_error` exception if conversion fails,
for example when the regex syntax is invalid.

To compile your application, link your application against the `libreflex`
library and `-lpcre2-8`:

    c++ myapp.cpp -lreflex -lpcre2-8

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


std::regex matcher classes                                         {#regex-std}
--------------------------

The `reflex::StdMatcher` class inherits `reflex::PatternMatcher<std::regex>` as
a base.  The `reflex::StdEcmaMatcher` and `reflex::StdPosixMatcher` are derived
classes from `reflex::StdMatcher`:

  ![](classreflex_1_1_std_matcher__inherit__graph.png)

An instance of `reflex::StdEcmaMatcher` is initialized with regex syntax option
`std::regex::ECMAScript`.  This is also the default std::regex syntax.

An instance of `reflex::StdPosixMatcher` creates a POSIX AWK-based matcher.  So
that lazy quantifiers are not supported and the *leftmost longest rule* applies
to pattern matching.  This instance is initialized with the regex syntax option
`std::regex::awk`.

The C++11 std::regex library does not support `match_partial` that is needed to
match patterns on real streams with an adaptive internal buffer that grows when
longer matches are made when more input becomes available.  Therefore all input
is buffered with the C++11 std::regex class matchers.

With respect to performance, as of this time of writing, std::regex matching is
much slower than other matchers, slower by a factor 10 or more.

The std::regex syntax is more limited than Boost.Regex, PCRE2, and RE/flex.
Also the matching behavior differs and cannot be controlled with mode
modifiers:

- `.` (dot) matches anything except `\0` (NUL);
- `\177` is erroneously interpreted as a backreference, `\0177` does not match;
- `\x7f` is not supported in POSIX mode;
- `\cX` is not supported in POSIX mode;
- `\Q..\E` is not supported;
- no mode modifiers `(?imsux-imsux:φ)`;
- no `\A`, `\z`, `\<` and `\>` anchors;
- no `\b` and `\B` anchors in POSIX mode;
- no non-capturing groups `(?:φ)` in POSIX mode;
- empty regex patterns and matcher option `"N"` (nullable) may cause issues
  (std::regex `match_not_null` appears not to work as documented);
- `interactive()` is not supported.

To work around these limitations that the std::regex syntax imposes, you can
convert an expressive regex of the form defined in section \ref reflex-patterns
to a regex that the std::regex engine can handle:

~~~{.cpp}
    #include <reflex/stdmatcher.h>

    static const std::string regex = reflex::StdMatcher::convert( string, [ flags ]);

    reflex::StdMatcher matcher( regex, reflex::Input [, "options"] )
~~~

The converter is specific to the matcher selected, i.e.
`reflex::StdMatcher::convert`, `reflex::StdEcmaMatcher::convert`, and
`reflex::StdPosixMatcher::convert`.  The converters also translates Unicode
`\p` character classes to UTF-8 patterns, converts bracket character classes
containing Unicode, and groups UTF-8 multi-byte sequences in the regex string.

The converter throws a `reflex::regex_error` exception if conversion fails,
for example when the regex syntax is invalid.

To compile your application, link your application against the `libreflex`
and enable `std::regex` with `-std=c++11`:

    c++ -std=c++11 myapp.cpp -lreflex

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


The reflex::Matcher class                                      {#regex-matcher}
-------------------------

The RE/flex framework includes a POSIX regex matching library `reflex::Matcher`
that inherits the API from `reflex::PatternMatcher<reflex::Pattern>`:

  ![](classreflex_1_1_matcher__inherit__graph.png)

where the RE/flex `reflex::Pattern` class represents a regex pattern.  Patterns
as regex texts are internally compiled into deterministic finite state machines
by the `reflex::Pattern` class.  The machines are used by the `reflex::Matcher`
for fast matching of regex patterns on some given input.  The `reflex::Matcher`
is faster than the Boost.Regex and PCRE2 matchers.

A `reflex::Matcher` engine is constructed from a `reflex::Pattern` object, or a
string regex, and some given input:

~~~{.cpp}
    #include <reflex/matcher.h>

    reflex::Matcher matcher( reflex::Pattern or string, reflex::Input [, "options"] )
~~~

The regex is specified as a string or a `reflex::Pattern` object, see
\ref regex-pattern below.

We use option `"N"` to permit empty matches when searching input with
`reflex::Matcher::find`.  Option `"T=8"` sets the tab size to 8 for
\ref reflex-pattern-dents matching.

For input you can specify a string, a wide string, a file, or a stream object.

A regex string with Unicode patterns can be converted for Unicode matching as
follows:

~~~{.cpp}
    #include <reflex/matcher.h>

    static const std::string regex = reflex::Matcher::convert( string, [ flags ]);

    reflex::Matcher matcher( regex, reflex::Input [, "options"] )
~~~

The converter is specific to the matcher and translates Unicode `\p` character
classes to UTF-8 patterns, converts bracket character classes containing
Unicode, and groups UTF-8 multi-byte sequences in the regex string.

To compile your application, link your application against the `libreflex`:

    c++ myapp.cpp -lreflex

See \ref reflex-patterns for more details on regex patterns.

See \ref regex-input for more details on the `reflex::Input` class.

See \ref regex-methods for more details on pattern matching methods.

See \ref regex-convert for more details on regex converters.

🔝 [Back to table of contents](#)


The reflex::Pattern class                                      {#regex-pattern}
-------------------------

The `reflex::Pattern` class is used by the `reflex::matcher` for pattern
matching.  The `reflex::Pattern` class converts a regex pattern to an efficient
FSM and takes a regex string and options to construct the FSM internally.
The pattern instance is passed to a `reflex::Matcher` constructor:

~~~{.cpp}
    #include <reflex/matcher.h>

    [static] reflex:Pattern pattern(string [, "options"] )

    reflex::Matcher matcher(pattern, reflex::Input [, "options"] )
~~~

It may also be used to replace a matcher's current pattern, see \ref intro2.

To improve performance, it is recommended to create a `static` instance of the
pattern if the regex string is fixed.  This avoids repeated FSM construction at
run time.

The following options are combined in a string and passed to the
`reflex::Pattern` constructor:

  Option        | Effect
  ------------- | -------------------------------------------------------------
  `b`           | bracket lists are parsed without converting escapes
  `e=c;`        | redefine the escape character
  `f=file.cpp;` | save finite state machine code to `file.cpp`
  `f=file.gv;`  | save deterministic finite state machine to `file.gv`
  `i`           | case-insensitive matching, same as `(?i)X`
  `m`           | multiline mode, same as `(?m)X`
  `n=name;`     | use `reflex_code_name` for the machine (instead of FSM)
  `q`           | Flex/Lex-style quotations "..." equals `\Q...\E`
  `r`           | throw regex syntax error exceptions, otherwise ignore errors
  `s`           | dot matches all (aka. single line mode), same as `(?s)X`
  `x`           | inline comments, same as `(?x)X`
  `w`           | display regex syntax errors before raising them as exceptions

The compilation of a `reflex::Pattern` object into a FSM may throw an exception
with option `"r"` when the regex string has problems:

~~~{.cpp}
    try
    {
      reflex::Pattern pattern(argv[1], "r"); // "r" option throws syntax error exceptions
      ...
      // code that uses the pattern object
      ...
    }
    catch (reflex::regex_error& e)
    {
      switch (e.code())
      {
        case reflex::regex_error::mismatched_parens:     std::cerr << "mismatched ( )"; break;
        case reflex::regex_error::mismatched_braces:     std::cerr << "mismatched { }"; break;
        case reflex::regex_error::mismatched_brackets:   std::cerr << "mismatched [ ]"; break;
        case reflex::regex_error::mismatched_quotation:  std::cerr << "mismatched \\Q...\\E quotation"; break;
        case reflex::regex_error::empty_expression:      std::cerr << "regex (sub)expression should not be empty"; break;
        case reflex::regex_error::empty_class:           std::cerr << "character class [...] is empty, e.g. [a&&[b]]"; break;
        case reflex::regex_error::invalid_class:         std::cerr << "invalid character class name"; break;
        case reflex::regex_error::invalid_class_range:   std::cerr << "invalid character class range, e.g. [Z-A]"; break;
        case reflex::regex_error::invalid_escape:        std::cerr << "invalid escape character"; break;
        case reflex::regex_error::invalid_anchor:        std::cerr << "invalid anchor or boundary"; break;
        case reflex::regex_error::invalid_repeat:        std::cerr << "invalid repeat, e.g. {10,1}"; break;
        case reflex::regex_error::invalid_quantifier:    std::cerr << "invalid lazy or possessive quantifier"; break;
        case reflex::regex_error::invalid_modifier:      std::cerr << "invalid (?ismux:) modifier"; break;
        case reflex::regex_error::invalid_collating:     std::cerr << "invalid collating element"; break;
        case reflex::regex_error::invalid_backreference: std::cerr << "invalid backreference"; break;
        case reflex::regex_error::invalid_syntax:        std::cerr << "invalid regex syntax"; break;
        case reflex::regex_error::exceeds_length:        std::cerr << "exceeds length limit, pattern is too long"; break;
        case reflex::regex_error::exceeds_limits:        std::cerr << "exceeds complexity limits, e.g. {n,m} range too large"; break;
      }
      std::cerr << std::endl << e.what();
    }
~~~

By default, the `reflex::Pattern` constructor solely throws the
`reflex::regex_error::exceeds_length` and `reflex::regex_error::exceeds_limits`
exceptions and silently ignores syntax errors.

Likewise, the `reflex::Matcher::convert`, `reflex::BoostPerlMatcher::convert`,
`reflex::BoostMatcher::convert`, `reflex::BoostPosixMatcher::convert`,
`reflex::PCRE2Matcher::convert`, and `reflex::PCRE2UTFMatcher::convert`
functions may throw a `reflex_error` exception.  See the next section for
details.

The `reflex::Pattern` class has the following public methods:

  Method        | Result
  ------------- | -------------------------------------------------------------
  `assign(r,o)` | (re)assign regex string `r` with string of options `o`
  `assign(r)`   | (re)assign regex string `r` with default options
  `=r`          | same as above
  `size()`      | returns the number of top-level sub-patterns
  `[0]`         | operator returns the regex string of the pattern
  `[n]`         | operator returns the `n`th sub-pattern regex string
  `reachable(n)`| true if sub-pattern `n` is reachable in the FSM

The assignment methods may throw exceptions, which are the same as the
constructor may throw.

The `reflex::Pattern::reachable` method verifies which top-level grouped
alternations are reachable.  This means that the sub-pattern of an alternation
has a FSM accepting state that identifies the sub-pattern.  For example:

~~~{.cpp}
    #include <reflex/matcher.h>

    reflex::Pattern pattern("(a+)|(a)", "r");
    std::cout << "regex = " << pattern[0] << std::endl;
    for (size_t i = 1; i <= pattern.size(); ++i)
      if (!pattern.reachable(i))
        std::cerr << pattern[i] << " is not reachable" << std::endl;
~~~

When executed this code prints:

    regex = (a+)|(a)
    (a) is not reachable

For this example regex, `(a)` is not reachable as the pattern is subsumed by
`(a+)`.  The `reflex::Matcher::accept` method will never return 2 when
matching the input `a` and always return 1, as per leftmost longest match
policy.  The same observation holds for the `reflex::Matcher::matches`,
`reflex::Matcher::find`, `reflex::Matcher::scan`, and `reflex::Matcher::split`
method and functors.  Reversing the alternations resolves this: `(a)|(a+)`.

@note The `reflex::Pattern` regex forms support capturing groups at the
top-level only, i.e. among the top-level alternations.

🔝 [Back to table of contents](#)


Regex converters                                               {#regex-convert}
----------------

To work around limitations of regex libraries and to support Unicode matching,
RE/flex offers converters to translate expressive regex syntax forms (with
Unicode patterns defined in section \ref reflex-patterns) to regex strings
that the selected regex engines can handle.

The converters translate `\p` Unicode classes, translate character
class set operations such as `[a-z−−[aeiou]]`, convert escapes such as `\X`,
and enable/disable `(?imsux-imsux:φ)` mode modifiers to a regex string that the
underlying regex library understands and can use.

Each converter is specific to the regex engine.  You can use a converter for
the matcher of your choice:

- `std::string reflex::BoostMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with Boost.Regex;
- `std::string reflex::BoostPerlMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with Boost.Regex in Perl mode;
- `std::string reflex::BoostPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with Boost.Regex in POSIX mode;
- `std::string reflex::PCRE2Matcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with PCRE2;
- `std::string reflex::PCRE2UTFMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with PCRE2 native Unicode matching;
- `std::string reflex::StdMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with C++ std::regex;
- `std::string reflex::StdEcmaMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with C++ std::regex in ECMA mode;
- `std::string reflex::StdPosixMatcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with C++ std::regex in POSIX mode;
- `std::string reflex::Matcher::convert(const std::string& regex, reflex::convert_flag_type flags)`
  converts an enhanced `regex` for use with the RE/flex POSIX regex library;

where `flags` is optional.  When specified, it may be a combination of the
following `reflex::convert_flag` flags:

  Flag                               | Effect
  ---------------------------------- | ---------------------------------------------------------------
  `reflex::convert_flag::none`       | no conversion
  `reflex::convert_flag::basic`      | convert basic regular expression syntax (BRE) to extended regular expression syntax (ERE)
  `reflex::convert_flag::unicode`    | `.`, `\s`, `\w`, `\l`, `\u`, `\S`, `\W`, `\L`, `\U` match Unicode, same as `(?u)`
  `reflex::convert_flag::recap`      | remove capturing groups and add capturing groups to the top level
  `reflex::convert_flag::lex`        | convert Flex/Lex regular expression syntax
  `reflex::convert_flag::u4`         | convert `\uXXXX` (shorthand for `\u{XXXX}`), may conflict with `\u` (upper case letter).
  `reflex::convert_flag::notnewline` | inverted character classes do not match newline `\n`, e.g. `[^a-z]` does not match `\n`
  `reflex::convert_flag::permissive` | when used with `unicode`, produces a more compact FSM that tolerates some invalid UTF-8 sequences

The following `reflex::convert_flag` flags correspond to the common `(?imsx)`
modifiers.  These flags or modifiers may be specified, or both.  Modifiers are
removed from the converted regex if the regex library does not support them:

  Flag                              | Effect
  --------------------------------- | ---------------------------------------------------------------
  `reflex::convert_flag::anycase`   | convert regex to ignore case, same as `(?i)`
  `reflex::convert_flag::freespace` | convert regex by removing all freespace-mode spacing, same as `(?x)`
  `reflex::convert_flag::dotall`    | convert `.` (dot) to match all (match newline), same as `(?s)`
  `reflex::convert_flag::multiline` | adds/asserts if `(?m)` is supported for multiline anchors `^` and `$`

The following example enables Unicode matching by converting the regex pattern
with the `reflex::convert_flag::unicode` flag:

~~~{.cpp}
    #include <reflex/matcher.h> // reflex::Matcher, reflex::Input, reflex::Pattern

    // use a Matcher to check if sentence is in Greek:
    static const reflex::Pattern pattern(reflex::Matcher::convert("[\\p{Greek}\\p{Zs}\\pP]+", reflex::convert_flag::unicode));
    if (reflex::Matcher(pattern, sentence).matches())
      std::cout << "This is Greek" << std::endl;
~~~

The following example enables dotall mode to count the number of characters
(including newlines) in the given `example` input:

~~~{.cpp}
    #include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

    // construct a Boost.Regex matcher to count wide characters:
    std::string regex = reflex::BoostMatcher::convert("(?su).");
    reflex::BoostMatcher boostmatcher(regex, example);
    size_t n = std::distance(boostmatcher.scan.begin(), boostmatcher.scan.end());
~~~

Note that we could have used `"\X"` instead to match any wide character without
using the `(?su)` modifiers.

A converter throws a `reflex::regex_error` exception if conversion fails, for
example when the regex syntax is invalid:

~~~{.cpp}
    std::string regex;
    try
    {
      regex = reflex::BoostMatcher::convert(argv[1], reflex::convert_flag::unicode));
    }
    catch (reflex::regex_error& e)
    {
      std::cerr << e.what();
      switch (e.code())
      {
        case reflex::regex_error::mismatched_parens:     std::cerr << "mismatched ( )"; break;
        case reflex::regex_error::mismatched_braces:     std::cerr << "mismatched { }"; break;
        case reflex::regex_error::mismatched_brackets:   std::cerr << "mismatched [ ]"; break;
        case reflex::regex_error::mismatched_quotation:  std::cerr << "mismatched \\Q...\\E quotation"; break;
        case reflex::regex_error::empty_expression:      std::cerr << "regex (sub)expression should not be empty"; break;
        case reflex::regex_error::empty_class:           std::cerr << "character class [...] is empty, e.g. [a&&[b]]"; break;
        case reflex::regex_error::invalid_class:         std::cerr << "invalid character class name"; break;
        case reflex::regex_error::invalid_class_range:   std::cerr << "invalid character class range, e.g. [Z-A]"; break;
        case reflex::regex_error::invalid_escape:        std::cerr << "invalid escape character"; break;
        case reflex::regex_error::invalid_anchor:        std::cerr << "invalid anchor or boundary"; break;
        case reflex::regex_error::invalid_repeat:        std::cerr << "invalid repeat, e.g. {10,1}"; break;
        case reflex::regex_error::invalid_quantifier:    std::cerr << "invalid lazy or possessive quantifier"; break;
        case reflex::regex_error::invalid_modifier:      std::cerr << "invalid (?ismux:) modifier"; break;
        case reflex::regex_error::invalid_collating:     std::cerr << "invalid collating element"; break;
        case reflex::regex_error::invalid_backreference: std::cerr << "invalid backreference"; break;
        case reflex::regex_error::invalid_syntax:        std::cerr << "invalid regex syntax"; break;
      }
    }
~~~

🔝 [Back to table of contents](#)


Methods and iterators                                          {#regex-methods}
---------------------

The RE/flex abstract matcher, that every other RE/flex matcher inherits,
provides four operations for matching with an instance of a regex engine:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `matches()` | returns nonzero if the input from begin to end matches
  `find()`    | search input and return nonzero if a match was found
  `scan()`    | scan input and return nonzero if input at current position matches
  `split()`   | return nonzero for a split of the input at the next match

These methods return a nonzero *"accept"* value for a match, meaning the
`size_t accept()` value that corresponds to a group capture (or one if no
groups are used).  The methods are repeatable, where the last three return
additional matches.

The `find`, `scan`, and `split` methods are also implemented as input iterators
that apply filtering tokenization, and splitting:

  Iterator range                  | Acts as a | Iterates over
  ------------------------------- | --------- | -------------------------------
  `find.begin()`...`find.end()`   | filter    | all matches
  `scan.begin()`...`scan.end()`   | tokenizer | continuous matches
  `split.begin()`...`split.end()` | splitter  | text between matches

🔝 [Back to table of contents](#)

### matches                                            {#regex-methods-matches}

The `matches()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) if the given
input from begin to the end matches the specified pattern.

For example, to match a UUID string with PCRE2:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    std::string uuid = "123e4567-e89b-12d3-a456-426655440000";

    if (reflex::PCRE2Matcher("[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}", uuid).matches())
      std::cout << uuid << " is a string in UUID format" << std::endl;
~~~

When executed this code prints:

    123e4567-e89b-12d3-a456-426655440000 is a string in UUID format

The `matches()` method returns the group capture index that can be used as a
selector.  For example:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    const char *food = "hotdog";

    switch (reflex::PCRE2Matcher("(.*cat.*)|(.*dog.*)", food).matches())
    {
      case 0: std::cout << food << " has not cat or dog" << std::endl;
              break;
      case 1: std::cout << food << " has a cat" << std::endl;
              break;
      case 2: std::cout << food << " has a dog" << std::endl;
              break;
    }
~~~

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### find                                                  {#regex-methods-find}

The `find()` method and `find.begin()`...`find.end()` iterator range are used
to search for a match in the given input.

The `find()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) for a match
and zero otherwise.

For example, to find all words in a string with PCRE2:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    reflex::PCRE2Matcher matcher("\\w+", "How now brown cow.");
    while (matcher.find() != 0)
      std::cout << matcher.text() << std::endl;
~~~

When executed this code prints:

    How
    now
    brown
    cow

The iterator range `find.begin()`...`find.end()` serves as an input filter.

For example, in C++11 we can use a range-based loop to loop over matches using
the `find` iterator:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    for (auto& match : reflex::PCRE2Matcher("\\w+", "How now brown cow.").find)
      std::cout << match.text() << std::endl;
~~~

Iterators can be used with STL algorithms and other iterator functions.  For
example to count words in a string:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    reflex::PCRE2Matcher matcher("\\w+", "How now brown cow.");
    std::cout << std::distance(matcher.find.begin(), matcher.find.end()) << std::endl;
~~~

When executed this code prints:

    4

The `find()` method returns the group capture index that can be used as a
selector.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### scan                                                  {#regex-methods-scan}

The `scan()` method and `scan.begin()`...`scan.end()` iterator range are
similar to `find` but generate continuous matches in the given input.

The `scan()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) for a match
and zero otherwise.

For example, to scan for words, spacing, and punctuation in a sentence with
PCRE2:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    const char *tokens[4] = { "n/a", "word", "space", "other" };
    reflex::PCRE2Matcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
    size_t accept;
    while ((accept = matcher.scan()) != 0)
      std::cout << tokens[accept] << std::endl;
~~~

When executed this code prints:

    word
    space
    word
    space
    word
    space
    word
    other

The iterator range `scan.begin()`...`scan.end()` serves as an input tokenizer
and produces continuous matches.

For example, tokenizing a string into a vector of numeric tokens:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    reflex::PCRE2Matcher matcher("(\\w+)|(\\s+)|(.)", "How now brown cow.");
    std::vector<size_t> tokens(matcher.scan.begin(), matcher.scan.end());
    std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<size_t>(std::cout, " "));
~~~

When executed the code prints:

    1 2 1 2 1 2 1 3

If the pattern does not match the input immediately succeeding a previous
match, then the `scan()` method returns false and the iterator returns
`scan.end()`.  To determine if all input was scanned and end of input was
reached, use the `at_end()` method, see \ref regex-methods-props.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### split                                                {#regex-methods-split}

The `split()` method and `split.begin()`...`split.end()` iterator range return
text between matches in the given input.

The `split()` method returns a nonzero "accept" value (the `size_t accept()`
group capture index value or the value 1 if no groups are used) of the
matching text (that is, the text that comes after the split part) and zero
otherwise.

When matches are adjacent then empty text is returned.  Also the start of input
and end of input return text that may be empty.

For example, to split text into words by matching non-words with PCRE2:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    reflex::PCRE2Matcher matcher("\\W+", "How now brown cow.");
    while (matcher.split() != 0)
      std::cout << "'" << matcher.text() << "'" << std::endl;
~~~

When executed this code prints:

    'How'
    'now'
    'brown'
    'cow'
    ''

This produces five text splits where the last text is empty because the period
at the end of the sentence matches the pattern and empty input remains.

The iterator range `split.begin()`...`split.end()` serves as an input splitter.

For example, to display the contents of a text file while normalizing spacing:

~~~{.cpp}
    #include <reflex/pcre2matcher.h> // reflex::PCRE2Matcher, reflex::Input

    reflex::PCRE2Matcher matcher("\\s+", fopen("abstract.txt", "r"));
    if (matcher.in.file() != NULL)
    {
      std::copy(matcher.split.begin(), matcher.split.end(), std::ostream_iterator<std::string>(std::cout, " "));
      fclose(matcher.in.file());
    }
~~~

@note The "accept" value of the last split is
`reflex::PCRE2Matcher::Const::EMPTY` to indicate that empty text after the
split was found and matched.  This special value is also returned by `accept()`
and is also used with any other RE/flex matcher's `split` method.

See also \ref regex-methods-props.

🔝 [Back to table of contents](#)

### Properties of a match                                {#regex-methods-props}

To obtain properties of a match, use the following methods:

  Method          | Result
  --------------- | ---------------------------------------------------------------
  `accept()`      | returns group capture index (or zero if not captured/matched)
  `text()`        | returns `const char*` to 0-terminated text match (ends in `\0`)
  `str()`         | returns `std::string` text match (preserves `\0`s)
  `wstr()`        | returns `std::wstring` wide text match (converted from UTF-8)
  `chr()`         | returns first 8-bit character of the text match (`str()[0]`)
  `wchr()`        | returns first wide character of the text match (`wstr()[0]`)
  `pair()`        | returns `std::pair<size_t,std::string>(accept(),str())`
  `wpair()`       | returns `std::pair<size_t,std::wstring>(accept(),wstr())`
  `size()`        | returns the length of the text match in bytes
  `wsize()`       | returns the length of the match in number of wide characters
  `lines()`       | returns the number of lines in the text match (>=1)
  `columns()`     | returns the number of columns of the text match (>=0)
  `begin()`       | returns `const char*` to non-0-terminated text match begin
  `end()`         | returns `const char*` to non-0-terminated text match end
  `rest()`        | returns `const char*` to 0-terminated rest of input
  `span()`        | returns `const char*` to 0-terminated match enlarged to span the line
  `line()`        | returns `std::string` line with the matched text as a substring
  `wline()`       | returns `std::wstring` line with the matched text as a substring
  `more()`        | tells the matcher to append the next match (when using `scan()`)
  `less(n)`       | cuts `text()` to `n` bytes and repositions the matcher
  `lineno()`      | returns line number of the match, starting at line 1
  `columno()`     | returns column number of the match, starting at 0
  `lineno_end()`  | returns ending line number of the match, starting at line 1
  `columno_end()` | returns ending column number of the match, starting at 0
  `border()`      | returns byte offset from the start of the line of the match
  `first()`       | returns input position of the first character of the match
  `last()`        | returns input position + 1 of the last character of the match
  `at_bol()`      | true if matcher reached the begin of a new line
  `at_bob()`      | true if matcher is at the begin of input and no input consumed
  `at_end()`      | true if matcher is at the end of input
  `[0]`           | operator returns `std::pair<const char*,size_t>(begin(),size())`
  `[n]`           | operator returns n'th capture `std::pair<const char*,size_t>`

The `accept()` method returns nonzero for a succesful match, returning the
group capture index.  The RE/flex matcher engine `reflex::Matcher` only
recognizes group captures at the top level of the regex (i.e. among the
top-level alternations), because it uses an efficient FSM for matching.

The `text()`, `str()`, and `wstr()` methods return the matched text.  To get
the first character of a match, use `chr()` or `wchr()`.  The `chr()` and
`wchr()` methods are much more efficient than `str()[0]` (or `text()[0]`) and
`wstr()[0]`, respectively.  Normally, a match cannot be empty unless option
`"N"` is specified to explicitly initialize a matcher, see \ref regex-boost,
\ref regex-pcre2, and \ref regex-matcher.

The `begin()`, `operator[0]`, and `operator[n]` return non-0-terminated
strings.  You must use `end()` with `begin()` to determine the span of the
match.  Basically, `text()` is the 0-terminated version of the string spanned
by `begin()` to `end()`, where `end()` points the next character after the
match, which means that `end()` = `begin()` + `size()`.  Use the size of the
capture returned by `operator[n]` to determine the end of the captured match.

The `lineno()` method returns the line number of the match, starting at line 1.
The ending line number is `lineno_end()`, which is identical to the value of
`lineno()` + `lines()` - 1.  

The `columno()` method returns the column offset of a match from the start of
the line, beginning at column 0.  This method takes tab spacing and wide
characters into account.  The inclusive ending column number is given by
`columno_end()`, which is equal or larger than `columno()` if the match does
not span multiple lines.  Otherwise, if the match spans multiple lines,
`columno_end()` is the ending column of the match on the last matching line.

The starting byte offset of the match on a line is `border()` and the ending
byte offset of the match is `border() + size() - 1`.

The `lines()` and `columns()` methods return the number of lines and columns
matched, where `columns()` takes tab spacing and wide characters into account.
If the match spans multiple lines, `columns()` counts columns over all lines,
without counting the newline characters.

@note A wide character is counted as one, thus `columno()`, `columno_end()`,
and `columns()` do not take the character width of full-width and combining
Unicode characters into account.  It is recommended to use the `wcwidth`
function or
[wcwidth.c](https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c) to determine Unicode
character widths.

The `rest()` method returns the rest of the input character sequence as a
0-terminated `char*` string.  This method buffers all remaining input to return
the string.

The `span()` method enlarges the text matched to span the entire line and
returns the matching line as a 0-terminated `char*` string without the `\n`.

The `line()` and `wline()` methods return the entire line as a (wide) string
with the matched text as a substring.  These methods can be used to obtain the
context of a match.

@warning The methods `span()`, `line()`, and `wline()` invalidate the previous
`text()`, `begin()`, and `end()` string pointers.  Call these methods again to
retrieve the updated pointer or call `str()` or `wstr()` to obtain a string
copy of the match:
~~~{.cpp}
    // INCORRECT, because t is invalid after line():
    const char *t = text();
    std::string s = line();
    std::cout << t << " in " << s << std::endl;
    // OK with line():
    std::string s = line();
    const char *t = text();
    std::cout << t << " in " << s << std::endl;
    // OK with span():
    std::string t = str();
    const char *s = span();
    std::cout << t << " in " << s << std::endl;
~~~
The start of a line is truncated when the line is too long.  The length of the
line's contents before the pattern match on the line is restricted to 8KB,
which is the size specified by `reflex::AbstractMatcher::Const::BLOCK`.  When
this length is exceeded, the line's length before the match is truncated to
8KB.  This ensures that pattern matching binary files or files with very long
lines cannot cause memory allocation exceptions.

The `matcher().more()` method is used to create longer matches by stringing
together consecutive matches in the input after scanning the input with the
`scan()` method.  When this method is invoked, the next match with `scan()` has
its matched text prepended to it.  The `matcher().more()` operation is often
used in lexers and was introduced in Lex.

The `less(n)` method reduces the size of the matched text to `n` bytes.
This method has no effect if `n` is larger than `size()`.  The value of `n`
should not be `0`.  The `less(n)` operation is often used in lexers and was
introduced in Lex.

The `first()` and `last()` methods return the position in the input stream
of the match, counting in bytes from the start of the input at position 0.
If the input stream is a wide character sequence, the UTF-8 positions are
returned as a result of the internally-converted UTF-8 wide character input.

All methods take constant time to execute except for `str()`, `wstr()`,
`pair()`, `wpair()`, `wsize()`, `lines()`, `columns()`, and `columno()` that
require an extra pass over the matched text.

In addition, the following type casts of matcher objects and iterators may be
used for convenience:

- Casting to `size_t` gives the matcher's `accept()` index.
- Casting to `std::string` is the same as invoking `str()`
- Casting to `std::wstring` is the same as invoking `wstr()`.
- Casting to `std::pair<size_t,std::string>` is the same as `pair()`.
- Casting to `std::pair<size_t,std::wstring>` is the same as `wpair()`.

The following example prints some of the properties of each match:

~~~{.cpp}
    #include <reflex/boostmatcher.h> // reflex::BoostMatcher, reflex::Input, boost::regex

    reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
    while (matcher.find() != 0)
      std::cout <<
        "accept: " << matcher.accept() <<
        "text:   " << matcher.text() <<
        "size:   " << matcher.size() <<
        "line:   " << matcher.lineno() <<
        "column: " << matcher.columno() <<
        "first:  " << matcher.first() <<
        "last:   " << matcher.last() << std::endl;
~~~

When executed this code prints:

    accept: 1 text: How size: 3 line: 1 column: 0 first: 0 last: 3
    accept: 1 text: now size: 3 line: 1 column: 4 first: 4 last: 7
    accept: 1 text: brown size: 5 line: 1 column: 8 first: 8 last: 13
    accept: 1 text: cow size: 3 line: 1 column: 14 first: 14 last: 17

🔝 [Back to table of contents](#)

### Public data members                                        {#regex-members}

Four public data members of a matcher object are accesible:

  Variable | Usage
  -------- | ------------------------------------------------------------------
  `in`     | the `reflex::Input` object used by the matcher
  `find`   | the `reflex::AbstractMatcher::Operation` functor for searching
  `scan`   | the `reflex::AbstractMatcher::Operation` functor for scanning
  `split`  | the `reflex::AbstractMatcher::Operation` functor for splitting

Normally only the `in` variable should be used which holds the current input
object of the matcher.  See \ref regex-input for details.

The functors provide `begin()` and `end()` methods that return iterators and
hold the necessary state information for the iterators.  A functor invocation
essentially invokes the corresponding method listed in \ref regex-methods.

🔝 [Back to table of contents](#)

### Pattern methods                                    {#regex-methods-pattern}

To change a matcher's pattern or check if a pattern was assigned, you can use
the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `pattern(p)`    | set pattern to `p` (string regex or `reflex::Pattern`)
  `has_pattern()` | true if the matcher has a pattern assigned to it
  `own_pattern()` | true if the matcher has a pattern to manage and delete
  `pattern()`     | get the pattern object associated with the matcher

The first method returns a reference to the matcher, so multiple method
invocations may be chained together.

🔝 [Back to table of contents](#)

### Input methods                                        {#regex-methods-input}

To assign a new input source to a matcher or set the input to buffered or
interactive, you can use the following methods:

  Method          | Result
  --------------- | -----------------------------------------------------------
  `input(i)`      | set input to `reflex::Input i` (string, stream, or `FILE*`)
  `buffer()`      | buffer all input at once, returns true if successful
  `buffer(n)`     | set the adaptive buffer size to `n` bytes to buffer input
  `buffer(b, n)`  | use buffer of `n` bytes at address `b` with to a string of `n`-1 bytes (zero copy)
  `interactive()` | sets buffer size to 1 for console-based (TTY) input
  `flush()`       | flush the remaining input from the internal buffer
  `reset()`       | resets the matcher, restarting it from the remaining input
  `reset(o)`      | resets the matcher with new options string `o` ("A?N?T?")

The first method returns a reference to the matcher, so multiple method
invocations may be chained together.

The following methods may be used to read the input stream provided to a
matcher directly, even when you use the matcher's search and match methods:

  Method     | Result
  ---------- | ----------------------------------------------------------------
  `input()`  | returns next 8-bit char from the input, matcher then skips it
  `winput()` | returns next wide character from the input, matcher skips it
  `unput(c)` | put 8-bit char `c` back unto the stream, matcher then takes it
  `wunput(c)`| put (wide) char `c` back unto the stream, matcher then takes it
  `peek()`   | returns next 8-bit char from the input without consuming it
  `skip(c)`  | skip input until character `c` (`char` or `wchar_t`) is consumed
  `skip(s)`  | skip input until UTF-8 string `s` is consumed
  `rest()`   | returns the remaining input as a non-NULL `char*` string

The `input()`, `winput()`, and `peek()` methods return a non-negative character
code and EOF (-1) when the end of input is reached.

A matcher reads from the specified input source using its virtual method
`size_t get(char *s, size_t n)`.  This method is the same as invoking
`matcher().in.get(s, n)` to directly read data from the `reflex::Input` source
`in`, but also handles interactive input when enabled with
`matcher().interactive()` to not read beyond the next newline character.  

The following protected methods may be overriden by a derived matcher class to
customize reading:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `get(s, n)` | fill `s[0..n-1]` with next input, returns number of bytes read
  `wrap()`    | returns false (may be overriden to wrap input after EOF)

When a matcher reaches the end of input, it invokes the virtual method `wrap()`
to check if more input is available.  This method returns false by default, but
this behavior may be changed by overriding `wrap()` to set a new input source
and return `true`, for example:

~~~{.cpp}
    class WrappedInputMatcher : public reflex::Matcher {
     public:
      WrappedInputMatcher() : reflex::Matcher(), source_select(0)
      { }
     private:
      virtual bool wrap()
      {
        // read a string, a file, and a string:
        switch (source_select++)
        {
          case 0: in = "Hello World!";
                  return true;
          case 1: in = fopen("hello.txt", "r");
                  return in.file() != NULL;
          case 2: fclose(in.file());
                  in = "Goodbye!";
                  return true;
        }
        return false;
      }
      int source_select;
    };
~~~

Note that the constructor in this example does not specify a pattern and input.
To set a pattern for the matcher after its instantiation use the `pattern(p)`
method.  In this case the input does not need to be specified, which allows us
to immediately force reading the sources of input that we assigned in our
`wrap()` method.

For details of the `reflex::Input` class, see \ref regex-input.

🔝 [Back to table of contents](#)


The Input class                                                  {#regex-input}
---------------

A matcher may accept several types of input, but can only read from one input
source at a time.  Input to a matcher is represented by a single
`reflex::Input` class instance that the matcher uses internally.

🔝 [Back to table of contents](#)

### Assigning input                                       {#regex-input-assign}

An input object is constructed by specifying a string, a file, or a stream to
read from.  You can also reassign input to read from new input.

More specifically, you can pass a `std::string`, `char*`, `std::wstring`,
`wchar_t*`, `FILE*`, or a `std::istream` to the constructor.

A `FILE*` file descriptor is a special case.  The input object handles various
file encodings.  If a UTF Byte Order Mark (BOM) is detected then the UTF input
will be normalized to UTF-8.  When no UTF BOM is detected then the input is
considered plain ASCII, binary, or UTF-8 and passed through unconverted.  To
override the file encoding when no UTF BOM was present, and normalize Latin-1,
ISO-8859-1 through ISO-8859-15, CP 1250 through 1258, CP 437, CP 850, CP 858,
KOI8, MACROMAN, EBCDIC, and other encodings to UTF-8, see \ref regex-input-file.

🔝 [Back to table of contents](#)

### Input strings                                        {#regex-input-strings}

An input object constructed from an 8-bit string `char*` or `std::string` just
passes the string to the matcher engine.  The string should contain UTF-8 when
Unicode patterns are used.

An input object constructed from a wide string `wchar_t*` or `std::wstring`
translates the wide string to UTF-8 for matching, which effectively normalizes
the input for matching with Unicode patterns.  This conversion is illustrated
below.  The copyright symbol `©` with Unicode U+00A9 is matched against its
UTF-8 sequence `C2 A9` of `©`:

~~~{.cpp}
    if (reflex::Matcher("©", L"©").matches())
      std::cout << "copyright symbol matches\n";
~~~

To ensure that Unicode patterns in UTF-8 strings are grouped properly, use
\ref regex-convert, for example as follows:

~~~{.cpp}
    static reflex::Pattern CR(reflex::Matcher::convert("(?u:\u{00A9})"));
    if (reflex::Matcher(CR, L"©").matches())
      std::cout << "copyright symbol matches\n";
~~~

Here we made the converted pattern static to avoid repeated conversion and
construction overheads.

@note The `char*`, `wchar_t*`, and `std::wstring` strings cannot contain a `\0`
(NUL) character and the first `\0` terminates matching.  To match strings
and binary input that contain `\0`, use `std::string` or `std::istringstream`.

🔝 [Back to table of contents](#)

### Input streams                                        {#regex-input-streams}

An input object constructed from a `std::istream` (or a derived class) just
passes the input text to the matcher engine.  The stream should contain ASCII
and may contain UTF-8.

🔝 [Back to table of contents](#)

### FILE encodings                                          {#regex-input-file}

File content specified with a `FILE*` file descriptor can be encoded in ASCII,
binary, UTF-8/16/32, ISO-8859-1 through ISO-8859-15, CP 1250 through 1258, CP
437, CP 850, CP 858, or EBCDIC.

A [UTF Byte Order Mark (BOM)](www.unicode.org/faq/utf_bom.html) is detected in
the content of a file scanned by the matcher, which enables UTF-8 normalization
of the input automatically.

Otherwise, if no file encoding is explicitly specified, the matcher expects
raw UTF-8, ASCII, or plain binary by default.  File formats can be decoded and
translated to UTF-8 on the fly for matching by means of specifying encodings.

The current file encoding used by a matcher is obtained with the
`reflex::Input::file_encoding()` method, which returns an
`reflex::Input::file_encoding` constant of type
`reflex::Input::file_encoding_type`:

  Constant                                   | File encoding
  ------------------------------------------ | --------------------------------
  `reflex::Input::file_encoding::plain`      | plain octets, ASCII/binary/UTF-8
  `reflex::Input::file_encoding::utf8`       | UTF-8 (UTF BOM detected)
  `reflex::Input::file_encoding::utf16be`    | UTF-16 big endian (UTF BOM detected)
  `reflex::Input::file_encoding::utf16le`    | UTF-16 little endian (UTF BOM detected)
  `reflex::Input::file_encoding::utf32be`    | UTF-32 big endian (UTF BOM detected)
  `reflex::Input::file_encoding::utf32le`    | UTF-32 little endian (UTF BOM detected)
  `reflex::Input::file_encoding::latin`      | ASCII with Latin-1, ISO-8859-1
  `reflex::Input::file_encoding::cp437`      | DOS CP 437
  `reflex::Input::file_encoding::cp850`      | DOS CP 850
  `reflex::Input::file_encoding::cp858`      | DOS CP 858
  `reflex::Input::file_encoding::ebcdic`     | EBCDIC
  `reflex::Input::file_encoding::cp1250`     | Windows CP 1250
  `reflex::Input::file_encoding::cp1251`     | Windows CP 1251
  `reflex::Input::file_encoding::cp1252`     | Windows CP 1252
  `reflex::Input::file_encoding::cp1253`     | Windows CP 1253
  `reflex::Input::file_encoding::cp1254`     | Windows CP 1254
  `reflex::Input::file_encoding::cp1255`     | Windows CP 1255
  `reflex::Input::file_encoding::cp1256`     | Windows CP 1256
  `reflex::Input::file_encoding::cp1257`     | Windows CP 1257
  `reflex::Input::file_encoding::cp1258`     | Windows CP 1258
  `reflex::Input::file_encoding::iso8859_2`  | ISO-8859-2
  `reflex::Input::file_encoding::iso8859_3`  | ISO-8859-3
  `reflex::Input::file_encoding::iso8859_4`  | ISO-8859-4
  `reflex::Input::file_encoding::iso8859_5`  | ISO-8859-5
  `reflex::Input::file_encoding::iso8859_6`  | ISO-8859-6
  `reflex::Input::file_encoding::iso8859_7`  | ISO-8859-7
  `reflex::Input::file_encoding::iso8859_8`  | ISO-8859-8
  `reflex::Input::file_encoding::iso8859_9`  | ISO-8859-9
  `reflex::Input::file_encoding::iso8859_10` | ISO-8859-10
  `reflex::Input::file_encoding::iso8859_11` | ISO-8859-11
  `reflex::Input::file_encoding::iso8859_13` | ISO-8859-13
  `reflex::Input::file_encoding::iso8859_14` | ISO-8859-14
  `reflex::Input::file_encoding::iso8859_15` | ISO-8859-15
  `reflex::Input::file_encoding::iso8859_16` | ISO-8859-16
  `reflex::Input::file_encoding::macroman`   | Macintosh Roman + CR to LF translation
  `reflex::Input::file_encoding::koi8_r`     | KOI8-R
  `reflex::Input::file_encoding::koi8_u`     | KOI8-U
  `reflex::Input::file_encoding::koi8_ru`    | KOI8-RU
  `reflex::Input::file_encoding::custom`     | user-defined custom code page

To set the file encoding when assigning a file to read with `reflex::Input`,
use `reflex::Input(file, enc)` with one of the encoding constants shown in
the table.

For example, use `reflex::Input::file_encoding::latin` to override the encoding
when the file contains ISO-8859-1.  This way you can match its content using
Unicode patterns (matcher engines internally normalizes ISO-8859-1 to UTF-8):

~~~{.cpp}
    reflex::Input input(stdin, reflex::Input::file_encoding::latin);
    reflex::Matcher matcher(pattern, input);
~~~

This sets the standard input encoding to ISO-8859-1, but only if no UTF BOM was
detected on the standard input, because the UTF encoding of a `FILE*` that
starts with a UTF BOM cannot be overruled.

To define a custom code page to translate files, define a code page table with
256 entries that maps each 8-bit input character to a 16-bit Unicode character
(UCS-2).  Then use `reflex::Input::file_encoding::custom` with a pointer to
your code page to construct an input object.  For example:

~~~{.cpp}
    static const unsigned short CP[256] = {
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
       48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
       64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
       80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
       96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
      112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    };
    reflex::Input input(stdin, reflex::Input::file_encoding::custom, CP);
    reflex::Matcher matcher(pattern, input);
~~~

This example translates all control characters and characters above 127 to
spaces before matching.

🔝 [Back to table of contents](#)

### Input properties                                  {#regex-input-properties}

To obtain the properties of an input source use the following methods:

  Method      | Result
  ----------- | ---------------------------------------------------------------
  `size()`    | size in bytes of the remaining input, zero when EOF or unknown
  `good()`    | input is available to read (no error and not EOF)
  `eof()`     | end of input (but use only `at_end()` with matchers!)
  `cstring()` | the current `const char*` (of a `std::string`) or NULL
  `wstring()` | the current `const wchar_t*` (of a `std::wstring`) or NULL
  `file()`    | the current `FILE*` file descriptor or NULL
  `istream()` | a `std::istream*` pointer to the current stream object or NULL

🔝 [Back to table of contents](#)

### Input streambuf                                    {#regex-input-streambuf}

We can use a `reflex::Input` object as a `std::streambuf` and pass it to a
`std::istream`.  This is useful when a `std::istream` is required where a
`reflex::Input` object cannot be directly used.  The `std::istream`
automatically normalizes the input to UTF-8 using the underlying
`reflex::Input` object.  For example:

~~~{.cpp}
    reflex::Input input(...);            // create an Input object for some given input
    reflex::Input::streambuf buf(input); // create a streambuf
    std::istream is(&buf);
    if (is.good())
    {
      // read the stream
    }
~~~

The `reflex::Input` object may be created from strings, wide strings, streams,
and `FILE*` values.  These are readable as a `std::istream` via
`reflex::Input::streambuf` that returns normalized UTF-8 characters.  For
`FILE*` values we can specify \ref regex-input-file to normalize the encoded
input to UTF-8.

Keep in mind that adding a `std::istream` with `reflex::Input::streambuf` layer
on top of the efficient `reflex::Input` class will impact file reading
performance, especially because `reflex::Input::streambuf` is unbuffered
(despite its name).  When performance is important, use the buffered version
`reflex::BufferedInput::streambuf`:

~~~{.cpp}
    reflex::Input input(...);                    // create an Input object for some given input
    reflex::BufferedInput::streambuf buf(input); // create a buffered streambuf
    std::istream is(&buf);
    if (is.good())
    {
      // read the stream
    }
~~~

Because the buffered vesion reads ahead to fill its buffer, the buffered
version may not be suitable for interactive input.

See also \ref regex-input-dosstreambuf.

🔝 [Back to table of contents](#)

### Windows CRLF pairs                              {#regex-input-dosstreambuf}

Reading files in Windows "binary mode" is recommended when the file is encoded
in UTF-16 or UTF-32.  Reading a file in the default "text mode" replaces CRLF
by LF and interprets ^Z (0x1A) as EOF.  Because a ^Z code may be part of a
UTF-16 or UTF-32 multibyte sequence, this can cause premature EOF on Windows
machines.  The latest RE/flex releases automatically switch `FILE*` input to
binary mode on Windows systems when the file is encoded in UTF-16 or UTF-32.

In addition, DOS files and other DOS or Windows input sources typically end
lines with CRLF byte pairs, see \ref crlf.  Reading a file in binary mode
retains these CRLF pairs.

To automatically replace CRLF by LF when reading files in binary mode on
Windows you can use the `reflex::Input::dos_streambuf` class to construct a
`std::istream` object.  This normalized stream can then be used as input to a
RE/flex scanner or to a regex matcher:

~~~{.cpp}
    reflex::Input input(...);                // create an Input object for some given input
    reflex::Input::dos_streambuf buf(input); // create a dos_streambuf
    std::istream is(&buf);
    if (is.good())
    {
      // read the stream
    }
~~~

Once the stream object is created it can be used to create a new input object
for a RE/flex scanner, for example:

~~~{.cpp}
    if (is.good())
    {
      Lexer lexer(is); // create a lexer
      lexer.lex();     // scan the normalized input (DOS CRLF -> LF)
    }
~~~

or for a regex matcher that uses Boost.Regex or PCRE2:

~~~{.cpp}
    if (is.good())
    {
      reflex::BoostMatcher matcher("\\w+", is); // read normalized input (DOS CRLF -> LF)
      while (matcher.find() != 0)
        std::cout << "Found " << matcher.text() << std::endl;
    }
~~~

Note that when the input is a `FILE*`, CRLF pairs are replaced by LF and
UTF-16/32 encodings are automatically normalized to UTF-8 (when a UTF BOM is
present in the file or you can specify \ref regex-input-file).

@warning The `reflex::Input::size` method returns the number of bytes available
that includes CRLF pairs.  The actual number of bytes read may be smaller after
replacing CRLF by LF.

When performance is important, use the buffered version
`reflex::BufferedInput::dos_streambuf`:

~~~{.cpp}
    reflex::Input input(...);                        // create an Input object for some given input
    reflex::BufferedInput::dos_streambuf buf(input); // create a buffered dos_streambuf
    std::istream is(&buf);
    if (is.good())
    {
      // read the stream
    }
~~~

Because the buffered vesion reads ahead to fill its buffer, the buffered
version may not be suitable for interactive input:

See also \ref regex-input-streambuf.

🔝 [Back to table of contents](#)


Examples                                                      {#regex-examples}
--------

This section includes several examples to demonstrate the concepts discussed.

🔝 [Back to table of contents](#)

### Example 1

This example illustrates the `find` and `split` methods and iterators with a
RE/flex `reflex::Matcher` and a `reflex::BoostMatcher` using a C++11
range-based loop:

~~~{.cpp}
    #include <reflex/matcher.h>
    #include <reflex/boostmatcher.h>

    using namespace reflex;

    // some random text (with extra spacing)
    std::string example("Monty\n Python's    Flying  Circus");

    // construct a fixed pattern that is case insensitive
    static Pattern pattern("monty|python", "i");

    // construct a matcher to search the example text
    Matcher matcher(pattern, example);
    while (matcher.find() != 0)
      std::cout << matcher.text()
                << " at " << matcher.lineno() << "," << matcher.columno()
                << " spans " << matcher.first() << ".." << matcher.last()
                << std::endl;

    // construct a Boost.Regex matcher to count the words (all non-spaces "\S+")
    BoostMatcher boostmatcher("\\S+", example);
    boostmatcher.buffer(); // because Boost.Regex partial_match is broken!
    std::cout << std::distance(boostmatcher.find.begin(), boostmatcher.find.end())
              << " words"
              << std::endl;

    // reuse the BoostMatcher with a C++11 range-based loop to split the example text on space "\s+":
    for (auto& split : boostmatcher.pattern("\\s+").input(example).split)
      std::cout << split.text() << " ";
    std::cout << std::endl;

    // reuse the BoostMatcher to split on whitespace using an iterator, sort into a set, and print
    boostmatcher.input(example).buffer();
    std::set<std::string> words(boostmatcher.split.begin(), boostmatcher.split.end());
    std::copy(words.begin(), words.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;
~~~

When executed this code prints:

    Monty at 1,0 spans 0..5
    Python at 2,1 spans 7..13
    4 words
    Monty Python's Flying Circus
    Circus Flying Monty Python's


🔝 [Back to table of contents](#)

### Example 2

This example shows how a URL can be matched by using two patterns: one pattern
to extract the host:port/path parts and another pattern to extract the query
string key-value pairs in a loop.  A PCRE2 matcher or a Boost.Regex matcher may
be used, since both support group captures.

~~~{.cpp}
    #include <reflex/boostmatcher.h>
    #include <iostream>

    using namespace reflex;

    const char *URL = "...";

    // match URL host:port/path using group captures for these
    BoostMatcher re("https?://([^:/]*):?(\\d*)/?([^?#]*)", URL);

    if (re.scan())
    {
      // found a partial match at start, now check if we have a host
      if (re[1].first != NULL)
      {
        std::string host(re[1].first, re[1].second);
        std::cout << "host: " << host << std::endl;

        // check of we have a port
        if (re[2].first != NULL && re[2].second != 0)
        {
          std::string port(re[2].first, re[2].second);
          std::cout << "port: " << port << std::endl;
        }

        // check of we have a path
        if (re[3].first != NULL && re[3].second != 0)
        {
          std::string path(re[3].first, re[3].second);
          std::cout << "path: " << path << std::endl;
        }
      }

      // check if we have a query string
      if (re.input() == '?')
      {
        // now switch patterns to match the query string
        re.pattern("([^=&]*)=?([^&]*)&?");
        while (re.scan())
          std::cout <<
            "query key: " << std::string(re[1].first, re[1].second) <<
            ", value: " << std::string(re[2].first, re[2].second) << std::endl;
      }
      else if (!re.at_end())
      {
        // not a query string and not the end, we expect an # anchor
        std::cout << "anchor: " << re.rest() << std::endl;
      }
    }
    else
    {
      std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
    }
~~~

See also Example 8 below for a more powerful URL pattern matcher.

🔝 [Back to table of contents](#)

### Example 3

This example shows how input can be reassigned in each iteration of a loop that
matches wide strings against a word pattern `\w+`:

~~~{.cpp}
    #include <reflex/pcre2matcher.h>

    using namespace reflex;

    // four words
    const wchar_t *words[] = { L"Monty", L"Python's", L"Flying", L"Circus" };

    // construct a PCRE2 matcher for words, given empty input initially
    PCRE2Matcher wordmatcher("\\w+", Input());

    // check if each string in words[] is a word
    for (int i = 0; i < 4; i++)
      if (wordmatcher.input(words[i]).matches())
        std::cout << wordmatcher.text() << ", ";
~~~

When executed this code prints:

    Monty, Flying, Circus, 

🔝 [Back to table of contents](#)

### Example 4

This example counts the number of words, lines, and chars from the `std::cin`
stream:

~~~{.cpp}
    #include <reflex/stdmatcher.h>

    using namespace reflex;

    // construct a std::regex matcher like the wc command (a word is a series of nonspaces)
    StdMatcher word("\\S+", std::cin);

    size_t words = std::distance(word.find.begin(), word.find.end());
    size_t lines = word.lineno() - 1;
    size_t chars = word.last();

    std::cout << lines << " " << words << " " << chars << std::endl;
~~~

🔝 [Back to table of contents](#)

### Example 5

This example tokenizes a string by grouping the subpatterns in a regex and by
using the group index of the capture obtained with `accept()` in a C++11
range-based loop:

~~~{.cpp}
    #include <reflex/matcher.h>

    using namespace reflex;

    static Pattern pattern =
      "(\\w*cat\\w*)|" // 1st group = token 1
      "(\\w*dog\\w*)|" // 2nd group = token 2
      "(\\w+)|"        // 3rd group = token 3
      "(.)";           // 4th group = token 4

    Matcher tokenizer(pattern, "cats love hotdogs!");

    for (auto& token : tokenizer.scan)
      std::cout << "Token = " << token.accept()
                << ": matched '" << token.text() << "'"
                << " with '" << token.pattern()[token.accept()] << "'\n";
    assert(tokenizer.at_end());
~~~

When executed this code prints:

    Token = 1: matched 'cats' with '(\\w*cat\\w*)'
    Token = 4: matched ' ' with '(.)'
    Token = 3: matched 'love' with '(\\w+)'
    Token = 4: matched ' ' with '(.)'
    Token = 2: matched 'hotdogs' with '(\\w*dog\\w*)'
    Token = 4: matched '!' with '(.)'

🔝 [Back to table of contents](#)

### Example 6

This example reads a file with embedded credit card numbers to extract.  The
numbers are sorted into five sets for each type of major credit card:

~~~{.cpp}
    #include <reflex/matcher.h>

    using namespace reflex;

    static Pattern card_patterns =
      "(?# MasterCard)(5[1-5]\\d{14})|"                   // 1st group = MC
      "(?# Visa)(4\\d{12}(?:\\d{3})?)|"                   // 2nd group = VISA
      "(?# AMEX)(3[47]\\d{13})|"                          // 3rd group = AMEX
      "(?# Discover)(6011\\d{14})|"                       // 4th group = Discover
      "(?# Diners Club)((?:30[0-5]|36\\d|38\\d)\\d{11})"; // 5th group = Diners

    static const char *card_data =
      "mark 5212345678901234\n"
      "vinny 4123456789012\n"
      "victor 4123456789012345\n"
      "amy 371234567890123\n"
      "dirk 601112345678901234\n"
      "doc 38812345678901 end\n";

    std::set<std::string> cards[5];

    Matcher matcher(card_patterns, card_data);
    for (Matcher::iterator match = matcher.find.begin(); match != matcher.find.end(); ++match)
      cards[match.accept() - 1].insert(match.text());

    for (int i = 0; i < 5; ++i)
      for (std::set<std::string>::const_iterator j = cards[i].begin(); j != cards[i].end(); ++j)
        std::cout << i << ": " << *j << std::endl;
~~~

When executed this code prints:

    0: 5212345678901234
    1: 4123456789012
    1: 4123456789012345
    2: 371234567890123
    3: 601112345678901234
    4: 38812345678901

🔝 [Back to table of contents](#)

### Example 7

The RE/flex matcher engine `reflex::Matcher` only recognizes group captures at
the top level of the regex (i.e. among the top-level alternations), because it
uses an efficient FSM for matching.

By contrast, the PCRE2 matcher can capture groups within a regex:

~~~{.cpp}
    #include <reflex/pcre2matcher.h>

    // a PCRE2Matcher to find 'TODO' lines on stdin and capture their content to display
    reflex::PCRE2Matcher matcher("TODO ([^\\n]+)", stdin);
    while (matcher.find())
      std::cout
        << matcher.lineno() << ": "
        << std::string(matcher[1].first, matcher[1].second)
        << std::endl;
~~~

The Boost.Regex library also supports group captures.  It als supports partial
matches, but that feature appears to be broken, so all input must be buffered:

~~~{.cpp}
    #include <reflex/boostmatcher.h>

    // a BoostMatcher to find 'TODO' lines on stdin and capture their content to display
    reflex::BoostMatcher matcher("TODO ([^\\n]+)", stdin);
    matcher.buffer(); // because Boost.Regex partial_match is broken!
    while (matcher.find())
      std::cout
        << matcher.lineno() << ": "
        << std::string(matcher[1].first, matcher[1].second)
        << std::endl;
~~~

🔝 [Back to table of contents](#)

### Example 8

This is a more advanced example, in which we will use the
`reflex::BoostMatcher` class to decompose URLs into parts: the host, port,
path, optional ?-query string key=value pairs, and an optional #-anchor.

To do so, we change the pattern of the matcher to partially match each of the
URL's parts and also use `input()` to check the input character:

~~~{.cpp}
    #include <reflex/boostmatcher.h>
    #include <iostream>

    using namespace reflex;

    int main(int argc, char **argv)
    {
      if (argc < 2)
      {
        std::cerr << "Usage: url 'URL'" << std::endl;
        exit(EXIT_SUCCESS);
      }

      BoostMatcher re("https?://([^:/]*):?(\\d*)/?([^?#]*)", argv[1]);

      if (re.scan())
      {
        // found a partial match at start, now check if we have a host
        if (re[1].first != NULL)
        {
          std::string host(re[1].first, re[1].second);
          std::cout << "host: " << host << std::endl;

          // check of we have a port
          if (re[2].first != NULL && re[2].second != 0)
          {
            std::string port(re[2].first, re[2].second);
            std::cout << "port: " << port << std::endl;
          }

          // check of we have a path
          if (re[3].first != NULL && re[3].second != 0)
          {
            std::string path(re[3].first, re[3].second);
            std::cout << "path: " << path << std::endl;
          }
        }

        // check if we have a query string
        if (re.input() == '?')
        {
          // now switch patterns to match the rest of the input
          // i.e. a query string or an anchor
    #if 0
          // 1st method: a pattern to split query strings at '&'
          re.pattern("&");
          while (re.split())
            std::cout << "query: " << re << std::endl;
    #else
          // 2nd method: a pattern to capture key-value pairs between the '&'
          re.pattern("([^=&]*)=?([^&]*)&?");
          while (re.scan())
            std::cout <<
              "query key: " << std::string(re[1].first, re[1].second) <<
              ", value: " << std::string(re[2].first, re[2].second) << std::endl;
    #endif
        }
        else if (!re.at_end())
        {
          // not a query string and not the end, we expect an # anchor
          std::cout << "anchor: " << re.rest() << std::endl;
        }
      }
      else
      {
        std::cout << "Error, not a http/s URL: " << re.rest() << std::endl;
      }

      return EXIT_SUCCESS;
    }
~~~

Note that there are two ways to split the query string into key-value pairs.
Both methods are shown in the two `#if` branches in the code above, with the
first branch disabled with `#if 0`.

When executing

    ./url 'https://localhost:8080/test/me?name=reflex&license=BSD-3'

this code prints:

    host: localhost
    port: 8080
    path: test/me
    query key: name, value: reflex
    query key: license, value: BSD-3

🔝 [Back to table of contents](#)

### Example 9

This example shows how a `FILE*` file descriptor is used as input.  The file
encoding is obtained from the UTF BOM, when present in the file.  Note that the
file's state is accessed through the matcher's member variable `in`:

~~~{.cpp}
    #include <reflex/boostmatcher.h>

    using namespace reflex;

    BoostMatcher matcher("\\s+", fopen("filename", "r"));

    if (matcher.in.file() != NULL && matcher.in.good())
    {
      switch (matcher.in.file_encoding())
      {
        case Input::file_encoding::plain:      std::cout << "plain ASCII/binary/UTF-8"; break;
        case Input::file_encoding::utf8:       std::cout << "UTF-8 with BOM";           break;
        case Input::file_encoding::utf16be:    std::cout << "UTF-16 big endian";        break;
        case Input::file_encoding::utf16le:    std::cout << "UTF-16 little endian";     break;
        case Input::file_encoding::utf32be:    std::cout << "UTF-32 big endian";        break;
        case Input::file_encoding::utf32le:    std::cout << "UTF-32 little endian";     break;
        case Input::file_encoding::latin:      std::cout << "ASCII+Latin-1/ISO-8859-1"; break;
        case Input::file_encoding::cp437:      std::cout << "DOS CP 437";               break;
        case Input::file_encoding::cp850:      std::cout << "DOS CP 850";               break;
        case Input::file_encoding::cp858:      std::cout << "DOS CP 858";               break;
        case Input::file_encoding::ebcdic:     std::cout << "EBCDIC";                   break;
        case Input::file_encoding::cp1250:     std::cout << "Windows CP 1250";          break;
        case Input::file_encoding::cp1251:     std::cout << "Windows CP 1251";          break;
        case Input::file_encoding::cp1252:     std::cout << "Windows CP 1252";          break;
        case Input::file_encoding::cp1253:     std::cout << "Windows CP 1253";          break;
        case Input::file_encoding::cp1254:     std::cout << "Windows CP 1254";          break;
        case Input::file_encoding::cp1255:     std::cout << "Windows CP 1255";          break;
        case Input::file_encoding::cp1256:     std::cout << "Windows CP 1256";          break;
        case Input::file_encoding::cp1257:     std::cout << "Windows CP 1257";          break;
        case Input::file_encoding::cp1258:     std::cout << "Windows CP 1258";          break;
        case Input::file_encoding::iso8859_2:  std::cout << "ISO-8859-2";               break;
        case Input::file_encoding::iso8859_3:  std::cout << "ISO-8859-3";               break;
        case Input::file_encoding::iso8859_4:  std::cout << "ISO-8859-4";               break;
        case Input::file_encoding::iso8859_5:  std::cout << "ISO-8859-5";               break;
        case Input::file_encoding::iso8859_6:  std::cout << "ISO-8859-6";               break;
        case Input::file_encoding::iso8859_7:  std::cout << "ISO-8859-7";               break;
        case Input::file_encoding::iso8859_8:  std::cout << "ISO-8859-8";               break;
        case Input::file_encoding::iso8859_9:  std::cout << "ISO-8859-9";               break;
        case Input::file_encoding::iso8859_10: std::cout << "ISO-8859-10";              break;
        case Input::file_encoding::iso8859_11: std::cout << "ISO-8859-11";              break;
        case Input::file_encoding::iso8859_13: std::cout << "ISO-8859-13";              break;
        case Input::file_encoding::iso8859_14: std::cout << "ISO-8859-14";              break;
        case Input::file_encoding::iso8859_15: std::cout << "ISO-8859-15";              break;
        case Input::file_encoding::iso8859_16: std::cout << "ISO-8859-16";              break;
        case Input::file_encoding::mac_roman:  std::cout << "Macintosh Roman";          break;
        case Input::file_encoding::koi8_r:     std::cout << "KOI8-R";                   break;
        case Input::file_encoding::koi8_u:     std::cout << "KOI8-U";                   break;
        case Input::file_encoding::koi8_ru:    std::cout << "KOI8-RU";                  break;
      }
      std::cout << " of " << matcher.in.size() << " converted bytes to read\n";
      matcher.buffer(); // because Boost.Regex partial_match is broken!
      if (matcher.split() != 0)
        std::cout << "Starts with: " << matcher.text() << std::endl;
      std::cout << "Rest of the file is: " << matcher.rest();
      fclose(matcher.in.file());
    }
~~~

The default encoding is `reflex::Input::file_encoding::plain` when no UTF BOM
is detected at the start of the input file.  The encodings
`reflex::Input::file_encoding::latin`, `reflex::Input::file_encoding::cp1252`,
`reflex::Input::file_encoding::cp437`, `reflex::Input::file_encoding::cp850`,
`reflex::Input::file_encoding::ebcdic` are never detected automatically,
because plain encoding is implicitly assumed to be the default encoding.  To
convert these files, set the file encoding format explicitly in your code.  For
example, if you expect the source file to contain ISO-8859-1 8-bit characters
(ASCII and the latin-1 supplement) then set the default file encoding to
`reflex::Input::file_encoding::latin` as follows:

~~~{.cpp}
    reflex::Input input(fopen("filename", "r"), reflex::Input::file_encoding::latin);
    if (input.file() != NULL)
    {
      reflex::BoostMatcher matcher("\\s+", input);
      matcher.buffer(); // because Boost.Regex partial_match is broken!
      if (matcher.split() != 0)
        std::cout << "Starts with: " << matcher.text() << std::endl;
      std::cout << "Rest of the file is: " << matcher.rest();
      fclose(input.file());
    }
~~~

This sets the file encoding to ISO-8859-1, but only if no UTF BOM was detected
in the file.  Files with a UTF BOM are always decoded as UTF, which cannot be
overruled.

🔝 [Back to table of contents](#)


Tips, tricks, and gotchas                                             {#tricks}
=========================

🔝 [Back to table of contents](#)


Errors when declaring extern yyin, yytext, yylineno              {#extern-yyin}
---------------------------------------------------

For backward compatibility with Flex, option `−−flex` defines macros to expand
`yyin`, `yyout`, `yylineno`, `yytext`, and `yyleng`.  The macro expansion
depends on the `−−bison` option or `−−bison-locations`, `−−bison-cc` and so on.

When used with `−−flex`, option `−−bison` generates global "yy" variables and
functions, see \ref reflex-bison-mt-safe for details.  This means that
`yytext`, `yyleng`, and `yylineno` are global variables.  More specifically,
the following declarations are generated with `−−flex` and `−−bison`:

<div class="alt">
~~~{.cpp}
    char *yytext;
    yy_size_t yyleng;
    int yylineno;
    int yylex();
~~~
</div>

Note that `yyin` is not a global variable, because the `yyin` macro expands to
a pointer to the `reflex::Input` of the matcher.  This offers advanced input
handling capabilities with `reflex::Input` that is more useful compared to the
traditional global `FILE *yyin` variable.

However, the following declaration, when present in a Lex/Flex lexer
specification, may cause a compilation error:

<div class="alt">
~~~{.cpp}
    extern FILE *yyin; // ERROR: fails to compile (remove this line)
~~~
</div>

Option `−−yy` enables `−−flex` and `−−bison`.  In addition, this option
generates the following declarations to define the `yyin` and `yyout` as
global `FILE*` type variables:

<div class="alt">
~~~{.cpp}
    FILE *yyin;
    FILE *yyout;
~~~
</div>

Note that without option `−−yy`, when options `−−flex` and `−−bison` are used,
`yyin` is a pointer to a `reflex::Input` object.  This means that `yyin` is not
restricted to `FILE*` types and accepts files, steams and strings:

<div class="alt">
~~~{.cpp}
    std::ifstream ifs(filename, std::ios::in);
    yyin = &ifs; // FILE* or std::istream
~~~
</div>

<div class="alt">
~~~{.cpp}
    yyin = "..."; // (wide) strings
~~~
</div>

See \ref reflex-input.

🔝 [Back to table of contents](#)


Compilation errors when using yy functions                      {#yy-functions}
------------------------------------------

To use Flex' `yy` functions in your scanner's actions, use option `−−flex` for
Flex compatibility (see also previous section).

In addition, note that by default the <b>`reflex`</b> command generates a
reentrant C++ scanner class, unless option `−−bison` is used.  This means that
by default all `yy` functions are scanner class methods, not global functions.
This obviously means that `yy` functions cannot be globally invoked, e.g. from
your parser.  These are the alternatives:

- Generate global `yy` functions like Flex with option `−−yy` (or `−−flex` and
  `−−bison`).  This approach is not thread safe.
- Generate thread-safe C++ scanner class (by default) and pass the scanner
  object to your parser to invoke the scanner's methods.  Note that scanner
  methods are not `yy` functions, see the list of scanner methods listed in 
  \ref reflex-spec-rules.
- Generate a thread-safe C++ scanner class and let the parser class inherit the
  scanner class.  All scanner methods are available in the parser too.
- Generate a thread-safe C++ scanner class but also `#define YY_SCANNER`
  (redefine) in your parser and in other parts of the program that need to
  invoke `yy` functions:
<div class="alt">
~~~{.cpp}
    #include "lexer.hpp"  // generated with --header-file=lexer.hpp
    ...
    #undef YY_SCANNER
    #define YY_SCANNER lexer
    int parser(Lexer& lexer, ...) {
      ...
      int c = yyinput();  // is lexer.input() which is lexer.matcher().input()
      ...
    }
~~~
</div>
Note that the `yyinput()` macro expands to `YY_SCANNER.input()`, where
`YY_SCANNER` is normally `(*this)`, i.e. the current scanner object, or
`YY_SCANNER` is the global scanner object/state when option `−−bison` is used
to generate global `yy` variables and functions stored in the global
`YY_SCANNER` object.

🔝 [Back to table of contents](#)


Invalid UTF encodings                                            {#invalid-utf}
---------------------

It may be tempting to write a pattern with `.` (dot) as a wildcard in a lexer
specification, but beware that in Unicode mode enabled with
<i>`%%option unicode`</i> or with modifier `(?u:φ)`, the dot matches any code
point, including code points outside of the valid Unicode character range and
invalid overlong UTF-8 (except that it won't match newline unless
<i>`%%option dotall`</i> is specified.)  The reason for this design choice is
that a lexer should be able to implement a "catch all else" rule to report
errors in the input:

<div class="alt">
~~~{.cpp}
    .    std::cerr << "lexical error, full stop!" << std::endl;
         return 0;
~~~
</div>

If dot in Unicode mode would be restricted to match valid Unicode only, then
the action above will never be triggered when invalid input is encountered.
Because all non-dot regex patterns are valid Unicode in RE/flex, it would be
impossible to write a "catch all else" rule that catches input format errors!

The dot in Unicode mode is self-synchronizing and consumes text up to to the
next ASCII or Unicode character.

To accept valid Unicode input in regex patterns, make sure to avoid `.` (dot)
and use `\p{Unicode}` or `\X` instead, and reserve dot to catch anything,
such as invalid UTF encodings.  We use `.|\n` or <i>`%%option dotall`</i> to
catch anything including `\n` and invalid UTF-8/16/32 encodings.

Furthermore, before matching any input, invalid UTF-16 input is detected
automatically by the `reflex::Input` class and replaced with the
`::REFLEX_NONCHAR` code point U+200000 that lies outside the valid Unicode
range.  This code point is never matched by non-dot regex patterns and is easy
to detect by a regex pattern with a dot and a corresponding error action as
shown above.

Note that character classes written as bracket lists may produce invalid
Unicode ranges when used improperly.  This is not a problem for matching, but
may prevent rejecting surrogate halves that are invalid Unicode.  For example,
`[\u{00}-\u{10FFFF}]` obviously includes the invalid range of surrogate halves
`[\u{D800}-\u{DFFF}]`.  You can always remove surrogate halves from any
character class by intersecting the class with `[\p{Unicode}]`, that is
`[...&&[\p{Unicode}]]`.  Furthermore, character class negation with `^` results
in classes that are within range U+0000 to U+10FFFF and excludes surrogate
halves.

🔝 [Back to table of contents](#)


Error reporting and recovery                                          {#errors}
----------------------------

When your scanner or parser encounters an error in the input, the scanner or
parser should report it and attempt to continue processing the input by
recovering from the error condition.  Most compilers recover from an error to
continue processing the input until a threshold on the maximum number of errors
is exceeded.

In our lexer specification of a scanner, we may define a "catch all else" rule
with pattern `.` to report an unmatched "mystery character" that is not
recognized, for example:

<div class="alt">
~~~{.cpp}
    %class{
      static const size_t max_errors = 10;
      size_t errors;
    }

    %init{
      errors = 0;
    }

    %%
    ...  // lexer rules

    .    std::string line = matcher().line();
         std::cerr << "Error: mystery character at line " << lineno() << ":\n" << line << std::endl;
         for (size_t i = columno(); i > 0; --i)
           std::cerr << " ";
         std::cerr << "\\__ here" << std::endl;
         if (++errors >= max_errors)
           return 0;
    %%
~~~
</div>

The error message indicates the offending line number with `lineno()` and
prints the problematic line of input using `matcher().line()`.  The position on
the line is indicated with an arrow placed below the line at offset `columno()`
from the start of the line, where `columno()` takes tabs and wide characters
into account.

This error message does not take the window width into account, which may
result in misplacing the arrow when the line is too long and overflows onto the
next rows in the window, unless changes are made to the code to print the
relevant part of the line only.

There are other ways to indicate the location of an error, for example as
`-->` `<--` and highlighting the error using the ANSI SGI escape sequence for
bold typeface:

<div class="alt">
~~~{.cpp}
    .    std::string line = lexer->matcher().line();
         std::string before = line.substr(0, border());
         std::string after = line.substr(border() + size());
         std::cerr << "Error: mystery character at line " << lineno() << ":" << std::endl;
         std::cerr << before << "\033[1m --> " << str() << " <-- \033[0m" << after << std::endl;
         if (++errors >= max_errors)
           return 0;
~~~
</div>

This prints the start of the line up to the mismatching position on the line
returned by `border()`, followed by the highlighted "mystery character".
Beware that this can be a control code or invalid Unicode code point, which
should be checked before displaying it.

This scanner terminates when 10 lexical errors are encountered in the input, as
defined by `max_errors`.

By default, Bison invokes `yyerror()` (or `yy::parser::error()` with
\ref reflex-bison-cc parsers) to report syntax errors.  However, it is
recommended to use Bison error productions to handle and resolve syntax errors
intelligently by synchronizing on tokens that allow the parser to continue, for
example on a semicolon in a \ref reflex-bison-bridge parser:

<div class="alt">
~~~{.cpp}
    %{
      #include "lex.yy.h"
      #define YYPARSE_PARAM lexer
      #define YYLEX_PARAM   lexer
      void yyerror(Lexer *lexer, const char *msg);
    %}

    %pure-parser
    %lex-param { Lexer *lexer }
    %parse-param { Lexer *lexer }

    %%
    ...          // grammar rules

    | error ';'  { yyerrok; if (++lexer->errors >= lexer->max_errors) YYABORT; }
    ;
    %%
~~~
</div>

@note `%%pure-parser` is deprecated and replaced with `%%define api.pure`.

We keep track of the number of errors by incrementing `lexer->errors`.  When
the maximum number of lexical and syntax errors is reached, we bail out.

The line of input where the syntax error occurs is reported with `yyerror()` for
the \ref reflex-bison-bridge parser:

<div class="alt">
~~~{.cpp}
    void yyerror(Lexer *lexer, const char *msg)
    {
      std::string line = lexer->matcher().line();
      std::string before = line.substr(0, border());
      std::string after = line.substr(border() + size());
      std::cerr << "Error: " << msg << " at line " << lexer->lineno() << ":" << std::endl;
      std::cerr << before << "\033[1m --> " << lexer->str() << " <-- \033[0m" << after << std::endl;
    }
~~~
</div>

With option `−−flex`, the definitions part of the lexer specification is
updated as follows:

<div class="alt">
~~~{.cpp}
    %{
      #include "lex.yy.h"
      void yyerror(yyscan_t, const char*);
      #define YYPARSE_PARAM scanner
      #define YYLEX_PARAM   scanner
    %}

    %option flex

    %pure-parser
    %lex-param { void *scanner }
    %parse-param { void *scanner }
~~~
</div>

And the `yyerror()` function is updated as follows:

<div class="alt">
~~~{.cpp}
    void yyerror(yyscan_t scanner, const char *msg)
    {
      yyFlexLexer *lexer = static_cast<yyscanner_t*>(scanner);
      std::string line = lexer->matcher().line();
      std::string before = line.substr(0, border());
      std::string after = line.substr(border() + size());
      std::cerr << "Error: " << msg << " at line " << lexer->lineno() << ":" << std::endl;
      std::cerr << before << "\033[1m --> " << lexer->str() << " <-- \033[0m" << after << std::endl;
    }
~~~
</div>

@note `%%pure-parser` is deprecated and replaced with `%%define api.pure`.

These examples assume that the syntax error was detected immediately at the
last token scanned and displayed with `lexer->str()`, which may not always be
the case.

With \ref reflex-bison-bridge-locations parsers (and optionally `−−flex`),
we obtain the first and the last line of an error and we can use this
information to report the error.  For example as follows:

<div class="alt">
~~~{.cpp}
    void yyerror(YYLTYPE *yylloc, yyscan_t scanner, const char *msg)
    {
      yyFlexLexer *lexer = static_cast<yyscanner_t*>(scanner);
      std::cerr << "Error: " << msg << " at line " << yylloc->first_line << ":" << std::endl;
      if (yylloc->first_line == yylloc->last_line && yylloc->first_line == lexer->lineno())
      {
        std::cerr << lexer->matcher().line() << std::endl;
        for (int i = 0; i < yylloc->first_column; ++i)
           std::cerr << " ";
        for (int i = yylloc->first_column; i <= yylloc->last_column; ++i)
           std::cerr << "~";
        std::cerr << std::endl;
      }
      else
      {
        FILE *file = lexer->in().file(); // the current FILE* being scanned
        if (file != NULL)
        {
          YY_BUFFER_STATE buf = yy_create_buffer(file, YY_BUF_SIZE, scanner);
          yypush_buffer_state(buf, scanner); // push current buffer (matcher), use buf
          off_t pos = ftell(file); // save current position in the file
          fseek(file, 0, SEEK_SET); // go to the start of the file
          for (int i = 1; i < yylloc->first_line; ++i)
            buf->skip('\n'); // skip to the next line
          for (int i = yylloc->first_line; i <= yylloc->last_line; ++i)
          {
            std::cerr << buf->line() << std::endl; // display offending line
            buf->skip('\n'); // next line
          }
          fseek(file, pos, SEEK_SET); // restore position in the file to continue scanning
          yypop_buffer_state(scanner); // restore buffer (matcher)
        }
      }
    }
~~~
</div>

Because we use Flex-compatible reentrant functions `yy_create_buffer()`,
`yypush_buffer_state()`, and `yypop_buffer_state()` that take an extra scanner
argument, we also use options `−−flex` and `−−reentrant` in addition to
`−−bison-bridge` and `−−bison-locations` to generate the reentrant scanner for
the example shown above.

Similarly, with \ref reflex-bison-complete-locations parsers, syntax errors can
be reported as follows (without option `−−flex`):

<div class="alt">
~~~{.cpp}
    void yy::parser::error(const location& loc, const std::string& msg)
    {
      std::cerr << loc << ": " << msg << std::endl;
      if (loc.begin.line == loc.end.line && loc.begin.line == lexer.lineno())
      {
        std::cerr << lexer.matcher().line() << std::endl;
        for (size_t i = 0; i < loc.begin.column; ++i)
          std::cerr << " ";
        for (size_t i = loc.begin.column; i <= loc.end.column; ++i)
          std::cerr << "~";
        std::cerr << std::endl;
      }
      else
      {
        FILE *file = lexer.in().file(); // the current file being scanned
        if (file != NULL)
        {
          yy::scanner::Matcher *m = lexer.new_matcher(file); // new matcher
          lexer.push_matcher(m); // save the current matcher
          off_t pos = ftell(file); // save current position in the file
          fseek(file, 0, SEEK_SET); // go to the start of the file
          for (size_t i = 1; i < loc.begin.line; ++i)
            m->skip('\n'); // skip to the next line
          for (size_t i = loc.begin.line; i <= loc.end.line; ++i)
          {
            std::cerr << m->line() << std::endl; // display offending line
            m->skip('\n'); // next line
          }
          fseek(file, pos, SEEK_SET); // restore position in the file to continue scanning
          lexer.pop_matcher(); // restore matcher
        }
      }
    }
~~~
</div>

If option `−−exception` is specified with a lexer specification, for example as
follows:

<div class="alt">
~~~{.cpp}
    %option exception="yy::parser::error(location(), \"Unknown token.\")"
~~~
</div>

then we should make sure to consume some input in the exception handler to
advance the scanner forward to skip the offending input and to allow the
scanner to recover:

<div class="alt">
~~~{.cpp}
    void yy::parser::error(const location& loc, const std::string& msg)
    {
      if (lexer.size() == 0) // if token is unknown (no match)
          lexer.matcher().winput(); // skip character
      ...
    }
~~~
</div>

Error reporting can be combined with Bison Lookahead Correction (LAC), which is
enabled with:

<div class="alt">
~~~{.cpp}
    %define parse.lac full
~~~
</div>

For more details on Bison error messaging, resolution, and LAC, please see the
Bison documentation.

🔝 [Back to table of contents](#)


On using setlocale                                                 {#setlocale}
------------------

The RE/flex scanners and regex matchers use an internal buffer with UTF-8
encoded text content to scan wide strings and UTF-16/UTF-32 input.  This means
that Unicode input is normalized to UTF-8 prior to matching.  This internal
conversion is independent of the current C locale and is performed
automatically by the `reflex::Input` class that passes the UTF-8-normalized
input to the matchers.

Furthermore, RE/flex lexers may invoke the `wstr()`, `wchr()`, and `wpair()`
methods to extract wide string and wide character matches.  These methods are
also independent of the current C locale.

This means that setting the C locale in an application will not affect the
performance of RE/flex scanners and regex matchers.

As a side note, to display wide strings properly and to save wide strings to
UTF-8 text files, it is generally recommended to set the UTF-8 locale.  For
example:

~~~{.cpp}
    std::setlocale(LC_ALL, "en_US.UTF-8");        // setlocale UTF-8
    std::ifstream ifs("file.txt", std::ios::in);  // open UTF-8/16/32 text file
    reflex::BoostMatcher matcher("\\w+", ifs);    // not affected by setlocale
    while (matcher.find() != 0)
    {
      std::wstring& match = matcher.wstr();       // not affected by setlocale
      std::wcout << match << std::endl;           // affected by setlocale
    }
    ifs.close();
~~~

This displays wide string matches in UTF-8 on most consoles and terminals, but
not on all systems (I'm looking at you, Mac OS X terminal!)  Instead of
`std::wcout` we can use `std::cout` instead to display UTF-8 content directly:

~~~{.cpp}
    std::setlocale(LC_ALL, "en_US.UTF-8");       // setlocale UTF-8
    std::ifstream ifs("file.txt", std::ios::in); // open UTF-8/16/32 text file
    reflex::BoostMatcher matcher("\\w+", ifs);   // not affected by setlocale
    while (matcher.find() != 0)
    {
      std::string& match = matcher.str();        // not affected by setlocale
      std::cout << match << std::endl;           // not affected by setlocale
    }
    ifs.close();
~~~

🔝 [Back to table of contents](#)


Scanning ISO-8859-1 (latin-1) files with a Unicode scanner        {#iso-8859-1}
----------------------------------------------------------

Scanning files encoded in ISO-8859-1 by a Unicode scanner that expects UTF-8
will cause the scanner to misbehave or throw errors.

Many text files are still encoded in ISO-8859-1 (also called latin-1).  To set
up your scanner to safely scan ISO-8859-1 content when your scanner rules use
Unicode (with the `−−unicode` option and your patterns that use UTF-8
encodings), set the default file encoding to `latin`:

~~~{.cpp}
    reflex::Input input(stdin, reflex::Input::file_encoding::latin);
    Lexer lexer(input);
    lexer.lex();
~~~

This scans files from standard input that are encoded in ISO-8859-1, unless the
file has a [UTF Byte Order Mark (BOM)](www.unicode.org/faq/utf_bom.html).  When
a BOM is detected the scanner switches to UTF scanning.

See \ref regex-input-file to set file encodings.

🔝 [Back to table of contents](#)


Files with CRLF pairs                                                   {#crlf}
---------------------

DOS files and other DOS or Windows input sources typically end lines with CRLF
byte pairs.  There are two ways to effectively deal with CRLF pairs:

1. Use `reflex::Input::dos_streambuf` to automatically convert
   \ref regex-input-dosstreambuf by creating a `std::istream` for the specified
   `reflex::Input::dos_streambuf`.  Due to the extra layer introduced in the
   input processing stack, this option adds some overhead but requires no
   changes to the patterns and application code.

2. Rewrite the patterns to match both `\n` and `\r\n` to allow CRLF line
   endings.  This is option is fast to process input, but requires specialized
   patterns and the matched multi-line text will include `\r` (CR) characters
   that may need to be dealt with by the application code.

To rewrite your patterns to support CRLF end-of-line matching:

- Replace `\n` in patterns by `\r?\n`.

- Replace `.*` in patterns by `([^\n\r]|\r[^\n])*` to match any non-newline
  characters.  Likewise replace `.+` by its longer version.  Note that a single
  `.` can still be used in patterns but may match a `\r` just before a `\n`
  when a CRLF is encountered.

With the above changes, reading files on Windows systems in "binary mode" is
recommended, i.e. open `FILE*` files with the `"rb"` mode.

Reading a file in the default "text mode" interprets ^Z (0x1A) as EOF.  The
latest RE/flex releases automatically switch `FILE*` input to binary mode on
Windows systems when the file is encoded in UTF-16 or UTF-32, but not UTF-8.

🔝 [Back to table of contents](#)


Handling old Macintosh files containing CR newlines                       {#cr}
---------------------------------------------------

Old Macintosh OS file formats prior to Mac OS X use CR to end lines instead of
LF.  To automatically read and normalize files encoded in MacRoman containing
CR as newlines, you can use the `reflex::Input::file_encoding::macroman` file
encoding format.  This normalizes the input to UTF-8 and translates CR newlines
to LF newlines.  See \ref regex-input-file for details.

Alternatively, you can define a custom code page to translate CR to LF without
normalizing to UTF-8:

~~~{.cpp}
    #define LF 10
    static const unsigned short CR2LF[256] = {
        0,   1,   2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   LF,   14,   15,
       16,  17,  18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
       32,  33,  34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
       48,  49,  50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
       64,  65,  66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
       80,  81,  82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
       96,  97,  98,   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
      112, 113, 114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
      128, 129, 130,  131,  132,  133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
      144, 145, 146,  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
      160, 161, 162,  163,  164,  165,  166,  167,  168,  169,  170,  171,  172,  173,  174,  175,
      176, 177, 178,  179,  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,  190,  191,
      192, 193, 194,  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,  205,  206,  207,
      208, 209, 210,  211,  212,  213,  214,  215,  216,  217,  218,  219,  220,  221,  222,  223,
      224, 225, 226,  227,  228,  229,  230,  231,  232,  233,  234,  235,  236,  237,  238,  239,
      240, 241, 242,  243,  244,  245,  246,  247,  248,  249,  250,  251,  252,  253,  254,  255
    };
    reflex::Input input(stdin, reflex::Input::file_encoding::custom, CR2LF);
~~~

Then use the `input` object to read `stdin` or any other `FILE*`.  See also
\ref regex-input-file.

🔝 [Back to table of contents](#)


Lazy repetitions                                                        {#lazy}
----------------

Repetitions (`*`, `+`, and `{n,m}`) and the optional pattern (`?`) are greedy,
unless marked with an extra `?` to make them lazy.  Lazy repetitions are
useless when the regex pattern after the lazy repetitions permits empty input.
For example, `.*?a?` only matches one `a` or nothing at all, because `a?`
permits an empty match.

🔝 [Back to table of contents](#)


Lazy optional patterns and trigraphs                               {#trigraphs}
------------------------------------

This C/C++ trigraph problem work-around does not apply to lexer specifications
that the <b>`reflex`</b> command converts while preventing trigraphs.

Trigraphs in C/C++ strings are special tripple-character sequences, beginning
with two question marks and followed by a character that is translated.  For
example, `"x??(y|z)"` is translated to `"x[y|z)"`.

Fortunately, most C++ compilers ignore trigraphs unless in standard-conforming
modes, such as `-ansi` and `-std=c++98`.

When using the lazy optional pattern `φ??` in a regex C/C++ string for pattern
matching with one of the RE/flex matchers for example, use `φ?\?` instead,
which the C/C++ compiler translates to `φ??`.

Otherwise, lazy optional pattern constructs will appear broken.

🔝 [Back to table of contents](#)


Repeately switching to the same input                              {#switching}
-------------------------------------

The state of the input object `reflex::Input` changes as the scanner's matcher
consumes more input.  If you switch to the same input again (e.g. with `in(i)`
or `switch_stream(i)` for input source `i`), a portion of that input may end up
being discarded as part of the matcher's internal buffer is flushed when input
is assigned.  Therefore, the following code will not work because stdin is
flushed repeately:

~~~{.cpp}
    Lexer lexer(stdin);       // a lexer that reads stdin
    lexer.in(stdin);          // this is OK, nothing read yet
    while (lexer.lex(stdin))  // oops, assigning stdin again and again
      std::cout << "we're not getting anywhere?" << std::endl;
~~~

If you need to read a file or stream again, you have two options:

1. Save the current matcher and its input state with `push_matcher(m)` or
   `yypush_buffer_state(m)` to start using a new matcher `m`, e.g. created
   with `Matcher m = new_matcher(i)` to consume the specified input `i`.
   Restore the original matcher with `pop_matcher()` or `yypop_buffer_state()`.
   See also \ref reflex-multiple-input.

2. Rewind the file to the location in the file to start reading.  Beware
   that `FILE*` input is checked against an UTF BOM at the start of a file,
   which means that you cannot reliably move to an arbitrary location in the
   file to start reading when the file is encoded in UTF-8, UTF-16, or UTF-32.

🔝 [Back to table of contents](#)


Where is FlexLexer.h?                                              {#flexlexer}
---------------------

RE/flex uses its own header file <i>`reflex/flexlexer.h`</i> for compatibility
with Flex, instead of the Flex file `FlexLexer.h`.  The latter is specific to
Flex and cannot be used with RE/flex.  You should not have to include
<i>`FlexLexer.h`</i> but if you do, use:

~~~{.cpp}
    #include <reflex/flexlexer.h>
~~~

The `FlexLexer` class defined in <i>`reflex/flexlexer.h`</i> is the base class
of the generated `yyFlexLexer` class.  A name for the generated lexer class can
be specified with option `−−lexer=NAME`.

🔝 [Back to table of contents](#)


Interactive input with GNU readline                                 {#readline}
-----------------------------------

Option `-I` for interactive input generates a scanner that uses `fgetc()` to
read input from a `FILE*` descriptor (stdin by default).  Interactive input is
made more user-friendly with the GNU readline library that provides basic line
editing and a history mechanism.

To use `readline()` in your lexer, call `readline()` in your Lexer's
constructor and in the `wrap()` method as follows:

<div class="alt">
~~~{.cpp}
    %top{
      #include <stdlib.h>
      #include <stdio.h>
      #include <readline/readline.h>
      #include <readline/history.h>
    }

    %class{
      const char *prompt;
      // we use wrap() to read the next line
      virtual bool wrap() {
        if (line)
        {
          free((void*)line);
          line = readline(prompt);
          if (line != NULL)
          {
            if (*line)
              add_history(line);
            linen.assign(line).push_back('\n');
            in(linen);
          }
        }
        // wrap() == true means OK: wrapped after EOF
        return line != NULL;
      }
      // the line returned by readline() without \n
      char *line;
      // the line with \n appended
      std::string linen;
    }

    %init{
      prompt = NULL;
      line = readline(prompt);
      if (line != NULL)
      {
        if (*line)
          add_history(line);
        linen.assign(line).push_back('\n');
      }
      in(linen);
    }
~~~
</div>

With option `−−flex` you will need to replace `wrap()` by a Flex-like
`yywrap()` and change it to return 0 on success:

<div class="alt">
~~~{.cpp}
      // we use yywrap() in Flex mode to read the next line
      virtual int yywrap() {
        if (line)
        {
          free((void*)line);
          line = readline(prompt);
          if (line != NULL)
          {
            if (*line)
              add_history(line);
            linen.assign(line).push_back('\n');
            in(linen);
          }
        }
        // yywrap() == 0 means OK: wrapped after EOF
        return line != NULL ? 0 : 1;
      }
~~~
</div>

The rules can be matched as usual, where `\n` matches the end of a line, for
example:

<div class="alt">
~~~{.cpp}
    %%

    .+  echo(); // ECHO the entire line
    \n  echo(); // ECHO end of the line

    %%
~~~
</div>

🔝 [Back to table of contents](#)


Registering a handler to support non-blocking reads                 {#nonblock}
---------------------------------------------------

When `FILE*` input is read, the read operation performed with an `fread` by the
`reflex::Input` class should normally block until data is available.
Otherwise, when no data is available, an EOF condition is set and further reads
are blocked.

To support error recovery and non-blocking `FILE*` input, an event handler
can be registered.  This handler is invoked when no input is available (i.e.
`fread` returns zero) and the end of the file is not reached yet (i.e. `feof()`
returns zero).

The handler should be derived from the `reflex::Inout::Handler` abstract base
functor class as follows:

~~~{.cpp}
    struct NonBlockHandler : public reflex::Input::Handler {
      NonBlockHandler(State& state)
      :
        state(state)
      { }

      // state info: the handler does not need to be stateless
      // for example this can be the FILE* or reflex::Input object
      State& state;

      // the functor operator invoked by the reflex::Input class when fread()==0 and feof()==0
      int operator()()
      {
        ... // perform some operation here

        return 0; // do not continue, signals end of input
        return 1; // continue reading, which may fail again
      }
    };
~~~

When your event handler allows non-blocking reads to continue, make sure that
your handler does not return nonzero without delay.  A busy loop is otherwise
the result that unnecessarily burns CPU cycles.  Instead of a fixed delay,
`select()` can be effectively used to wait for input to become ready again:

~~~{.cpp}
    while (true)
    {
      struct timeval tv;
      fd_set fds;
      FD_ZERO(&fds);
      int fd = fileno(in.file());
      FD_SET(fd, &fds);
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      int r = ::select(fd + 1, &fds, NULL, &fds, &tv);
      if (r < 0 && errno != EINTR)
        return 0;
      if (r > 0)
        return 1;
    }
~~~

Here we wait in periods of one second until data is pending on the `FILE*`
stream `in.file()`, where `in` is a `reflex::Input` object.  This object can
be part of the `NonBlockHandler` state.  A timeout can be implemented by
bounding the number of loop iterations.

Note that a `FILE*` stream is set to non-blocking mode in Unix/Linux with
`fcntl()`.  Your handler is registered with `reflex::Input::set_handler()`:

~~~{.cpp}
    #include <fcntl.h>

    FILE *file = ...;
    int fd = fileno(file);
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    reflex::Input in(file);
    NonBlockHandler nonblock_handler(in);
    in.set_handler(&nonblock_handler);
    ...
    fclose(in.file());
~~~

The custom event handler can also be used to detect and clear `FILE*` errors by
checking if an error conditions exist on the `FILE*` input indicated by
`ferror()`.  Errors are cleared with `clearerr()`.  Note that a non-blocking
read that returns zero always produces nonzero `ferror()` values.

🔝 [Back to table of contents](#)


Undefined symbols and link errors                                {#link-errors}
---------------------------------

Some hints when dealing with undefined symbols and link errors when building
RE/flex applications:

- Compilation requires `libreflex` which is linked using compiler option
  `-lreflex`:

      c++ ... -lreflex

  If `libreflex` was not installed on your system then header files cannot be
  found and linking with `-lreflex` fails.  Instead, you can specify the
  include path and link the library with:

      c++ -I<path>/reflex/include ... -L<path>/reflex/lib -lreflex

  where `<path>` is the directory path to the top directory of the downloaded
  RE/flex package.

- When using an IDE such as Visual Studio C++, compile all of the <i>.cpp</i>
  source code files located in the <i>`reflex/lib`</i> and the
  <i>`reflex/unicode`</i> directories of the RE/flex download package.  The
  header files are located in the <i>`reflex/include/reflex`</i> directory.

- When Boost.Regex is used as a matcher engine, also link `libboost_regex`
  (or a variant, e.g. `libboost_regex-mt`, depending on your Boost
  installation configuration):

      c++ ... -lreflex -lboost_regex

- When PCRE2 is used as a matcher engine, also link `libpcre2-8`:

      c++ ... -lreflex -lpcre2-8

- If you get compilation errors with the `std::regex` matching engine, you
  should compile the source code as C++11:

      c++ -std=c++11 ... -lreflex

- When generating scanners with the <b>`reflex`</b> tool, the generated
  <i>`lex.yy.cpp`</i> lexer logic should be compiled and linked with your
  application.  We use <b>`reflex`</b> option `−−header-file` to generate
  <i>`lex.yy.h`</i> with the lexer class to include in the source code of your
  lexer application.

🔝 [Back to table of contents](#)


Minimized library and cross compiling                                {#linking}
-------------------------------------

RE/flex scanners generated by <b>`reflex`</b> can be linked against a minimized
version of the RE/flex library `libreflexmin`:

    c++ ... -lreflexmin

The regex Unicode converters and the Unicode tables that are not used at run
time are excluded from the minimized library.

If the RE/flex library is not installed, for example when cross-compiling
a RE/flex scanner to a different platform, then compile directly from the
RE/flex C++ source files located in the `reflex/lib` and `reflex/include`
directories:

    c++ -I. -Iinclude lex.yy.cpp lib/debug.cpp lib/error.cpp \
        lib/input.cpp lib/matcher.cpp lib/pattern.cpp lib/utf8.cpp

This compiles the code without SIMD optimizations.  SIMD intrinsics for SSE/AVX
and ARM NEON/AArch64 are used to  speed up string search and newline detection
in the library.  These optimizations are not applicable to scanners.

🔝 [Back to table of contents](#)


How to minimize runtime memory usage                                {#memusage}
------------------------------------

Runtime memory usage is determined by two entities, the pattern DFA and the
input buffer:

- Use <b>`reflex`</b> option `−−full` to create a statically-allocated table
  DFA for the scanner's regular expression patterns or option `−−fast` to
  generate a direct-coded DFA.  Without one of these options, by default a DFA
  is created at runtime and stored in heap space.

- Compile the generated source code with `-DREFLEX_BLOCK_SIZE=4096` to override
  the internal buffer `reflex::AbstractMatcher::Const::BLOCK` size.  By
  default, the `reflex::AbstractMatcher::Const::BLOCK` size is 256K for a large
  512K buffer optimized for high-performance file searching and tokenization.
  The buffer is a sliding window over the input, i.e. input files may be much
  larger than the buffer size.  A reasonably small `REFLEX_BLOCK_SIZE` is 8192
  for a 16K buffer that grows in increments of 8192 bytes to fit pattern
  matches that exceed 16K.  A small buffer automatically expands to accommodate
  larger pattern matches.  However, when using the `line()` and `wline()`
  methods, very long lines may not fit and the return string values of `line()`
  and `wline()` may be truncated as a result.  Furtheremore, a small buffer
  increase processing time, i.e. to frequently move the buffered window along a
  file and increases the cost to decode UTF-16/32 into UTF-8 multibyte
  sequences.

@warning The value of `REFLEX_BLOCK_SIZE` should not be less than 4096.

🔝 [Back to table of contents](#)


MSVC++ compiler bug                                                     {#msvc}
-------------------

Some older MSVC++ compilers may cause problems with C++11 range-based loops.
When a matcher object is constructed in a range-based loop it is destroyed
before the first loop iteration.  This means that the following example
crashes:

~~~{.cpp}
    for (auto& match : reflex::BoostMatcher("\\w+", "How now brown cow.").find)
      std::cout << match.text() << std::endl;
~~~

Instead, we should write the following:

~~~{.cpp}
    reflex::BoostMatcher matcher("\\w+", "How now brown cow.");
    for (auto& match : matcher.find)
      std::cout << match.text() << std::endl;
~~~

🔝 [Back to table of contents](#)


Bugs                                                                    {#bugs}
====

Please report bugs as RE/flex GitHub
[issues](https://github.com/Genivia/RE-flex/issues).

Please make sure to install the RE/flex library you download and remove old
versions of RE/flex or otherwise prevent mixing old with new versions.  Mixing
old with new versions may cause problems.  For example, when new versions of
RE/flex header files are imported into your project but an old RE/flex library
version is still linked with your code, the library may likely misbehave.

🔝 [Back to table of contents](#)


Installing RE/flex                                                  {#download}
==================

Download RE/flex from [SourceForge](https://sourceforge.net/projects/re-flex)
or visit the RE/flex GitHub [repository](https://github.com/Genivia/RE-flex).

🔝 [Back to table of contents](#)


License and copyright                                                {#license}
=====================

RE/flex software is released under the BSD-3 license.  All parts of the
software have reasonable copyright terms permitting free redistribution.  This
includes the ability to reuse all or parts of the RE/flex source tree.

> Copyright (c) 2016, Robert van Engelen, Genivia Inc. All rights reserved.   
>                                                                             
> Redistribution and use in source and binary forms, with or without          
> modification, are permitted provided that the following conditions are met: 
>                                                                             
>   (1) Redistributions of source code must retain the above copyright notice,
>       this list of conditions and the following disclaimer.                 
>                                                                             
>   (2) Redistributions in binary form must reproduce the above copyright     
>       notice, this list of conditions and the following disclaimer in the   
>       documentation and/or other materials provided with the distribution.  
>                                                                             
>   (3) The name of the author may not be used to endorse or promote products 
>       derived from this software without specific prior written permission. 
>                                                                             
> THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
> WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        
> MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO  
> EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      
> SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
> PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
> OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    
> WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR     
> OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF      
> ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                  

The Free Software Foundation maintains a
[BSD-3 License Wiki](http://directory.fsf.org/wiki/License:BSD_3Clause).

🔝 [Back to table of contents](#)

Copyright (c) 2016-2020, Robert van Engelen, Genivia Inc. All rights reserved.


