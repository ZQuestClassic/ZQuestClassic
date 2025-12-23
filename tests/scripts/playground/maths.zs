// #include "std.zh"

class A {}
class B {
    untyped ptr;
    untyped ptrs[10];
}

// void traceme(A ptr)
// {
//     Trace(ptr);
// }

A traceme2(A ptr)
{
    Trace(ptr);
    return ptr;
}

void run2(A a)
{
    // untyped b = new A();
    // b = 1;

    B b2 = new B();
    b2->ptr = new B();
    b2->ptr = 1;
    b2->ptrs[0] = 2;
    b2->ptrs[b2->ptr] = new B();

    // B b3 = new B();
    // b3->ptrs = {1, 2, b3, new B(), 4};

    // if (b)
    // {
    //     untyped c1 = new A();
    //     c1 = 123;
    //     untyped c2 = new A();
    //     Trace(c1);
    // }
    // else
    // {
    //     auto c2 = new A();
    //     Trace(c2);
    // }

    // auto c3 = new A();
}

ffc script Maths
{
    void run()
    {
        run2(new A());

        // untyped c = 1;
        // c = new A();
        // traceme2(c);

        // auto d = new B();
        // d->ptr = new A();
        // bool firstTime = true;
        // while (true)
        // {
        //     for (int i = -10; i <= 10; i++)
        //     {
        //         doMaths(firstTime, i);
        //         doMaths(firstTime, i * 10000);
        //         doMaths(firstTime, i * 100000);
        //     }

        //     Waitframe();
        //     firstTime = false;
        // }
    }

    // void maybePrint(bool doPrint, char32[] str, int value)
    // {
    //     if (doPrint)
    //     {
    //         printf(str, value);
    //     }
    // }

    // void doMaths(bool firstTime, int i)
    // {
    //     maybePrint(firstTime, "maths for: %d\n", i);
    //     maybePrint(firstTime, "10 / i = %d", 10 / i);
    //     maybePrint(firstTime, "10 %% i = %d", 10 % i);
    //     maybePrint(firstTime, "i / 10 = %d", i / 10);
    //     maybePrint(firstTime, "i %% 10 = %d", i % 10);

	// 	maybePrint(firstTime, "i + i = %d\n", i + i);
    //     maybePrint(firstTime, "i - i = %d\n", i - i);
    //     maybePrint(firstTime, "i * i = %d\n", i * i);
    //     maybePrint(firstTime, "i * -i = %d\n", i * -i);
    //     maybePrint(firstTime, "i / i = %d\n", i / i);
    //     maybePrint(firstTime, "i / -i = %d\n", i / -i);
    //     maybePrint(firstTime, "i %% (i - 50) = %d\n", i % (i - 50));

    //     maybePrint(firstTime, "i + 100000 = %d\n", i + 100000);
    //     maybePrint(firstTime, "i - 100000 = %d\n", i - 100000);
    //     maybePrint(firstTime, "i * 100000 = %d\n", i * 100000);
    //     maybePrint(firstTime, "i * -100000 = %d\n", i * -100000);
    //     maybePrint(firstTime, "i / 100000 = %d\n", i / 100000);
    //     maybePrint(firstTime, "i / -100000 = %d\n", i / -100000);
    //     maybePrint(firstTime, "i %% 50 = %d\n", i % 50);
    //     maybePrint(firstTime, "i %% -50 = %d\n", i % -50);
    // }
}
