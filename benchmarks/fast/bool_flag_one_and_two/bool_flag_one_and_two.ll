; ModuleID = '/home/martin/school/bac/qm2c/benchmarks/../../sbt-fizzer/out/bool_flag_one_and_two/bool_flag_one_and_two.c'
source_filename = "/home/martin/school/bac/qm2c/benchmarks/../../sbt-fizzer/out/bool_flag_one_and_two/bool_flag_one_and_two.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i16, align 2
  %3 = alloca i16, align 2
  %4 = alloca i16, align 2
  %5 = alloca i16, align 2
  %6 = alloca i8, align 1
  %7 = alloca i8, align 1
  store i32 0, ptr %1, align 4
  %8 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %8, ptr %2, align 2
  %9 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %9, ptr %3, align 2
  %10 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %10, ptr %4, align 2
  %11 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %11, ptr %5, align 2
  %12 = load i16, ptr %2, align 2
  %13 = sext i16 %12 to i32
  %14 = load i16, ptr %3, align 2
  %15 = sext i16 %14 to i32
  %16 = add nsw i32 %15, 123
  %17 = icmp eq i32 %13, %16
  br i1 %17, label %18, label %19

18:                                               ; preds = %0
  store i8 1, ptr %6, align 1
  br label %20

19:                                               ; preds = %0
  store i8 0, ptr %6, align 1
  br label %20

20:                                               ; preds = %19, %18
  %21 = load i16, ptr %4, align 2
  %22 = sext i16 %21 to i32
  %23 = load i16, ptr %5, align 2
  %24 = sext i16 %23 to i32
  %25 = add nsw i32 %24, 123
  %26 = icmp ne i32 %22, %25
  br i1 %26, label %27, label %28

27:                                               ; preds = %20
  store i8 1, ptr %7, align 1
  br label %29

28:                                               ; preds = %20
  store i8 0, ptr %7, align 1
  br label %29

29:                                               ; preds = %28, %27
  %30 = load i8, ptr %6, align 1
  %31 = trunc i8 %30 to i1
  %32 = zext i1 %31 to i32
  %33 = icmp eq i32 %32, 1
  br i1 %33, label %34, label %41

34:                                               ; preds = %29
  %35 = load i8, ptr %7, align 1
  %36 = trunc i8 %35 to i1
  %37 = zext i1 %36 to i32
  %38 = icmp eq i32 %37, 1
  br i1 %38, label %39, label %40

39:                                               ; preds = %34
  store i32 1, ptr %1, align 4
  br label %42

40:                                               ; preds = %34
  br label %41

41:                                               ; preds = %40, %29
  store i32 0, ptr %1, align 4
  br label %42

42:                                               ; preds = %41, %39
  %43 = load i32, ptr %1, align 4
  ret i32 %43
}

declare signext i16 @__VERIFIER_nondet_short(...) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Debian clang version 18.1.8 (++20240731024826+3b5b5c1ec4a3-1~exp1~20240731144843.145)"}
