
#include "Defines.h"

class GlobalScript1
{
	int a;

	//called once when this object is created.
	GlobalScript1()
	{
		a = Rand(60, 60); //always 60.
		Print("Global1 standing by.\n");

		SetGlobalCallback(0, @OnInit);
		SetGlobalCallback(1, @OnRandomCallback);
	}

	//called every frame until they are defeated.
	void OnPigeonRaid()
	{
		//while(true){
		vec2 v1 = 88.0f; //let's see if those bastards can do 90! :P
		vec2 v2(0.6f, 8.46f);

		// simulate maths firing an object somewhere... or something
		v2.Normalize();
		float d = v1.Distance(v2);
		if(d != 0)
			v1 += v1 - v2 * 3.0f;
		else
			v1 -= v1 - v2 * 3.0f;

		d = Max(d, v1.x);
		d = Min(d, v1.y);

		//Print(a + "\n");
		Print("Please help us! We are completely defensless against them!!! " + a + "\n");
		Waitframes(a--);
		if(a == 0)
			throw("We are doomed...\n");

		//Waitframe();
		//}
	}

	//called once when this object is destroyed.
	void OnDestroy()
	{
		Print("Global1 signing off.\n");
	}
}


void OnInit() // todo: called once on init
{
	Print("OnInit() called.\n");
}


void OnRandomCallback()
{
	Print("OnRandomCallback() called.\n");
}
