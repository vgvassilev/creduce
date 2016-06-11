//RUN: %clang_cc1 -fsyntax-only -verify %s
//RUN: %clangdelta --transformation=simplify-if --counter=1 %s | FileCheck %s

void prof_lookup(void) {
  void *prof_tdata;
  // clang constructs OpaqueValueExpr to help error recovery.
  if (prof_tdata->bt2cnt) { } // expected-error {{member reference base type 'void' is not a structure or union}}
//CHECK: {{[{][{]}}
//CHECK-NOT:if (prof_tdata->bt2cnt)
}

