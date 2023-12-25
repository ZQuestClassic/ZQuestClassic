#include "zc/zasm_optimize.h"
#include "base/general.h"
#include "base/zdefs.h"
#include "parser/parserDefs.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zasm_utils.h"
#include <cstdint>

static void expected_to_str(ffscript* s, size_t len)
{
	for (int i = 0; i < len; i++)
	{
		printf("%s\n", script_debug_command_to_string(s[i].command, s[i].arg1, s[i].arg2).c_str());
	}
}

static void expect(std::string name, script_data* script, std::vector<ffscript> s)
{
	bool success = script->size == s.size();
	for (int i = 0; i < s.size(); i++)
	{
		if (!success) break;
		if (script->zasm[i].command == NOP && s[i].command == NOP)
			continue;
		if (script->zasm[i] != s[i])
			success = false;
	}

	if (!success)
	{
		printf("failure: %s\n", name.c_str());
		printf("\n== expected:\n\n");
		expected_to_str(&s[0], s.size());
		printf("\n== got:\n\n");
		expected_to_str(&script->zasm[0], script->size);
	}
}
struct OptContext
{
	uint32_t saved;
	script_data* script;
	ZasmFunction fn;
	ZasmCFG cfg;
	std::vector<pc_t> block_starts;
};

static void optimize_by_block(OptContext& ctx, std::function<void(pc_t, pc_t, pc_t)> cb)
{
	for (pc_t i = 0; i < ctx.block_starts.size(); i++)
	{
		pc_t start_pc = ctx.block_starts[i];
		pc_t final_pc = i == ctx.block_starts.size() - 1 ?
			ctx.fn.final_pc :
			ctx.block_starts[i + 1] - 1;
		cb(i, start_pc, final_pc);
	}
}

static void optimize_pushr(OptContext& ctx)
{
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (int j = start_pc; j <= final_pc; j++)
		{
			int command = ctx.script->zasm[j].command;
			int arg1 = ctx.script->zasm[j].arg1;
			if (command != PUSHR) continue;

			int start = j;
			int end = j + 1;
			while (end < final_pc)
			{
				int next_command = ctx.script->zasm[end].command;
				int next_arg1 = ctx.script->zasm[end].arg1;
				if (next_command != PUSHR || next_arg1 != arg1)
					break;

				end++;
			}

			int count = end - start;
			if (count > 1)
			{
				ctx.script->zasm[start].command = PUSHARGSR;
				ctx.script->zasm[start].arg2 = count;
				for (int i = start + 1; i < end; i++)
					ctx.script->zasm[i].command = NOP;
				ctx.saved += count - 1;
				j = end - 1;
			}
		}
	});
}

static void optimize_pop(OptContext& ctx)
{
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (int j = start_pc; j <= final_pc; j++)
		{
			int command = ctx.script->zasm[j].command;
			int arg1 = ctx.script->zasm[j].arg1;
			if (command != POP) continue;

			int start = j;
			int end = j + 1;
			while (end < final_pc)
			{
				int next_command = ctx.script->zasm[end].command;
				int next_arg1 = ctx.script->zasm[end].arg1;
				if (next_command != POP || next_arg1 != arg1)
					break;

				end++;
			}

			int count = end - start;
			if (count > 1)
			{
				ctx.script->zasm[start].command = POPARGS;
				ctx.script->zasm[start].arg2 = count;
				for (int i = start + 1; i < end; i++)
					ctx.script->zasm[i].command = NOP;
				ctx.saved += count - 1;
				j = end - 1;
			}
		}
	});
}

// SETV, PUSHR -> PUSHV
// Ex:
//   SETV            D2              5420000
//   PUSHR           D2
// ->
//   PUSHV           D2              5420000
static void optimize_setv_pushr(OptContext& ctx)
{
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (int j = start_pc; j < final_pc; j++)
		{
			if (ctx.script->zasm[j].command != SETV) continue;
			if (ctx.script->zasm[j + 1].command != PUSHR) continue;
			if (ctx.script->zasm[j].arg1 != ctx.script->zasm[j + 1].arg1) continue;

			// `zasm_construct_structured` is sensitive to a PUSH being just before
			// a function call, so unlike other places assign the NOP to the first
			// instruction.
			ctx.script->zasm[j + 1] = {PUSHV, ctx.script->zasm[j].arg2};
			ctx.script->zasm[j].command = NOP;
			ctx.saved += 1;
		}
	});
}

