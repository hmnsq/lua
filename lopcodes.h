/*
** $Id: lopcodes.h $
** Opcodes for Lua virtual machine
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lopcodes_h
#నిర్వచించు lopcodes_h

#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lobject.h"


/*===========================================================================
  We assume that instructions are unsigned 32-bit integers.
  All instructions have an opcode in the first 7 bits.
  Instructions can have the following formats:

        3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
        1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
iABC          C(8)     |      B(8)     |k|     A(8)      |   Op(7)     |
ivABC         vC(10)     |     vB(6)   |k|     A(8)      |   Op(7)     |
iABx                Bx(17)               |     A(8)      |   Op(7)     |
iAsBx              sBx (signed)(17)      |     A(8)      |   Op(7)     |
iAx                           Ax(25)                     |   Op(7)     |
isJ                           sJ (signed)(25)            |   Op(7)     |

  ('v' stands for "variant", 's' for "signed", 'x' for "extended".)
  A signed argument is represented in excess K: The represented value is
  the written unsigned value minus K, where K is half (rounded down) the
  maximum value for the corresponding unsigned argument.
===========================================================================*/


/* basic instruction formats */
గణన Op_బహుళకం {iABC, ivABC, iABx, iAsBx, iAx, isJ};


/*
** size and position of opcode arguments.
*/
#నిర్వచించు కొలత_C		8
#నిర్వచించు కొలత_v_C		10
#నిర్వచించు కొలత_B		8
#నిర్వచించు కొలత_v_B		6
#నిర్వచించు కొలత_Bx		(కొలత_C + కొలత_B + 1)
#నిర్వచించు కొలత_A		8
#నిర్వచించు కొలత_Ax		(కొలత_Bx + కొలత_A)
#నిర్వచించు కొలత_s_J		(కొలత_Bx + కొలత_A)

#నిర్వచించు కొలత_OP		7

#నిర్వచించు POS_OP		0

#నిర్వచించు POS_A		(POS_OP + కొలత_OP)
#నిర్వచించు POS_k		(POS_A + కొలత_A)
#నిర్వచించు POS_B		(POS_k + 1)
#నిర్వచించు POS_vB		(POS_k + 1)
#నిర్వచించు POS_C		(POS_B + కొలత_B)
#నిర్వచించు POS_vC		(POS_vB + కొలత_v_B)

#నిర్వచించు POS_Bx		POS_k

#నిర్వచించు POS_Ax		POS_A

#నిర్వచించు POS_sJ		POS_A


/*
** limits for opcode arguments.
** we use (signed) 'int' to manipulate most arguments,
** so they must fit in ints.
*/

/*
** Check whether type 'int' has at least 'b' + 1 bits.
** 'b' < 32; +1 for the sign bit.
*/
#నిర్వచించు L_INTHASBITS(b)		((UINT_MAX >> (b)) >= 1)


#ఒకవేళ L_INTHASBITS(కొలత_Bx)
#నిర్వచించు MAXARG_Bx	((1<<కొలత_Bx)-1)
#లేకపోతే
#నిర్వచించు MAXARG_Bx	గరిష్ఠ_పూర్ణాంకం
#అయితే_ముగింపు

#నిర్వచించు స్థానభ్రంశం_s_Bx	(MAXARG_Bx>>1)         /* 'sBx' is signed */


#ఒకవేళ L_INTHASBITS(కొలత_Ax)
#నిర్వచించు MAXARG_Ax	((1<<కొలత_Ax)-1)
#లేకపోతే
#నిర్వచించు MAXARG_Ax	గరిష్ఠ_పూర్ణాంకం
#అయితే_ముగింపు

#ఒకవేళ L_INTHASBITS(కొలత_s_J)
#నిర్వచించు MAXARG_sJ	((1 << కొలత_s_J) - 1)
#లేకపోతే
#నిర్వచించు MAXARG_sJ	గరిష్ఠ_పూర్ణాంకం
#అయితే_ముగింపు

#నిర్వచించు స్థానభ్రంశం_s_J	(MAXARG_sJ >> 1)


#నిర్వచించు MAXARG_A	((1<<కొలత_A)-1)
#నిర్వచించు MAXARG_B	((1<<కొలత_B)-1)
#నిర్వచించు MAXARG_vB	((1<<కొలత_v_B)-1)
#నిర్వచించు MAXARG_C	((1<<కొలత_C)-1)
#నిర్వచించు MAXARG_vC	((1<<కొలత_v_C)-1)
#నిర్వచించు స్థానభ్రంశం_s_C	(MAXARG_C >> 1)

