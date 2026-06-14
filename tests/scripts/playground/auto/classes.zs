#include "auto/test_runner.zs"

class Car
{
	int speed;

	Car()
	{
		speed = 100;
		Trace(speed);
	}

	Car(int speed)
	{
		this->speed = speed;
		Trace(speed);
	}

	~Car()
	{
		Trace(-speed);
	}
}

generic script classes
{
	void run()
	{
		Test::Init();

		Car mycar = new Car();
		{
			#option WARN_DEPRECATED off
			delete mycar;
		}
		Waitframe();

		Car mycar2 = new Car(2000);
		Car mycar3 = new Car(5000);
		// Script exits, so last two instances dtors will be called.
	}
}