static void optimize_compare(OptContext& ctx)
{
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (int j = start_pc; j < final_pc; j++)
		{
			const auto& edges = ctx.cfg.block_edges.at(block_index);
			if (edges.size() != 2)
				continue;

			int command = ctx.script->zasm[j].command;
			if (command != COMPARER) continue;

			// Find pairs of SET, COMPARE
			bool bail = true;
			int k = j + 1;
			int cmp = 0;
			for (; k <= final_pc; k += 2)
			{
				bool end = false;
				int command = ctx.script->zasm[k].command;
				switch (command)
				{
					case SETFALSEI:
					case SETLESSI:
					case SETMOREI:
					case SETTRUEI:
					case SETCMP:
					case SETFALSE:
					case SETLESS:
					case SETMORE:
					case SETTRUE:
					{
						if (k + 1 > final_pc || ctx.script->zasm[k + 1].command != COMPAREV)
						{
							end = true;
							break;
						}

						// These should only ever appear once.
						#define ASSIGN_ONCE(v) {ASSERT(!cmp); cmp = v;}
						if (command == SETCMP) ASSIGN_ONCE(ctx.script->zasm[k].arg2 & CMP_FLAGS)
						if (command == SETLESS || command == SETLESSI) ASSIGN_ONCE(CMP_LE)
						if (command == SETMORE || command == SETMOREI) ASSIGN_ONCE(CMP_GE)
						if (command == SETFALSE || command == SETFALSEI) ASSIGN_ONCE(CMP_NE)

						// SETTRUE may be first after the COMPARER, in which case it matters.
						// Otherwise, it's a silly nop.
						if (command == SETTRUE || command == SETTRUEI) if (!cmp) cmp = CMP_EQ;

						if (ctx.script->zasm[k + 1].arg2 == 0)
							cmp = INVERT_CMP(cmp);
					}
					break;

					case GOTOTRUE:
					{
						bail = false;
						end = true;
					}
					break;

					default:
					{
						end = true;
					}
				}
				if (end) break;
			}

			if (bail)
				continue;

			// Useful binary search tool for finding problems.
			// static int c = 0;
			// c++;
			// if (!(1433 <= c && c < 1434))
			// {
			// 	continue;
			// }
			// 

			int start = j;
			int final = k;
			int count = k - j + 1;
			int goto_pc = ctx.script->zasm[final].arg1;

			pc_t block_1 = edges[0];
			pc_t block_2 = edges[1];
			pc_t block_2_start = ctx.block_starts.at(block_2);
			auto cmd1 = ctx.script->zasm[ctx.block_starts.at(block_1)];
			auto cmd2 = ctx.script->zasm[ctx.block_starts.at(block_2)];
			if (cmd2.command == COMPAREV)
			{
				// For now, skip this. Needs much work.
				if (ctx.script)
					break;

				// Affirm that this is a negating COMPAREV.
				if (cmd2.arg2 != 0)
					continue;

				// TODO: test case for when cmd2.arg2 == 1
				// stellar_seas_randomizer.qst/zasm-ffc-1-SolarElemental.txt
				// 176: COMPARER        D2              D3           
				// 177: SETTRUEI        D2                           
				// 178: COMPAREV        D2              0            
				// 179: GOTOTRUE        186                          
				// 180: SETR            D6              D4           [Block 6 -> 7]
				// 181: ADDV            D6              60000        
				// 182: LOADI           D2              D6           
				// 183: COMPAREV        D2              0            
				// 184: SETTRUEI        D2                           
				// 185: CASTBOOLF       D2                           
				// 186: COMPAREV        D2              1            [Block 7 -> 8, 9]
				// 187: SETMOREI        D2                           
				// 188: COMPAREV        D2              0            
				// 189: GOTOTRUE        195                     

				if (ctx.script->zasm[block_2_start + 3].command != GOTOTRUE)
					break;
				// Test case for above.
				/*
				3342: COMPARER        D3              D2           
				3343: SETCMP          D2              12           
				3344: COMPAREV        D2              0            
				3345: GOTOTRUE        3352                         
				3346: LOADD           D2              50000        [Block 105 -> 106]
				3347: PUSHR           D2                           
				3348: LOADD           D2              20000        
				3349: POP             D3                           
				3350: COMPARER        D3              D2           
				3351: SETCMP          D2              12           
				3352: COMPAREV        D2              0            [Block 106 -> 107, 108]
				3353: SETCMP          D2              11           
				3354: COMPAREV        D2              0            
				3355: GOTOFALSE       3368                         
				*/

				// Fix the first edge.

				// Find the last instruction of the first block.
				pc_t block_1_end = block_1 == ctx.block_starts.size() - 1 ?
					ctx.fn.final_pc :
					ctx.block_starts.at(block_1 + 1) - 1;
				auto& block_1_end_cmd = ctx.script->zasm[block_1_end];

				ASSERT(block_1_end_cmd.command == SETCMP);
				block_1_end_cmd.command = GOTOCMP;
				block_1_end_cmd.arg1 = ctx.block_starts.at(ctx.cfg.block_edges.at(block_2).at(1));
				block_1_end_cmd.arg2 = INVERT_CMP(block_1_end_cmd.arg2);

				// Fix the second edge.
				ASSERT(ctx.script->zasm[block_2_start + 1].command == SETCMP);
				ASSERT(ctx.script->zasm[block_2_start + 2].command == COMPAREV);
				ASSERT(ctx.script->zasm[block_2_start + 2].arg2 == 0);
				ASSERT(ctx.script->zasm[block_2_start + 3].command == GOTOTRUE);

				ctx.script->zasm[block_2_start] = {GOTOCMP, block_1_end_cmd.arg1, INVERT_CMP(ctx.script->zasm[block_2_start + 1].arg2)};

				for (int i = 0; i < 4; i++)
					ctx.script->zasm[block_2_start + i].command = NOP;
				ctx.saved += 4;

				// Lastly
				goto_pc = block_1_end_cmd.arg1;
			}

			ctx.script->zasm[start + 1] = ctx.script->zasm[final];
			ctx.script->zasm[start + 1].command = GOTOCMP;
			ctx.script->zasm[start + 1].arg1 = goto_pc;
			ctx.script->zasm[start + 1].arg2 = cmp;

			for (int i = start + 2; i <= final; i++)
				ctx.script->zasm[i].command = NOP;

			ctx.saved += count;
			j += count;
		}
	});
}

