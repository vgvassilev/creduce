//===----------------------------------------------------------------------===//
//
// Copyright (c) 2016 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef SIMPLIFY_BODY_H
#define SIMPLIFY_BODY_H

#include "Transformation.h"

#include "clang/AST/Decl.h"

#include <set>
#include <vector>

namespace clang {
  class ASTContext;
  class FunctionDecl;
}

class SimplifyBody : public Transformation {
private:
  std::vector<clang::FunctionDecl *> Decls;
  std::set<clang::FunctionDecl *> DeclsLookup;
  virtual void Initialize(clang::ASTContext &context) override;
  virtual void HandleTranslationUnit(clang::ASTContext &Ctx) override;

public:
  SimplifyBody(const char *TransName, const char *Desc)
    : Transformation(TransName, Desc, /*MultipleRewrites*/true)
  { }

  void addDecl(clang::FunctionDecl *val) {
    if (!DeclsLookup.count(val)) {
      DeclsLookup.insert(val);
      Decls.push_back(val);
      ++ValidInstanceNum;
    }
  }
};
#endif // SIMPLIFY_BODY_H