#నిర్వచించు int2sC(i)	((i) + స్థానభ్రంశం_s_C)
#నిర్వచించు sC2int(i)	((i) - స్థానభ్రంశం_s_C)


/* creates a mask with 'n' 1 bits at position 'p' */
#నిర్వచించు MASK1(n,p)	((~((~(Instruction)0)<<(n)))<<(p))

/* creates a mask with 'n' 0 bits at position 'p' */
#నిర్వచించు MASK0(n,p)	(~MASK1(n,p))

/*
** the following macros help to manipulate instructions
*/

#నిర్వచించు GET_OPCODE(i)	(cast(Op_సంకేతం, ((i)>>POS_OP) & MASK1(కొలత_OP,0)))
#నిర్వచించు సమితి_OPCODE(i,o)	((i) = (((i)&MASK0(కొలత_OP,POS_OP)) | \
		((cast_Inst(o)<<POS_OP)&MASK1(కొలత_OP,POS_OP))))

#నిర్వచించు checkopm(i,m)	(get_Op_బహుళకం(GET_OPCODE(i)) == m)


#నిర్వచించు getarg(i,pos,కొలత)	(cast_int(((i)>>(pos)) & MASK1(కొలత,0)))
#నిర్వచించు setarg(i,v,pos,కొలత)	((i) = (((i)&MASK0(కొలత,pos)) | \
                ((cast_Inst(v)<<pos)&MASK1(కొలత,pos))))

#నిర్వచించు GETARG_A(i)	getarg(i, POS_A, కొలత_A)
#నిర్వచించు SETARG_A(i,v)	setarg(i, v, POS_A, కొలత_A)

#నిర్వచించు GETARG_B(i)  \
	check_exp(checkopm(i, iABC), getarg(i, POS_B, కొలత_B))
#నిర్వచించు GETARG_vB(i)  \
	check_exp(checkopm(i, ivABC), getarg(i, POS_vB, కొలత_v_B))
#నిర్వచించు GETARG_sB(i)	sC2int(GETARG_B(i))
#నిర్వచించు SETARG_B(i,v)	setarg(i, v, POS_B, కొలత_B)
#నిర్వచించు SETARG_vB(i,v)	setarg(i, v, POS_vB, కొలత_v_B)

#నిర్వచించు GETARG_C(i)  \
	check_exp(checkopm(i, iABC), getarg(i, POS_C, కొలత_C))
#నిర్వచించు GETARG_vC(i)  \
	check_exp(checkopm(i, ivABC), getarg(i, POS_vC, కొలత_v_C))
#నిర్వచించు GETARG_sC(i)	sC2int(GETARG_C(i))
#నిర్వచించు SETARG_C(i,v)	setarg(i, v, POS_C, కొలత_C)
#నిర్వచించు SETARG_vC(i,v)	setarg(i, v, POS_vC, కొలత_v_C)

#నిర్వచించు TESTARG_k(i)	(cast_int(((i) & (1u << POS_k))))
#నిర్వచించు GETARG_k(i)	getarg(i, POS_k, 1)
#నిర్వచించు SETARG_k(i,v)	setarg(i, v, POS_k, 1)

#నిర్వచించు GETARG_Bx(i)	check_exp(checkopm(i, iABx), getarg(i, POS_Bx, కొలత_Bx))
#నిర్వచించు SETARG_Bx(i,v)	setarg(i, v, POS_Bx, కొలత_Bx)

#నిర్వచించు GETARG_Ax(i)	check_exp(checkopm(i, iAx), getarg(i, POS_Ax, కొలత_Ax))
#నిర్వచించు SETARG_Ax(i,v)	setarg(i, v, POS_Ax, కొలత_Ax)

#నిర్వచించు GETARG_sBx(i)  \
	check_exp(checkopm(i, iAsBx), getarg(i, POS_Bx, కొలత_Bx) - స్థానభ్రంశం_s_Bx)
#నిర్వచించు SETARG_sBx(i,b)	SETARG_Bx((i),cast_uint((b)+స్థానభ్రంశం_s_Bx))

#నిర్వచించు GETARG_sJ(i)  \
	check_exp(checkopm(i, isJ), getarg(i, POS_sJ, కొలత_s_J) - స్థానభ్రంశం_s_J)
#నిర్వచించు SETARG_sJ(i,j) \
	setarg(i, cast_uint((j)+స్థానభ్రంశం_s_J), POS_sJ, కొలత_s_J)


