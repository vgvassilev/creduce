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

using namespace clang;

static const char *DescriptionMsg =
"Remove a decl from a decl context, such as namespace, class, etc. \n";

static RegisterTransformation<RemoveDecl> Trans("remove-decl", DescriptionMsg);

///Returns true if oldD contains newD.
static bool containsDecl(Decl* oldD, Decl* newD, const SourceManager& SM) {
  // struct S {
  //   int a; //oldD
  //   int b; //newD
  // };
  SourceRange newR = newD->getSourceRange();
  SourceRange oldR = oldD->getSourceRange();
  SourceLocation oldB = oldR.getBegin();
  SourceLocation oldE = oldR.getEnd();
  SourceLocation newB = newR.getBegin();
  SourceLocation newE = newR.getEnd();

  if ((SM.isBeforeInSLocAddrSpace(oldB, newB) || oldB == newB) &&
      (SM.isBeforeInSLocAddrSpace(newE, oldE) || newE == oldE))
    // already in the range
    return true;
#ifndef NDEBUG
  // In cases where the one sloc is in the old decl and the other is not.
  if (SM.isBeforeInSLocAddrSpace(oldB, newB))
    TransAssert(SM.isBeforeInSLocAddrSpace(oldE, newE)
                && "Partial source range.");
  if (SM.isBeforeInSLocAddrSpace(newE, oldE))
    TransAssert(SM.isBeforeInSLocAddrSpace(newB, oldB)
                && "Partial source range.");
#endif //NDEBUG
  return false;
}

bool RemoveDecl::isInSourceRangeOfAlreadyRemovedDecl(Decl* newD) const {
  // Handle cases like:
  // void f(int param);
  // We will have one time f and param. In that case f might get removed before
  // param causing an assert.
  for (auto* D : RemovedDecls)
    if (containsDecl(D, newD, *SrcManager))
      return true;
  return false;
}

void RemoveDecl::checkAndReplaceIfDeclEnclosesAnyExistingDecl(Decl* newD) {
  for (auto* D : Decls) {
    if (containsDecl(newD, D, *SrcManager)) {
      removeDecl(D);
      maybeAddDecl(newD);
    }
  }
}

bool RemoveDecl::VisitDecl(Decl *D) {
  if (isInIncludedFile(D))
    return true;

  if (D->isImplicit() || !D->getSourceRange().isValid())
    return true;

  maybeAddDecl(D);
  if (auto *DC = dyn_cast<DeclContext>(D))
    return VisitDeclContext(DC);
  return true;
}

bool RemoveDecl::VisitDeclContext(DeclContext *DC) {
  // Go in depth.
  for (auto *D : DC->decls()) {
    TraverseDecl(D);
  }

  return true;
}

void RemoveDecl::Initialize(ASTContext &context)
{
  Transformation::Initialize(context);
}

void RemoveDecl::HandleTranslationUnit(ASTContext &Ctx)
{
  TraverseDecl(Ctx.getTranslationUnitDecl());

  if (QueryInstanceOnly)
    return;

  if (TransformationCounter > ValidInstanceNum) {
    TransError = TransMaxInstanceError;
    return;
  }

  if (ToCounter > ValidInstanceNum) {
    TransError = TransToCounterTooBigError;
    return;
  }

  Ctx.getDiagnostics().setSuppressAllDiagnostics(false);

  TransAssert(Decls.size() && "NULL TheDecl!");
  Decl* D = nullptr;
  if (ToCounter < TransformationCounter) {
    // Removing the last decl in the TU gives more chances to success.
    D = Decls[Decls.size() - TransformationCounter];
    removeDecl(D);
    //std::string s;
    //auto R = D->getSourceRange();
    //RewriteHelper->getStringBetweenLocs(s, R.getBegin(), R.getEnd());
  }
  else {
    // ... --to-counter=2 --counter=1
    for (int i = ToCounter - 1; i >= TransformationCounter; --i) {
      D = Decls[i-1];
      if (!isInSourceRangeOfAlreadyRemovedDecl(D))
        removeDecl(D);
    }
  }

  if (Ctx.getDiagnostics().hasErrorOccurred() ||
      Ctx.getDiagnostics().hasFatalErrorOccurred())
    TransError = TransInternalError;
}
