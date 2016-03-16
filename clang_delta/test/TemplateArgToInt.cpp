//RUN: %clangdelta --transformation=template-arg-to-int --counter=1 %s | FileCheck %s
template <typename T> struct TriaIterator{};
struct CellAccessor{};

typedef TriaIterator<CellAccessor> cell_iterator;

//CHECK: typedef TriaIterator<int> cell_iterator;
