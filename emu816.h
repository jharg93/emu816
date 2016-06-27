#ifndef EMU816
#define EMU816
#include "mem816.h"

#if 1
# define TRACE(MNEM)	{ dump(MNEM, ea); }
# define BYTES(N)		{ bytes(N); pc += N; }
# define SHOWPC()		{ show(); }
# ifdef CHIPKIT
# define ENDL()			{ Serial.println (); }
# else
# define ENDL()			{ cout << endl; }
# endif
#else
# define TRACE(MNEM)
# define BYTES(N)		{ pc += N; }
# define SHOWPC()
# define ENDL()
#endif

class emu816 :
	public wdc816
{
public:
	emu816(mem816 &mem);
	~emu816();

	void reset();
	void step();

private:
	union {
		struct {
			Bit				f_c : 1;
			Bit				f_z : 1;
			Bit				f_i : 1;
			Bit				f_d : 1;
			Bit				f_x : 1;
			Bit				f_m : 1;
			Bit				f_v : 1;
			Bit				f_n : 1;
		};
		Byte			b;
	}   p;

	Bit				e;

	union {
		Byte			b;
		Word			w;
	}   a, x, y, sp, dp;

	Word			pc;
	Byte			pbr, dbr;

	mem816		   &mem;

	bool			interrupted;
	unsigned long	cycles;

	void show();
	void bytes(unsigned int);
	void dump(const char *, Addr);

	INLINE void pushByte(Byte value)
	{
		mem.setByte(sp.w, value);

		if (e)
			--sp.b;
		else
			--sp.w;
	}

	INLINE void pushWord(Word value)
	{
		pushByte(hi(value));
		pushByte(lo(value));
	}

	INLINE Byte pullByte()
	{
		if (e)
			++sp.b;
		else
			++sp.w;

		return (mem.getByte(sp.w));
	}

	INLINE Word pullWord()
	{
		register Byte	l = pullByte();
		register Byte	h = pullByte();

		return (join(l, h));
	}

	// Absolute - a
	INLINE Addr am_absl()
	{
		register Addr	ea = join (dbr, mem.getWord(bank(pbr) | pc));

		BYTES(2);
		return (ea);
	}

	// Absolute Indexed X - a,X
	INLINE Addr am_absx()
	{
		register Addr	ea = join(dbr, mem.getWord(bank(pbr) | pc)) + x.w;

		BYTES(2);
		return (ea);
	}

	// Absolute Indexed Y - a,Y
	INLINE Addr am_absy()
	{
		register Addr	ea = join(dbr, mem.getWord(bank(pbr) | pc)) + y.w;

		BYTES(2);
		return (ea);
	}

	// Absolute Indirect - (a)
	INLINE Addr am_absi()
	{
		register Addr ia = join(0, mem.getWord(bank(pbr) | pc));

		BYTES(2);
		return (join(0, mem.getWord(ia)));
	}

	// Absolute Indexed Indirect - (a,X)
	INLINE Addr am_abxi()
	{
		register Addr ia = join(pbr, mem.getWord(bank(pbr) | pc)) + x.w;

		BYTES(2);
		return (join(pbr, mem.getWord(ia)));
	}

	// Absolute Long - >a
	INLINE Addr am_alng()
	{
		Addr ea = mem.getAddr(bank(pbr) | pc);

		BYTES(3);
		return (ea);
	}

	// Absolute Long Indexed - >a,X
	INLINE Addr am_alnx()
	{
		register Addr ea = mem.getAddr(bank(pbr) | pc) + x.w;

		BYTES(3);
		return (ea);
	}

	// Absolute Indirect Long - [a]
	INLINE Addr am_abil()
	{
		register Addr ia = bank(0) | mem.getWord(bank(pbr) | pc);

		BYTES(2);
		return (mem.getAddr(ia));
	}

	// Direct Page - d
	INLINE Addr am_dpag()
	{
		Byte offset = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (bank(0) | (Word)(dp.w + offset));
	}

	// Direct Page Indexed X - d,X
	INLINE Addr am_dpgx()
	{
		Byte offset = mem.getByte(bank(pbr) | pc) + x.b;

		BYTES(1);
		return (bank(0) | (Word)(dp.w + offset));
	}

	// Direct Page Indexed Y - d,Y
	INLINE Addr am_dpgy()
	{
		Byte offset = mem.getByte(bank(pbr) | pc) + y.b;

		BYTES(1);
		return (bank(0) | (Word)(dp.w + offset));
	}

	// Direct Page Indirect - (d)
	INLINE Addr am_dpgi()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (bank(dbr) | mem.getWord(bank(0) | (Word)(dp.w + disp)));
	}

	// Direct Page Indexed Indirect - (d,x)
	INLINE Addr am_dpix()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (bank(dbr) | mem.getWord(bank(0) | (Word)(dp.w + disp + x.w)));
	}

	// Direct Page Indirect Indexed - (d),Y
	INLINE Addr am_dpiy()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (bank(dbr) | mem.getWord(bank(0) | (dp.w + disp)) + y.w);
	}

	// Direct Page Indirect Long - [d]
	INLINE Addr am_dpil()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (mem.getAddr(bank(0) | (Word)(dp.w + disp)));
	}

	// Direct Page Indirect Long Indexed - [d],Y
	INLINE Addr am_dily()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (mem.getAddr(bank(0) | (Word)(dp.w + disp)) + y.w);
	}

	// Implied/Stack
	INLINE Addr am_impl()
	{
		BYTES(0);
		return (0);
	}

	// Accumulator
	INLINE Addr am_acc()
	{
		BYTES(0);
		return (0);
	}

	// Immediate Byte
	INLINE Addr am_immb()
	{
		Addr ea = bank(pbr) | pc;

		BYTES(1);
		return (ea);
	}

	// Immediate Word
	INLINE Addr am_immw()
	{
		Addr ea = bank(pbr) | pc;

		BYTES(2);
		return (ea);
	}

	// Immediate based on size of A/M
	INLINE Addr am_immm()
	{
		Addr ea = bank(pbr) | pc;
		unsigned int size = (e || p.f_m) ? 1 : 2;

		BYTES(size);
		return (ea);
	}

	// Immediate based on size of X/Y
	INLINE Addr am_immx()
	{
		Addr ea = bank(pbr) | pc;
		unsigned int size = (e || p.f_x) ? 1 : 2;

		BYTES(size);
		return (ea);
	}

	INLINE Addr am_lrel()
	{
		Word disp = mem.getWord(bank(pbr) | pc);

		BYTES(2);
		return (bank(pbr) | (Word)(pc + (signed short)disp));
	}

	INLINE Addr am_rela()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);
		return (bank(pbr) | (Word)(pc + (signed char)disp));
	}

	// Stack Relative - d,S
	INLINE Addr am_srel()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);

		BYTES(1);

		if (e)
			return((bank(0) | join(sp.b + disp, hi(sp.w))));
		else
			return (bank(0) | (Word)(sp.w + disp));
	}

	// Stack Relative Indirect Indexed Y - (d,S),Y
	INLINE Addr am_sriy()
	{
		Byte disp = mem.getByte(bank(pbr) | pc);
		register Word ia;

		BYTES(1);

		if (e)
			ia = mem.getWord(join(sp.b + disp, hi(sp.w)));
		else
			ia = mem.getWord(bank(0) | (sp.w + disp));

		return (bank(dbr) | (Word)(ia + y.w));
	}

	INLINE void setn(unsigned int flag)
	{
		p.f_n = flag ? 1 : 0;
	}

	INLINE void setv(unsigned int flag)
	{
		p.f_v = flag ? 1 : 0;
	}

	INLINE void setd(unsigned int flag)
	{
		p.f_d = flag ? 1 : 0;
	}

	INLINE void seti(unsigned int flag)
	{
		p.f_i = flag ? 1 : 0;
	}

	INLINE void setz(unsigned int flag)
	{
		p.f_z = flag ? 1 : 0;
	}

	INLINE void setc(unsigned int flag)
	{
		p.f_c = flag ? 1 : 0;
	}

	INLINE void setnz_b(Byte value)
	{
		setn(value & 0x80);
		setz(value == 0);
	}

	INLINE void setnz_w(Word value)
	{
		setn(value & 0x8000);
		setz(value == 0);
	}

	INLINE void op_adc(Addr ea)
	{
		TRACE("ADC");

		if (e || p.f_m) {
			Byte	data = mem.getByte(ea);
			Word	temp = a.b + data + p.f_c;

			setc(temp & 0x100);
			setv((~(a.b ^ data)) & (a.b ^ temp) & 0x80);
			setnz_b(a.b = lo(temp));
		}
		else {
			Word	data = mem.getWord(ea);
			int		temp = a.w + data + p.f_c;

			setc(temp & 0x100);
			setv((~(a.w ^ data)) & (a.w ^ temp) & 0x8000);
			setnz_w(a.w = (Word)temp);
		}
	}

	INLINE void op_and(Addr ea)
	{
		TRACE("AND");

		if (e || p.f_m)
			setnz_b(a.b &= mem.getByte(ea));
		else
			setnz_w(a.w &= mem.getWord(ea));
	}

	INLINE void op_asl(Addr ea)
	{
		TRACE("ASL");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			setc(data & 0x80);
			setnz_b(data <<= 1);
			mem.setByte(ea, data);
		}
		else {
			register Word data = mem.getWord(ea);

			setc(data & 0x8000);
			setnz_w(data <<= 1);
			mem.setWord(ea, data);
		}
	}

	INLINE void op_asla(Addr ea)
	{
		TRACE("ASL");

		if (e || p.f_m) {
			setc(a.b & 0x80);
			setnz_b(a.b <<= 1);
			mem.setByte(ea, a.b);
		}
		else {
			setc(a.w & 0x8000);
			setnz_w(a.w <<= 1);
			mem.setWord(ea, a.w);
		}
	}

	INLINE void op_bcc(Addr ea)
	{
		TRACE("BCC");

		if (p.f_c == 0)
			pc = (Word)ea;
	}

	INLINE void op_bcs(Addr ea)
	{
		TRACE("BCS");

		if (p.f_c == 1)
			pc = (Word)ea;
	}

	INLINE void op_beq(Addr ea)
	{
		TRACE("BEQ");

		if (p.f_z == 1)
			pc = (Word)ea;
	}

	INLINE void op_bit(Addr ea)
	{
		TRACE("BIT");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			setz((a.b & data) == 0);
			setn(data & 0x80);
			setv(data & 0x40);
		}
		else {
			register Word data = mem.getWord(ea);

			setz((a.w & data) == 0);
			setn(data & 0x8000);
			setv(data & 0x4000);
		}
	}

	INLINE void op_biti(Addr ea)
	{
		TRACE("BIT");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			setz((a.b & data) == 0);
		}
		else {
			register Word data = mem.getWord(ea);

			setz((a.w & data) == 0);
		}
	}

	INLINE void op_bmi(Addr ea)
	{
		TRACE("BMI");

		if (p.f_n == 1)
			pc = (Word)ea;
	}

	INLINE void op_bne(Addr ea)
	{
		TRACE("BNE");

		if (p.f_z == 0)
			pc = (Word)ea;
	}

	INLINE void op_bpl(Addr ea)
	{
		TRACE("BPL");

		if (p.f_n == 0)
			pc = (Word)ea;
	}

	INLINE void op_bra(Addr ea)
	{
		TRACE("BRA");

		pc = (Word)ea;
	}

	INLINE void op_brk(Addr ea)
	{
		TRACE("BRK");

		if (e) {
			pushWord(pc);
			pushByte(p.b | 0x10);

			p.f_i = 1;
			p.f_d = 0;
			pbr = 0;

			pc = mem.getWord(0xfffe);
		}
		else {
			pushByte(pbr);
			pushWord(pc);
			pushByte(p.b);

			p.f_i = 1;
			p.f_d = 0;
			pbr = 0;

			pc = mem.getWord(0xffe6);
		}
	}

	INLINE void op_brl(Addr ea)
	{
		TRACE("BRL");

		pc = (Word)ea;
	}

	INLINE void op_bvc(Addr ea)
	{
		TRACE("BVC");

		if (p.f_v == 0)
			pc = (Word)ea;
	}

	INLINE void op_bvs(Addr ea)
	{
		TRACE("BVS");

		if (p.f_v == 1)
			pc = (Word)ea;
	}

	INLINE void op_clc(Addr ea)
	{
		TRACE("CLC");

		setc(0);
	}

	INLINE void op_cld(Addr ea)
	{
		TRACE("CLD")

			setd(0);
	}

	INLINE void op_cli(Addr ea)
	{
		TRACE("CLI")

			seti(0);
	}

	INLINE void op_clv(Addr ea)
	{
		TRACE("CLD")

			setv(0);
	}

	INLINE void op_cmp(Addr ea)
	{
		TRACE("CMP");

		if (e || p.f_m) {
			Byte	data = mem.getByte(ea);
			Word	temp = a.b - data;

			setc(temp & 0x100);
			setnz_b(lo(temp));
		}
		else {
			Word	data = mem.getWord(ea);
			Addr	temp = a.w - data;

			setc(temp & 0x10000L);
			setnz_w((Word)temp);
		}
	}

	INLINE void op_cop(Addr ea)
	{
		TRACE("COP");

		if (e) {
			pushWord(pc);
			pushByte(p.b);

			p.f_i = 1;
			p.f_d = 0;
			pbr = 0;

			pc = mem.getWord(0xfff4);
		}
		else {
			pushByte(pbr);
			pushWord(pc);
			pushByte(p.b);

			p.f_i = 1;
			p.f_d = 0;
			pbr = 0;

			pc = mem.getWord(0xffe4);
		}
	}

	INLINE void op_cpx(Addr ea)
	{
		TRACE("CPX");

		if (e || p.f_x) {
			Byte	data = mem.getByte(ea);
			setnz_b(x.b - data);
			setc(x.b >= data);
		}
		else {
			Word	data = mem.getWord(ea);
			setnz_w(x.w - data);
			setc(x.w >= data);
		}
	}

	INLINE void op_cpy(Addr ea)
	{
		TRACE("CPY");

		if (e || p.f_x) {
			Byte	data = mem.getByte(ea);
			setnz_b(y.b - data);
			setc(y.b >= data);
		}
		else {
			Word	data = mem.getWord(ea);
			setnz_w(y.w - data);
			setc(y.w >= data);
		}
	}

	INLINE void op_dec(Addr ea)
	{
		TRACE("DEC");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			mem.setByte(ea, ++data);
			setnz_b(data);
		}
		else {
			register Word data = mem.getWord(ea);

			mem.setWord(ea, ++data);
			setnz_w(data);
		}
	}

	INLINE void op_deca(Addr ea)
	{
		TRACE("DEC");

		if (e || p.f_m)
			setnz_b(--a.b);
		else
			setnz_w(--a.w);
	}

	INLINE void op_dex(Addr ea)
	{
		TRACE("DEX");

		if (e || p.f_x)
			setnz_b(x.b -= 1);
		else
			setnz_w(x.w -= 1);
	}

	INLINE void op_dey(Addr ea)
	{
		TRACE("DEY");

		if (e || p.f_x)
			setnz_b(y.b -= 1);
		else
			setnz_w(y.w -= 1);
	}

	INLINE void op_eor(Addr ea)
	{
		TRACE("EOR");

		if (e || p.f_m)
			setnz_b(a.b ^= mem.getByte(ea));
		else
			setnz_w(a.w ^= mem.getWord(ea));
	}

	INLINE void op_inc(Addr ea)
	{
		TRACE("INC");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			mem.setByte(ea, ++data);
			setnz_b(data);
		}
		else {
			register Word data = mem.getWord(ea);

			mem.setWord(ea, ++data);
			setnz_w(data);
		}
	}

	INLINE void op_inca(Addr ea)
	{
		TRACE("INC");

		if (e || p.f_m)
			setnz_b(++a.b);
		else
			setnz_w(++a.w);
	}

	INLINE void op_inx(Addr ea)
	{
		TRACE("INX");

		if (e || p.f_x)
			setnz_b(++x.b);
		else
			setnz_w(++x.w);
	}

	INLINE void op_iny(Addr ea)
	{
		TRACE("INY");

		if (e || p.f_x)
			setnz_b(++y.b);
		else
			setnz_w(++y.w);
	}

	INLINE void op_jmp(Addr ea)
	{
		TRACE("JMP");

		pbr = lo(ea >> 16);
		pc = (Word)ea;
	}

	INLINE void op_jsl(Addr ea)
	{
		TRACE("JSL");

		pushByte(pbr);
		pushWord(pc - 1);

		pbr = lo(ea >> 16);
		pc = (Word)ea;
	}

	INLINE void op_jsr(Addr ea)
	{
		TRACE("JSR");

		pushWord(pc - 1);

		pc = (Word)ea;
	}

	INLINE void op_lda(Addr ea)
	{
		TRACE("LDA");

		if (e || p.f_m)
			setnz_b(a.b = mem.getByte(ea));
		else
			setnz_w(a.w = mem.getWord(ea));
	}

	INLINE void op_ldx(Addr ea)
	{
		TRACE("LDX");

		if (e || p.f_x)
			setnz_b(lo(x.w = mem.getByte(ea)));
		else
			setnz_w(x.w = mem.getWord(ea));
	}

	INLINE void op_ldy(Addr ea)
	{
		TRACE("LDY");

		if (e || p.f_x)
			setnz_b(lo(y.w = mem.getByte(ea)));
		else
			setnz_w(y.w = mem.getWord(ea));
	}

	INLINE void op_lsr(Addr ea)
	{
		TRACE("LSR");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			setc(data & 0x01);
			setnz_b(data >>= 1);
			mem.setByte(ea, data);
		}
		else {
			register Word data = mem.getWord(ea);

			setc(data & 0x0001);
			setnz_w(data >>= 1);
			mem.setWord(ea, data);
		}
	}

	INLINE void op_lsra(Addr ea)
	{
		TRACE("LSR");

		if (e || p.f_m) {
			setc(a.b & 0x01);
			setnz_b(a.b >>= 1);
			mem.setByte(ea, a.b);
		}
		else {
			setc(a.w & 0x0001);
			setnz_w(a.w >>= 1);
			mem.setWord(ea, a.w);
		}
	}

	INLINE void op_mvn(Addr ea)
	{
		TRACE("MVN");
	}

	INLINE void op_mvp(Addr ea)
	{
		TRACE("MVP");
	}

	INLINE void op_nop(Addr ea)
	{
		TRACE("NOP");
	}

	INLINE void op_ora(Addr ea)
	{
		TRACE("ORA");

		if (e || p.f_m)
			setnz_b(a.b |= mem.getByte(ea));
		else
			setnz_w(a.w |= mem.getWord(ea));
	}

	INLINE void op_pea(Addr ea)
	{
		TRACE("PEA");

		pushWord(mem.getWord(ea));
	}

	INLINE void op_pei(Addr ea)
	{
		TRACE("PEI");
	}

	INLINE void op_per(Addr ea)
	{
		TRACE("PER");
	}

	INLINE void op_pha(Addr ea)
	{
		TRACE("PHA");

		if (e || p.f_m)
			pushByte(a.b);
		else
			pushWord(a.w);
	}

	INLINE void op_phb(Addr ea)
	{
		TRACE("PHB");

		pushByte(dbr);
	}

	INLINE void op_phd(Addr ea)
	{
		TRACE("PHD");

		pushWord(dp.w);
	}

	INLINE void op_phk(Addr ea)
	{
		TRACE("PHK");

		pushByte(pbr);
	}

	INLINE void op_php(Addr ea)
	{
		TRACE("PHP");

		pushByte(p.b);
	}

	INLINE void op_phx(Addr ea)
	{
		TRACE("PHX");

		if (e || p.f_x)
			pushByte(x.b);
		else
			pushWord(x.w);
	}

	INLINE void op_phy(Addr ea)
	{
		TRACE("PHY");

		if (e || p.f_x)
			pushByte(y.b);
		else
			pushWord(y.w);
	}

	INLINE void op_pla(Addr ea)
	{
		TRACE("PLA");

		if (e || p.f_m)
			setnz_b(a.b = pullByte());
		else
			setnz_w(a.w = pullWord());
	}

	INLINE void op_plb(Addr ea)
	{
		TRACE("PLB");

		setnz_b(dbr = pullByte());
	}

	INLINE void op_pld(Addr ea)
	{
		TRACE("PLD");

		setnz_w(dp.w = pullWord());
	}

	INLINE void op_plk(Addr ea)
	{
		TRACE("PLK");

		setnz_b(dbr = pullByte());
	}

	INLINE void op_plp(Addr ea)
	{
		TRACE("PLP");

		if (e)
			p.b = pullByte() | 0x30;
		else {
			p.b = pullByte();

			if (p.f_x) {
				x.w = x.b;
				y.w = y.b;
			}
		}
	}

	INLINE void op_plx(Addr ea)
	{
		TRACE("PLX");

		if (e || p.f_x)
			setnz_b(lo(x.w = pullByte()));
		else
			setnz_w(x.w = pullWord());
	}

	INLINE void op_ply(Addr ea)
	{
		TRACE("PLY");

		if (e || p.f_x)
			setnz_b(lo(y.w = pullByte()));
		else
			setnz_w(y.w = pullWord());
	}

	INLINE void op_rep(Addr ea)
	{
		TRACE("REP");

		p.b &= ~mem.getByte(ea);
		if (e) p.f_m = p.f_x = 1;
	}

	INLINE void op_rol(Addr ea)
	{
		TRACE("ROL");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);
			register Byte carry = p.f_c ? 0x01 : 0x00;

			setc(data & 0x80);
			setnz_b(data = (data << 1) | carry);
			mem.setByte(ea, data);
		}
		else {
			register Word data = mem.getWord(ea);
			register Word carry = p.f_c ? 0x0001 : 0x0000;

			setc(data & 0x8000);
			setnz_w(data = (data << 1) | carry);
			mem.setWord(ea, data);
		}
	}

	INLINE void op_rola(Addr ea)
	{
		TRACE("ROL");

		if (e || p.f_m) {
			register Byte carry = p.f_c ? 0x01 : 0x00;

			setc(a.b & 0x80);
			setnz_b(a.b = (a.b << 1) | carry);
			mem.setByte(ea, a.b);
		}
		else {
			register Word carry = p.f_c ? 0x0001 : 0x0000;

			setc(a.w & 0x8000);
			setnz_w(a.w = (a.w << 1) | carry);
			mem.setWord(ea, a.w);
		}
	}

	INLINE void op_ror(Addr ea)
	{
		TRACE("ROR");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);
			register Byte carry = p.f_c ? 0x80 : 0x00;

			setc(data & 0x80);
			setnz_b(data = (data >> 1) | carry);
			mem.setByte(ea, data);
		}
		else {
			register Word data = mem.getWord(ea);
			register Word carry = p.f_c ? 0x8000 : 0x0000;

			setc(data & 0x8000);
			setnz_w(data = (data >> 1) | carry);
			mem.setWord(ea, data);
		}
	}

	INLINE void op_rora(Addr ea)
	{
		TRACE("ROR");

		if (e || p.f_m) {
			register Byte carry = p.f_c ? 0x80 : 0x00;

			setc(a.b & 0x80);
			setnz_b(a.b = (a.b >> 1) | carry);
			mem.setByte(ea, a.b);
		}
		else {
			register Word carry = p.f_c ? 0x8000 : 0x0000;

			setc(a.w & 0x8000);
			setnz_w(a.w = (a.w >> 1) | carry);
			mem.setWord(ea, a.w);
		}
	}

	INLINE void op_rtl(Addr ea)
	{
		TRACE("RTL");

		pc = pullWord();
		pbr = pullByte();
	}

	INLINE void op_rti(Addr ea)
	{
		TRACE("RTI");

		if (e) {
			p.b = pullByte();
			pc = pullWord();
		}
		else {
			p.b = pullByte();
			pc = pullWord();
			pbr = pullByte();
		}
		p.f_i = 0;
	}

	INLINE void op_rts(Addr ea)
	{
		TRACE("RTS");

		pc = pullWord() + 1;
	}

	INLINE void op_sbc(Addr ea)
	{
		TRACE("SBC");

		if (e || p.f_m) {
			Byte	data = ~mem.getByte(ea);
			Word	temp = a.b + data + p.f_c;

			setc(temp & 0x100);
			setv((~(a.b ^ data)) & (a.b ^ temp) & 0x80);
			setnz_b(a.b = lo(temp));
		}
		else {
			Word	data = ~mem.getWord(ea);
			int		temp = a.w + data + p.f_c;

			setc(temp & 0x100);
			setv((~(a.w ^ data)) & (a.w ^ temp) & 0x8000);
			setnz_w(a.w = (Word)temp);
		}
	}

	INLINE void op_sec(Addr ea)
	{
		TRACE("SEC");

		setc(1);
	}

	INLINE void op_sed(Addr ea)
	{
		TRACE("SED");

		setd(1);
	}

	INLINE void op_sei(Addr ea)
	{
		TRACE("SEI");

		seti(1);
	}

	INLINE void op_sep(Addr ea)
	{
		TRACE("SEP");

		p.b |= mem.getByte(ea);
		if (e) p.f_m = p.f_x = 1;

		if (p.f_x) {
			x.w = x.b;
			y.w = y.b;
		}
	}

	INLINE void op_sta(Addr ea)
	{
		TRACE("STA");

		if (e || p.f_m)
			mem.setByte(ea, a.b);
		else
			mem.setWord(ea, a.w);
	}

	INLINE void op_stp(Addr ea)
	{
		TRACE("STP");

		if (!interrupted) {
			pc -= 1;
		}
		else
			interrupted = false;
	}

	INLINE void op_stx(Addr ea)
	{
		TRACE("STX");

		if (e || p.f_x)
			mem.setByte(ea, x.b);
		else
			mem.setWord(ea, x.w);
	}

	INLINE void op_sty(Addr ea)
	{
		TRACE("STY");

		if (e || p.f_x)
			mem.setByte(ea, y.b);
		else
			mem.setWord(ea, y.w);
	}

	INLINE void op_stz(Addr ea)
	{
		TRACE("STZ");

		if (e || p.f_m)
			mem.setByte(ea, 0);
		else
			mem.setWord(ea, 0);
	}

	INLINE void op_tax(Addr ea)
	{
		TRACE("TAX");

		if (e || p.f_x)
			setnz_b(lo(x.w = a.b));
		else
			setnz_w(x.w = a.w);
	}

	INLINE void op_tay(Addr ea)
	{
		TRACE("TAY");

		if (e || p.f_x)
			setnz_b(lo(y.w = a.b));
		else
			setnz_w(y.w = a.w);
	}

	INLINE void op_tcd(Addr ea)
	{
		TRACE("TCD");

		dp.w = a.w;
	}

	INLINE void op_tdc(Addr ea)
	{
		TRACE("TDC");

		if (e || p.f_m)
			setnz_b(lo(a.w = dp.w));
		else
			setnz_w(a.w = dp.w);
	}

	INLINE void op_tcs(Addr ea)
	{
		TRACE("TCS");

		sp.w = e ? (0x0100 | a.b) : a.w;
	}

	INLINE void op_trb(Addr ea)
	{
		TRACE("TRB");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			mem.setByte(ea, data & ~a.b);
			setz((a.b & data) == 0);
		}
		else {
			register Word data = mem.getWord(ea);

			mem.setWord(ea, data & ~a.w);
			setz((a.w & data) == 0);
		}
	}

	INLINE void op_tsb(Addr ea)
	{
		TRACE("TSB");

		if (e || p.f_m) {
			register Byte data = mem.getByte(ea);

			mem.setByte(ea, data | a.b);
			setz((a.b & data) == 0);
		}
		else {
			register Word data = mem.getWord(ea);

			mem.setWord(ea, data | a.w);
			setz((a.w & data) == 0);
		}
	}

	INLINE void op_tsc(Addr ea)
	{
		TRACE("TSC");

		if (e || p.f_m)
			setnz_b(lo(a.w = sp.w));
		else
			setnz_w(a.w = sp.w);
	}

	INLINE void op_tsx(Addr ea)
	{
		TRACE("TSX");
	}

	INLINE void op_txa(Addr ea)
	{
		TRACE("TXA");
	}

	INLINE void op_txs(Addr ea)
	{
		TRACE("TXS");
	}

	INLINE void op_txy(Addr ea)
	{
		TRACE("TXY");

		if (e || p.f_x)
			setnz_b(lo(y.w = x.w));
		else
			setnz_w(y.w = x.w);
	}

	INLINE void op_tya(Addr ea)
	{
		TRACE("TYA");

		if (e || p.f_m)
			setnz_b(a.b = y.b);
		else
			setnz_w(a.w = p.f_x ? y.b : y.w);
	}

	INLINE void op_tyx(Addr ea)
	{
		TRACE("TYX");

		if (e || p.f_x)
			setnz_b(lo(x.w = y.w));
		else
			setnz_w(x.w = y.w);
	}

	INLINE void op_wai(Addr ea)
	{
		TRACE("WAI");

		if (!interrupted) {
			pc -= 1;
		}
		else
			interrupted = false;
	}

	INLINE void op_wdm(Addr ea)
	{
		TRACE("WDM");
	}

	INLINE void op_xba(Addr ea)
	{
		TRACE("XBA");

		a.w = swap(a.w);
		setnz_b(a.b);
	}

	INLINE void op_xce(Addr ea)
	{
		TRACE("XCE");

		unsigned char	oe = e;

		e = p.f_c;
		p.f_c = oe;

		if (e) {
			sp.w = 0x0100 | sp.b;
			dp.w = 0x0000;
		}
	}
};
#endif