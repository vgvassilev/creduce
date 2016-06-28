//RUN: %clang_cc1 -fsyntax-only -verify %s
//RUN: %clangdelta --transformation=template-arg-to-int --counter=1 %s | FileCheck %s | %clang_cc1 -fsyntax-only -x c++ -

template <typename T> struct TriaIterator{};
struct CellAccessor{};

typedef TriaIterator<CellAccessor> cell_iterator;

//CHECK: typedef TriaIterator<int> cell_iterator;

namespace complex {
  // Make sure we don't crash on C99 _Complex types.
  void f(__complex__ float __z) { (void) __z; }
}

namespace dependent_template_types {
  // Make sure we don't crash on instantiation-dependent template names.
  template <typename T, template<typename X> class Allocator>
  class zero_allocator : public Allocator<T> {};
}

namespace atomic_types {
  // Make sure we don't crash on _Atomic types.
  typedef _Atomic( int ) atomic_int_least32_t;
  void atomic_increment( atomic_int_least32_t * use ) { (void)use; }
}
//expected-no-diagnostics
