# LLVMCookBook
This project will work throughout the example inside "LLVM CookBook", which write a frontend and optimizer for a TOY language. 
## Requirements
- LLVM 7.0.0
- clang 7.0.0
## Chapter-1
Introduces some common-used llvm tools
## Chapter-2
Introduces a front-end for a TOY language. 
example:
```
def foo(x y)
x + y * 16;
```
Use the following command to generate IR for example:
```
$make
$./toy example
```
LLVM IR:
```
; ModuleID = 'example'
source_filename = "example"

define i32 @foo(i32 %x, i32 %y) {
entry:
  %multmp = mul i32 %y, 16
  %addtmp = add i32 %x, %multmp
  ret i32 %addtmp
}
```
## Chapter-3
Add the if/for/binary/unary statement or operations for front-end, and also add the JIT support, go to the corresponding directory:
```
$make
$./toy example
```

### if-else
example:
```
def fib(x)
if x < 3 then
1
else
fib(x-1)+fib(x-2)
```
LLVM IR:
```
; ModuleID = 'example'
source_filename = "example"

define i32 @fib(i32 %x) {
entry:
  %cmptmp = icmp ult i32 %x, 3
  %booltmp = zext i1 %cmptmp to i32
  %ifcond = icmp ne i32 %booltmp, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  %subtmp = sub i32 %x, 1
  %calltemp = call i32 @fib(i32 %subtmp)
  %subtmp1 = sub i32 %x, 2
  %calltemp2 = call i32 @fib(i32 %subtmp1)
  %addtmp = add i32 %calltemp, %calltemp2
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 1, %then ], [ %addtmp, %else ]
  ret i32 %iftmp
}
```
### loop
example:
```
def printstar(n x)
    for i = 1, i < n, 1 in
        x + 1

```
LLVM IR:
```
; ModuleID = 'example'
source_filename = "example"

define i32 @printstar(i32 %n, i32 %x) {
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi i32 [ 1, %entry ], [ %nextvar, %loop ]
  %addtmp = add i32 %x, 1
  %nextvar = add i32 %i, 1
  %cmptmp = icmp ult i32 %i, %n
  %booltmp = zext i1 %cmptmp to i32
  %loopcond = icmp ne i32 %booltmp, 0
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  ret i32 0
}
```
### binary
example:
```
def binary| 5 (LHS RHS)
    if LHS then
        1
    else if RHS then
        1
    else
        0;
```
LLVM IR:
```; ModuleID = 'example'
source_filename = "example"

define i32 @"binary|"(i32 %LHS, i32 %RHS) {
entry:
  %ifcond = icmp ne i32 %LHS, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont4

else:                                             ; preds = %entry
  %ifcond1 = icmp ne i32 %RHS, 0
  br i1 %ifcond1, label %then2, label %else3

then2:                                            ; preds = %else
  br label %ifcont

else3:                                            ; preds = %else
  br label %ifcont

ifcont:                                           ; preds = %else3, %then2
  %iftmp = phi i32 [ 1, %then2 ], [ 0, %else3 ]
  br label %ifcont4

ifcont4:                                          ; preds = %ifcont, %then
  %iftmp5 = phi i32 [ 1, %then ], [ %iftmp, %ifcont ]
  ret i32 %iftmp5
}
```
### unary
example:
```
def unary!(v)
    if v then
        0
    else
        1;
```
LLVM IR:
```
define i32 @"unary!"(i32 %v) {
entry:
  %ifcond = icmp ne i32 %v, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi i32 [ 0, %then ], [ 1, %else ]
  ret i32 %iftmp
}

```
### JIT Support
example:
```
def f()
    4+5;

```
LLVM IR:
```
; ModuleID = 'example'
source_filename = "example"

define i32 @f() {
entry:
  ret i32 9
}
```
## Chapter-4
This chapter introduces writing the LLVM pass, there are a few tasks.
###1. Writing your own pass
The FuncBolckCount should be put under /dir/to/llvm/src/projects
Then go to /dir/to/llvm/src/../ and create a build folder
```
$cd build
$cmake -G "Unix Makefiles" ../llvm-7.0.0.src
$make
$opt -load dir/to/build/lib/funcBlockCountlib.dylib -funcblockcount sample.ll
```
Output:
```
Function foo
Function ha
```
###2. Using another pass in a new pass
The FuncBolckCount should be put under /dir/to/llvm/src/projects, go to the build folder created in the first task
```
$make
$opt -load ../../../llvm/build/lib/funcBlockCountlib.dylib -funcblockcount sample.ll
```
Output:
```
Function main
Loop level 0 has 11 blocks
Loop level 1 has 3 blocks
Loop level 1 has 3 blocks
Loop level 0 has 15 blocks
Loop level 1 has 7 blocks
Loop level 2 has 3 blocks
Loop level 1 has 3 blocks
```