#నిర్వచించు CREATE_ABCk(o,a,b,c,k)	((cast_Inst(o)<<POS_OP) \
			| (cast_Inst(a)<<POS_A) \
			| (cast_Inst(b)<<POS_B) \
			| (cast_Inst(c)<<POS_C) \
			| (cast_Inst(k)<<POS_k))

#నిర్వచించు CREATE_vABCk(o,a,b,c,k)	((cast_Inst(o)<<POS_OP) \
			| (cast_Inst(a)<<POS_A) \
			| (cast_Inst(b)<<POS_vB) \
			| (cast_Inst(c)<<POS_vC) \
			| (cast_Inst(k)<<POS_k))

#నిర్వచించు CREATE_ABx(o,a,bc)	((cast_Inst(o)<<POS_OP) \
			| (cast_Inst(a)<<POS_A) \
			| (cast_Inst(bc)<<POS_Bx))

#నిర్వచించు CREATE_Ax(o,a)		((cast_Inst(o)<<POS_OP) \
			| (cast_Inst(a)<<POS_Ax))

#నిర్వచించు CREATE_sJ(o,j,k)	((cast_Inst(o) << POS_OP) \
			| (cast_Inst(j) << POS_sJ) \
			| (cast_Inst(k) << POS_k))


#ఒకవేళ !defined(MAXINDEXRK)  /* (for debugging only) */
#నిర్వచించు MAXINDEXRK	MAXARG_B
#అయితే_ముగింపు


/*
** Maximum size for the stack of a Lua function. It must fit in 8 bits.
** The highest valid register is one less than this value.
*/
#నిర్వచించు MAX_FSTACK	MAXARG_A

/*
** Invalid register (one more than last valid register).
*/
#నిర్వచించు NO_REG		MAX_FSTACK



/*
** R[x] - register
** K[x] - constant (in constant table)
** RK(x) == if k(i) then K[x] else R[x]
*/


/*
** Grep "ORDER OP" if you change this enum.
** See "Notes" below for more information about some instructions.
*/

