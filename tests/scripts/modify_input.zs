ffc script ModifyInput
{
    void run()
    {
        while (true)
        {
            if (Link->PressA)
            {
                // With qr_BROKEN_INPUT_DOWN_STATE on, this has no effect on the sword button press. keys_modify.zplay tests this.
                // When off, it completely prevents a sword swing. playground_keys_modify.zplay tests this.
                Link->InputA = false;
            }

            Waitframe();
        }
    }
}
