//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2013, 2015 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "SimplifyBody.h"

#include "TransformationManager.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;

static const char *DescriptionMsg =
"Simplify function bodies. There are 4 cases: \n\
  1) void f() { ... } \n\
     -> gets transformed into void f() { } \n\
  2) T* f() { ... return ...; } \n\
     -> gets transformed into T* f() {return 0;} \n\
  3) T& f() { ... return ...; } \n\
     -> gets transformed into T& f() {static T s; return s;} only if T can be \n\
     constructed \n\
  4) T f() { ... return ...; } \n\
     -> gets transformed into T f() {return T();} only if T can be constructed";

static RegisterTransformation<SimplifyBody> Trans("simplify-body", DescriptionMsg);

static bool isSupportedReturnType(QualType returnTy) {
  // TagDeclTypes host struct/union/class/enum. We need to handle structs and
  // unions since enums are integral and classes need special treatment.
  if (CXXRecordDecl* CXXRD = returnTy->getAsCXXRecordDecl()) {
    CXXRD = CXXRD->getDefinition();
    // In some scenarios, mostly when there the AST is broken (due to missing
    // include paths) we cannot ask the definition for default constructors.
    // For now just be conservative and skip the transformation.
    // FIXME: We should be able to tell the user to provide CREDUCE_INCLUDE_PATH
    if (!CXXRD)
      return false;

    return CXXRD->hasDefaultConstructor()
      || CXXRD->hasUserProvidedDefaultConstructor();
  }

  // For references we need to do the same trick, since we make an instance of
  // that type.
  if (returnTy->isReferenceType())
    return isSupportedReturnType(returnTy->getPointeeType());

  return returnTy->isVoidType() || returnTy->isAnyPointerType()
    || returnTy->isIntegralOrEnumerationType() || returnTy->isUnionType()
    || returnTy->isStructureType();
}

class SimplifyBodyVisitor : public RecursiveASTVisitor<SimplifyBodyVisitor> {

private:
  SimplifyBody *ConsumerInstance;

public:
  SimplifyBodyVisitor(SimplifyBody *Instance) : ConsumerInstance(Instance) { }

  bool VisitFunctionDecl(FunctionDecl *D) {
    if (ConsumerInstance->isInIncludedFile(D))
      return true;
    if (D->isImplicit() || !D->getSourceRange().isValid())
      return true;
    if (!D->doesThisDeclarationHaveABody())
      return true;
    if (!isSupportedReturnType(D->getReturnType()))
      return true;
    if (CompoundStmt* CS = dyn_cast<CompoundStmt>(D->getBody()))
      if (CS->body_empty())
        return true;

    ConsumerInstance->addDecl(D);
    return true;
  }

};

void SimplifyBody::Initialize(ASTContext &context)
{
  Transformation::Initialize(context);
}

static void simplifyFunctionBody(FunctionDecl* FD, Rewriter& RW) {
  // In general there are 5 cases:
  // 1. void f() { ... } - fairly straight-forward.
  // 2. some_type* f() { ...; return ...; } - we must inject return 0;
  // 3. some_type& f() { ...; return ...; } - we can use
  //                                          static some_type s; return s;
  // 4. some_type f() { ...; return ...; } - sometimes we can return some_type();
  // For now we will handle 1. and 2.
  TransAssert(FD);
  TransAssert(FD->hasBody() &&
              "Trying to work with a declaration not a definition");

  SourceLocation LBracLoc = FD->getBody()->getLocStart();
  SourceLocation RBracLoc = FD->getBody()->getLocEnd();
  if (CompoundStmt* CS = dyn_cast<CompoundStmt>(FD->getBody())) {
    LBracLoc = CS->getLBracLoc();
    RBracLoc = CS->getRBracLoc();
  }

  SourceRange Range
    = RW.getSourceMgr().getExpansionRange(SourceRange(LBracLoc, RBracLoc));
  TransAssert (RW.getRangeSize(Range) != -1 && "Bad range!");

  QualType returnType = FD->getReturnType();
  // Case 1.
  if (returnType->isVoidType()) {
    RW.ReplaceText(Range, "{}");
    return;
  }
  // Case 2.
  if (returnType->isAnyPointerType() // including objc-obj-ptr-type.
      || returnType->isIntegralOrEnumerationType()) {
    RW.ReplaceText(Range, "{return 0;}");
    return;
  }
  if (returnType->isReferenceType()) {
    // return type is something & and we need to create static something;
    std::string ctorExpr
      = QualType::getAsString(returnType->getPointeeType().split().Ty, Qualifiers());
    RW.ReplaceText(Range, "{static " + ctorExpr + " s; return s;}");
    return;
  }
  // Case 4.
  CXXRecordDecl* CXXRD = returnType->getAsCXXRecordDecl();
  if (returnType->isUnionType() || returnType->isStructureType()
      || CXXRD->hasDefaultConstructor()
      || CXXRD->hasUserProvidedDefaultConstructor()) {
    SplitQualType splitType = returnType.split();
    std::string ctorExpr
      = QualType::getAsString(splitType.Ty, Qualifiers());
    RW.ReplaceText(Range, "{return " + ctorExpr + "();}");
    return;
  }

  TransAssert(isSupportedReturnType(FD->getReturnType()));
}

void SimplifyBody::HandleTranslationUnit(ASTContext &Ctx)
{
  auto CollectionVisitor = SimplifyBodyVisitor(this);
  CollectionVisitor.TraverseDecl(Ctx.getTranslationUnitDecl());

  if (QueryInstanceOnly)
    return;

  if (TransformationCounter > ValidInstanceNum) {
    TransError = TransMaxInstanceError;
    return;
  }

  if (ToCounter > ValidInstanceNum ||
      (ToCounter > 0 && TransformationCounter >= ToCounter)) {
    TransError = TransToCounterTooBigError;
    return;
  }

  Ctx.getDiagnostics().setSuppressAllDiagnostics(false);

  TransAssert(Decls.size() && "NULL TheDecl!");
  // Sort the decls giving priority to the ones that will reduce the file most.
  std::sort(Decls.begin(), Decls.end(),
            [&](const FunctionDecl* a, const FunctionDecl* b) -> bool
            {
              const SourceManager& SM = TheRewriter.getSourceMgr();
              SourceRange RangeA = SM.getExpansionRange(a->getSourceRange());
              SourceRange RangeB = SM.getExpansionRange(b->getSourceRange());

              return TheRewriter.getRangeSize(RangeA)
                < TheRewriter.getRangeSize(RangeB);
            });

  FunctionDecl* D = nullptr;
  if (ToCounter < TransformationCounter) {
    // Removing the last decl in the TU gives more chances to success.
    D = Decls[Decls.size() - TransformationCounter];
    simplifyFunctionBody(D, TheRewriter);
  }
  else {
    // ... --to-counter=2 --counter=1
    for (int i = ToCounter - 1; i >= TransformationCounter; --i) {
      D = Decls[i-1];
      simplifyFunctionBody(D, TheRewriter);
    }
  }

  if (Ctx.getDiagnostics().hasErrorOccurred() ||
      Ctx.getDiagnostics().hasFatalErrorOccurred())
    TransError = TransInternalError;
}
