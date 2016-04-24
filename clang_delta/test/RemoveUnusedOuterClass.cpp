//RUN: %clangdelta --transformation=remove-unused-outer-class --counter=1 %s 2>&1 | FileCheck %s

template <typename T>
class TriaIterator {};
class TriaAccessor;
class CellAccessor {
  TriaIterator<TriaAccessor> b;
};
// CHECK-NOT: CellAccessor
// CHECK: TriaIterator<TriaAccessor> b;