static void optimize_unreachable_blocks(OptContext& ctx)
{
	std::set<pc_t> seen_ids;
	std::set<pc_t> pending_ids = {0};
	while (pending_ids.size())
	{
		pc_t id = *pending_ids.begin();
		pending_ids.erase(pending_ids.begin());
		seen_ids.insert(id);

		for (auto calls_id : ctx.cfg.block_edges.at(id))
		{
			if (!seen_ids.contains(calls_id))
				pending_ids.insert(calls_id);
		}
	}
	for (int i = 0; i < ctx.cfg.block_starts.size(); i++)
	{
		if (!seen_ids.contains((i)))
		{
			int start = ctx.block_starts.at(i);
			int end = i == ctx.cfg.block_starts.size() - 1 ? ctx.fn.final_pc : ctx.block_starts.at(i + 1) - 1;
			for (int j = start; j <= end; j++)
			{
				ctx.script->zasm[j].command = NOP;
			}
			ctx.saved += end - start + 1;

			// Don't ever remove the end marker.
			if (end == ctx.script->size - 1)
			{
				ctx.script->zasm[end].command = 0xFFFF;
				ctx.saved -= 1;
			}
		}
	}
}

static int optimize_function(script_data* script, const ZasmFunction& fn)
{
	OptContext ctx;
	ctx.saved = 0;
	ctx.script = script;
	ctx.fn = fn;
	ctx.cfg = zasm_construct_cfg(script, {{fn.start_pc, fn.final_pc}});
	ctx.block_starts = std::vector<pc_t>(ctx.cfg.block_starts.begin(), ctx.cfg.block_starts.end());

	optimize_unreachable_blocks(ctx);
	optimize_pushr(ctx);
	optimize_pop(ctx);
	optimize_setv_pushr(ctx);

	// Ideas for more opt passes

	// Remove unused writes to a D register

	// Always do this last, as it slightly invalidates the CFG.
	optimize_compare(ctx);

	return ctx.saved;
}

