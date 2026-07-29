ffc script CircularMotion
{
    #option DEFAULT_STATIC_SCRIPT_MEMBERS off

    @Export("Radius", "Horizontal radius of the motion, in pixels."),
    @ExportRange(0, 256)
    int radius = 10;

    @Export("Speed", "Degrees advanced per frame. Negative spins the other way."),
    @ExportRange(-90, 90)
    int speed = 3;

    @Export("Start Angle", "Starting angle, in degrees. Negative picks a random angle."),
    @ExportRange(-360, 360)
    int angle = -1;

    @Export("Radius 2", "Vertical radius, for ellipses. 0 matches Radius."),
    @ExportRange(0, 256)
    int radius2;

    @Export("Rotation", "Rotates the ellipse by this many degrees."),
    @ExportRange(-360, 360)
    int angle2;

    void run()
    {
        if(radius2 == 0) radius2 = radius; //Circle
        if(angle < 0) angle = Rand(360); //Random Start
        int cx = this->X;
        int cy = this->Y;
        while(true)
        {
            angle += speed;
            if(angle < -360)angle+=360; //Wrap if below -360.
            else if(angle > 360)angle-=360; //Wrap if above 360.
            if(angle2==0)
            {
                this->X = cx + radius*Cos(angle);
                this->Y = cy + radius2*Sin(angle);
            }
            else //Rotate at center.
            {
                this->X = cx + radius*Cos(angle)*Cos(angle2) - radius2*Sin(angle)*Sin(angle2);
                this->Y = cy + radius2*Sin(angle)*Cos(angle2) + radius*Cos(angle)*Sin(angle2);
            }
            Waitframe();
        }
    }
}