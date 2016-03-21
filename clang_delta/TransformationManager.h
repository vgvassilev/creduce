//===----------------------------------------------------------------------===//
//
// Copyright (c) 2012, 2013, 2015 The University of Utah
// All rights reserved.
//
// This file is distributed under the University of Illinois Open Source
// License.  See the file COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef TRANSFORMATION_MANAGER_H
#define TRANSFORMATION_MANAGER_H

#include <string>
#include <map>
#include <cassert>

#include "llvm/Support/raw_ostream.h"

class Transformation;
namespace clang {
  class CompilerInstance;
}

///\brief Keeps track of the way clang_delta was invoked.
///
struct ClangDeltaInvocationOptions {
  std::string TransformationName; //< The transformation name.
  bool IsQueryInstances; //< If we are in simulation or transformation mode.
  unsigned Counter; //< How many transformations we will make.
  unsigned ToCounter; //< In case of multi transformations, the end of range.
  std::string SourceFileName; //< The transformed file.
  std::string OutputFileName; //< The result file.
  ClangDeltaInvocationOptions()
    : TransformationName(""), IsQueryInstances(false), Counter(0), ToCounter(0),
      SourceFileName(""), OutputFileName("") {}
};

class TransformationManager {

public:

  static TransformationManager *GetInstance();

  static void Finalize();

  static void registerTransformation(const char *TransName, 
                                     Transformation *TransImpl);
  
  static bool isCXXLangOpt();

  static bool isCLangOpt();

  static bool isOpenCLLangOpt();

  static int ErrorInvalidCounter;

  bool doTransformation(const ClangDeltaInvocationOptions &Opts,
                        std::string &ErrorMsg, int &ErrorCode);

  bool verify(const ClangDeltaInvocationOptions &Opts, std::string &ErrorMsg,
              int &ErrorCode);

  bool hasTransformation(const std::string &TransName) {
    return TransformationsMap.find(TransName) != TransformationsMap.end();
  }

  void setTransformation(const std::string &Trans) {
    assert(hasTransformation(Trans) && "No transformation found.");
    CurrentTransName = Trans;
    CurrentTransformationImpl = TransformationsMap[Trans];
  }

  void setTransformationCounter(unsigned Counter) {
    TransformationCounter = Counter;
  }

  void setToCounter(unsigned Counter) {
    ToCounter = Counter;
  }

  void setSrcFileName(const std::string &FileName) {
    // This can happen if we do --query-instances=*
    if (SrcFileName == FileName)
      return;
    assert(SrcFileName.empty() && "Could only process one file each time");
    SrcFileName = FileName;
  }

  void setOutputFileName(const std::string &FileName) {
    OutputFileName = FileName;
  }

  void setQueryInstanceFlag(bool Flag) {
    QueryInstanceOnly = Flag;
  }

  bool initializeCompilerInstance(std::string &ErrorMsg);

  void outputNumTransformationInstances();

  void printTransformations();

  void printTransformationNames();

private:
  
  TransformationManager();

  ~TransformationManager();

  llvm::raw_ostream *getOutStream();

  void closeOutStream(llvm::raw_ostream *OutStream);

  static TransformationManager *Instance;

  static std::map<std::string, Transformation *> *TransformationsMapPtr;

  std::map<std::string, Transformation *> TransformationsMap;

  Transformation *CurrentTransformationImpl;

  int TransformationCounter;

  int ToCounter;

  std::string SrcFileName;

  std::string OutputFileName;

  std::string CurrentTransName;

  std::unique_ptr<clang::CompilerInstance> ClangInstance;

  bool QueryInstanceOnly;

  // Unimplemented
  TransformationManager(const TransformationManager &);

  void operator=(const TransformationManager&);

};

template<typename TransformationClass>
class RegisterTransformation {

public:
  RegisterTransformation(const char *TransName, const char *Desc) {
    Transformation *TransImpl = new TransformationClass(TransName, Desc);
    assert(TransImpl && "Fail to create TransformationClass");
 
    TransformationManager::registerTransformation(TransName, TransImpl);
  }

private:
  // Unimplemented
  RegisterTransformation(const RegisterTransformation &);

  void operator=(const RegisterTransformation &);

};

#endif
