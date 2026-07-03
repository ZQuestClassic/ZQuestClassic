// Tests for wasm_structurer.h (the "Beyond Relooper" CFG structuring used by
// the wasm JIT for non-yielding functions).
//
// A mock emitter prints a WAT-like structure and, crucially, validates that
// every emitted `br` depth resolves to the frame it intended, using a live
// scope stack. This is an executable spec: add a case here for any CFG shape
// that breaks the real backend.

#include "zc/wasm_structurer.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <string>
#include <vector>

namespace {

struct MockSink : StructSink {
	struct F { int kind; int node; }; // 0 loop, 1 block, 2 if
	std::vector<F> frames;
	std::string out;
	int indent = 0;
	bool ok = true;
	bool verbose = false;

	void line(const std::string& s) {
		for (int i = 0; i < indent; i++) out += "  ";
		out += s; out += "\n";
	}
	void emit_block(int node) override { line("(block $b" + std::to_string(node)); indent++; frames.push_back({1,node}); }
	void emit_loop(int node)  override { line("(loop $l" + std::to_string(node));  indent++; frames.push_back({0,node}); }
	void emit_if()   override { line("(if"); indent++; frames.push_back({2,-1}); line("(then"); indent++; }
	void emit_else() override { indent--; line(") (else"); indent++; }
	void emit_end()  override {
		if (!frames.empty() && frames.back().kind == 2) { indent--; line("))"); }
		else { indent--; line(")"); }
		if (!frames.empty()) frames.pop_back();
	}
	void emit_br(int depth, int target) override {
		validate(depth, target);
		line("(br " + std::to_string(depth) + ")  ;; -> " + label(target, depth));
	}
	void emit_br_if(int depth, int target) override {
		validate(depth, target);
		line("(br_if " + std::to_string(depth) + ")  ;; -> " + label(target, depth));
	}
	void emit_i32_eqz() override { line("i32.eqz"); }
	void emit_body(int b) override { line(";; body B" + std::to_string(b)); }
	void emit_cond(int b) override { line(";; cond B" + std::to_string(b)); }
	void emit_dispatch(int b, int ctx_depth) override {
		// The reported structurer depth must equal the mock's live frame count -
		// this is what lets the real sink compute the total branch depth to an
		// enclosing dispatch loop.
		if (ctx_depth != (int)frames.size()) {
			ok = false;
			fmt::println("!! dispatch ctx_depth {} != live frames {}", ctx_depth, frames.size());
		}
		line(";; dispatch B" + std::to_string(b) + " (ctx_depth " + std::to_string(ctx_depth) + ")");
	}

