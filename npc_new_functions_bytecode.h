class ONPCDead : public UnaryOpcode
{
public:
	ONPCDead(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCDead(a->clone());
	}
};
class ONPCCanSlide : public UnaryOpcode
{
public:
	ONPCCanSlide(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCCanSlide(a->clone());
	}
};
class ONPCSlide : public UnaryOpcode
{
public:
	ONPCSlide(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCSlide(a->clone());
	}
};
class ONPCRemove : public UnaryOpcode
{
public:
	ONPCRemove(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCRemove(a->clone());
	}
};
class ONPCStopSFX : public UnaryOpcode
{
public:
	ONPCStopSFX(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCStopSFX(a->clone());
	}
};
class ONPCAttack : public UnaryOpcode
{
public:
	ONPCAttack(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCAttack(a->clone());
	}
};
class ONPCNewDir : public UnaryOpcode
{
public:
	ONPCNewDir(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCNewDir(a->clone());
	}
};
class ONPCConstWalk : public UnaryOpcode
{
public:
	ONPCConstWalk(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCConstWalk(a->clone());
	}
};
class ONPCVarWalk : public UnaryOpcode
{
public:
	ONPCVarWalk(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCVarWalk(a->clone());
	}
};
class ONPCHaltWalk : public UnaryOpcode
{
public:
	ONPCHaltWalk(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCHaltWalk(a->clone());
	}
};
class ONPCFloatWalk : public UnaryOpcode
{
public:
	ONPCFloatWalk(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCFloatWalk(a->clone());
	}
};
class ONPCBreatheFire : public UnaryOpcode
{
public:
	ONPCBreatheFire(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCBreatheFire(a->clone());
	}
};
class ONPCNewDir8 : public UnaryOpcode
{
public:
	ONPCNewDir8(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCNewDir8(a->clone());
	}
};
class ONPCLinkInRange : public UnaryOpcode
{
public:
	ONPCLinkInRange(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCLinkInRange(a->clone());
	}
};
class ONPCAdd : public UnaryOpcode
{
public:
	ONPCAdd(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCAdd(a->clone());
	}
};
class ONPCCanMove : public UnaryOpcode
{
public:
	ONPCCanMove(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCCanMove(a->clone());
	}
};
class ONPCHitWith : public UnaryOpcode
{
public:
	ONPCHitWith(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new ONPCHitWith(a->clone());
	}
};