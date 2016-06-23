//RUN: %clang_cc1 -fsyntax-only -std=c++14  %s
//RUN: %clangdelta --transformation=remove-decl --counter=1 --to-counter=1 %s 2>&1 | FileCheck -check-prefix=CHECK-DIAG %s
//RUN: %clangdelta --transformation=remove-decl --counter=18 %s 2>&1 | FileCheck %s
//RUN: %clangdelta --transformation=remove-decl --counter=1 --to-counter=2 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI %s
//RUN: %clangdelta --transformation=remove-decl --counter=2 --to-counter=4 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-A %s
//RUN: %clangdelta --transformation=remove-decl --counter=6 --to-counter=8 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-B %s
//RUN: %clangdelta --transformation=remove-decl --counter=10 --to-counter=13 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-B1 %s
//RUN: %clangdelta --transformation=remove-decl --counter=1 --to-counter=31 %s 2>&1 | FileCheck  -check-prefix=CHECK-ALL %s
//RUN: %clangdelta --query-instances=remove-decl %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

//CHECK-DIAG: Error: The to-counter value exceeded the number of transformation instances!
struct S {
   int h;
//CHECK-MULTI-NOT: h;
   float b;
//CHECK-MULTI-A-NOT: float b;
};

enum e {
   a,
   b,
   c_
//CHECK-MULTI-B-NOT: b,
//CHECK-MULTI-B-NOT: c_
};

struct ConstantPlaceHolder {
   template <int> float &Op();
} ab;
//CHECK-MULTI-B1-NOT: struct ConstantPlaceHolder
//CHECK-MULTI-B1-NOT: template <int> float &Op();
//CHECK-MULTI-B1-NOT: ab;


;;;
int i;
//CHECK: ;;;

namespace QtPrivate {
   template <typename...> struct List ;
   template <typename Functor, typename ArgList> struct ComputeFunctorArgumentCount;
   template <typename Functor, typename... ArgList> struct ComputeFunctorArgumentCount<Functor, List<ArgList...>> {
      template <typename D> static D dummy;
      template <typename F> static auto test(F f) -> decltype(f.operator(), int());
   };
}

//CHECK-ALL-NOT: {{.*\S}}
//CHECK-QI: Available transformation instances: 31
