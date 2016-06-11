//RUN: %clang -fsyntax-only %s
//RUN: %clangdelta --query-instances=move-function-body %s 2>&1 | grep "instances: 4"

//RUN: %clangdelta --transformation=move-function-body --counter=1 %s 2>&1 | FileCheck --check-prefix CHECK-ONE %s
//RUN: %clangdelta --transformation=move-function-body --counter=1 %s 2>&1 | %clang -fsyntax-only -x c++ -
//RUN: %clangdelta --transformation=move-function-body --counter=2 %s 2>&1 | FileCheck --check-prefix CHECK-TWO %s
//RUN: %clangdelta --transformation=move-function-body --counter=2 %s 2>&1 | %clang -fsyntax-only -x c++ -
//RUN: %clangdelta --transformation=move-function-body --counter=3 %s 2>&1 | FileCheck --check-prefix CHECK-THREE %s
//RUN: %clangdelta --transformation=move-function-body --counter=3 %s 2>&1 | %clang -fsyntax-only -x c++ -
//RUN: %clangdelta --transformation=move-function-body --counter=4 %s 2>&1 | FileCheck --check-prefix CHECK-FOUR %s
//RUN: %clangdelta --transformation=move-function-body --counter=4 %s 2>&1 | %clang -fsyntax-only -x c++ -

template <typename>
class TriaIterator {};
class TriaAccessor;
class CellAccessor { void subface_case() ;  };
void CellAccessor::subface_case() {
   TriaIterator<TriaAccessor> b;
}
//CHECK-ONE: template <typename>
//CHECK-ONE-NEXT:class TriaIterator {};
//CHECK-ONE-NEXT:class TriaAccessor;
//CHECK-ONE-NEXT:class CellAccessor { void subface_case() {
//CHECK-ONE-NEXT:    TriaIterator<TriaAccessor> b;
//CHECK-ONE-NEXT:}
//CHECK-ONE: ;  };
//CHECK-ONE-NOT:void CellAccessor::subface_case() {

void functionDecl(int, float);
void functionDecl(int x, float y = 1.0);
void functionDecl(int x, float y) {
  x = y;
}
//CHECK-TWO: void functionDecl(int x, float y = 1.) {
//CHECK-TWO-NEXT: x = y;
//CHECK-TWO-NEXT: }
//CHECK-TWO-NOT: void functionDecl(int x, float y = 1.0);
//CHECK-TWO-NOT: void functionDecl(int, float);

struct S {
  void f() const;
};
void S::f() const {
  int i = 42;
}
//CHECK-THREE:struct S {
//CHECK-THREE-NEXT:  void f() const {
//CHECK-THREE-NEXT:    int i = 42;
//CHECK-THREE-NEXT:}
//CHECK-THREE-NOT:void S::f() const {

namespace A {
  void f();
  void f() { }
//CHECK-FOUR-NOT: void f();
  void dont_crash() { } // We don't have a decl to merge with, we should skip.
}
