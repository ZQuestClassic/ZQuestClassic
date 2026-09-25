// Checks the gamepad face button layouts against both ways SDL can number a
// pad's face buttons: by position (button 1 is south, as on an Xbox pad) or
// by printed label (button 1 is east, as on a Nintendo pad through HIDAPI).
//
// Run with `zplayer -test-zc <tests dir>`.

#include "test_runner/test_runner.h"
#include "zc/control_scheme.h"
#include <fmt/format.h>

static void check_face(TestResults& tr, const char* what, control_scheme const& scheme,
	int a, int b, int x, int y)
{
	tr.total++;
	if (scheme.btns[btnA] != a || scheme.btns[btnB] != b || scheme.btns[btnEx1] != x || scheme.btns[btnEx2] != y)
	{
		fmt::println("failed: {}: expected A={} B={} X={} Y={}, got A={} B={} X={} Y={}", what,
			a, b, x, y, scheme.btns[btnA], scheme.btns[btnB], scheme.btns[btnEx1], scheme.btns[btnEx2]);
		tr.failed++;
	}
}

TestResults test_control_scheme([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	const gamepad_face_buttons positional = {1, 2, 3, 4}; // south, east, west, north
	const gamepad_face_buttons by_label = {2, 1, 4, 3};

	control_scheme scheme;
	control_scheme untouched = scheme;

	// Nintendo layout: A east, B south, X north, Y west.
	set_gamepad_face_layout(scheme, positional, gamepad_face_layout::nintendo);
	check_face(tr, "nintendo layout, positional pad", scheme, 2, 1, 4, 3);
	set_gamepad_face_layout(scheme, by_label, gamepad_face_layout::nintendo);
	check_face(tr, "nintendo layout, label-numbered pad", scheme, 1, 2, 3, 4);

	// Xbox layout: A south, B east, X west, Y north.
	set_gamepad_face_layout(scheme, positional, gamepad_face_layout::xbox);
	check_face(tr, "xbox layout, positional pad", scheme, 1, 2, 3, 4);
	set_gamepad_face_layout(scheme, by_label, gamepad_face_layout::xbox);
	check_face(tr, "xbox layout, label-numbered pad", scheme, 2, 1, 4, 3);

	// Every other binding is left alone.
	tr.total++;
	for (int q = 0; q < NUM_SCHEME_KEYS; q++)
	{
		if (q == btnA || q == btnB || q == btnEx1 || q == btnEx2)
			continue;
		if (scheme.btns[q] != untouched.btns[q])
		{
			fmt::println("failed: layout changed button {} ({} -> {})", q, untouched.btns[q], scheme.btns[q]);
			tr.failed++;
			break;
		}
	}

	// Default sticks follow the gamepad model: move with the left thumb stick
	// (stick 0 is the dpad), and each stick reads Y from its second axis.
	const int default_sticks[control_scheme::num_sticks] = {ALLEGRO_GAMEPAD_STICK_LEFT_THUMB, ALLEGRO_GAMEPAD_STICK_RIGHT_THUMB};
	for (int stick = 0; stick < control_scheme::num_sticks; stick++)
	{
		auto const& data = untouched.stick_data[stick];
		tr.total++;
		if (data[control_scheme::axis_x][control_scheme::data_stick] != default_sticks[stick]
			|| data[control_scheme::axis_y][control_scheme::data_stick] != default_sticks[stick]
			|| data[control_scheme::axis_x][control_scheme::data_axis] != 0
			|| data[control_scheme::axis_y][control_scheme::data_axis] != 1)
		{
			fmt::println("failed: default stick {}: expected stick {} axes 0/1, got sticks {}/{} axes {}/{}", stick + 1,
				default_sticks[stick],
				data[control_scheme::axis_x][control_scheme::data_stick], data[control_scheme::axis_y][control_scheme::data_stick],
				data[control_scheme::axis_x][control_scheme::data_axis], data[control_scheme::axis_y][control_scheme::data_axis]);
			tr.failed++;
		}
	}

	return tr;
}
