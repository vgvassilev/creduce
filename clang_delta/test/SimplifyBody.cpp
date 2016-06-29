//RUN: %clang_cc1 -fsyntax-only -fcxx-exceptions %s
//RUN: %clangdelta --transformation=simplify-body --counter=1 %s 2>&1 | FileCheck %s | %clang_cc1 -fsyntax-only -xc++ -
//RUN: %clangdelta --transformation=simplify-body --counter=1 --to-counter=2 %s 2>&1 | FileCheck %s | %clang_cc1 -fsyntax-only -xc++ -
//RUN: %clangdelta --transformation=simplify-body --counter=1 --to-counter=5 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI %s | %clang_cc1 -fsyntax-only -xc++ -
//RUN: %clangdelta --query-instances=simplify-body %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

void f() {
  int i = 0, j;
  for (int f = i; i < 10; ++i)
    ++j;
}

struct S {
  int field;
  S* g() {
    if (field == 1)
      return new S();
    else if (field == 0)
      return 0;
    return 0;
  }
  // Since the simplify-body heuristics order the decls by size of reduction
  // this is our best candidate and thus it gets removed first.
  // CHECK: S* g() {return 0;}
  // CHECK-MULTI: S* g() {return 0;}
  S& forward_decl_ref();
  S& forward_decl();
  S& h() {
    if (field == 1)
      return forward_decl_ref();
    return forward_decl();
  }
  // CHECK-MULTI: S& h() {static S s; return s;}
  S i() {
    if (field == 1)
      return forward_decl();
    return forward_decl_ref();
  }
  // CHECK-MULTI: S i() {return S();}
};

int CXXTryStmtAsBody ()
  try {
    int i = 10;
    return i;
  }
  catch(...) {
    return 0;
  }
//CHECK-MULTI:int CXXTryStmtAsBody ()
//CHECK-MULTI-NEXT: {return 0;}

namespace no_repeat {
  // We must ignore these, since they come from simplify-body. I.e. they should
  // not be considered as viable candidates by clang_delta --query-instances.
  bool f() {return 0;}
  struct S {};
  S& return_S() {static S __CREDUCE_AUTO_GENERATED__; return __CREDUCE_AUTO_GENERATED__;}
}

//CHECK-QI: Available transformation instances: 5
