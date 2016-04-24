//RN: %clang -fsyntax-only %s
//RN: %clangdelta --transformation=param-to-global --counter=1 %s 2>&1 | FileCheck %s

template <typename T>
class TriaIterator {};
class TriaAccessor;
class CellAccessor {
  void subface_case() {
    TriaIterator<TriaAccessor> b;
  }
};
// CHECK: class CellAccessor {
// CHECK-NEXT: TriaIterator<TriaAccessor> b;
// CHECK-NEXT: };
