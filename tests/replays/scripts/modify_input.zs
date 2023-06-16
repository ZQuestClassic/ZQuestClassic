ffc script ModifyInput
{
    void run()
    {
        while (true)
        {
            if (Link->PressA)
            {
                Link->InputA = false;
            }

            Waitframe();
        }
    }
}
