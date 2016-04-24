//RUN: %clang -fsyntax-only %s
//RUN: %clangdelta --transformation=local-to-global --counter=1 %s 2>&1 | FileCheck %s

template <typename T>
class TriaIterator {};
class TriaAccessor;
class CellAccessor {
  void subface_case() {
    TriaIterator<TriaAccessor> b;
  }
};
// CHECK: TriaIterator<TriaAccessor> subface_case_b;
// CHECK-NEXT: class CellAccessor {
// CHECK-NOT: TriaIterator<TriaAccessor> b;
