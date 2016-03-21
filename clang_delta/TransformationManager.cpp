//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2013, 2014, 2015, 2016 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "TransformationManager.h"

#include "Transformation.h"

#include <sstream>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/Version.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/ParseAST.h"

#include "llvm/Support/ManagedStatic.h"

using namespace clang;

int TransformationManager::ErrorInvalidCounter = 1;

bool TransformationManager::initializeCompilerInstance(std::string &ErrorMsg)
{
  ClangInstance.reset(new CompilerInstance());

  ClangInstance->createDiagnostics();
  CompilerInvocation &Invocation = ClangInstance->getInvocation();

  // guess which is the language
  InputKind IK
     = FrontendOptions::getInputKindForExtension(StringRef(SrcFileName).rsplit('.').second);
  // If the CREDUCE_LANG env var is defined override the guess.
  // This is useful when reducing multiple files. For instance, we reduce a cpp
  // file with its header files. The guessing mechanism will consider all .h
  // files (and files with no extension) as C files and fail to build an AST.
  if (const char *Lang = getenv("CREDUCE_LANG")) {
    if (!strcmp(Lang, "CXX"))
      IK = IK_CXX;
    else if (!strcmp(Lang, "C"))
      IK = IK_C;
  }

  std::vector<const char*> Args;
  if ((IK == IK_C) || (IK == IK_PreprocessedC)) {
    Invocation.setLangDefaults(ClangInstance->getLangOpts(), IK_C);
  }
  else if ((IK == IK_CXX) || (IK == IK_PreprocessedCXX)) {
    // ISSUE: it might cause some problems when building AST
    // for a function which has a non-declared callee, e.g.,
    // It results an empty AST for the caller.

    Args.push_back("-x");
    Args.push_back("c++");
    Invocation.setLangDefaults(ClangInstance->getLangOpts(), IK_CXX);
  }
  else if(IK == IK_OpenCL) {
    //Commandline parameters
    Args.push_back("-x");
    Args.push_back("cl");
    Args.push_back("-Dcl_clang_storage_class_specifiers");

    const char *CLCPath = getenv("CREDUCE_LIBCLC_INCLUDE_PATH");

    ClangInstance->createFileManager();

    if(CLCPath != NULL && ClangInstance->hasFileManager() &&
       ClangInstance->getFileManager().getDirectory(CLCPath, false) != NULL) {
        Args.push_back("-I");
        Args.push_back(CLCPath);
    }

    Args.push_back("-include");
    Args.push_back("clc/clc.h");
    Args.push_back("-fno-builtin");

    Invocation.setLangDefaults(ClangInstance->getLangOpts(), IK_OpenCL);
  }
  else {
    ErrorMsg = "Unsupported file type!";
    return false;
  }

  CompilerInvocation::CreateFromArgs(Invocation, &Args[0],
                                     &Args[0] + Args.size(),
                                     ClangInstance->getDiagnostics());

  TargetOptions &TargetOpts = ClangInstance->getTargetOpts();

  if (const char *env = getenv("CREDUCE_TARGET_TRIPLE")) {
    TargetOpts.Triple = std::string(env);
  } else {
    TargetOpts.Triple = LLVM_DEFAULT_TARGET_TRIPLE;
  }

  TargetInfo *Target = 
    TargetInfo::CreateTargetInfo(ClangInstance->getDiagnostics(),
                                 ClangInstance->getInvocation().TargetOpts);
  ClangInstance->setTarget(Target);

  if (const char *env = getenv("CREDUCE_INCLUDE_PATH")) {
    HeaderSearchOptions &HeaderSearchOpts = ClangInstance->getHeaderSearchOpts();

    const std::size_t npos = std::string::npos;
    std::string text = env;

    std::size_t now = 0, next = 0;
    do {
      next = text.find(':', now);
      std::size_t len = (next == npos) ? npos : (next - now);
      HeaderSearchOpts.AddPath(text.substr(now, len), clang::frontend::Angled, false, false);
      now = next + 1;
    } while(next != npos);
  }

  ClangInstance->createFileManager();
  ClangInstance->createSourceManager(ClangInstance->getFileManager());
  ClangInstance->createPreprocessor(TU_Complete);

  DiagnosticConsumer &DgClient = ClangInstance->getDiagnosticClient();
  DgClient.BeginSourceFile(ClangInstance->getLangOpts(),
                           &ClangInstance->getPreprocessor());
  ClangInstance->createASTContext();

  assert(CurrentTransformationImpl && "Bad transformation instance!");
  ClangInstance->setASTConsumer(
    std::unique_ptr<ASTConsumer>(CurrentTransformationImpl));
  Preprocessor &PP = ClangInstance->getPreprocessor();
#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 7
    PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());
#else
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());
#endif
  if (!ClangInstance->InitializeSourceManager(FrontendInputFile(SrcFileName, IK))) {
    ErrorMsg = "Cannot open source file!";
    return false;
  }

  return true;
}

llvm::raw_ostream *TransformationManager::getOutStream()
{
  if (OutputFileName.empty())
    return &(llvm::outs());

  std::error_code EC;
  llvm::raw_fd_ostream *Out = new llvm::raw_fd_ostream(
      OutputFileName, EC, llvm::sys::fs::F_RW);
  assert(!EC && "Cannot open output file!");
  return Out;
}

void TransformationManager::closeOutStream(llvm::raw_ostream *OutStream)
{
  if (!OutputFileName.empty())
    delete OutStream;
}

