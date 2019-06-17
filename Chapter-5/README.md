In this chapter, the IR will be changed via the optimization pass.

##Task-1: DeadCodeElimination
In this task, the instruction which are not used will be removed from IR.

##Task-2:MyInliner
This task does always inline optimization of IR.
Version Updates Notes: Class "Inliner" and the corresponding methods are removed in llvm.
```
$opt -my-inline -S testcode.ll
```
Result:
```
; ModuleID = 'testcode.ll'
source_filename = "testcode.ll"

; Function Attrs: alwaysinline
define i32 @inner1() #0 {
  ret i32 1
}

define i32 @outer1() {
  ret i32 1
}

attributes #0 = { alwaysinline }
```
##Task-3:Use the memcpyopt pass built in llvm already
Source code: llvm/lib/Transforms/Scalar/MemCpyOptimizer.cpp
```
opt -memcpyopt -S testcode.ll
```
##Task-4: Add the patten match (A | (B ^ C)) ^ ((A ^ C) ^ B) for instcombine
Version Updates Notes: if (Op0I && Op1I) statement is no longer inside the file, just add the following code to InstCombiner::visitXor(BinaryOperator &I)
```C++
{
  Value *A, *B, *C;
  if (match(Op0, m_Or(m_Xor(m_Value(B), m_Value(C)), m_Value(A)))
    && match(Op1, m_Xor( m_Xor(m_Specific(A), m_Specific(C)),
    m_Specific(B)))) {
      return BinaryOperator::CreateAnd(A, Builder.CreateXor(B,C));
  }
}
```
```
opt -instcombine -S testcode.ll
```
Result:
'''
; ModuleID = 'testcode.ll'
source_filename = "testcode.ll"

define i32 @test19(i32 %x, i32 %y, i32 %z) {
  %1 = xor i32 %y, %z
  %res = and i32 %1, %x
  ret i32 %res
}
'''
##Task-5-6-7: In the following three task, the book tries to use multiple build-in optimization pass without code modification. Therefore, I put them together:
Loop-Invariant Code Motion (LICM) optimization:
```
opt licmtest.ll -licm -S
```
Loop deletion pass:
```
opt deletetest.ll -loop-deletion -S
```
Reassociating expressions:
```
opt -reassociate -die -S testreassociate.ll
```

