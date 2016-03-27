//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef REMOVE_DECL_H
#define REMOVE_DECL_H

#include "Transformation.h"

#include "clang/AST/Decl.h"

#include <vector>

namespace clang {
  class ASTContext;
  class Decl;
}

class RemoveDecl : public Transformation {
private:
  std::vector<clang::Decl *> Decls;
  std::set<clang::Decl *> DeclsLookup;
  virtual void Initialize(clang::ASTContext &context) override;
  virtual void HandleTranslationUnit(clang::ASTContext &Ctx) override;

public:
  void addDecl(clang::Decl *val) {
    if (!DeclsLookup.count(val)) {
      DeclsLookup.insert(val);
      Decls.push_back(val);
      ++ValidInstanceNum;
    }
  }
  RemoveDecl(const char *TransName, const char *Desc)
    : Transformation(TransName, Desc, /*MultipleRewrites*/true)
  { }
};
#endif // REMOVE_DECL_H