bool TransformationManager::doTransformation(const ClangDeltaInvocationOptions &Opts,
                                             std::string &ErrorMsg, int &ErrorCode)
{
  if (Opts.TransformationName == "*") {
    assert(Opts.IsQueryInstances && "Only available for --query-intances mode");
    for (auto &Trans : TransformationsMap) {
      // Intentional copy.
      ClangDeltaInvocationOptions CDInvOpts = Opts;
      CDInvOpts.TransformationName = Trans.first;
      llvm::outs() << "[ " << Trans.first << " ] -> ";
      if (!doTransformation(CDInvOpts, ErrorMsg, ErrorCode))
        return false;
    }
    return true;
  }

  ErrorMsg = "";

  setTransformation(Opts.TransformationName);
  if (Opts.IsQueryInstances)
    setQueryInstanceFlag(true);
  setTransformationCounter(Opts.Counter);
  if (Opts.ToCounter)
    setToCounter(Opts.ToCounter);
  setOutputFileName(Opts.OutputFileName);
  setSrcFileName(Opts.SourceFileName);

  if (!initializeCompilerInstance(ErrorMsg))
    return false;

  if (!verify(Opts, ErrorMsg, ErrorCode))
    return false;

  ClangInstance->createSema(TU_Complete, 0);
  DiagnosticsEngine &Diag = ClangInstance->getDiagnostics();
  Diag.setSuppressAllDiagnostics(true);
  Diag.setIgnoreAllWarnings(true);

  CurrentTransformationImpl->setQueryInstanceFlag(QueryInstanceOnly);
  CurrentTransformationImpl->setTransformationCounter(TransformationCounter);
  if (ToCounter > 0 && CurrentTransformationImpl->isMultipleRewritesEnabled()) {
    CurrentTransformationImpl->setToCounter(ToCounter);
  }

  ParseAST(ClangInstance->getSema());

  ClangInstance->getDiagnosticClient().EndSourceFile();

  if (QueryInstanceOnly) {
    outputNumTransformationInstances();
    return true;
  }

  llvm::raw_ostream *OutStream = getOutStream();
  bool RV;
  if (CurrentTransformationImpl->transSuccess()) {
    CurrentTransformationImpl->outputTransformedSource(*OutStream);
    RV = true;
  }
  else if (CurrentTransformationImpl->transInternalError()) {
    CurrentTransformationImpl->outputOriginalSource(*OutStream);
    RV = true;
  }
  else {
    CurrentTransformationImpl->getTransErrorMsg(ErrorMsg);
    if (CurrentTransformationImpl->isInvalidCounterError())
      ErrorCode = ErrorInvalidCounter;
    RV = false;
  }
  closeOutStream(OutStream);
  return RV;
}

bool TransformationManager::verify(const ClangDeltaInvocationOptions &Opts,
                                   std::string &ErrorMsg, int &ErrorCode)
{
  if (!hasTransformation(Opts.TransformationName)) {
    ErrorMsg = "Invalid transformation[" + Opts.TransformationName + "]";
    return false;
  }

  if (!CurrentTransformationImpl) {
    ErrorMsg = "Empty transformation instance!";
    return false;
  }

  if (CurrentTransformationImpl->skipCounter())
    return true;

  if (TransformationCounter <= 0) {
    ErrorMsg = "Invalid transformation counter!";
    ErrorCode = ErrorInvalidCounter;
    return false;
  }

  if ((ToCounter > 0) && (ToCounter < TransformationCounter)) {
    ErrorMsg = "to-counter value cannot be smaller than counter value!";
    ErrorCode = ErrorInvalidCounter;
    return false;
  }

  if (ToCounter > 0 && !CurrentTransformationImpl->isMultipleRewritesEnabled()) {
    ErrorMsg = "current transformation[";
    ErrorMsg += CurrentTransName;
    ErrorMsg += "] does not support multiple rewrites!";
    return false;
  }

  return true;
}

static llvm::ManagedStatic<TransformationManager> TransformationManagerObj;
TransformationManager *TransformationManager::getTransformationManager() {
  return &*TransformationManagerObj;
}


void TransformationManager::registerTransformation(const char *TransName,
                                                   Transformation *TransImpl)
{
  assert((TransImpl != NULL) && "NULL Transformation!");
  assert(!hasTransformation(TransName) && "Duplicated transformation!");
  TransformationsMap[TransName] = TransImpl;
}

void TransformationManager::printTransformations()
{
  llvm::outs() << "Registered Transformations:\n";

  std::map<std::string, Transformation *>::iterator I, E;
  for (I = TransformationsMap.begin(), 
       E = TransformationsMap.end();
       I != E; ++I) {
    llvm::outs() << "  [" << (*I).first << "]: "; 
    llvm::outs() << (*I).second->getDescription() << "\n";
  }
}

void TransformationManager::printTransformationNames()
{
  std::map<std::string, Transformation *>::iterator I, E;
  for (I = TransformationsMap.begin(), 
       E = TransformationsMap.end();
       I != E; ++I) {
    llvm::outs() << (*I).first << "\n";
  }
}

void TransformationManager::outputNumTransformationInstances()
{
  int NumInstances = 
    CurrentTransformationImpl->getNumTransformationInstances();
  llvm::outs() << "Available transformation instances: " 
               << NumInstances << "\n";
}

TransformationManager::TransformationManager()
  : CurrentTransformationImpl(NULL),
    TransformationCounter(-1),
    ToCounter(-1),
    SrcFileName(""),
    OutputFileName(""),
    CurrentTransName(""),
    QueryInstanceOnly(false)
{
  // Nothing to do
}

TransformationManager::~TransformationManager()
{
  // When clang shuts down it leaks the memory. Do the same.
}
