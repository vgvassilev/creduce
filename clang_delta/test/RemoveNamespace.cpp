//RUN: %clang -fsyntax-only %s
//RUN: %clangdelta --transformation=remove-namespace --counter=1 %s | FileCheck %s

# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X  {
# define _GLIBCXX_END_NAMESPACE }

_GLIBCXX_BEGIN_NAMESPACE(std)
_GLIBCXX_END_NAMESPACE

namespace std {}

//CHECK:# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X  {
//CHECK-NEXT:# define _GLIBCXX_END_NAMESPACE }
//CHECK-NOT: _GLIBCXX_BEGIN_NAMESPACE(std)
//CHECK-NOT: _GLIBCXX_END_NAMESPACE
//CHECK-NOT: namespace std {}
