#include "ffscript.h"
#include "zelda.h"
#include "link.h"
#include <string.h>

extern LinkClass Link;

void do_set(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	long temp;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]=temp; break;
		case D1:
			tmpscr->d1[i]=temp; break;
		case D2:
			tmpscr->d2[i]=temp; break;
		case D3:
			tmpscr->d3[i]=temp; break;
		case D4:
			tmpscr->d4[i]=temp; break;
		case D5:
			tmpscr->d5[i]=temp; break;
		case D6:
			tmpscr->d6[i]=temp; break;
		case D7:
			tmpscr->d7[i]=temp; break;
		case A1:
			tmpscr->a1[i]=temp; break;
		case A2:
			tmpscr->a2[i]=temp; break;
		case DATA:
			tmpscr->ffdata[i]=temp/10000; break;
		case FCSET:
			tmpscr->ffcset[i]=temp/10000; break;
		case DELAY:
			tmpscr->ffdelay[i]=temp/10000; break;
		case FX:
			tmpscr->ffx[i]=temp; break;
		case FY:
			tmpscr->ffy[i]=temp; break;
		case XD:
			tmpscr->ffxdelta[i]=temp; break;
		case YD:
			tmpscr->ffydelta[i]=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]=temp; break;
		case LX:
			Link.setX(temp/10000); break;
		case LY:
			Link.setY(temp/10000); break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]=(temp/10000);
				if(arg1==COMBOC(k)) tmpscr->cset[k]=(temp/10000);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]=(temp/10000);
			}
			break;
		}
}

void do_add(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	long temp;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]+=temp; break;
		case D1:
			tmpscr->d1[i]+=temp; break;
		case D2:
			tmpscr->d2[i]+=temp; break;
		case D3:
			tmpscr->d3[i]+=temp; break;
		case D4:
			tmpscr->d4[i]+=temp; break;
		case D5:
			tmpscr->d5[i]+=temp; break;
		case D6:
			tmpscr->d6[i]+=temp; break;
		case D7:
			tmpscr->d7[i]+=temp; break;
		case A1:
			tmpscr->a1[i]+=temp; break;
		case A2:
			tmpscr->a2[i]+=temp; break;
		case DATA:
			tmpscr->ffdata[i]+=(temp/10000); break;
		case FCSET:
			tmpscr->ffcset[i]+=(temp/10000); break;
		case DELAY:
			tmpscr->ffdelay[i]+=(temp/10000); break;
		case FX:
			tmpscr->ffx[i]+=temp; break;
		case FY:
			tmpscr->ffy[i]+=temp; break;
		case XD:
			tmpscr->ffxdelta[i]+=temp; break;
		case YD:
			tmpscr->ffydelta[i]+=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]+=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]+=temp; break;
		case LX:
			Link.setX((int)Link.getX()+(temp/10000)); break;
		case LY:
			Link.setY((int)Link.getY()+(temp/10000)); break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]+=(temp/10000);
				if(arg1==COMBOC(k)) tmpscr->cset[k]+=(temp/10000);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]+=(temp/10000);
			}
			break;
		}
}

void do_sub(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	long temp;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]-=temp; break;
		case D1:
			tmpscr->d1[i]-=temp; break;
		case D2:
			tmpscr->d2[i]-=temp; break;
		case D3:
			tmpscr->d3[i]-=temp; break;
		case D4:
			tmpscr->d4[i]-=temp; break;
		case D5:
			tmpscr->d5[i]-=temp; break;
		case D6:
			tmpscr->d6[i]-=temp; break;
		case D7:
			tmpscr->d7[i]-=temp; break;
		case A1:
			tmpscr->a1[i]-=temp; break;
		case A2:
			tmpscr->a2[i]-=temp; break;
		case DATA:
			tmpscr->ffdata[i]-=(temp/10000); break;
		case FCSET:
			tmpscr->ffcset[i]-=(temp/10000); break;
		case DELAY:
			tmpscr->ffdelay[i]-=(temp/10000); break;
		case FX:
			tmpscr->ffx[i]-=temp; break;
		case FY:
			tmpscr->ffy[i]-=temp; break;
		case XD:
			tmpscr->ffxdelta[i]-=temp; break;
		case YD:
			tmpscr->ffydelta[i]-=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]-=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]-=temp; break;
		case LX:
			Link.setX((int)Link.getX()-(temp/10000)); break;
		case LY:
			Link.setY((int)Link.getY()-(temp/10000)); break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]-=(temp/10000);
				if(arg1==COMBOC(k)) tmpscr->cset[k]-=(temp/10000);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]-=(temp/10000);
			}
			break;
		}
}

