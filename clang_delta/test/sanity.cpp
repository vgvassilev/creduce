//RUN: %clangdelta --help %s | FileCheck %s

//CHECK: Usage:
//CHECK-NEXT:  clang_delta --transformation=<name> --counter=<number> --output=<output_filename> <source_filename>

//CHECK-NOT: Error
