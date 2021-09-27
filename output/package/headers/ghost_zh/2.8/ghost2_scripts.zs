// ghost.zh
// Version 2.8.15

global script GhostZHActiveScript
{
    void run()
    {
        StartGhostZH();
        
        while(true)
        {
            UpdateGhostZH1();
            Waitdraw();
            UpdateGhostZH2();
            Waitframe();
        }
    }
}


item script GhostZHClockScript
{
    void run(int duration)
    {
        if(duration<=0)
            __ghzhData[__GHI_CLOCK_TIMER]=-1;
        else
            __ghzhData[__GHI_CLOCK_TIMER]=duration;
    }
}
