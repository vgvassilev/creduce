//RUN: %clangdelta --query-instances=remove-namespace %s | FileCheck --check-prefix CHECK-QI %s
//RUN: %clangdelta --transformation=remove-namespace --counter=1 %s | FileCheck --check-prefix CHECK-ONE %s
namespace N {
   int I;

namespace M { int F; }
//CHECK-ONE-NOT: namespace M
//CHECK-QI: Available transformation instances: 1
