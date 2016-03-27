//RUN: %clangdelta --transformation=remove-decl --counter=1 %s 2>&1 | FileCheck %s
//RUN: %clangdelta --transformation=remove-decl --counter=2 --to-counter=3 %s 2>&1 | FileCheck  -check-prefix=CHECK-MULTI %s
//RUN: %clangdelta --query-instances=remove-decl %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

struct S {
   int h;
   float b;
//CHECK-MULTI-NOT: float b;
};

enum e {
   a,
//CHECK-MULTI-NOT: a,
   b,
   c
};

;;;
int i;
//CHECK: ;;;
//CHECK-NOT: int i;


//CHECK-QI: Available transformation instances: 7
