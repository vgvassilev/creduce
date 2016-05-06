//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef MOVE_FUNCTION_BODY_H
#define MOVE_FUNCTION_BODY_H

#include "Transformation.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/SetVector.h"

namespace clang {
  class ASTContext;
  class FunctionDecl;
}

class MoveFunctionBody : public Transformation,
                         public clang::RecursiveASTVisitor<MoveFunctionBody> {

public:

  MoveFunctionBody(const char *TransName, const char *Desc)
    : Transformation(TransName, Desc) { }

  ~MoveFunctionBody();

  // Trasnformation.
  virtual void Initialize(clang::ASTContext &context) override;
  virtual void HandleTranslationUnit(clang::ASTContext &Ctx) override;

  // RecursiveASTVisitor overrides.
  bool VisitFunctionDecl(clang::FunctionDecl *FD);

private:
  llvm::SetVector<clang::FunctionDecl*> FuncDefs;

  void doRewriting(clang::FunctionDecl *Def);
  // Unimplemented
  MoveFunctionBody(void);

  MoveFunctionBody(const MoveFunctionBody &);

  void operator=(const MoveFunctionBody &);
};
#endif
