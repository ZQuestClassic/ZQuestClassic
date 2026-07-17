void badAnnotation()
{
    @AlwaysRunEndpoint("invalid")
    loop(int x in [0,16), 4) {

    }
}

@Authors("asd"),
@Author(1),
@ExportInitD0(2),
@ExportInitD1("Foo", 3),
@ExportInitD2("Bar", "Error", 4),
@ExportInitD3("FooBar", 5, 6),
@ExportInitD4(7, 8, 9),
@ExportInitD5(),
@ExportInitD6(1, 2, 3, 4, 5, 6, 7),
@ExportInitD7("Error", "Test", "Some Random Value"),
@Author(),
@InitD0("Overlap"),
@ExportInitD0("Overlap"),
@ExportInitD8("Doesn't Exist")
ffc script BadAnnotatedScript
{
	void run(){}
}
