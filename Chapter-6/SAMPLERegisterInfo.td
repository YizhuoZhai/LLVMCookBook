class SAMPLEReg<bits<16> End, string n> : Register<n> {
	let HWEncoding = Enc;
	let Namespace = "SAMPLE";
}

foreach i = 0-3 in {
	def R#i : R<i, "r"#i>;
}

def SP : SAMPLEREG<13, "sp">;
def LR : SAMPLEREG<14, "lr">;

def GRRegs : RegisterClass<"SAMPLE", [i32], 32 
	(add R0, R1, R2, R3, SP)>;
