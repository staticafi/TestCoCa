; ModuleID = '/home/martin/school/bac/qm2c/output/benchmarks/../../sbt-fizzer/out/bool_flag_one_and_two/bool_flag_one_and_two.ll'
source_filename = "/home/martin/school/bac/qm2c/benchmarks/../../sbt-fizzer/out/bool_flag_one_and_two/bool_flag_one_and_two.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__qmi_cond_br_count = constant i32 4, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @__qmi_original_main() #0 {
bb1:
  %0 = alloca i32, align 4
  %1 = alloca i16, align 2
  %2 = alloca i16, align 2
  %3 = alloca i16, align 2
  %4 = alloca i16, align 2
  %5 = alloca i8, align 1
  %6 = alloca i8, align 1
  store i32 0, ptr %0, align 4
  %7 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %7, ptr %1, align 2
  %8 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %8, ptr %2, align 2
  %9 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %9, ptr %3, align 2
  %10 = call signext i16 (...) @__VERIFIER_nondet_short()
  store i16 %10, ptr %4, align 2
  %11 = load i16, ptr %1, align 2
  %12 = sext i16 %11 to i32
  %13 = load i16, ptr %2, align 2
  %14 = sext i16 %13 to i32
  %15 = add nsw i32 %14, 123
  %16 = icmp eq i32 %12, %15
  call void @__qmi_process_br_instr(i32 1, i1 %16)
  br i1 %16, label %bb2, label %bb3

bb2:                                              ; preds = %bb1
  store i8 1, ptr %5, align 1
  br label %bb4

bb3:                                              ; preds = %bb1
  store i8 0, ptr %5, align 1
  br label %bb4

bb4:                                              ; preds = %bb3, %bb2
  %17 = load i16, ptr %3, align 2
  %18 = sext i16 %17 to i32
  %19 = load i16, ptr %4, align 2
  %20 = sext i16 %19 to i32
  %21 = add nsw i32 %20, 123
  %22 = icmp ne i32 %18, %21
  call void @__qmi_process_br_instr(i32 2, i1 %22)
  br i1 %22, label %bb5, label %bb6

bb5:                                              ; preds = %bb4
  store i8 1, ptr %6, align 1
  br label %bb7

bb6:                                              ; preds = %bb4
  store i8 0, ptr %6, align 1
  br label %bb7

bb7:                                              ; preds = %bb6, %bb5
  %23 = load i8, ptr %5, align 1
  %24 = trunc i8 %23 to i1
  %25 = zext i1 %24 to i32
  %26 = icmp eq i32 %25, 1
  call void @__qmi_process_br_instr(i32 3, i1 %26)
  br i1 %26, label %bb8, label %bb11

bb8:                                              ; preds = %bb7
  %27 = load i8, ptr %6, align 1
  %28 = trunc i8 %27 to i1
  %29 = zext i1 %28 to i32
  %30 = icmp eq i32 %29, 1
  call void @__qmi_process_br_instr(i32 4, i1 %30)
  br i1 %30, label %bb9, label %bb10

bb9:                                              ; preds = %bb8
  store i32 1, ptr %0, align 4
  br label %bb12

bb10:                                             ; preds = %bb8
  br label %bb11

bb11:                                             ; preds = %bb10, %bb7
  store i32 0, ptr %0, align 4
  br label %bb12

bb12:                                             ; preds = %bb11, %bb9
  %31 = load i32, ptr %0, align 4
  ret i32 %31
}

declare signext i16 @__VERIFIER_nondet_short(...) #1

declare void @__qmi_process_br_instr(i32)

declare void @__qmi_process_ver_error()

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
