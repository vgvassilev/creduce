//RUN: %clang -std=c++11 -fsyntax-only %s
//RUN: %clangdelta --transformation=remove-namespace --counter=1 %s | FileCheck --check-prefix=CHECK-ONE %s | %clang -std=c++11 -fsyntax-only -x c++ -
//RUN: %clangdelta --transformation=remove-namespace --counter=2 %s | FileCheck --check-prefix=CHECK-TWO %s | %clang -fsyntax-only -x c++ -
//RUN: %clangdelta --transformation=remove-namespace --counter=3 %s | FileCheck --check-prefix=CHECK-THREE %s | %clang -fsyntax-only -x c++ -

# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X  {
# define _GLIBCXX_END_NAMESPACE }

_GLIBCXX_BEGIN_NAMESPACE(std)
_GLIBCXX_END_NAMESPACE

namespace std {}

//CHECK-ONE:# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X  {
//CHECK-ONE-NEXT:# define _GLIBCXX_END_NAMESPACE }
//CHECK-ONE-NOT: _GLIBCXX_BEGIN_NAMESPACE(std)
//CHECK-ONE-NOT: _GLIBCXX_END_NAMESPACE
//CHECK-ONE-NOT: namespace std {}


namespace {
 struct A {
   template <typename F> static auto m_fn1(F f) -> decltype(f);
 };
}
//CHECK-TWO-NOT: namespace {
//CHECK-TWO: struct A {
//CHECK-TWO-NEXT: template <typename F> static auto m_fn1(F f)

namespace __gnu_cxx {
  enum _Lock_policy { _S_atomic } const __default_lock_policy = _S_atomic;
}
namespace {
  using __gnu_cxx::_Lock_policy;
  using __gnu_cxx::__default_lock_policy;
  _Lock_policy f = (_Lock_policy)1;
}

//CHECK-THREE-NOT: __gnu_cxx
//CHECK-THREE-NOT: using __gnu_cxx::
//CHECK-THREE: enum _Lock_policy
//CHECK-THREE: _Lock_policy f = (_Lock_policy)1