రకనిర్వచనం గణన {
/*----------------------------------------------------------------------
  name		args	description
------------------------------------------------------------------------*/
OP_MOVE,/*	A B	R[A] := R[B]					*/
OP_LOADI,/*	A sBx	R[A] := sBx					*/
OP_LOADF,/*	A sBx	R[A] := (lua_Number)sBx				*/
OP_LOADK,/*	A Bx	R[A] := K[Bx]					*/
OP_LOADKX,/*	A	R[A] := K[extra arg]				*/
OP_LOADFALSE,/*	A	R[A] := false					*/
OP_LFALSESKIP,/*A	R[A] := false; pc++				*/
OP_LOADTRUE,/*	A	R[A] := true					*/
OP_LOADNIL,/*	A B	R[A], R[A+1], ..., R[A+B] := nil		*/
OP_GETUPVAL,/*	A B	R[A] := UpValue[B]				*/
OP_SETUPVAL,/*	A B	UpValue[B] := R[A]				*/

OP_GETTABUP,/*	A B C	R[A] := UpValue[B][K[C]:shortstring]		*/
OP_GETTABLE,/*	A B C	R[A] := R[B][R[C]]				*/
OP_GETI,/*	A B C	R[A] := R[B][C]					*/
OP_GETFIELD,/*	A B C	R[A] := R[B][K[C]:shortstring]			*/

OP_SETTABUP,/*	A B C	UpValue[A][K[B]:shortstring] := RK(C)		*/
OP_SETTABLE,/*	A B C	R[A][R[B]] := RK(C)				*/
OP_SETI,/*	A B C	R[A][B] := RK(C)				*/
OP_SETFIELD,/*	A B C	R[A][K[B]:shortstring] := RK(C)			*/

OP_NEWTABLE,/*	A vB vC k	R[A] := {}				*/

OP_SELF,/*	A B C	R[A+1] := R[B]; R[A] := R[B][K[C]:shortstring]	*/

OP_ADDI,/*	A B sC	R[A] := R[B] + sC				*/

OP_ADDK,/*	A B C	R[A] := R[B] + K[C]:number			*/
OP_SUBK,/*	A B C	R[A] := R[B] - K[C]:number			*/
OP_MULK,/*	A B C	R[A] := R[B] * K[C]:number			*/
OP_MODK,/*	A B C	R[A] := R[B] % K[C]:number			*/
OP_POWK,/*	A B C	R[A] := R[B] ^ K[C]:number			*/
OP_DIVK,/*	A B C	R[A] := R[B] / K[C]:number			*/
OP_IDIVK,/*	A B C	R[A] := R[B] // K[C]:number			*/

OP_BANDK,/*	A B C	R[A] := R[B] & K[C]:integer			*/
OP_BORK,/*	A B C	R[A] := R[B] | K[C]:integer			*/
OP_BXORK,/*	A B C	R[A] := R[B] ~ K[C]:integer			*/

OP_SHLI,/*	A B sC	R[A] := sC << R[B]				*/
OP_SHRI,/*	A B sC	R[A] := R[B] >> sC				*/

OP_కూడు,/*	A B C	R[A] := R[B] + R[C]				*/
OP_SUB,/*	A B C	R[A] := R[B] - R[C]				*/
OP_MUL,/*	A B C	R[A] := R[B] * R[C]				*/
OP_MOD,/*	A B C	R[A] := R[B] % R[C]				*/
OP_POW,/*	A B C	R[A] := R[B] ^ R[C]				*/
OP_DIV,/*	A B C	R[A] := R[B] / R[C]				*/
OP_IDIV,/*	A B C	R[A] := R[B] // R[C]				*/

OP_BAND,/*	A B C	R[A] := R[B] & R[C]				*/
OP_BOR,/*	A B C	R[A] := R[B] | R[C]				*/
OP_BXOR,/*	A B C	R[A] := R[B] ~ R[C]				*/
OP_SHL,/*	A B C	R[A] := R[B] << R[C]				*/
OP_SHR,/*	A B C	R[A] := R[B] >> R[C]				*/

OP_MMBIN,/*	A B C	call C metamethod over R[A] and R[B]		*/
OP_MMBINI,/*	A sB C k	call C metamethod over R[A] and sB	*/
OP_MMBINK,/*	A B C k		call C metamethod over R[A] and K[B]	*/

OP_UNM,/*	A B	R[A] := -R[B]					*/
OP_BNOT,/*	A B	R[A] := ~R[B]					*/
OP_NOT,/*	A B	R[A] := not R[B]				*/
OP_LEN,/*	A B	R[A] := #R[B] (length operator)			*/

OP_CONCAT,/*	A B	R[A] := R[A].. ... ..R[A + B - 1]		*/

OP_CLOSE,/*	A	close all upvalues >= R[A]			*/
OP_TBC,/*	A	mark variable A "to be closed"			*/
OP_JMP,/*	sJ	pc += sJ					*/
OP_EQ,/*	A B k	if ((R[A] == R[B]) ~= k) then pc++		*/
OP_LT,/*	A B k	if ((R[A] <  R[B]) ~= k) then pc++		*/
OP_LE,/*	A B k	if ((R[A] <= R[B]) ~= k) then pc++		*/

OP_EQK,/*	A B k	if ((R[A] == K[B]) ~= k) then pc++		*/
OP_EQI,/*	A sB k	if ((R[A] == sB) ~= k) then pc++		*/
OP_LTI,/*	A sB k	if ((R[A] < sB) ~= k) then pc++			*/
OP_LEI,/*	A sB k	if ((R[A] <= sB) ~= k) then pc++		*/
OP_GTI,/*	A sB k	if ((R[A] > sB) ~= k) then pc++			*/
OP_GEI,/*	A sB k	if ((R[A] >= sB) ~= k) then pc++		*/

OP_TEST,/*	A k	if (not R[A] == k) then pc++			*/
OP_TESTSET,/*	A B k	if (not R[B] == k) then pc++ else R[A] := R[B]  */

OP_CALL,/*	A B C	R[A], ... ,R[A+C-2] := R[A](R[A+1], ... ,R[A+B-1]) */
OP_TAILCALL,/*	A B C k	return R[A](R[A+1], ... ,R[A+B-1])		*/

OP_RETURN,/*	A B C k	return R[A], ... ,R[A+B-2]			*/
OP_RETURN0,/*		return						*/
OP_RETURN1,/*	A	return R[A]					*/

OP_FORLOOP,/*	A Bx	update counters; if loop continues then pc-=Bx; */
OP_FORPREP,/*	A Bx	<check values and prepare counters>;
                        if not to run then pc+=Bx+1;			*/

OP_TFORPREP,/*	A Bx	create upvalue for R[A + 3]; pc+=Bx		*/
OP_TFORCALL,/*	A C	R[A+4], ... ,R[A+3+C] := R[A](R[A+1], R[A+2]);	*/
OP_TFORLOOP,/*	A Bx	if R[A+2] ~= nil then { R[A]=R[A+2]; pc -= Bx }	*/

OP_SETLIST,/*	A vB vC k	R[A][vC+i] := R[A+i], 1 <= i <= vB	*/

OP_సంవృతం,/*	A Bx	R[A] := closure(KPROTO[Bx])			*/

OP_VARARG,/*	A B C k	R[A], ..., R[A+C-2] = varargs  			*/

OP_GETVARG, /* A B C	R[A] := R[B][R[C]], R[B] is vararg parameter    */

OP_ERRNNIL,/*	A Bx	raise error if R[A] ~= nil (K[Bx - 1] is global name)*/

OP_VARARGPREP,/* 	(adjust varargs)				*/

OP_EXTRAARG/*	Ax	extra (larger) argument for previous opcode	*/
} Op_సంకేతం;


