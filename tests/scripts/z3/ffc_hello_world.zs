import "std.zh"

ffc script HelloWorld
{
    void run(int num)
    {
        printf("FFC script started, ID: %d x: %d y: %d\n", this->ID, this->X, this->Y);
        printf("num: %d\n", num);

        int i = 0;
        while(true)
        {
            if (i++ == 30)
                printf("FFC script is running, ID: %d x: %d y: %d\n", this->ID, this->X, this->Y);
            Waitframe();
        }
    }
}
