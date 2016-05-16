//RUN: %clangdelta --transformation=remove-decl --counter=1 %s 2>&1 | FileCheck %s
//RUN: %clangdelta --transformation=remove-decl --counter=1 --to-counter=2 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI %s
//RUN: %clangdelta --transformation=remove-decl --counter=2 --to-counter=4 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-A %s
//RUN: %clangdelta --transformation=remove-decl --counter=5 --to-counter=7 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI-B %s
//RUN: %clangdelta --query-instances=remove-decl %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

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
