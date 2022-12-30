global script GUITest
{
    void run()
    {
        GUI_Create();
        
        int win=Window_Create(GUI_ROOT); // Argument is parent
        Widget_SetSize(win, 90, 48); // ID, width, height (always in pixels)
        Widget_SetPos(win, 8, 8); // ID, X, Y (relative to parent)
        
        int btn1=Button_Create(win, 360, 0); // Parent, tile, CSet
        Widget_SetPos(btn1, 19, 8);
        
        int btn2=Button_Create(win, 380, 0);
        Widget_SetPos(btn2, 54, 8);
        
        int cbText[]="Window 2";
        int checkbox=Checkbox_Create(win, 420, 0, cbText, FONT_Z1, 1); // Parent, tile, CSet, text, font, text color
        Widget_SetPos(checkbox, 8, 32);
        
        int win2=Window_Create(GUI_ROOT);
        Widget_SetSize(win2, 17*8, 5*8);
        Widget_SetPos(win2, 112, 8);
        Window_SetBGColor(win2, 2);
        Widget_SetFlag(win2, WFLAG_HIDDEN);
        
        int labelText[]="Use mouse wheel";
        int label=Label_Create(win2, labelText, FONT_Z1); // Parent, text, font
        Widget_SetPos(label, 8, 8);
        
        int xText[]="X";
        int yText[]="Y";
        int rbGroup=RadioButtonGroup_Create();
        int rbX=RadioButton_Create(win2, rbGroup, 400, 0, xText, FONT_Z1, 1); // Parent, group, tile, CSet, font, text color
        Widget_SetPos(rbX, 8, 24);
        RadioButton_Select(rbX);
        int rbY=RadioButton_Create(win2, rbGroup, 400, 0, yText, FONT_Z1, 1);
        Widget_SetPos(rbY, 40, 24);
        
        int spinner=Spinner_Create(win2, -1, 241, Link->X); // Parent, min val, max val, init val
        Widget_SetPos(spinner, 72, 24);
        
        while(true)
        {
            if(Link->PressEx1)
            {
                GUI_Destroy();
                Quit();
            }
            
            GUI_Update();
            GUI_Draw();
            
            EventIter_Begin();
            while(EventIter_Next())
            {
                int type=EventIter_GetType();
                
                if(type==EVENT_BUTTON_CLICKED)
                {
                    if(EventIter_GetSender()==btn1)
                        Link->X-=16;
                    else // btn2
                        Link->X+=16;
                }
                else if(type==EVENT_CHECKBOX_CHANGED)
                {
                    // Only one possible sender
                    if(EventIter_GetData()==1) // Checked
                        Widget_UnsetFlag(win2, WFLAG_HIDDEN);
                    else // Unchecked
                        Widget_SetFlag(win2, WFLAG_HIDDEN);
                }
                else if(type==EVENT_SPINNER_CHANGED)
                {
                    // Only one possible sender
                    if(RadioButtonGroup_GetSelected(rbGroup)==rbX)
                        Link->X=EventIter_GetData();
                    else // rbY
                        Link->Y=EventIter_GetData();
                }
                else if(type==EVENT_RADIO_BUTTON_CHANGED)
                {
                    if(EventIter_GetSender()==rbX)
                    {
                        Spinner_SetRange(spinner, -1, 241);
                        Spinner_SetValue(spinner, Link->X);
                    }
                    else // rbY
                    {
                        Spinner_SetRange(spinner, -1, 161);
                        Spinner_SetValue(spinner, Link->Y);
                    }
                }
            }
            
            if(RadioButtonGroup_GetSelected(rbGroup)==rbX)
                Spinner_SetValue(spinner, Link->X);
            else
                Spinner_SetValue(spinner, Link->Y);
            Waitframe();
        }
    }
}

