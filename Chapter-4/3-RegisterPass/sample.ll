; ModuleID = 'example.c'
source_filename = "example.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @main(i32, i8**) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 8
  store i32 0, i32* %9, align 4
  store i32 0, i32* %6, align 4
  br label %10

; <label>:10:                                     ; preds = %42, %2
  %11 = load i32, i32* %6, align 4
  %12 = icmp slt i32 %11, 10
  br i1 %12, label %13, label %45

; <label>:13:                                     ; preds = %10
  store i32 0, i32* %7, align 4
  br label %14

; <label>:14:                                     ; preds = %28, %13
  %15 = load i32, i32* %7, align 4
  %16 = icmp slt i32 %15, 10
  br i1 %16, label %17, label %31

; <label>:17:                                     ; preds = %14
  store i32 0, i32* %8, align 4
  br label %18

; <label>:18:                                     ; preds = %24, %17
  %19 = load i32, i32* %8, align 4
  %20 = icmp slt i32 %19, 10
  br i1 %20, label %21, label %27

; <label>:21:                                     ; preds = %18
  %22 = load i32, i32* %9, align 4
  %23 = add nsw i32 %22, 1
  store i32 %23, i32* %9, align 4
  br label %24

; <label>:24:                                     ; preds = %21
  %25 = load i32, i32* %8, align 4
  %26 = add nsw i32 %25, 1
  store i32 %26, i32* %8, align 4
  br label %18

; <label>:27:                                     ; preds = %18
  br label %28

; <label>:28:                                     ; preds = %27
  %29 = load i32, i32* %7, align 4
  %30 = add nsw i32 %29, 1
  store i32 %30, i32* %7, align 4
  br label %14

; <label>:31:                                     ; preds = %14
  store i32 0, i32* %7, align 4
  br label %32

; <label>:32:                                     ; preds = %38, %31
  %33 = load i32, i32* %7, align 4
  %34 = icmp slt i32 %33, 10
  br i1 %34, label %35, label %41

; <label>:35:                                     ; preds = %32
  %36 = load i32, i32* %9, align 4
  %37 = add nsw i32 %36, 1
  store i32 %37, i32* %9, align 4
  br label %38

; <label>:38:                                     ; preds = %35
  %39 = load i32, i32* %7, align 4
  %40 = add nsw i32 %39, 1
  store i32 %40, i32* %7, align 4
  br label %32

; <label>:41:                                     ; preds = %32
  br label %42

; <label>:42:                                     ; preds = %41
  %43 = load i32, i32* %6, align 4
  %44 = add nsw i32 %43, 1
  store i32 %44, i32* %6, align 4
  br label %10

; <label>:45:                                     ; preds = %10
  store i32 0, i32* %6, align 4
  br label %46

; <label>:46:                                     ; preds = %70, %45
  %47 = load i32, i32* %6, align 4
  %48 = icmp slt i32 %47, 20
  br i1 %48, label %49, label %73

; <label>:49:                                     ; preds = %46
  store i32 0, i32* %7, align 4
  br label %50

; <label>:50:                                     ; preds = %56, %49
  %51 = load i32, i32* %7, align 4
  %52 = icmp slt i32 %51, 20
  br i1 %52, label %53, label %59

; <label>:53:                                     ; preds = %50
  %54 = load i32, i32* %9, align 4
  %55 = add nsw i32 %54, 1
  store i32 %55, i32* %9, align 4
  br label %56

; <label>:56:                                     ; preds = %53
  %57 = load i32, i32* %7, align 4
  %58 = add nsw i32 %57, 1
  store i32 %58, i32* %7, align 4
  br label %50

; <label>:59:                                     ; preds = %50
  store i32 0, i32* %7, align 4
  br label %60

; <label>:60:                                     ; preds = %66, %59
  %61 = load i32, i32* %7, align 4
  %62 = icmp slt i32 %61, 20
  br i1 %62, label %63, label %69

; <label>:63:                                     ; preds = %60
  %64 = load i32, i32* %9, align 4
  %65 = add nsw i32 %64, 1
  store i32 %65, i32* %9, align 4
  br label %66

; <label>:66:                                     ; preds = %63
  %67 = load i32, i32* %7, align 4
  %68 = add nsw i32 %67, 1
  store i32 %68, i32* %7, align 4
  br label %60

; <label>:69:                                     ; preds = %60
  br label %70

; <label>:70:                                     ; preds = %69
  %71 = load i32, i32* %6, align 4
  %72 = add nsw i32 %71, 1
  store i32 %72, i32* %6, align 4
  br label %46

; <label>:73:                                     ; preds = %46
  %74 = load i32, i32* %9, align 4
  ret i32 %74
}

attributes #0 = { noinline nounwind optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 14]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Apple LLVM version 10.0.1 (clang-1001.0.46.4)"}
