	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 14
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
Lfunc_begin0:
	.cfi_startproc
	.cfi_personality 155, ___gcc_personality_v0
	.cfi_lsda 16, Lexception0
## %bb.0:                               ## %entry
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	subq	$40, %rsp
	.cfi_def_cfa_offset 64
	.cfi_offset %rbx, -24
	.cfi_offset %rbp, -16
	movq	_llvm_gc_root_chain@GOTPCREL(%rip), %rbx
	movq	(%rbx), %rax
	leaq	___gc_main(%rip), %rcx
	movq	%rcx, 16(%rsp)
	movq	$0, 24(%rsp)
	movq	%rax, 8(%rsp)
	leaq	8(%rsp), %rax
	movq	%rax, (%rbx)
	movq	$0, 32(%rsp)
Ltmp0:
	movl	$1048576, %edi          ## imm = 0x100000
	callq	_llvm_gc_initialize
Ltmp1:
## %bb.1:                               ## %.noexc4
Ltmp2:
	movl	$10, %edi
	callq	_llvm_gc_allocate
Ltmp3:
## %bb.2:                               ## %Aptr.noexc
	movq	%rax, 24(%rsp)
Ltmp4:
	movl	$8, %edi
	callq	_llvm_gc_allocate
Ltmp5:
## %bb.3:                               ## %B.upgrd.1.noexc
	movq	%rax, 32(%rsp)
	movq	24(%rsp), %rcx
	movq	%rcx, (%rax)
	movl	$10000000, %ebp         ## imm = 0x989680
	.p2align	4, 0x90
LBB0_4:                                 ## %AllocLoop
                                        ## =>This Inner Loop Header: Depth=1
Ltmp6:
	movl	$100, %edi
	callq	_llvm_gc_allocate
Ltmp7:
## %bb.5:                               ## %.noexc
                                        ##   in Loop: Header=BB0_4 Depth=1
	decl	%ebp
	jne	LBB0_4
## %bb.6:                               ## %Exit
	movq	8(%rsp), %rax
	movq	%rax, (%rbx)
	xorl	%eax, %eax
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	retq
LBB0_7:                                 ## %gc_cleanup
Ltmp8:
	movq	8(%rsp), %rcx
	movq	%rcx, (%rbx)
	movq	%rax, %rdi
	callq	__Unwind_Resume
	ud2
Lfunc_end0:
	.cfi_endproc
	.section	__TEXT,__gcc_except_tab
	.p2align	2
GCC_except_table0:
Lexception0:
	.byte	255                     ## @LPStart Encoding = omit
	.byte	255                     ## @TType Encoding = omit
	.byte	1                       ## Call site Encoding = uleb128
	.uleb128 Lcst_end0-Lcst_begin0
Lcst_begin0:
	.uleb128 Ltmp0-Lfunc_begin0     ## >> Call Site 1 <<
	.uleb128 Ltmp7-Ltmp0            ##   Call between Ltmp0 and Ltmp7
	.uleb128 Ltmp8-Lfunc_begin0     ##     jumps to Ltmp8
	.byte	0                       ##   On action: cleanup
	.uleb128 Ltmp7-Lfunc_begin0     ## >> Call Site 2 <<
	.uleb128 Lfunc_end0-Ltmp7       ##   Call between Ltmp7 and Lfunc_end0
	.byte	0                       ##     has no landing pad
	.byte	0                       ##   On action: cleanup
Lcst_end0:
	.p2align	2
                                        ## -- End function
	.section	__DATA,__data
	.globl	_llvm_gc_root_chain     ## @llvm_gc_root_chain
	.weak_definition	_llvm_gc_root_chain
	.p2align	3
_llvm_gc_root_chain:
	.quad	0

	.section	__TEXT,__const
	.p2align	3               ## @__gc_main
___gc_main:
	.long	2                       ## 0x2
	.long	0                       ## 0x0


.subsections_via_symbols
