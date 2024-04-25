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

ffc script Classes
{
	void run()
	{
		Car mycar = new Car();
		delete mycar;
		Waitframe();

		Car mycar2 = new Car(2000);
		Car mycar3 = new Car(5000);
		// Script exits, so last two instances dtors will be called.
	}
}
