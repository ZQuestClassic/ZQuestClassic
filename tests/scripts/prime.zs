// Calculates and prints to the screen the factors of the given number.
// Calculated every frame, as the point of this is to be a performance test.

ffc script Prime
{
    void run(int num)
    {
        while (true)
        {
            for (int i = 2; i <= num; i++)
            {
                if (num % i != 0) continue;
                if (is_prime(i)) Trace(i);
            }


            Waitframe();
        }
    }

    bool is_prime(int x)
    {
        for (int i = 2; i <= x/2; i++)
        {
            if (x % i == 0)
            {
                return false;
            }
        }

        return true;
    }
}
