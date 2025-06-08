// ok
int optional_params_1(int a = 0) {
	return a;
}

// ok
int optional_params_2(int a = 0, int b = 1 + 1) {
	return a;
}

// ok
int optional_params_3(char32[] a = "") {
	return a;
}

// ok
int optional_params_4(int a = Player->X) {
	return a;
}

// ok
int optional_params_5(int a = optional_params_1()) {
	return a;
}

// not ok
int optional_params_6(int a = <int>(3/0)) {
	return a;
}

void fn() {
	optional_params_1();
	optional_params_1(1);
	optional_params_2();
	optional_params_2(1);
	optional_params_2(1, 2);
	optional_params_3();
	optional_params_4();
	optional_params_5();
	optional_params_6();
}
