How to contribute to RE/flex
============================

Thank you for taking the time to contribute.  Let's keep moving forward
together to make RE/flex the best lexical analyzer generator for C++ on the
planet.

The following is a set of guidelines for contributing to RE/flex and its
packages.

Contribution and participation in the RE/flex project requires adherence to our
[code of conduct](CODE_OF_CONDUCT.md).

Did you find a bug?
-------------------

- Ensure the bug was not already reported by searching on GitHub under
  [Issues](https://github.com/Genivia/RE-flex/issues).
- If you're unable to find an open issue addressing the problem, open a new one.
  Be sure to include a title and clear description, as much relevant
  information as possible, and a code sample or an executable test case
  demonstrating the expected behavior that is not occurring.

Do you have a patch that fixes a bug?
-------------------------------------

- Use the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
  as a guideline to write the source code for the patch.
- Open a new GitHub pull request with the patch.
- Write a clear log message for your commits.  One-line messages are fine for
  small changes, but bigger changes should look like this:

      $ git commit -m "A brief summary of the commit
      > 
      > A paragraph describing what changed and its impact."

Do you intend to add a new feature or change an existing one?
-------------------------------------------------------------

We love to hear from you!  [Contact us](https://www.genivia.com/contact.html).

Contributions are covered under the [BSD-3 license](LICENSE.txt).

Prepating a pull request
-------------------------------------------------------------

When creating a pull request, please consider the following steps before
commiting your changes:

~~~
./clean.sh                       # clean all before we start
./build.sh                       # make sure this step works
cd tests; make -f Make; cd ..    # make sure this step works
cd examples; make -f Make; cd .. # make sure this step works
./clean.sh
aclocal
autoheader
rm -f config.guess config.sub ar-lib compile depcomp install-sh missing
automake --add-missing --foreign
autoconf
automake
~~~

When these steps succeed, the Travis continuous integration builds and tests
can succeed.

Thanks!

Robert van Engelen, Genivia Inc.
