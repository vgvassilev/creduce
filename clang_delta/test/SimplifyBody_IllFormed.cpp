//RUN: %clangdelta --query-instances=simplify-body %s 2>&1 | FileCheck -check-prefix=CHECK-QI %s

// This is broken source code but it might happen if CREDUCE_INCLUDE_PATH is not
// properly set. We need to make sure we don't crash.
class FwdDeclS;
FwdDeclS& should_ignore() {
  S s;
  return reinterpret_cast<FwdDeclS&>(s);
}

FwdDeclS& FwdDeclsFactory();

FwdDeclS should_ignore1() { return FwdDeclsFactory(); }
// End broken code.

//CHECK-QI: Available transformation instances: 0