int zasm_optimize(script_data* script)
{
	auto start_time = std::chrono::steady_clock::now();
	auto structured_zasm = zasm_construct_structured(script);

	int saved = 0;

	// Optimize unused functions.
	// TODO:
	// This works, but by construction there is never a function that is not called
	// (see comment in zasm_construct_structured). Besides, the zscript compiler
	// probably has always done a good job with pruning unused functions.
	// However, this could have an effect if any opt passes are made that remove dead
	// code, so keep the code nearby until then.
	// std::set<pc_t> seen_ids;
	// std::set<pc_t> pending_ids = {0};
	// while (pending_ids.size())
	// {
	// 	pc_t id = *pending_ids.begin();
	// 	pending_ids.erase(pending_ids.begin());
	// 	seen_ids.insert(id);

	// 	for (auto calls_id : structured_zasm.functions[id].calls_functions)
	// 	{
	// 		if (!seen_ids.contains(calls_id))
	// 			pending_ids.insert(calls_id);
	// 	}
	// }

	for (const auto& fn : structured_zasm.functions)
	{
		// if (!seen_ids.contains(fn.id))
		// {
		// 	for (int i = fn.start_pc; i <= fn.final_pc; i++)
		// 		script->zasm[i].command = NOP;
		// 	saved += fn.final_pc - fn.start_pc + 1;
		// 	continue;
		// }

		saved += optimize_function(script, fn);
	}

	// TODO: remove NOPs and update GOTOs.

	auto end_time = std::chrono::steady_clock::now();
	int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	int pct = 100.0 * saved / script->size;
	printf("optimize %zu instr, %zu fns, saved %d instr (%d%%), %dms\n", script->size, structured_zasm.functions.size(), saved, pct, ms);
	return saved;
}

void zasm_optimize()
{
	int saved = 0;
	int size = 0;
	auto start_time = std::chrono::steady_clock::now();

	zasm_for_every_script([&](auto script){
		size += script->size;
		saved += zasm_optimize(script);
	});

	if (size == 0)
		return;

	auto end_time = std::chrono::steady_clock::now();
	int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	int pct = 100.0 * saved / size;
	printf("\noptimized all scripts. saved %d instr (%d%%), %dms\n", saved, pct, ms);
}

