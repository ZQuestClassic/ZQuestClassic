
enum foo
{
	A, B, C
};

void badAnnotation()
{
	@Increment(foo)
	enum bar
	{
		D, E, F
	};
	@AlwaysRunEndpoint("invalid")
	loop(int x in [0,16), 4) {

	}
}

