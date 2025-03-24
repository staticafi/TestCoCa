; ModuleID = '/home/martin/school/bac/qm2c/output/benchmarks/../../sbt-fizzer/out/bit_and_simple/bit_and_simple.ll'
source_filename = "/home/martin/school/bac/qm2c/benchmarks/../../sbt-fizzer/out/bit_and_simple/bit_and_simple.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__qmi_cond_br_count = constant i32 1, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @__qmi_original_main() #0 {
bb1:
  %0 = alloca i32, align 4
  %1 = alloca i32, align 4
  store i32 0, ptr %0, align 4
  %2 = call i32 @__VERIFIER_nondet_int()
  store i32 %2, ptr %1, align 4
  %3 = load i32, ptr %1, align 4
  %4 = and i32 %3, 65280
  %5 = icmp eq i32 %4, 768
  call void @__qmi_process_br_instr(i32 1, i1 %5)
  br i1 %5, label %bb2, label %bb3

bb2:                                              ; preds = %bb1
  store i32 1, ptr %0, align 4
  br label %bb4

bb3:                                              ; preds = %bb1
  store i32 0, ptr %0, align 4
  br label %bb4

bb4:                                              ; preds = %bb3, %bb2
  %6 = load i32, ptr %0, align 4
  ret i32 %6
}

declare i32 @__VERIFIER_nondet_int() #1

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
