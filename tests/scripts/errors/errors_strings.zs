#option NO_ERROR_HALT on

void strings() {
	char32 c1 = 'should have been double quotes'; // char too long
	char32 c2 = '\c'; // invalid escape
	char32 c3 = 'should have been double quotes';
	char32 c4 = 'o'; // ok
	char32 c5 = '\n'; // ok
}
