// wasm_structurer.h
//
// Reconstructs structured WebAssembly control flow (block / loop / if / br)
// from a reducible ZASM control-flow graph, replacing the loop-switch
// (br_table dispatch) trampoline.
//
// Algorithm: dominator-tree-guided recursive translation, following
// Norman Ramsey, "Beyond Relooper" (ICFP 2022). Handles any *reducible*
// CFG; irreducible CFGs are detected up front (run() returns false) so the
// caller can fall back to the loop-switch for that one function.
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
#include <cassert>
#include <algorithm>

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
		WasmStructurer(int num_blocks, int entry, const std::vector<BlockInfo>& blocks, StructSink& sink)
				: n(num_blocks), entry(entry), blocks(blocks), sink(sink) {}

		// Returns false if the CFG is irreducible (caller should fall back to the
		// loop-switch). On true, structural control flow has been emitted.
		bool run() {
				build_succs_preds();
				compute_rpo();
				if ((int)rpo.size() != reachable_count) {
						// Unreachable blocks exist; they are simply never emitted. Fine.
				}
				compute_idom();
				classify();
				if (!reducible) return false;
				build_merge_children();
				placed.assign(n, false);
				do_tree(entry);
				return true;
		}

private:
		int n, entry;
		const std::vector<BlockInfo>& blocks;
		StructSink& sink;

		std::vector<std::vector<int>> succ, pred;
		std::vector<int> rpo;            // blocks in reverse postorder
		std::vector<int> rpo_num;        // rpo_num[b] = index in rpo (entry = 0), or -1 if unreachable
		std::vector<int> idom;           // immediate dominator, idom[entry] = entry
		std::vector<uint8_t> is_backedge_target; // loop header
		std::vector<uint8_t> is_merge;   // >= 2 forward predecessors
		std::vector<std::vector<int>> merge_children; // dom children that are merge nodes, sorted
		std::vector<uint8_t> placed;
		int reachable_count = 0;
		bool reducible = true;

		// set of retreating edges found in DFS, as (from,to) pairs flattened
		std::vector<std::pair<int,int>> retreating;

		static std::vector<int> succs_of(const BlockInfo& b) {
				switch (b.term) {
						case Term::Exit:     return {};
						case Term::Dispatch: return {};
						case Term::Uncond:   return {b.succ_true};
						case Term::Cond:     return {b.succ_true, b.succ_false};
				}
				return {};
		}

		void build_succs_preds() {
				succ.assign(n, {});
				pred.assign(n, {});
				for (int b = 0; b < n; b++) {
						for (int s : succs_of(blocks[b])) {
								assert(s >= 0 && s < n);
								succ[b].push_back(s);
								pred[s].push_back(b);
						}
				}
		}

		// Iterative DFS producing postorder, then reverse it for RPO. Also records
		// retreating edges (target currently on the DFS stack = "gray").
		void compute_rpo() {
				rpo_num.assign(n, -1);
				std::vector<uint8_t> state(n, 0); // 0=white,1=gray,2=black
				std::vector<int> post;
				// explicit stack of (node, next-successor-index)
				std::vector<std::pair<int,int>> st;
				st.push_back({entry, 0});
				state[entry] = 1;
				while (!st.empty()) {
						auto& [u, i] = st.back();
						if (i < (int)succ[u].size()) {
								int v = succ[u][i++];
								if (state[v] == 0) { state[v] = 1; st.push_back({v, 0}); }
								else if (state[v] == 1) { retreating.push_back({u, v}); } // gray => retreating
						} else {
								state[u] = 2;
								post.push_back(u);
								st.pop_back();
						}
				}
				reachable_count = (int)post.size();
				rpo.assign(post.rbegin(), post.rend());
				for (int i = 0; i < (int)rpo.size(); i++) rpo_num[rpo[i]] = i;
		}

		// Cooper–Harvey–Kennedy "A Simple, Fast Dominance Algorithm".
		void compute_idom() {
				idom.assign(n, -1);
				idom[entry] = entry;
				bool changed = true;
				while (changed) {
						changed = false;
						for (int b : rpo) {
								if (b == entry) continue;
								int new_idom = -1;
								for (int p : pred[b]) {
										if (rpo_num[p] == -1) continue;   // unreachable pred
										if (idom[p] == -1) continue;      // not processed yet
										new_idom = (new_idom == -1) ? p : intersect(p, new_idom);
								}
								if (new_idom != -1 && idom[b] != new_idom) { idom[b] = new_idom; changed = true; }
						}
				}
		}

		int intersect(int a, int b) const {
				while (a != b) {
						while (rpo_num[a] > rpo_num[b]) a = idom[a];
						while (rpo_num[b] > rpo_num[a]) b = idom[b];
				}
				return a;
		}

		bool dominates(int a, int b) const {
				// a dominates b iff a is on b's idom chain.
				if (rpo_num[a] == -1 || rpo_num[b] == -1) return false;
				while (true) {
						if (b == a) return true;
						if (b == entry) return false;
						b = idom[b];
				}
		}

		void classify() {
				is_backedge_target.assign(n, 0);
				is_merge.assign(n, 0);

				// Reducibility: every retreating edge's head must dominate its tail.
				for (auto& [u, v] : retreating) {
						if (!dominates(v, u)) { reducible = false; return; }
						is_backedge_target[v] = 1; // v is a loop header
				}

				// Merge node = >= 2 forward (non-back) predecessors.
				for (int b = 0; b < n; b++) {
						if (rpo_num[b] == -1) continue;
						int fwd = 0;
						for (int p : pred[b]) {
								if (rpo_num[p] == -1) continue;
								if (is_backedge(p, b)) continue;
								fwd++;
						}
						if (fwd >= 2) is_merge[b] = 1;
				}
		}

		bool is_backedge(int from, int to) const {
				// In a reducible CFG this is exactly: `to` dominates `from`.
				return dominates(to, from);
		}

		void build_merge_children() {
				merge_children.assign(n, {});
				for (int b : rpo) {
						if (b == entry) continue;
						if (is_merge[b]) merge_children[idom[b]].push_back(b);
				}
				// Nest so the *largest* RPO merge child is the OUTERMOST block (its code
				// is laid out last); this keeps every forward branch reaching a scope
				// that is still open. Sort descending by rpo_num.
				for (auto& v : merge_children)
						std::sort(v.begin(), v.end(),
											[&](int x, int y){ return rpo_num[x] > rpo_num[y]; });
		}

		// ---- context stack ----
		struct Frame { enum Kind { LOOP, BLOCK, IF } kind; int node; };
		std::vector<Frame> ctx;

		int depth_to_block(int target) const {
				for (int i = (int)ctx.size() - 1, d = 0; i >= 0; --i, ++d)
						if (ctx[i].kind == Frame::BLOCK && ctx[i].node == target) return d;
				assert(false && "block target not in context"); return -1;
		}
		int depth_to_loop(int target) const {
				for (int i = (int)ctx.size() - 1, d = 0; i >= 0; --i, ++d)
						if (ctx[i].kind == Frame::LOOP && ctx[i].node == target) return d;
				assert(false && "loop target not in context"); return -1;
		}

		bool is_br_target(int from, int to) const {
				return is_merge[to] || is_backedge(from, to);
		}
		// Emit an unconditional edge transfer that is known to be a br target.
		void emit_edge_br(int from, int to) {
				if (is_backedge(from, to)) sink.emit_br(depth_to_loop(to), to);
				else                        sink.emit_br(depth_to_block(to), to);
		}
		void emit_edge_br_if(int from, int to) {
				if (is_backedge(from, to)) sink.emit_br_if(depth_to_loop(to), to);
				else                        sink.emit_br_if(depth_to_block(to), to);
		}

		// ---- recursive translation ----
		void do_tree(int node) {
				assert(!placed[node] && "node placed twice");
				placed[node] = true;

				if (is_backedge_target[node]) {
						sink.emit_loop(node);
						ctx.push_back({Frame::LOOP, node});
						node_within(node);
						ctx.pop_back();
						sink.emit_end();
				} else {
						node_within(node);
				}
		}

		void node_within(int node) {
				const auto& ms = merge_children[node]; // sorted desc by rpo
				for (int m : ms) { sink.emit_block(m); ctx.push_back({Frame::BLOCK, m}); }

				emit_block_and_terminator(node);

				for (auto it = ms.rbegin(); it != ms.rend(); ++it) {
						ctx.pop_back();
						sink.emit_end();
						do_tree(*it);
				}
		}

		void emit_block_and_terminator(int node) {
				sink.emit_body(node);
				const BlockInfo& bi = blocks[node];
				switch (bi.term) {
						case Term::Exit:
								// emit_body already emitted the trap/return.
								break;
						case Term::Dispatch:
								sink.emit_dispatch(node, (int)ctx.size());
								break;
						case Term::Uncond:
								do_branch(node, bi.succ_true);
								break;
						case Term::Cond: {
								int t = bi.succ_true, f = bi.succ_false;
								bool t_br = is_br_target(node, t);
								bool f_br = is_br_target(node, f);
								sink.emit_cond(node); // leaves i32; nonzero => take t
								if (t_br && !f_br) {
										emit_edge_br_if(node, t);   // branch if true
										do_branch(node, f);         // inline the fall-through
								} else if (!t_br && f_br) {
										sink.emit_i32_eqz();        // invert
										emit_edge_br_if(node, f);   // branch if false
										do_branch(node, t);         // inline the taken side
								} else if (t_br && f_br) {
										emit_edge_br_if(node, t);
										emit_edge_br(node, f);
								} else {
										// both sides inline: a genuine if/else diamond.
										sink.emit_if();
										ctx.push_back({Frame::IF, -1});
										do_branch(node, t);
										sink.emit_else();
										do_branch(node, f);
										ctx.pop_back();
										sink.emit_end();
								}
								break;
						}
				}
		}

		void do_branch(int from, int to) {
				if (is_br_target(from, to)) emit_edge_br(from, to);
				else                        do_tree(to); // dominated solely here => inline
		}
};