void do_mult(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	double temp;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	temp=temp/10000.0;
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]*=temp; break;
		case D1:
			tmpscr->d1[i]*=temp; break;
		case D2:
			tmpscr->d2[i]*=temp; break;
		case D3:
			tmpscr->d3[i]*=temp; break;
		case D4:
			tmpscr->d4[i]*=temp; break;
		case D5:
			tmpscr->d5[i]*=temp; break;
		case D6:
			tmpscr->d6[i]*=temp; break;
		case D7:
			tmpscr->d7[i]*=temp; break;
		case A1:
			tmpscr->a1[i]*=temp; break;
		case A2:
			tmpscr->a2[i]*=temp; break;
		case DATA:
			tmpscr->ffdata[i]*=(temp); break;
		case FCSET:
			tmpscr->ffcset[i]*=(temp); break;
		case DELAY:
			tmpscr->ffdelay[i]*=(temp); break;
		case FX:
			tmpscr->ffx[i]*=temp; break;
		case FY:
			tmpscr->ffy[i]*=temp; break;
		case XD:
			tmpscr->ffxdelta[i]*=temp; break;
		case YD:
			tmpscr->ffydelta[i]*=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]*=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]*=temp; break;
		case LX:
			Link.setX((int)Link.getX()*(temp)); break;
		case LY:
			Link.setY((int)Link.getY()*(temp)); break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]*=(temp);
				if(arg1==COMBOC(k)) tmpscr->cset[k]*=(temp);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]*=(temp);
			}
			break;
		}
}

void do_div(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	double temp;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	temp = temp/10000.0;
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]/=temp; break;
		case D1:
			tmpscr->d1[i]/=temp; break;
		case D2:
			tmpscr->d2[i]/=temp; break;
		case D3:
			tmpscr->d3[i]/=temp; break;
		case D4:
			tmpscr->d4[i]/=temp; break;
		case D5:
			tmpscr->d5[i]/=temp; break;
		case D6:
			tmpscr->d6[i]/=temp; break;
		case D7:
			tmpscr->d7[i]/=temp; break;
		case A1:
			tmpscr->a1[i]/=temp; break;
		case A2:
			tmpscr->a2[i]/=temp; break;
		case DATA:
			tmpscr->ffdata[i]/=(temp); break;
		case FCSET:
			tmpscr->ffcset[i]/=(temp); break;
		case DELAY:
			tmpscr->ffdelay[i]/=(temp); break;
		case FX:
			tmpscr->ffx[i]/=temp; break;
		case FY:
			tmpscr->ffy[i]/=temp; break;
		case XD:
			tmpscr->ffxdelta[i]/=temp; break;
		case YD:
			tmpscr->ffydelta[i]/=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]/=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]/=temp; break;
		case LX:
			Link.setX((int)Link.getX()/(temp)); break;
		case LY:
			Link.setY((int)Link.getY()/(temp)); break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]/=(temp);
				if(arg1==COMBOC(k)) tmpscr->cset[k]/=(temp);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]/=(temp);
			}
			break;
		}
}

void do_comp(int script, word *pc, int i, bool v)
{
	long arg1;
	long arg2;
	long temp;
	long temp2;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(v) {temp = arg2;}
	else
	{
		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		case LX:
			temp=(int)(Link.getX())*10000; break;
		case LY:
			temp=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	}
	switch(arg1)
		{
		case D0:
			temp2=tmpscr->d0[i]; break;
		case D1:
			temp2=tmpscr->d1[i]; break;
		case D2:
			temp2=tmpscr->d2[i]; break;
		case D3:
			temp2=tmpscr->d3[i]; break;
		case D4:
			temp2=tmpscr->d4[i]; break;
		case D5:
			temp2=tmpscr->d5[i]; break;
		case D6:
			temp2=tmpscr->d6[i]; break;
		case D7:
			temp2=tmpscr->d7[i]; break;
		case A1:
			temp2=tmpscr->a1[i]; break;
		case A2:
			temp2=tmpscr->a2[i]; break;
		case DATA:
			temp2=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp2=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp2=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp2=tmpscr->ffx[i]; break;
		case FY:
			temp2=tmpscr->ffy[i]; break;
		case XD:
			temp2=tmpscr->ffxdelta[i]; break;
		case YD:
			temp2=tmpscr->ffydelta[i]; break;
		case XD2:
			temp2=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp2=tmpscr->ffydelta2[i]; break;
		case LX:
			temp2=(int)(Link.getX())*10000; break;
		case LY:
			temp2=(int)(Link.getY())*10000; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) temp2=tmpscr->data[k]*10000;
				if(arg1==COMBOC(k)) temp2=tmpscr->cset[k]*10000;
				if(arg1==COMBOF(k)) temp2=tmpscr->sflag[k]*10000;
			}
			break;
		}

	if(temp2 >= temp) tmpscr->scriptflag[i] |= MOREFLAG; else tmpscr->scriptflag[i] &= ~MOREFLAG;
	if(temp2 == temp) tmpscr->scriptflag[i] |= TRUEFLAG; else tmpscr->scriptflag[i] &= ~TRUEFLAG;
}