#నిర్వచించు అంకె_OPCODES	((పూర్ణాంకం)(OP_EXTRAARG) + 1)



/*===========================================================================
  Notes:

  (*) Opcode OP_LFALSESKIP is used to convert a condition to a boolean
  value, in a code equivalent to (not cond ? false : true).  (It
  produces false and skips the next instruction producing true.)

  (*) Opcodes OP_MMBIN and variants follow each arithmetic and
  bitwise opcode. If the operation succeeds, it skips this next
  opcode. Otherwise, this opcode calls the corresponding metamethod.

  (*) Opcode OP_TESTSET is used in short-circuit expressions that need
  both to jump and to produce a value, such as (a = b or c).

  (*) In OP_CALL, if (B == 0) then B = top - A. If (C == 0), then
  'top' is set to last_result+1, so next open instruction (OP_CALL,
  OP_RETURN*, OP_SETLIST) may use 'top'.

  (*) In OP_VARARG, if (C == 0) then use actual number of varargs and
  set top (like in OP_CALL with C == 0). 'k' means function has a
  vararg table, which is in R[B].

  (*) In OP_RETURN, if (B == 0) then return up to 'top'.

  (*) In OP_LOADKX and OP_NEWTABLE, the next instruction is always
  OP_EXTRAARG.

  (*) In OP_SETLIST, if (B == 0) then real B = 'top'; if k, then
  real C = EXTRAARG _ C (the bits of EXTRAARG concatenated with the
  bits of C).

  (*) In OP_NEWTABLE, vB is log2 of the hash size (which is always a
  power of 2) plus 1, or zero for size zero. If not k, the array size
  is vC. Otherwise, the array size is EXTRAARG _ vC.

  (*) In OP_ERRNNIL, (Bx == 0) means index of global name doesn't
  fit in Bx. (So, that name is not available for the error message.)

  (*) For comparisons, k specifies what condition the test should accept
  (true or false).

  (*) In OP_MMBINI/OP_MMBINK, k means the arguments were flipped
  (the constant is the first operand).

  (*) All comparison and test instructions assume that the instruction
  being skipped (pc++) is a jump.

  (*) In instructions OP_RETURN/OP_TAILCALL, 'k' specifies that the
  function builds upvalues, which may need to be closed. C > 0 means
  the function has hidden vararg arguments, so that its 'func' must be
  corrected before returning; in this case, (C - 1) is its number of
  fixed parameters.

  (*) In comparisons with an immediate operand, C signals whether the
  original operand was a float. (It must be corrected in case of
  metamethods.)

===========================================================================*/


/*
** masks for instruction properties. The format is:
** bits 0-2: op mode
** bit 3: instruction set register A
** bit 4: operator is a test (next instruction must be a jump)
** bit 5: used by 'luaP_isIT'
** bit 6: used by 'luaP_isOT'
** bit 7: instruction is an MM instruction (call a metamethod)
*/

LUAI_DDEC(స్థిరరాశి lu_byte luaP_opmodes[అంకె_OPCODES];)

#నిర్వచించు get_Op_బహుళకం(m)	(cast(గణన Op_బహుళకం, luaP_opmodes[m] & 7))
#నిర్వచించు testAMode(m)	(luaP_opmodes[m] & (1 << 3))
#నిర్వచించు testTMode(m)	(luaP_opmodes[m] & (1 << 4))
#నిర్వచించు testMMMode(m)	(luaP_opmodes[m] & (1 << 7))


/* Check whether instruction sets top for next instruction, that is,
** it results in multiple values. Used only for tests.
*/
#నిర్వచించు luaP_isOT(i)  \
	(GET_OPCODE(i) == OP_TAILCALL || \
	 ((luaP_opmodes[GET_OPCODE(i)] & (1 << 6)) && GETARG_C(i) == 0))


LUAI_FUNC పూర్ణాంకం luaP_isIT (Instruction i);


#అయితే_ముగింపు
