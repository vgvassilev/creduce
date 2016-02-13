##C-Reduce


Before compiling C-Reduce yourself, you might want to see if your OS
comes with a precompiled package for C-Reduce. Ubuntu, Debian, Gentoo,
and Mac OS X (Homebrew) all do.


##Prereqs:


C-Reduce is written in Perl, C++, and C.  To compile and run C-Reduce,
you will need a development environment that supports these languages.
C-Reduce's build system requires GNU Make (*not* BSD Make).

Beyond the basic compile/build tools, C-Reduce depends on a set of
third-party software packages, including LLVM.

### Ubuntu or Mint

The prerequisites other than LLVM can be installed like this:

```
  sudo apt-get install \
    libexporter-lite-perl libfile-which-perl libgetopt-tabular-perl \
    libregexp-common-perl flex build-essential \
    zlib1g-dev
```

### OS X

Perlbrew provides an easy and flexible way to get Perl and Perl modules
installed:

  http://perlbrew.pl/

### Others

Install these packages either manually or using a package manager:

* Flex: http://flex.sourceforge.net/

* LLVM/Clang 3.7.0: http://llvm.org/releases/download.html#3.7.0

  (No need to compile it: the appropriate "Clang binaries" package is
  all you need.  If you use one of the binary packages, you may need to
  install additional packages that the binary package depends on. For
  example, the "Clang for x86_64 Ubuntu 14.04" binary package depends on
  "libedit2" and "libtinfo5". You may need to install these, e.g.:
  "sudo apt-get install libedit-dev libtinfo-dev".)

* Perl modules:
  * Exporter::Lite
  * File::Which
  * Getopt::Tabular
  * Regexp::Common

For example, (perhaps as root):
```
  cpan -i 'Exporter::Lite'
  cpan -i 'File::Which'
  cpan -i 'Getopt::Tabular'
  cpan -i 'Regexp::Common'
```
* zlib: http://www.zlib.net/


##Optional prereqs:


GNU Indent, astyle, Term::ReadKey, and Sys::CPU are optional; C-Reduce
will use whichever of them are installed.

In particular, we recommend installing Sys::CPU; without this module
C-Reduce will not use multiple cores unless explicitly requested to do
so with the -n command line option.

### Ubuntu or Mint:

  sudo apt-get install indent astyle libterm-readkey-perl libsys-cpu-perl

### OS X (with Homebrew + Perlbrew installed):

```
  brew install astyle gnu-indent
  cpan -i 'Term::ReadKey'
  cpan -i 'Sys::CPU'
```

Otherwise, install the packages either manually or using the package
manager:

### Others

* astyle: http://astyle.sourceforge.net/

* GNU Indent: http://www.gnu.org/software/indent/


##Building and installing C-Reduce:


From either the source directory or a build directory:

```
  [source-path/]configure [options]
  make
  make install
```

The `configure` script was generated by GNU Autoconf, and therefore
accepts the usual options for naming the installation directories,
choosing the compilers you want to use, and so on. `configure --help`
summarizes the command-line options.

If LLVM/Clang is not in your search path, you can tell the `configure'
script where to find LLVM/Clang:

```
  # Use the LLVM/Clang tree rooted at /opt/llvm
  configure --with-llvm=/opt/llvm
```

Note that assertions are disabled by default.  To enable assertions:

```
  configure --enable-trans-assert
```

The generated Makefiles require GNU Make.  BSD Make will not work.
If you see weird make-time errors, please check that you are using
GNU Make.


##Building and installing C-Reduce with cmake:


```
cmake /path/to/src -DTOPFORMFLAT=/path/to/delta-2006.08.03/ -DCMAKE_INSTALL_PREFIX=/path/to/inst/folder/ -DLLVM_DIR=/path/to/debug/installation/of/llvm/share/llvm/cmake/
make -j4
make install
```


##Regarding LLVM versions:


Released versions of C-Reduce, and also our master branch at Github,
need to be compiled against specific released versions of LLVM, as
noted in this file.

Our GitHub repo usually also has a branch called llvm-svn-compatible
that supports building C-Reduce against the current LLVM SVN head.


##Developers' Corner:


###Building with current LLVM trunk and cmake:
```
cmake /path/to/src -DCMAKE_BUILD_TYPE=Debug \
  -DTOPFORMFLAT=/path/to/delta-2006.08.03/ \
  -DCMAKE_INSTALL_PREFIX=/path/to/inst/folder/ \
  -DLLVM_DIR=/path/to/debug/installation/of/llvm/share/llvm/cmake/
make -j4
make install
```

In order to enable LIT testing support of clang_delta one needs
 `-DLLVM_DIR=/path/to/llvm/build/folder/share/llvm/cmake`. Then type
 `make check-clangdelta` to run the test suite.