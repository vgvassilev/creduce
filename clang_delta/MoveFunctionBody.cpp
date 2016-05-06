//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2015 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "MoveFunctionBody.h"

#include "TransformationManager.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

using namespace clang;

static const char *DescriptionMsg =
"Move function body towards its declaration. \
Note that this pass would generate incompilable code. \n";

static RegisterTransformation<MoveFunctionBody>
         Trans("move-function-body", DescriptionMsg);

void MoveFunctionBody::Initialize(ASTContext &context)
{
  Transformation::Initialize(context);
}

bool MoveFunctionBody::VisitFunctionDecl(FunctionDecl *FD) {
  // FIXME: This greatly reduces the power of this pass. For out of line
  // definitions (eg. MyClass::MyFunc() { ... }) it will only work if the def
  // and the decl are in the same source file. The majority of cases are in
  // different files (.h and .cxx). However this would be still very helpful for
  // preprocessed file reductions.
  if (isInIncludedFile(FD))
    return true;

  if (FD->isThisDeclarationADefinition() && !FuncDefs.count(FD))
    FuncDefs.insert(FD);
  return true;
}

void MoveFunctionBody::HandleTranslationUnit(ASTContext &Ctx)
{
  TraverseDecl(Ctx.getTranslationUnitDecl());

  ValidInstanceNum = FuncDefs.size();

  if (QueryInstanceOnly)
    return;

  if (TransformationCounter > ValidInstanceNum) {
    TransError = TransMaxInstanceError;
    return;
  }

  Ctx.getDiagnostics().setSuppressAllDiagnostics(false);

  doRewriting(FuncDefs[TransformationCounter-1]);

  if (Ctx.getDiagnostics().hasErrorOccurred() ||
      Ctx.getDiagnostics().hasFatalErrorOccurred())
    TransError = TransInternalError;
}

// Clone from functionDeclHasDefaultArgument SemaDeclCXX.cpp
static bool hasDefaultArgument(const FunctionDecl *FD) {
  for (unsigned NumParams = FD->getNumParams(); NumParams > 0; --NumParams) {
    const ParmVarDecl *PVD = FD->getParamDecl(NumParams-1);
    if (!PVD->hasDefaultArg())
      return false;
    if (!PVD->hasInheritedDefaultArg())
      return true;
  }
  return false;
}

void MoveFunctionBody::doRewriting(clang::FunctionDecl *Def)
{
  TransAssert(Def);
  TransAssert(Def->isThisDeclarationADefinition() && "Not a definition!");
  TransAssert(Def->hasBody() && "Trying to work with a decl not a def.");

  std::string bodyStr;
  llvm::raw_string_ostream Out(bodyStr);
  Def->print(Out);
  Out.flush();

  FunctionDecl *CanonFD = Def->getCanonicalDecl();
  TransAssert(!CanonFD->isThisDeclarationADefinition() && "A definition!");

  SourceManager &SM = TheRewriter.getSourceMgr();
  SourceRange ReplacementRange = SM.getExpansionRange(CanonFD->getSourceRange());
  int ReplacementRangeSize = TheRewriter.getRangeSize(ReplacementRange);
  TransAssert(ReplacementRangeSize != -1 && "Bad range!");
  TheRewriter.ReplaceText(CanonFD->getLocStart(), ReplacementRangeSize, bodyStr);
  RewriteHelper->removeDecl(Def);

  // Remove the redecl with the default args, because the default args must
  // occur only once in the redecl chain.
  for (FunctionDecl *Prev = CanonFD->getMostRecentDecl(); Prev;
       Prev = Prev->getPreviousDecl()) {
    // FIXME: If the default params are in a redecl in a different file we
    // create ill-formed code.
    if (isInIncludedFile(Prev))
      continue;

    if (Prev != Def && hasDefaultArgument(Prev))
      RewriteHelper->removeDecl(Prev);
  }
}

MoveFunctionBody::~MoveFunctionBody()
{
  // pin the vtable here.
}
