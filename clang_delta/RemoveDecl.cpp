//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2013, 2015 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "RemoveDecl.h"

#include "TransformationManager.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

static const char *DescriptionMsg =
"Remove a decl from a decl context, such as namespace, class, etc. \n";

static RegisterTransformation<RemoveDecl> Trans("remove-decl", DescriptionMsg);
class RemoveDeclVisitor : public RecursiveASTVisitor<RemoveDeclVisitor> {

private:
  RemoveDecl *ConsumerInstance;

public:
  RemoveDeclVisitor(RemoveDecl *Instance) : ConsumerInstance(Instance) { }

  bool VisitDecl(Decl *D) {
    if (ConsumerInstance->isInIncludedFile(D))
      return true;
    if (auto *DC = dyn_cast<DeclContext>(D))
      return VisitDeclContext(DC);

    if (D->isImplicit() || D->isInvalidDecl() || !D->getSourceRange().isValid())
      return true;

    //if (!D->isThisDeclarationReferenced())
    ConsumerInstance->addDecl(D);
    return true;
  }

  bool VisitDeclContext(DeclContext *DC) {
    // Go in depth.
    for (auto *D : DC->decls()) {
      TraverseDecl(D);
    }

    return true;
  }
};

void RemoveDecl::Initialize(ASTContext &context)
{
  Transformation::Initialize(context);
}

void RemoveDecl::HandleTranslationUnit(ASTContext &Ctx)
{
  auto CollectionVisitor = RemoveDeclVisitor(this);
  CollectionVisitor.TraverseDecl(Ctx.getTranslationUnitDecl());

  if (QueryInstanceOnly)
    return;

  if (TransformationCounter > ValidInstanceNum) {
    TransError = TransMaxInstanceError;
    return;
  }

  Ctx.getDiagnostics().setSuppressAllDiagnostics(false);

  TransAssert(Decls.size() && "NULL TheDecl!");
  Decl* D = nullptr;
  if (ToCounter < TransformationCounter) {
    // Removing the last decl in the TU gives more chances to success.
    D = Decls[Decls.size() - TransformationCounter];
    RewriteHelper->removeDecl(D);
    //std::string s;
    //auto R = D->getSourceRange();
    //RewriteHelper->getStringBetweenLocs(s, R.getBegin(), R.getEnd());
  }
  else {
    for (int i = ToCounter; i >= TransformationCounter; --i) {
      D = Decls[i];
      RewriteHelper->removeDecl(D);
    }
  }

  if (Ctx.getDiagnostics().hasErrorOccurred() ||
      Ctx.getDiagnostics().hasFatalErrorOccurred())
    TransError = TransInternalError;
}
