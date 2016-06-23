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
#include "clang/AST/RecursiveASTVisitor.h"

#include "llvm/ADT/SetVector.h"

#include <vector>

namespace clang {
  class ASTContext;
  class Decl;
  class DeclContext;
  class SourceManager;
}

class RemoveDecl : public Transformation,
                   public clang::RecursiveASTVisitor<RemoveDecl> {
private:
  llvm::SetVector<clang::Decl *> Decls;
  std::vector<clang::Decl *> RemovedDecls;

  bool isInSourceRangeOfAlreadyRemovedDecl(clang::Decl* newD) const;
  void checkAndReplaceIfDeclEnclosesAnyExistingDecl(clang::Decl* val);
  void removeDecl(clang::Decl *D) {
    TransAssert(!isInSourceRangeOfAlreadyRemovedDecl(D)
                && "Deleted by previous operation!");
    bool success = Decls.remove(D);
    TransAssert(success && "Cannot find what to remove.");
    success = RewriteHelper->removeDecl(D);
    TransAssert(success && "Cannot find remove decl.");
    RemovedDecls.push_back(D);
  }

public:
  RemoveDecl(const char *TransName, const char *Desc)
    : Transformation(TransName, Desc, /*MultipleRewrites*/true)
  { }
  void maybeAddDecl(clang::Decl *val) {
    if (!Decls.count(val)) {
      Decls.insert(val);
      ++ValidInstanceNum;
    }
  }

  // Transformation
  virtual void Initialize(clang::ASTContext &context) override;
  virtual void HandleTranslationUnit(clang::ASTContext &Ctx) override;

  //RecursiveASTVisitor
  bool VisitDecl(clang::Decl *D);
  bool VisitDeclContext(clang::DeclContext *DC);

};
#endif // REMOVE_DECL_H
