//RUN: %clang -fsyntax-only %s
//RUN: %clangdelta --transformation=param-to-local --counter=1 %s 2>&1 | FileCheck %s
template<int>
struct DoFHandler {
  void finite_elements();
};
void point_gradient(DoFHandler<3> dof) { dof.finite_elements(); }
// CHECK: void point_gradient(void) {
// CHECK-NEXT:   DoFHandler<3> dof; dof.finite_elements(); }
