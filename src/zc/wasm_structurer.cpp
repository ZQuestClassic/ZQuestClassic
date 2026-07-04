// See wasm_structurer.h for what this is and an overview of the algorithm.

#include "zc/wasm_structurer.h"

#include <algorithm>
#include <cassert>

bool WasmStructurer::analyze() {
	build_succs_preds();
	compute_rpo();
	compute_idom();
	classify();
	analyzed = true;
	return reducible;
}

void WasmStructurer::emit(StructSink& out) {
	assert(analyzed && reducible);
	sink = &out;
	build_merge_children();
	placed.assign(n, false);
	do_tree(entry);
}

std::vector<int> WasmStructurer::succs_of(const BlockInfo& b) {
	switch (b.term) {
		case Term::Exit:     return {};
		case Term::Dispatch: return {};
		case Term::Uncond:   return {b.succ_true};
		case Term::Cond:     return {b.succ_true, b.succ_false};
	}
	return {};
}

void WasmStructurer::build_succs_preds() {
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
void WasmStructurer::compute_rpo() {
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
	rpo.assign(post.rbegin(), post.rend());
	for (int i = 0; i < (int)rpo.size(); i++) rpo_num[rpo[i]] = i;
}

// Cooper–Harvey–Kennedy "A Simple, Fast Dominance Algorithm".
void WasmStructurer::compute_idom() {
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

int WasmStructurer::intersect(int a, int b) const {
	while (a != b) {
		while (rpo_num[a] > rpo_num[b]) a = idom[a];
		while (rpo_num[b] > rpo_num[a]) b = idom[b];
	}
	return a;
}

bool WasmStructurer::dominates(int a, int b) const {
	// a dominates b iff a is on b's idom chain.
	if (rpo_num[a] == -1 || rpo_num[b] == -1) return false;
	while (true) {
		if (b == a) return true;
		if (b == entry) return false;
		b = idom[b];
	}
}

void WasmStructurer::classify() {
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

bool WasmStructurer::is_backedge(int from, int to) const {
	// In a reducible CFG this is exactly: `to` dominates `from`.
	return dominates(to, from);
}

void WasmStructurer::build_merge_children() {
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

int WasmStructurer::depth_to_block(int target) const {
	for (int i = (int)ctx.size() - 1, d = 0; i >= 0; --i, ++d)
		if (ctx[i].kind == Frame::BLOCK && ctx[i].node == target) return d;
	assert(false && "block target not in context"); return -1;
}

int WasmStructurer::depth_to_loop(int target) const {
	for (int i = (int)ctx.size() - 1, d = 0; i >= 0; --i, ++d)
		if (ctx[i].kind == Frame::LOOP && ctx[i].node == target) return d;
	assert(false && "loop target not in context"); return -1;
}

bool WasmStructurer::is_br_target(int from, int to) const {
	return is_merge[to] || is_backedge(from, to);
}

// Emit an unconditional edge transfer that is known to be a br target.
void WasmStructurer::emit_edge_br(int from, int to) {
	if (is_backedge(from, to)) sink->emit_br(depth_to_loop(to), to);
	else                        sink->emit_br(depth_to_block(to), to);
}

void WasmStructurer::emit_edge_br_if(int from, int to) {
	if (is_backedge(from, to)) sink->emit_br_if(depth_to_loop(to), to);
	else                        sink->emit_br_if(depth_to_block(to), to);
}

// ---- recursive translation ----

void WasmStructurer::do_tree(int node) {
	assert(!placed[node] && "node placed twice");
	placed[node] = true;

	if (is_backedge_target[node]) {
		sink->emit_loop(node);
		ctx.push_back({Frame::LOOP, node});
		node_within(node);
		ctx.pop_back();
		sink->emit_end();
	} else {
		node_within(node);
	}
}

void WasmStructurer::node_within(int node) {
	const auto& ms = merge_children[node]; // sorted desc by rpo
	for (int m : ms) { sink->emit_block(m); ctx.push_back({Frame::BLOCK, m}); }

	emit_block_and_terminator(node);

	for (auto it = ms.rbegin(); it != ms.rend(); ++it) {
		ctx.pop_back();
		sink->emit_end();
		do_tree(*it);
	}
}

void WasmStructurer::emit_block_and_terminator(int node) {
	sink->emit_body(node);
	const BlockInfo& bi = blocks[node];
	switch (bi.term) {
		case Term::Exit:
			// emit_body already emitted the trap/return.
			break;
		case Term::Dispatch:
			sink->emit_dispatch(node, (int)ctx.size());
			break;
		case Term::Uncond:
			do_branch(node, bi.succ_true);
			break;
		case Term::Cond: {
			int t = bi.succ_true, f = bi.succ_false;
			bool t_br = is_br_target(node, t);
			bool f_br = is_br_target(node, f);
			sink->emit_cond(node); // leaves i32; nonzero => take t
			if (t_br && !f_br) {
				emit_edge_br_if(node, t);   // branch if true
				do_branch(node, f);         // inline the fall-through
			} else if (!t_br && f_br) {
				sink->emit_i32_eqz();        // invert
				emit_edge_br_if(node, f);   // branch if false
				do_branch(node, t);         // inline the taken side
			} else if (t_br && f_br) {
				emit_edge_br_if(node, t);
				emit_edge_br(node, f);
			} else {
				// both sides inline: a genuine if/else diamond.
				sink->emit_if();
				ctx.push_back({Frame::IF, -1});
				do_branch(node, t);
				sink->emit_else();
				do_branch(node, f);
				ctx.pop_back();
				sink->emit_end();
			}
			break;
		}
	}
}

void WasmStructurer::do_branch(int from, int to) {
	if (is_br_target(from, to)) emit_edge_br(from, to);
	else                        do_tree(to); // dominated solely here => inline
}
