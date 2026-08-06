
@ExportInitD0("Speed", "In pixels/frame"),
@ExportInitD1("Delay", "In frames"),
@ExportInitD2("Secrets?", "Trigger secrets or not?", "B"),
@ExportInitD3("Screen", "Other screen to interact with", "H")
ffc script foo
{
	void run(int a, int b, int c, int d)
	{}
}

const int FOO = 10;
@Increment(FOO)
enum
{
	A, B, C, D
};
CONST_ASSERT(A == 0 && B == 10 && C == 20 && D == 30);
