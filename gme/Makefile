CC=g++
LD=ld
AR=ar
ARFLAGS += libgme.a
RANLIB = ranlib
CXXFLAGS +=
OBJS= Blip_Buffer.o Classic_Emu.o Dual_Resampler.o Effects_Buffer.o Fir_Resampler.o Gb_Apu.o Gb_Cpu.o Gb_Oscs.o Gbs_Emu.o Gym_Emu.o Gzip_File.o Multi_Buffer.o Music_Emu.o Nes_Apu.o Nes_Cpu.o Nes_Fme7_Apu.o Nes_Namco_Apu.o Nes_Oscs.o Nes_Vrc6_Apu.o Nsf_Emu.o Sms_Apu.o Snes_Spc.o Spc_Cpu.o Spc_Dsp.o Spc_Emu.o Vgm_Emu.o Vgm_Emu_Impl.o Ym2413_Emu.o Ym2612_Emu.o abstract_file.o
 
HDRS = Blip_Buffer.h Classic_Emu.h Dual_Resampler.h Effects_Buffer.h Fir_Resampler.h Gb_Apu.h Gb_Cpu.h Gb_Oscs.h Gbs_Emu.h Gym_Emu.h Gzip_File.h Multi_Buffer.h Music_Emu.h Nes_Apu.h Nes_Cpu.h Nes_Fme7_Apu.h Nes_Namco_Apu.h Nes_Oscs.h Nes_Vrc6_Apu.h Nsf_Emu.h Sms_Apu.h Sms_Oscs.h Snes_Spc.h Spc_Cpu.h Spc_Dsp.h Spc_Emu.h Vgm_Emu.h Vgm_Emu_Impl.h Ym2413_Emu.h Ym2612_Emu.h abstract_file.h blargg_common.h blargg_endian.h blargg_source.h
 
all: gme
 
%.o: %.cpp
	$(CC) -c $(CXXFLAGS) $< -o $@
 
gme: $(OBJS)
	$(AR) $(ARFLAGS) $(OBJS)
	$(RANLIB) libgme.a
 
.PHONY: clean
clean:
	rm *.o
	rm *.a