void do_loada(int script, word *pc, int i, int a)
{
	long arg1;
	long arg2;
	long temp;
	int j;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(a) j = (tmpscr->a2[i]/10000)-1;
	else j = (tmpscr->a1[i]/10000)-1;

		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[j]; break;
		case D1:
			temp=tmpscr->d1[j]; break;
		case D2:
			temp=tmpscr->d2[j]; break;
		case D3:
			temp=tmpscr->d3[j]; break;
		case D4:
			temp=tmpscr->d4[j]; break;
		case D5:
			temp=tmpscr->d5[j]; break;
		case D6:
			temp=tmpscr->d6[j]; break;
		case D7:
			temp=tmpscr->d7[j]; break;
		case A1:
			temp=tmpscr->a1[j]; break;
		case A2:
			temp=tmpscr->a2[j]; break;
		case DATA:
			temp=tmpscr->ffdata[j]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[j]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[j]*10000; break;
		case FX:
			temp=tmpscr->ffx[j]; break;
		case FY:
			temp=tmpscr->ffy[j]; break;
		case XD:
			temp=tmpscr->ffxdelta[j]; break;
		case YD:
			temp=tmpscr->ffydelta[j]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[j]; break;
		case YD2:
			temp=tmpscr->ffydelta2[j]; break;
		}
	switch(arg1)
		{
		case D0:
			tmpscr->d0[i]=temp; break;
		case D1:
			tmpscr->d1[i]=temp; break;
		case D2:
			tmpscr->d2[i]=temp; break;
		case D3:
			tmpscr->d3[i]=temp; break;
		case D4:
			tmpscr->d4[i]=temp; break;
		case D5:
			tmpscr->d5[i]=temp; break;
		case D6:
			tmpscr->d6[i]=temp; break;
		case D7:
			tmpscr->d7[i]=temp; break;
		case A1:
			tmpscr->a1[i]=temp; break;
		case A2:
			tmpscr->a2[i]=temp; break;
		case DATA:
			tmpscr->ffdata[i]=temp/10000; break;
		case FCSET:
			tmpscr->ffcset[i]=temp/10000; break;
		case DELAY:
			tmpscr->ffdelay[i]=temp/10000; break;
		case FX:
			tmpscr->ffx[i]=temp; break;
		case FY:
			tmpscr->ffy[i]=temp; break;
		case XD:
			tmpscr->ffxdelta[i]=temp; break;
		case YD:
			tmpscr->ffydelta[i]=temp; break;
		case XD2:
			tmpscr->ffxdelta2[i]=temp; break;
		case YD2:
			tmpscr->ffydelta2[i]=temp; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg1==COMBOD(k)) tmpscr->data[k]=(temp/10000);
				if(arg1==COMBOC(k)) tmpscr->cset[k]=(temp/10000);
				if(arg1==COMBOF(k)) tmpscr->sflag[k]=(temp/10000);
			}
			break;
		}
}

