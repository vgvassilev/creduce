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
clang -Xclang -header-include-file -Xclang included_files.txt -fsyntax-only T.cpp && sed -i '/<command line>/d' included_files.txt
```

or using the script `localize_headers`.

3. Copy the system headers locally.
```
cat included_files.txt | xargs -I$ cp --parents $ includes/
```

4. Define our interestingness test (test.sh).

```
#!/bin/bash
gcc -fsyntax-only --sysroot=./includes/ T.cpp &&  \
grep -q "std::basic_string<char> myStr" T.cpp
```

Don't forget to use the --sysroot option, which tells the compiler to switch its
includes to the local ones, which we will reduce.

5. Do it.

```
creduce --tidy test.sh T.cpp includes/
