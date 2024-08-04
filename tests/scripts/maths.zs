#include "std.zh"

ffc script Maths
{
    void run()
    {
        bool firstTime = true;
        while (true)
        {
            for (int i = -10; i <= 10; i++)
            {
                doMaths(firstTime, i);
                doMaths(firstTime, i * 10000);
                doMaths(firstTime, i * 100000);
            }

            Waitframe();
            firstTime = false;
        }
    }

    void maybePrint(bool doPrint, char32[] str, int value)
    {
        if (doPrint)
        {
            printf(str, value);
        }
    }

    void doMaths(bool firstTime, int i)
    {
        maybePrint(firstTime, "maths for: %d\n", i);
        maybePrint(firstTime, "10 / i = %d", 10 / i);
        maybePrint(firstTime, "10 %% i = %d", 10 % i);
        maybePrint(firstTime, "i / 10 = %d", i / 10);
        maybePrint(firstTime, "i %% 10 = %d", i % 10);

		maybePrint(firstTime, "i + i = %d\n", i + i);
        maybePrint(firstTime, "i - i = %d\n", i - i);
        maybePrint(firstTime, "i * i = %d\n", i * i);
        maybePrint(firstTime, "i * -i = %d\n", i * -i);
        maybePrint(firstTime, "i / i = %d\n", i / i);
        maybePrint(firstTime, "i / -i = %d\n", i / -i);
        maybePrint(firstTime, "i %% (i - 50) = %d\n", i % (i - 50));

        maybePrint(firstTime, "i + 100000 = %d\n", i + 100000);
        maybePrint(firstTime, "i - 100000 = %d\n", i - 100000);
        maybePrint(firstTime, "i * 100000 = %d\n", i * 100000);
        maybePrint(firstTime, "i * -100000 = %d\n", i * -100000);
        maybePrint(firstTime, "i / 100000 = %d\n", i / 100000);
        maybePrint(firstTime, "i / -100000 = %d\n", i / -100000);
        maybePrint(firstTime, "i %% 50 = %d\n", i % 50);
        maybePrint(firstTime, "i %% -50 = %d\n", i % -50);
    }
}
