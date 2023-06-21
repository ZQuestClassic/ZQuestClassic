import "std.zh"

global script Global
{
    void run()
    {
        int r = 20;
        int angle = 0;
        int da = 5;

        while (true)
        {
            Viewport->Mode = VIEW_MODE_SCRIPT;
            Viewport->X = Link->X - Viewport->Width / 2 + r * Cos(angle);
            Viewport->Y = Link->Y - Viewport->Height / 2 + r * Sin(angle);

            angle += da;
            if (angle < -360) angle += 360;
            else if (angle > 360) angle -= 360;

            Waitframe();
        }
    }
}