bool zasm_optimize_test()
{
	script_data script(ScriptType::None, 0);
	std::string name;

	{
		name = "PUSHR -> PUSHARGSR (1)";
		ffscript s[] = {
			{PUSHR, D(3)},
			{PUSHR, D(3)},
			{PUSHR, D(3)},
			{PUSHR, D(3)},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{PUSHARGSR, D(3), 4},
			{NOP},
			{NOP},
			{NOP},
			{0xFFFF},
		});
	}

	{
		name = "PUSHR -> PUSHARGSR (2)";
		ffscript s[] = {
			{PUSHR, D(3)},
			{PUSHR, D(3)},
			{PUSHR, D(2)},
			{PUSHR, D(3)},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{PUSHARGSR, D(3), 2},
			{NOP},
			{PUSHR, D(2)},
			{PUSHR, D(3)},
			{0xFFFF},
		});
	}

	{
		name = "POP -> POPARGS";
		ffscript s[] = {
			{POP, D(3)},
			{POP, D(3)},
			{POP, D(3)},
			{POP, D(3)},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{POPARGS, D(3), 4},
			{NOP},
			{NOP},
			{NOP},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETCMP CMP_LE";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETCMP, D(2), CMP_LE},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 5},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 5, CMP_GT},
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETCMP CMP_LT | CMP_SETI";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETCMP, D(2), CMP_LT | CMP_SETI},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 5},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 5, CMP_GE}, // SETI is removed - not needed.
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETFALSE/COMPAREV";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETFALSE, D(2)},
			{COMPAREV, D(2), 0},
			{SETTRUE, D(2)},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 7},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 7, CMP_NE},
			{NOP},
			{NOP},
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETLESS/COMPAREV";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETLESS, D(2)},
			{COMPAREV, D(2), 0},
			{SETTRUE, D(2)},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 7},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 7, CMP_LE},
			{NOP},
			{NOP},
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETMORE/COMPAREV (1)";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETMORE, D(2)},
			{COMPAREV, D(2), 0},
			{SETTRUE, D(2)},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 7},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 7, CMP_GE},
			{NOP},
			{NOP},
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	{
		name = "COMPARER w/ SETMORE/COMPAREV (2)";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},
			{SETMORE, D(2)},
			{COMPAREV, D(2), 0},
			{SETTRUE, D(2)},
			{COMPAREV, D(2), 1},
			{GOTOTRUE, 7},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			{COMPARER, D(3), D(2)},
			{GOTOCMP, 7, CMP_LT},
			{NOP},
			{NOP},
			{NOP},
			{NOP},

			{TRACEV, 0},

			{QUIT},
			{0xFFFF},
		});
	}

	// This one is tricky. From bumper.zplay:
	//    if(Distance(CenterX(this), CenterY(this), CenterLinkX(), CenterLinkY())<this->TileWidth*8+2&&Link->Z==0){
	// (but w/o the function call)
	//
	// The ZASM does short circuiting logic. Each segment jumps to a common node upon failure, before it then jumps to
	// the block following the non-true edge of the conditional. Instead, I try to detect this scenario and rewrite the
	// each segment to jump directly to the non-true edge. This allowed for simpler usage of COMPARER/GOTOCMP, rather than
	// complicate sharing of D2 across blocks.
	{
		name = "COMPARER across blocks";
		ffscript s[] = {
			{COMPARER, D(3), D(2)},                 // 0: [Block 0 -> 1, 2]
			{SETCMP, D(2), CMP_LT | CMP_SETI},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 7},

			{SETR, D(2), LINKZ},                    // 4: [Block 1 -> 2]
			{COMPAREV, D(2), 0},
			{SETCMP, D(2), CMP_EQ | CMP_SETI},

			{COMPAREV, D(2), 0},                    // 7: [Block 2 -> 3, 4]
			{SETCMP, D(2), CMP_NE | CMP_SETI},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 12},

			{TRACEV, 0},                            // 11: [Block 3 -> 4]

			{QUIT},                                 // 12: [Block 4 ->  ]
			{0xFFFF},
		};
		script.zasm = s;
		script.recalc_size();
		zasm_optimize(&script);

		expect(name, &script, {
			// {COMPARER, D(3), D(2)},
			// {GOTOCMP, 12, CMP_GT | CMP_EQ | CMP_SETI},
			// {NOP},
			// {NOP},

			// {SETR, D(2), LINKZ},
			// {COMPAREV, D(2), 0},
			// {GOTOCMP, 12, CMP_NE | CMP_SETI},

			// {NOP},
			// {NOP},
			// {NOP},
			// {NOP},

			// {TRACEV, 0},

			// {QUIT},
			// {0xFFFF},

			// TODO: for now, just ensure this type of input wouldn't change.
			{COMPARER, D(3), D(2)},                 // 0: [Block 0 -> 1, 2]
			{SETCMP, D(2), CMP_LT | CMP_SETI},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 7},

			{SETR, D(2), LINKZ},                    // 4: [Block 1 -> 2]
			{COMPAREV, D(2), 0},
			{SETCMP, D(2), CMP_EQ | CMP_SETI},

			{COMPAREV, D(2), 0},                    // 7: [Block 2 -> 3, 4]
			{SETCMP, D(2), CMP_NE | CMP_SETI},
			{COMPAREV, D(2), 0},
			{GOTOTRUE, 12},

			{TRACEV, 0},                            // 11: [Block 3 -> 4]

			{QUIT},                                 // 12: [Block 4 ->  ]
			{0xFFFF},
		});
	}

	script.zasm = nullptr;
	return true;
}
