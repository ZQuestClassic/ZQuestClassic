ffc script ControlFlow
{
	void run(int num)
	{
		while (true)
		{
			fn_1(num);
			fn_2(num);
			fn_3(num);
			fn_4(num);
			fn_5(num);
			fn_6(num);
			fn_7(num);
			fn_8(num);
			Waitframe();
		}
	}

	void fn_1(int a)
	{
		if (a == 10)
		{
			Trace(1);
		}
		Trace(0);
	}

	void fn_2(int a)
	{
		if (a == 10)
		{
			Trace(1);
		}
		else
		{
			Trace(2);
		}
		Trace(0);
	}

	void fn_3(int a)
	{
		if (a >= 10)
		{
			Trace(1);
		}
		else if (a < 10)
		{
			Trace(2);
		}
		Trace(0);
	}

	void fn_4(int a)
	{
		if (a >= 10)
		{
			Trace(1);
		}
		else if (a < 10)
		{
			Trace(2);
		}
		else
		{
			Trace(3);
		}
		Trace(0);
	}

	void fn_5(int a)
	{
		if (a >= 10)
		{
			Trace(1);
		}
		else if (a < 10)
		{
			Trace(2);
		}
		else if (a == 15)
		{
			Trace(3);
		}
		else
		{
			Trace(4);
		}
		Trace(0);
	}

	void fn_6(int a)
	{
		switch (a)
		{
			case 10:
				Trace(1);
				break;
			case 15:
				Trace(2);
				break;
			case 25:
				Trace(3);
				break;
			case 30:
				Trace(4);
				break;
		}
		Trace(0);
	}

	void fn_7(int a)
	{
		if (a > 10 && a < 20 && a != 14 && a != 16)
		{
			Trace(1);
		}
		else if (a > 15 && a <= 30)
		{
			Trace(2);
		}
		Trace(0);
	}

	void fn_8(int a)
	{
		if (a > 10 && a < 20 && a != 14 && a != 16)
		{
			if (a != 11 && a != 12 && a != 13) Trace(1);
		}
		else if (a > 15 && a <= 30)
		{
			Trace(2);
		}
		Trace(0);
	}
}
