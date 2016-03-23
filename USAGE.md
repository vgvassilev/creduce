## Example usage scenarios for C-Reduce.

### Single file reductions.

### Multiple files reductions.

Currently it requires several steps. For instance, we want to reduce the
dependencies of `std::basic_string<char>`.

1. T.cpp

```
#include <string>
std::basic_string<char> myStr;
int main() { return 0; }
```

2. Find all include files in the translation unit.

This can be done either by
```
gcc -fsyntax-only  T.cpp -H
```
or
```
clang -Xclang -header-include-file -Xclang included_files.txt -fsyntax-only T.cpp && sed -i.bak '/<command line>/d' included_files.txt
```

or using the script `localize_headers`.

3. Copy the system headers locally.
```
cat included_files.txt | xargs -I$  python -c "import os,sys; print os.path.abspath(sys.argv[1])" $ | xargs -I$ rsync -R -L $ includes/
```

`rsync` requires normalized paths. The easiest is to use python to do it.

4. Define our interestingness test (test.sh).

```
#!/bin/bash
export CREDUCE_LANG=CXX
gcc -fsyntax-only --sysroot=./includes/ T.cpp &&  \
grep -q "std::basic_string<char> myStr" T.cpp
```

We export the CREDUCE_LANG=CXX to tell creduce that all reductions should happen
in C++ mode. Without this setting creduce (or more precisely clang_delta) would
try to guess the language by looking at the file extension. This is not enough
when reducing header files (coming from STL, etc) because mislead clang_delta to
switch to C mode, making the reduction very slow and most of the time impossible.

Don't forget to use the --sysroot option, which tells the compiler to switch its
includes to the local ones, which we will reduce.

5. Do it.

```
creduce --tidy test.sh T.cpp includes/
```
