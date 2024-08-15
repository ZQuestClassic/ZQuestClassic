#option NO_ERROR_HALT on

// No errors.
// https://discord.com/channels/876899628556091432/1267950596804051086
void DoWeirdScopeStuff() {
	if (1)
		Trace(1);

	for (int i=0; i<=1; i++) {
		return;
	}
	else return;
}
