//RUN: %clangdelta --transformation=remove-decl --counter=1 %s 2>&1 | FileCheck %s

;;;
int;
//CHECK: ;;
//CHECK-NOT: ;;;
