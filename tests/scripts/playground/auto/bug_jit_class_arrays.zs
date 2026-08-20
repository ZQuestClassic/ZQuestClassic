// Regression test for a JIT bug in the shared liveness analysis (zasm_utils.cpp).
//
// An instruction that writes one arg and reads the same D register via another
// arg (here READPODARRAYR D2 D2, from the ternary-indexed `pal[...]` read)
// applied the write's kill before the read's gen, so the read was never
// recorded. The register-cache dead-drop at the branch-merge block then
// believed D2 dead, skipped flushing the dirty cached value, and the compiled
// READPODARRAYR read a stale ri->d[2] from memory - producing a wrong array
// index. Whether the stale value happened to be correct depended on the last
// flushed value, so small loops passed while larger ones failed.
//
// Distilled from a NES emulator quest whose Screen->PutPixels output rendered
// with wrong colors under the JIT (both native backends share the liveness
// analysis and flush policy).

#include "auto/test_runner.zs"

class RPPU {
	int ctrl;
	int vram[0x4000];
	int chr[0x2000];
	int pal[32];
}

class RNES {
	RPPU ppu;

	RNES() {
		ppu = new RPPU();
	}

	int Draw() {
		for (int i = 0; i < 4; i++) ppu->vram[i] = (i ^ Floor(i / 32)) & 1;
		ppu->pal[0] = 0x0F;
		ppu->pal[1] = 0x16;
		ppu->pal[2] = 0x2A;

		int csum = 0;
		int ntBase = (ppu->ctrl & 0x03) * 0x0400;
		int bgTable = (ppu->ctrl & 0x10) ? 0x1000 : 0x0000;

		for (int j = 0; j < 30; j++) {
			for (int i = 0; i < 32; i++) {
				int tile = ppu->vram[ntBase + j * 32 + i];

				int attr = ppu->vram[ntBase + 0x3C0 + Floor(j / 4) * 8 + Floor(i / 4)];
				int quadShift = ((j & 2) << 1) | (i & 2);
				int subPal = (attr >> quadShift) & 0x03;

				for (int ty = 0; ty < 8; ty++) {
					int lo = ppu->chr[bgTable + tile * 16 + ty];
					int hi = ppu->chr[bgTable + tile * 16 + ty + 8];

					for (int tx = 0; tx < 8; tx++) {
						int px = ((lo >> (7 - tx)) & 1) | (((hi >> (7 - tx)) & 1) << 1);
						int color = 0xC0 + (ppu->pal[px == 0 ? 0 : subPal * 4 + px] & 0x3F);
						csum = (csum * 31 + color) % 4999;
					}
				}
			}
		}
		return csum;
	}
}

generic script bug_jit_class_arrays
{
	void run()
	{
		Test::Init();

		auto nes = new RNES();
		// Multiple frames: the stale-memory value the bug exposes depends on
		// what earlier executions left behind, so later frames can diverge
		// even when the first happens to pass.
		for (int f = 0; f < 3; f++) {
			int csum = nes->Draw();
			printf("csum=%d\n", csum);
			Test::AssertEqual(csum, 2742);
			Waitframe();
		}

		Test::End();
	}
}
