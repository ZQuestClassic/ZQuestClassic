// Minimal global script. You probably don't actually want to use this.


global script TangoActiveScript
{
    void run()
    {
        Tango_Start();
        
        while(true)
        {
            Tango_Update1();
            Waitdraw();
            Tango_Update2();
            Waitframe();
        }
    }
}
