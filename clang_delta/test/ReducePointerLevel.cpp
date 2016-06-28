//RUN: %clang_cc1 -fsyntax-only %s
//RUN: %clangdelta --query-instances=reduce-pointer-level %s | FileCheck --check-prefix=CHECK-QI %s
//RUN: %clangdelta --transformation=reduce-pointer-level --counter=1 %s | FileCheck --check-prefix=CHECK-ONE %s | %clang -fsyntax-only -x c++ -

namespace CXXNewRHS {
  struct variant {
    variant(const variant&);
    void member ();
  };
  typedef const variant ** variants;
  typedef const variants* const_variants_ptr;

  variant::variant( const variant& v ) {
    // Make sure CXXNewExpr on the RHS don't crash clang delta.
    *reinterpret_cast<variants**>(this)
      = new variants(**reinterpret_cast<const const_variants_ptr*>(&v));
  }
}

void f() {
  int ** i;
  int *** j = &i;
//CHECK-ONE: int ** j = i;
}

//CHECK-QI: Available transformation instances: 1
