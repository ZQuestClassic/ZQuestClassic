// ZASM has goto; WebAssembly doesn't. Wasm only has structured control flow
// - nested block/loop/if constructs - and `br N`, which jumps to the Nth
// enclosing construct: a br to a `block` jumps forward to its end (break), a
// br to a `loop` jumps back to its start (continue). This file rebuilds a
// ZASM control-flow graph as nested constructs so every original jump lands
// on a construct it is nested inside, replacing the loop-switch (br_table
// dispatch) trampoline.
//
// Algorithm: dominator-tree-guided recursive translation, following Norman
// Ramsey, "Beyond Relooper" (ICFP 2022). Code is laid out along the
// dominator tree, and two kinds of blocks earn a wrapper construct:
//
//   - a loop header (back-edge target) is wrapped in `loop`, so jumps back
//     to it are brs to that loop;
//   - a merge node (>= 2 forward predecessors) gets a `block` opened inside
//     its immediate dominator, so every path that joins there brs forward to
//     the block's end, where the merge node's own code is emitted.
//
// Everything else is reached from exactly one place and is simply inlined.
//
// Handles any *reducible* CFG - every loop entered only through its header -
// which is what guarantees those two wrappers always suffice. Irreducible
// CFGs (e.g. a jump into the middle of a loop) are detected by analyze()
// (returns false) so the caller can fall back to the loop-switch for that
// one function. Analysis and emission are separate steps: callers can probe
// reducibility without a sink, and emit later from the same analysis (see
// detect_yielder_regions).
//
// You bind two things:
//   1. StructSink  -- structural WASM emission (block/loop/if/br/...). In the
//      real backend these call wasm.emitBlock(), wasm.emitLoop(), etc. The
//      node/target int args are only for debug validation; ignore them.
//   2. emit_body(b) / emit_cond(b) -- your existing per-instruction lowering:
//        emit_body(b): emit the straight-line WASM for block b's instructions,
//                      EXCLUDING its terminating control-flow op. For an Exit
//                      block, emit_body must itself emit the trap/return
//                      (QUIT / RETURNFUNC / script-end); the structurer emits
//                      nothing after it.
//        emit_cond(b): for a Conditional block, emit the operands + compare so
//                      that a single i32 is left on the stack (nonzero => take
//                      the `succ_true` edge).
//
// Blocks are integers [0, num_blocks). You describe each block's terminator
// with BlockInfo. Successors are block ids.

#pragma once
#include <vector>
#include <cstdint>
#include <utility>

enum class Term {
		Exit,
		Uncond,
		Cond,
		// The block transfers to a target only reachable through an enclosing
		// dispatch (loop-switch) that the structured region is nested inside - a
		// suspension/return/cross-region edge. The structurer emits the body, then
		// calls emit_dispatch(node, ctx_depth); the sink sets the dispatch target
		// id and branches out of the region (ctx_depth = how many frames the
		// structurer currently has open, so the sink can compute the total branch
		// depth to the dispatch loop). Like Exit, it has no in-region successors.
		Dispatch,
};

struct BlockInfo {
		Term term = Term::Exit;
		int succ_true  = -1; // Uncond: the sole successor. Cond: taken (cond != 0) target.
		int succ_false = -1; // Cond: fall-through (cond == 0) target.
};

// Structural WASM emitter. Real impl forwards to WasmAssembler; the int params
// beyond `depth` exist only so a mock can validate that a br resolves to the
// frame it intended. Real impl ignores `node`/`target`.
struct StructSink {
		virtual ~StructSink() = default;
		virtual void emit_block(int node) = 0;   // wasm.emitBlock()
		virtual void emit_loop(int node)  = 0;    // wasm.emitLoop()
		virtual void emit_if()            = 0;    // wasm.emitIf()
		virtual void emit_else()          = 0;    // wasm.emitElse()
		virtual void emit_end()           = 0;    // wasm.emitEnd()
		virtual void emit_br(int depth, int target)     = 0; // wasm.emitBr(depth)
		virtual void emit_br_if(int depth, int target)  = 0; // wasm.emitBrIf(depth)
		virtual void emit_i32_eqz()       = 0;    // wasm.emitI32Eqz()
		virtual void emit_body(int block) = 0;    // your straight-line lowering
		virtual void emit_cond(int block) = 0;    // your compare -> i32 on stack
		// Term::Dispatch only: set the dispatch target and branch out of the
		// region. ctx_depth = frames the structurer has open at this point.
		virtual void emit_dispatch(int block, int ctx_depth) = 0;
};

class WasmStructurer {
public:
		// Copies `blocks` so the structurer can be kept (e.g. cached in a plan)
		// independently of the caller's vector.
		WasmStructurer(int num_blocks, int entry, std::vector<BlockInfo> blocks)
				: n(num_blocks), entry(entry), blocks(std::move(blocks)) {}

		// Returns false if the CFG is irreducible (caller should fall back to
		// the loop-switch). Emits nothing. Unreachable blocks are fine; they
		// are simply never emitted.
		bool analyze();

		// Emits structural control flow through `out`. Requires a successful
		// analyze(); call at most once.
		void emit(StructSink& out);

private:
		int n, entry;
		std::vector<BlockInfo> blocks;
		StructSink* sink = nullptr;

		std::vector<std::vector<int>> succ, pred;
		std::vector<int> rpo;            // blocks in reverse postorder
		std::vector<int> rpo_num;        // rpo_num[b] = index in rpo (entry = 0), or -1 if unreachable
		std::vector<int> idom;           // immediate dominator, idom[entry] = entry
		std::vector<uint8_t> is_backedge_target; // loop header
		std::vector<uint8_t> is_merge;   // >= 2 forward predecessors
		std::vector<std::vector<int>> merge_children; // dom children that are merge nodes, sorted
		std::vector<uint8_t> placed;
		bool reducible = true;
		bool analyzed = false;

		// set of retreating edges found in DFS, as (from,to) pairs flattened
		std::vector<std::pair<int,int>> retreating;

		// Context stack of currently-open wasm frames, for resolving br depths.
		struct Frame { enum Kind { LOOP, BLOCK, IF } kind; int node; };
		std::vector<Frame> ctx;

		static std::vector<int> succs_of(const BlockInfo& b);
		void build_succs_preds();
		void compute_rpo();
		void compute_idom();
		int intersect(int a, int b) const;
		bool dominates(int a, int b) const;
		void classify();
		bool is_backedge(int from, int to) const;
		void build_merge_children();
		int depth_to_block(int target) const;
		int depth_to_loop(int target) const;
		bool is_br_target(int from, int to) const;
		void emit_edge_br(int from, int to);
		void emit_edge_br_if(int from, int to);
		void do_tree(int node);
		void node_within(int node);
		void emit_block_and_terminator(int node);
		void do_branch(int from, int to);
};
