//RUN: %clang_cc1 -fsyntax-only -verify %s
//RUN: %clangdelta --transformation=reduce-class-template-param --counter=1 %s | FileCheck %s | %clang_cc1 -fsyntax-only -x c++ -
//XFAIL:*

template <typename T, template<typename X> class Allocator>
class zero_allocator : public Allocator<T> {};

//CHECK: template <typename T>
//CHECK-NEXT: class zero_allocator : public T {};


//expected-no-diagnostics
