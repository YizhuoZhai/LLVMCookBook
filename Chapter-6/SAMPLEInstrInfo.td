def ADDrr : InstSAMPLE <(outs GRRegs:$dst), 
			(ins GRRegs:$src1, GRRegs:$src2),
			"add $dst, $src1, $src2",
			[(set i32:$dst, (add i32:$src1, i32:$src2))]>;

