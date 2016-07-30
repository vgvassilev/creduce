## Example usage scenarios for C-Reduce

### Single file reductions

<a name="MultiFileReductions"></a>
### Multiple files reductions

Use ramdisk when working with many files.

  - Ubuntu
     ```
     sudo mkdir -p /media/ramdisk1
     sudo mount -t tmpfs -o size=256M tmpfs /media/ramdisk1/
     creduce --tempdir /media/ramdisk1/ ...

     ```

  - OSX
     ```
     # The disk size is calculated as: desired_size * 2018. Eg. 256 * 2048 = 524288
     diskutil erasevolume HFS+ 'RAM Disk' `hdiutil attach -nomount ram://524288`
     # Disable system logs on this drive
     mdutil -i off /Volumes/RAMDisk1/
     cd /Volumes/RAMDisk1/
     mkdir .fseventsd
     touch .fseventsd/no_log .metadata_never_index .Trashes
     cd -
     creduce --tempdir /Volumes/RAMDisk1/ ...
     ```

Currently it requires several steps. For instance, we want to reduce the
dependencies of `std::basic_string<char>`.

1. T.cpp

```bash
#include <string>
std::basic_string<char> myStr;
int main() { return 0; }
```

2. Find all include files in the translation unit.

This can be done either by
```bash
gcc -fsyntax-only  T.cpp -H
```
or
```bash
clang -Xclang -header-include-file -Xclang included_files.txt -fsyntax-only T.cpp && sed -i.bak '/<command line>/d' included_files.txt
```

or using the script `localize_headers`.

3. Copy the system headers locally.
```bash
cat included_files.txt | xargs -I$  python -c "import os,sys; print os.path.abspath(sys.argv[1])" $ | xargs -I$ rsync -R -L $ includes/
```

`rsync` requires normalized paths. The easiest is to use python to do it.

4. Define our interestingness test (test.sh).

```bash
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

```bash
creduce --tidy test.sh T.cpp includes/
```

### Reducing Clang C++ Modules Bugs

Most of the time we cannot preprocess the file and pipe it in creduce because
the issues disappear. Instead we need to follow the procedure described in
[Multiple files reductions](#MultiFileReductions).

There are a few things we need to do "by hand" before running creduce:

  * Reduce the compilation flags. Especially `-fimplicit-module-maps` needs to be
replaced by one or more `-fmodule-map-file=relative/path/to/concrete/module/map`.
We need to also tell the compiler it shouldn't look for any system includes by
default, using `-nostdsysteminc`

  * Some system module maps (mostly on OS X) impose "hidden" dependencies due to
the implicit module creation. For example a submodule X is being built as part
of a top module M, when requesting submodule Y. This means that we have to copy
everything relative to the modulemap file.

  * Some of the copied subfolders are not present in the module map and thus
redundant. One can reduce them by using the script located in
[scripts](scripts/reduce_include_folders.sh).

```bash
# The crash needs `-fmodule-map-file=/usr/include/module.modulemap`. This means
# that we need to copy everything in /usr/include/ locally to our dedicated
# `includes` folder.
cp -r /usr/include includes/usr/

# The crashing compilation line looks like this:
"/Users/vvassilev/workspace/llvm-git/inst/bin/clang" -cc1 -nostdsysteminc \
-I includes/Users/vvassilev/workspace/root_obj/include/ \
-I includes/usr/include/ \
-I includes/Users/vvassilev/workspace/llvm-git/inst/include/c++/v1/ \
-stdlib=libc++ -std=c++11   -fmodules -fmodules-cache-path=./mod_cache/ \
-fmodule-map-file=includes/Users/vvassilev/workspace/root_obj/include/module.modulemap \
-fmodule-map-file=includes/usr/include/module.modulemap \
-fsyntax-only -x c++ TStopwatch.cxx 2>&1

# The test.sh looks like this:
cat test.sh
#!/bin/bash
export CREDUCE_LANG=CXX
export CREDUCE_INCLUDE_PATH="includes/Users/vvassilev/workspace/root_obj/include/:includes/usr/include/:includes/Users/vvassilev/workspace/llvm-git/inst/include/c++/v1/"

# First we compile without modules to make sure that the transformation was
# syntactically correct.
"/Users/vvassilev/workspace/llvm-git/inst/bin/clang" -cc1 -nostdsysteminc  \
-I includes/Users/vvassilev/workspace/root_obj/include/ \
-I includes/usr/include/ \
-I includes/Users/vvassilev/workspace/llvm-git/inst/include/c++/v1/ \
-stdlib=libc++ -std=c++11  -fsyntax-only -x c++ TStopwatch.cxx 2>/dev/null \
    && "/Users/vvassilev/workspace/llvm-git/inst/bin/clang" -cc1 -nostdsysteminc \
    -I includes/Users/vvassilev/workspace/root_obj/include/ \
    -I includes/usr/include/ \
    -I includes/Users/vvassilev/workspace/llvm-git/inst/include/c++/v1/ \
    -stdlib=libc++ -std=c++11 -fmodules -fmodules-cache-path=./mod_cache/ \
    -fmodule-map-file=includes/Users/vvassilev/workspace/root_obj/include/module.modulemap \
    -fmodule-map-file=includes/usr/include/module.modulemap \
    -fsyntax-only -x c++ TStopwatch.cxx 2>&1 \
    | grep -q 'Assertion failed: (DeclKind != Decl::LinkageSpec'
# Note the 2>&1 allowing grep to scan stderr too.

# Then run the reduce_include_folders.sh script
cp path/to/creduce/src/tree/scripts/reduce_include_folders.sh .
./reduce_include_folders.sh

# TODO: Soon I will have the script reducing the modulemap file. It reduces
# modulemap's submodules, which will help ./reduce_include_folders.sh to remove
# more useless modulemap implicitly dependent files/folders.

# Run creduce and patiently wait :) Please note that if you use a debug version
# of the compiler in your interestingness test creduce can be a lot slower (~10x)
# than using a release build of your compiler.

~/path/to/creduce/binary/creduce --tidy test.sh TStopwatch.cxx includes/*

```