void do_seta(int script, word *pc, int i, int a)
{
	long arg1;
	long arg2;
	long temp;
	int j;

	arg1 = ffscripts[script][*pc].arg1;
	arg2 = ffscripts[script][*pc].arg2;

	if(a) j = (tmpscr->a2[i]/10000)-1;
	else j = (tmpscr->a1[i]/10000)-1;

		switch(arg2)
		{
		case D0:
			temp=tmpscr->d0[i]; break;
		case D1:
			temp=tmpscr->d1[i]; break;
		case D2:
			temp=tmpscr->d2[i]; break;
		case D3:
			temp=tmpscr->d3[i]; break;
		case D4:
			temp=tmpscr->d4[i]; break;
		case D5:
			temp=tmpscr->d5[i]; break;
		case D6:
			temp=tmpscr->d6[i]; break;
		case D7:
			temp=tmpscr->d7[i]; break;
		case A1:
			temp=tmpscr->a1[i]; break;
		case A2:
			temp=tmpscr->a2[i]; break;
		case DATA:
			temp=tmpscr->ffdata[i]*10000; break;
		case FCSET:
			temp=tmpscr->ffcset[i]*10000; break;
		case DELAY:
			temp=tmpscr->ffdelay[i]*10000; break;
		case FX:
			temp=tmpscr->ffx[i]; break;
		case FY:
			temp=tmpscr->ffy[i]; break;
		case XD:
			temp=tmpscr->ffxdelta[i]; break;
		case YD:
			temp=tmpscr->ffydelta[i]; break;
		case XD2:
			temp=tmpscr->ffxdelta2[i]; break;
		case YD2:
			temp=tmpscr->ffydelta2[i]; break;
		default:
			for(int k=0;k<176;k++)
			{
				if(arg2==COMBOD(k)) temp=tmpscr->data[k]*10000;
				if(arg2==COMBOC(k)) temp=tmpscr->cset[k]*10000;
				if(arg2==COMBOF(k)) temp=tmpscr->sflag[k]*10000;
			}
			break;
		}
	switch(arg1)
		{
		case D0:
			tmpscr->d0[j]=temp; break;
		case D1:
			tmpscr->d1[j]=temp; break;
		case D2:
			tmpscr->d2[j]=temp; break;
		case D3:
			tmpscr->d3[j]=temp; break;
		case D4:
			tmpscr->d4[j]=temp; break;
		case D5:
			tmpscr->d5[j]=temp; break;
		case D6:
			tmpscr->d6[j]=temp; break;
		case D7:
			tmpscr->d7[j]=temp; break;
		case A1:
			tmpscr->a1[j]=temp; break;
		case A2:
			tmpscr->a2[j]=temp; break;
		case DATA:
			tmpscr->ffdata[j]=temp/10000; break;
		case FCSET:
			tmpscr->ffcset[j]=temp/10000; break;
		case DELAY:
			tmpscr->ffdelay[j]=temp/10000; break;
		case FX:
			tmpscr->ffx[j]=temp; break;
		case FY:
			tmpscr->ffy[j]=temp; break;
		case XD:
			tmpscr->ffxdelta[j]=temp; break;
		case YD:
			tmpscr->ffydelta[j]=temp; break;
		case XD2:
			tmpscr->ffxdelta2[j]=temp; break;
		case YD2:
			tmpscr->ffydelta2[j]=temp; break;
		}
}

// Let's do this
int run_script(int script, int i)
{
	word command;
	long arg1=0;
	long arg2=0;
  arg1=arg2; //to avoid unused variables warnings
	word *pc = &(tmpscr->pc[i]);

	bool increment = true;

	while((ffscripts[script][*pc].command!=0xFFFF)&&(ffscripts[script][*pc].command!=WAITFRAME))
	{
		command = ffscripts[script][*pc].command;
		switch(command)
		{
		case SETV:
			do_set(script, pc, i, true); break;
		case SETR:
			do_set(script, pc, i, false); break;
		case ADDV:
			do_add(script, pc, i, true); break;
		case ADDR:
			do_add(script, pc, i, false); break;
		case SUBV:
			do_sub(script, pc, i, true); break;
		case SUBR:
			do_sub(script, pc, i, false); break;
		case MULTV:
			do_mult(script, pc, i, true); break;
		case MULTR:
			do_mult(script, pc, i, false); break;
		case DIVV:
			do_div(script, pc, i, true); break;
		case DIVR:
			do_div(script, pc, i, false); break;
		case GOTO:
			*pc = ffscripts[script][*pc].arg1; increment = false; break;
		case CHECKTRIG:
			break;
		case WARP:
			tmpscr->sidewarpdmap[0] = ffscripts[script][*pc].arg1;
			tmpscr->sidewarpscr[0] = ffscripts[script][*pc].arg2;
			Link.ffwarp = true; break;
		case COMPAREV:
			do_comp(script, pc, i, true); break;
		case COMPARER:
			do_comp(script, pc, i, false); break;
		case GOTOTRUE:
			if(tmpscr->scriptflag[i] & TRUEFLAG)
			{*pc = ffscripts[script][*pc].arg1; increment = false;} break;
		case GOTOFALSE:
			if(!(tmpscr->scriptflag[i] & TRUEFLAG))
			{*pc = ffscripts[script][*pc].arg1; increment = false;} break;
		case GOTOMORE:
			if(tmpscr->scriptflag[i] & MOREFLAG)
			{*pc = ffscripts[script][*pc].arg1; increment = false;} break;
		case GOTOLESS:
			if(!(tmpscr->scriptflag[i] & MOREFLAG))
			{*pc = ffscripts[script][*pc].arg1; increment = false;} break;
		case LOAD1:
			do_loada(script, pc, i, 0); break;
		case LOAD2:
			do_loada(script, pc, i, 1); break;
		case SETA1:
			do_seta(script, pc, i, 0); break;
		case SETA2:
			do_seta(script, pc, i, 1); break;
		case QUIT:
			tmpscr->ffscript[i] = 0;
		}
		if(increment) *pc+=1;
		increment = true;
	}
	if(ffscripts[script][*pc].command==0xFFFF)
		tmpscr->ffscript[i] = 0;
	else *pc+=1;
	return 0;
}

int ffscript_engine()
{
	for(int i=0;i<32;i++)
	{
		if(tmpscr->ffscript[i])
		{
			run_script(tmpscr->ffscript[i], i);
		}
	}
	return 0;
}
