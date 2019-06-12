In this chapter, the IR will be changed via the optimization pass.

##Task-1: DeadCodeElimination
In this task, the instruction which are not used will be removed from IR.

##Task-2:MyInliner
This task does always inline optimization of IR.
Version Updates Notes: Class "Inliner" and the corresponding methods are removed in llvm.
```
$opt -inline-threshold=0 -always-inline -S testcode.ll
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
```