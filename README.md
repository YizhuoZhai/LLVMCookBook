# LLVMCookBook
This project will work throughout the example inside "LLVM CookBook", which wirte a frontend and optimizer for a TOY language. 
## Requirements
- LLVM 7.0.0
- clang 7.0.0
## Chapter-1
Introduces some common-used llvm tools
## Chapter-2
Introduces a front end for a TOY language. Use the following command to generate IR for example
```
$make
$./toy example
```
Result should looks like:
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

