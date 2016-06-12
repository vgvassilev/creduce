//RUN: %clang -fsyntax-only %s
//RUN: %clangdelta --transformation=class-template-to-class --counter=1 %s 2>&1 | FileCheck %s | %clang -fsyntax-only -x c++ -

enum _Lock_policy { _S_atomic };
template <_Lock_policy> class _Sp_counted_base {
public:
  ~_Sp_counted_base() {}
};
//CHECK: class _Sp_counted_base
//CHECK-NOT: template <_Lock_policy> class _Sp_counted_base
