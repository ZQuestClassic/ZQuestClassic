// ok
int optional_params_1(int a = 0) {
	return a;
}

// ok
int optional_params_2(int a = 0, int b = 1 + 1) {
	return a;
}

// not ok, strings currently aren't considered compile time values (but probably should be)
int optional_params_3(char32[] a = "") {
	return a;
}

// this should be not ok, b/c it is not a compile time value
// But there is no error right now...
// `DataType const* getType = (*it)->getReadType(scope, this);` in `RegistrationVisitor::caseFuncDecl` is somehow returning NULL
// for this
// compiled function doesn't work at all.
int optional_params_4(int a = Player->X) {
	return a;
}

// not ok, not a compile time value
int optional_params_5(int a = optional_params_1()) {
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
}
