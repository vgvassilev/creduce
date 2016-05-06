//RUN: %clangdelta --help %s | FileCheck %s
//RUN: %clangdelta --help %s | FileCheck --check-prefix CHECK-SPACE %s

//CHECK: Usage:
// CHECK-SPACE: Usage:
//CHECK-NEXT:  clang_delta --transformation=<name> --counter=<number> --output=<output_filename> <source_filename>

//CHECK-NOT: Error
//        CHECK-SPACE-NOT: Error