	std::string label(int target, int depth) {
		int idx = (int)frames.size() - 1 - depth;
		if (idx < 0 || idx >= (int)frames.size()) return "OUT-OF-RANGE";
		const F& f = frames[idx];
		std::string k = f.kind==0?"loop":f.kind==1?"block":"if";
		return k + "$" + std::to_string(f.node);
	}
	void validate(int depth, int target) {
		int idx = (int)frames.size() - 1 - depth;
		if (idx < 0 || idx >= (int)frames.size()) {
			ok = false;
			fmt::println("!! br depth {} out of range (stack {})", depth, frames.size());
			return;
		}
		if (frames[idx].node != target) {
			ok = false;
			fmt::println("!! br depth {} resolves to node {}, expected {}", depth, frames[idx].node, target);
		}
	}
};

bool g_verbose;

// Runs the structurer over the CFG and validates every br depth. Returns
// whether the CFG was reducible (asserting depth-correctness if it was).
static bool structure(int nblocks, int entry, std::vector<BlockInfo> blocks)
{
	MockSink sink;
	WasmStructurer s(nblocks, entry, blocks, sink);
	bool reducible = s.run();
	if (g_verbose && reducible)
		fmt::print("{}", sink.out);
	if (reducible)
		assertTrue(sink.ok);
	return reducible;
}

// Case 1: diamond   0 -> (1,2) -> 3
static void test_diamond()
{
	assertTrue(structure(4, 0, {
		{Term::Cond, 1, 2},    // B0
		{Term::Uncond, 3, -1}, // B1
		{Term::Uncond, 3, -1}, // B2
		{Term::Exit, -1, -1},  // B3 (merge)
	}));
}

// Case 2: while loop
//   0 -> 1(header); 1 -> (2 body, 3 exit); 2 -> 1 (back-edge); 3 exit
static void test_while_loop()
{
	assertTrue(structure(4, 0, {
		{Term::Uncond, 1, -1}, // B0 -> header
		{Term::Cond, 2, 3},    // B1 header: body or exit
		{Term::Uncond, 1, -1}, // B2 body -> back to header
		{Term::Exit, -1, -1},  // B3 exit
	}));
}

// Case 3: loop with an early break (if inside loop)
//   0->1; 1 header -> (2, 5exit); 2 -> (3 break->5, 4 continue); 3->5 ; 4->1
static void test_loop_with_break()
{
	assertTrue(structure(6, 0, {
		{Term::Uncond, 1, -1}, // B0
		{Term::Cond, 2, 5},    // B1 header
		{Term::Cond, 3, 4},    // B2: break? / continue?
		{Term::Uncond, 5, -1}, // B3 break -> exit (merge target 5)
		{Term::Uncond, 1, -1}, // B4 continue -> header (back-edge)
		{Term::Exit, -1, -1},  // B5 exit (merge)
	}));
}

// Case 4: nested loops
//   0->1; 1 outer header ->(2,6exit); 2 inner header ->(3 body,5 after-inner);
//   3->4; 4->2 (inner back-edge); 5->1 (outer back-edge); 6 exit
static void test_nested_loops()
{
	assertTrue(structure(7, 0, {
		{Term::Uncond, 1, -1}, // B0
		{Term::Cond, 2, 6},    // B1 outer header
		{Term::Cond, 3, 5},    // B2 inner header
		{Term::Uncond, 4, -1}, // B3 inner body
		{Term::Uncond, 2, -1}, // B4 -> inner header (inner back-edge)
		{Term::Uncond, 1, -1}, // B5 -> outer header (outer back-edge)
		{Term::Exit, -1, -1},  // B6 exit
	}));
}

// Case 5: irreducible (two entries into a loop) -> must report irreducible so
// the backend falls back to the loop-switch.
//   0 ->(1,2); 1->2; 2->1  (1 and 2 form a loop entered from both)
static void test_irreducible()
{
	assertTrue(!structure(3, 0, {
		{Term::Cond, 1, 2},    // B0
		{Term::Uncond, 2, -1}, // B1 -> 2
		{Term::Uncond, 1, -1}, // B2 -> 1
	}));
}

// Case 6: loop header that is also a merge node (two forward entries plus a
// back-edge): gets both an outer block (for the join) and an inner loop.
//   0 ->(1,2); 1->3; 2->3; 3 header ->(4 body, 5 exit); 4->3 back-edge; 5 exit
static void test_merge_loop_header()
{
	assertTrue(structure(6, 0, {
		{Term::Cond, 1, 2},    // B0
		{Term::Uncond, 3, -1}, // B1
		{Term::Uncond, 3, -1}, // B2
		{Term::Cond, 4, 5},    // B3 header + merge
		{Term::Uncond, 3, -1}, // B4 body -> back-edge
		{Term::Exit, -1, -1},  // B5 exit
	}));
}

// Case 7: unreachable block (dead code after an exit) is simply never emitted.
static void test_unreachable_block()
{
	assertTrue(structure(3, 0, {
		{Term::Uncond, 2, -1}, // B0 -> B2
		{Term::Exit, -1, -1},  // B1 unreachable
		{Term::Exit, -1, -1},  // B2 exit
	}));
}

// Case 8: a loop whose break leaves the region through the dispatch (the
// yielder's 80/20 cut: a structured loop nested inside the loop-switch, whose
// exit edge targets another dispatch region). The dispatch's ctx_depth is
// validated against the live frame stack inside the loop.
//   0 -> 1 header; 1 -> (2 body, 3 dispatch-out); 2 -> 1 back-edge
static void test_loop_with_dispatch_break()
{
	assertTrue(structure(4, 0, {
		{Term::Uncond, 1, -1},     // B0
		{Term::Cond, 2, 3},        // B1 header: body or leave
		{Term::Uncond, 1, -1},     // B2 body -> back-edge
		{Term::Dispatch, -1, -1},  // B3 leaves the region via the dispatch
	}));
}

// Case 9: two branches dispatching to a shared trampoline (the merge-node
// shape a Cond-with-out-of-region-taken-edge produces when both sides funnel
// into one synthesized Dispatch block).
//   0 -> (1, 2); 1 -> 3; 2 -> 3; 3 = shared dispatch trampoline
static void test_shared_dispatch_trampoline()
{
	assertTrue(structure(4, 0, {
		{Term::Cond, 1, 2},        // B0
		{Term::Uncond, 3, -1},     // B1
		{Term::Uncond, 3, -1},     // B2
		{Term::Dispatch, -1, -1},  // B3 (merge) -> dispatch
	}));
}

// Case 10: a suspension point inside a loop, as the 80/20 yielder cut lowers
// it when the loop is NOT eligible for structuring: the region gets cut at the
// suspension, leaving a straight-line region that ends in a Dispatch (the
// resume point is a separate dispatch region, so the back-edge never appears
// here). This documents that a suspending loop stays dispatch-driven.
//   0 -> 1 -> Dispatch (the wait); loop closure lives in the dispatch, not here.
static void test_suspending_loop_region()
{
	assertTrue(structure(2, 0, {
		{Term::Uncond, 1, -1},     // B0 (loop header code)
		{Term::Dispatch, -1, -1},  // B1 body up to the wait -> dispatch
	}));
}

} // namespace

TestResults test_wasm_structurer(bool verbose)
{
	g_verbose = verbose;
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "diamond", test_diamond },
		{ "while_loop", test_while_loop },
		{ "loop_with_break", test_loop_with_break },
		{ "nested_loops", test_nested_loops },
		{ "irreducible", test_irreducible },
		{ "merge_loop_header", test_merge_loop_header },
		{ "unreachable_block", test_unreachable_block },
		{ "loop_with_dispatch_break", test_loop_with_dispatch_break },
		{ "shared_dispatch_trampoline", test_shared_dispatch_trampoline },
		{ "suspending_loop_region", test_suspending_loop_region },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}
