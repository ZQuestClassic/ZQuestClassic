import "std.zh"

ffc script HelloWorld
{
    void run(int num)
    {
        printf("FFC script started, ID: %d\n", this->ID);
        printf("num: %d\n", num);

        int i = 0;
        while(true)
        {
            if (i++ == 30)
                printf("FFC script is running, ID: %d\n", this->ID);
            Waitframe();
        }
    }
}
