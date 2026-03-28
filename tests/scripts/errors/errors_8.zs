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

// https://discord.com/channels/876899628556091432/1487233484924784682
@Author(1.11111) // expected error: Bad Annotation Value: @Author expects a String, not a Number
ffc script DecimalCrash
{
    void run()
    {
    }
}
