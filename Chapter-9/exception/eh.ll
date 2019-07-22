; ModuleID = 'eh.bc'
source_filename = "eh.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%class.Ex1 = type { i8 }

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS3Ex1 = linkonce_odr constant [5 x i8] c"3Ex1\00"
@_ZTI3Ex1 = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @_ZTS3Ex1, i32 0, i32 0) }

; Function Attrs: noinline optnone ssp uwtable
define void @_Z15throw_exceptionii(i32, i32) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca %class.Ex1, align 1
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %6 = load i32, i32* %3, align 4
  %7 = load i32, i32* %4, align 4
  %8 = icmp sgt i32 %6, %7
  br i1 %8, label %9, label %12

; <label>:9:                                      ; preds = %2
  %10 = call i8* @__cxa_allocate_exception(i64 1) #1
  %11 = bitcast i8* %10 to %class.Ex1*
  call void @__cxa_throw(i8* %10, i8* bitcast ({ i8*, i8* }* @_ZTI3Ex1 to i8*), i8* null) #2
  unreachable

; <label>:12:                                     ; preds = %2
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

; Function Attrs: noinline optnone ssp uwtable
define i32 @_Z14test_try_catchv() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %1 = alloca i32, align 4
  %2 = alloca i8*
  %3 = alloca i32
  invoke void @_Z15throw_exceptionii(i32 2, i32 1)
          to label %4 unwind label %5

; <label>:4:                                      ; preds = %0
  br label %12

; <label>:5:                                      ; preds = %0
  %6 = landingpad { i8*, i32 }
          catch i8* null
  %7 = extractvalue { i8*, i32 } %6, 0
  store i8* %7, i8** %2, align 8
  %8 = extractvalue { i8*, i32 } %6, 1
  store i32 %8, i32* %3, align 4
  br label %9

; <label>:9:                                      ; preds = %5
  %10 = load i8*, i8** %2, align 8
  %11 = call i8* @__cxa_begin_catch(i8* %10) #1
  store i32 1, i32* %1, align 4
  call void @__cxa_end_catch()
  br label %13

; <label>:12:                                     ; preds = %4
  store i32 0, i32* %1, align 4
  br label %13

; <label>:13:                                     ; preds = %12, %9
  %14 = load i32, i32* %1, align 4
  ret i32 %14
}

declare i32 @__gxx_personality_v0(...)

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 14]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Apple LLVM version 10.0.1 (clang-1001.0.46.4)"}
