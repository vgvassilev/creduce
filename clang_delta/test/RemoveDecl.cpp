//RUN: %clangdelta --transformation=remove-decl --counter=1 %s 2>&1 | FileCheck %s
//RUN: %clangdelta --transformation=remove-decl --counter=2 --to-counter=3 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-A %s
//RUN: %clangdelta --transformation=remove-decl --counter=5 --to-counter=6 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-B %s
//RUN: %clangdelta --query-instances=remove-decl %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

struct S {
   int h;
   float b;
//CHECK-MULTI-A-NOT: float b;
};

enum e {
   a,
//CHECK-MULTI-A-NOT: a,
   b,
   c_
//CHECK-MULTI-B-NOT: c_
};

struct ConstantPlaceHolder {
   template <int> float &Op();
} ab;
//CHECK-MULTI-B-NOT: ab


;;;
int i;
//CHECK: ;;;
//CHECK-NOT: int i;


//CHECK-QI: Available transformation instances: 8
