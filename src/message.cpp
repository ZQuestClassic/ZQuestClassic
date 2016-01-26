#include "message.h"

#include "link.h"
#include "messageManager.h"
#include "pal.h"
#include "sound.h"
#include "zc_sys.h"
#include "zdefs.h"
#include "zelda.h"
#include <allegro.h>


Message::Message(const MsgStr* msgstr, MessageManager& mm):
    manager(mm),
    stream(msgstr->s),
    msgX(msgstr->x),
    msgY(msgstr->y),
    textSFX(msgstr->sfx),
    textSpeed(zinit.msg_speed),
    wordWrappingEnabled((msgstr->stringflags&STRINGFLAG_WRAP)!=0),
    timer(textSpeed)
{
    renderer.initialize(msgstr);
}

Message::Message(int dmap, MessageManager& mm):
    manager(mm),
    stream(DMaps[dmap].intro),
    msgX(24),
    msgY(32),
    textSFX(18),
    textSpeed(zinit.msg_speed), // Is that right? Maybe it's fixed...
    wordWrappingEnabled(false),
    timer(textSpeed)
{
    renderer.initialize(0);
}

void Message::update(updateSpeed speed)
{
    if(stream.atEnd()) // Might happen if given an empty message
        return;
    
    if(speed==spd_normal && timer>0)
    {
        timer--;
        if(timer>0)
            return;
    }
    
    processNext(speed);
    timer=textSpeed;
}

void Message::processNext(updateSpeed speed)
{
    do
    {
        if(stream.nextIsWhitespace())
        {
            do
            {
                if(stream.getCharacter()==' ')
                    renderer.putChar(' ');
                else // Line break; stream already advanced
                    renderer.startNewLine();
            } while(stream.nextIsWhitespace());
            
            if(wordWrappingEnabled && !renderer.atLineStart())
                checkWordWrapping();
        }
        
        char c=stream.getCharacter();
        if(c>' ')
        {
            renderer.putChar(c);
            if(speed!=spd_instant)
                sfx(textSFX);
        }
        else
            executeControlCode(c-1);
    } while(speed==spd_instant && !stream.atEnd());
}

void Message::checkWordWrapping()
{
    unsigned int streamPos=stream.tell();
    int wordWidth=0;
    int wordLength=0;
    
    Font font=renderer.getFont();
    
    // Find the width of the word, not counting spacing
    while(!(stream.nextIsWhitespace() || stream.atEnd()))
    {
        char c=stream.getCharacter();
        
        if(c>' ')
        {
            wordWidth+=font.getWidth(c);
            wordLength++;
        }
        else
        {
            // Control code; skip past it
            for(int i=getNumControlCodeArgs(c-1); i>0; i--)
                stream.getControlCodeArgument();
        }
    }
    
    if(!renderer.wordFits(wordWidth, wordLength))
        renderer.startNewLine();
    
    stream.seek(streamPos);
}

void Message::executeControlCode(char code)
{
    int arg1, arg2, arg3;
    int numArgs=getNumControlCodeArgs(code);
    
    if(numArgs>=1)
        arg1=stream.getControlCodeArgument();
    if(numArgs>=2)
        arg2=stream.getControlCodeArgument();
    if(numArgs==3)
        arg3=stream.getControlCodeArgument();
    
    switch(code)
    {
        // Message property change codes
    case MSGC_SPEED:
        textSpeed=arg1;
        break;
        
    case MSGC_COLOUR:
        renderer.setColor(CSET(arg1)+arg2);
        break;
        
        // Message switch codes
    case MSGC_GOTOIFRAND:
        if(arg1<=1 || rand()%arg1==0)
            manager.switchTo(arg2);
        break;
        
    case MSGC_GOTOIFGLOBAL: // Screen->D[], not global. Should be renamed.
        {
            arg1=vbound(arg1, 0, 7);
            int scr=(get_currdmap()<<7)+get_currscr();
            if(DMaps[get_currdmap()].type!=dmOVERW)
                scr-=DMaps[get_currdmap()].xoff;
            
            if(game->screen_d[scr][arg1]>=arg2)
                manager.switchTo(arg3);
            break;
        }
        
    case MSGC_GOTOIF: // If item. Rename this one, too.
        if(arg1>=0 && arg1<MAXITEMS && game->item[arg1])
            manager.switchTo(arg2);
        break;
        
    case MSGC_GOTOIFCTR:
        if(game->get_counter(arg1)>=arg2)
            manager.switchTo(arg3);
        break;
        
    case MSGC_GOTOIFCTRPC:
        {
            int amount=(arg2*game->get_maxcounter(arg1))/100;
            if(game->get_counter(arg1)>=amount)
                manager.switchTo(arg3);
            break;
        }
        
    case MSGC_GOTOIFTRICOUNT:
        if(TriforceCount()>=arg1)
            manager.switchTo(arg2);
        break;
        
    case MSGC_GOTOIFTRI:
        if(arg1>=0 && arg1<MAXLEVELS && (game->lvlitems[arg1]&liTRIFORCE)!=0)
            manager.switchTo(arg2);
        break;
        
        // Item and counter codes
    case MSGC_CTRUP:
        game->change_counter(arg2, arg1);
        break;
    
    case MSGC_CTRDN:
        game->change_counter(-arg2, arg1);
        break;
    
    case MSGC_CTRSET:
        game->set_counter(vbound(arg2, 0, game->get_maxcounter(arg1)), arg1);
        break;
    
    case MSGC_CTRUPPC:
        {
            int amount=(arg2*game->get_maxcounter(arg1))/100;
            game->change_counter(arg2, amount);
            break;
        }
        
    case MSGC_CTRDNPC:
        {
            int amount=(arg2*game->get_maxcounter(arg1))/100;
            game->change_counter(-arg2, amount);
            break;
        }
        
    case MSGC_CTRSETPC:
        {
            int amount=vbound((arg2*game->get_maxcounter(arg1))/100,
              0, game->get_maxcounter(arg1));
            game->set_counter(amount, arg1);
            break;
        }
        
    case MSGC_GIVEITEM:
        getitem(arg1);
        break;
        
    case MSGC_TAKEITEM:
        takeitem(arg1);
        break;
        
        // Other codes
    case MSGC_MIDI:
        if(arg1==0)
            music_stop();
        else
            jukebox(arg1+(ZC_MIDI_COUNT-1));
        break;
        
    case MSGC_SFX:
        sfx(arg1);
        break;
    }
}

int Message::getNumControlCodeArgs(char code) const
{
    // MSGC_NEWLINE isn't handled here
    switch(code)
    {
    case MSGC_GOTOIFGLOBAL:
    case MSGC_GOTOIFCTRPC:
    case MSGC_GOTOIFCTR:
        return 3;
        
    case MSGC_COLOUR:
    case MSGC_CTRUP:
    case MSGC_CTRDN:
    case MSGC_CTRSET:
    case MSGC_CTRUPPC:
    case MSGC_CTRDNPC:
    case MSGC_CTRSETPC:
    case MSGC_GOTOIF:
    case MSGC_GOTOIFTRI:
    case MSGC_GOTOIFTRICOUNT:
    case MSGC_GOTOIFRAND:
        return 2;
        
    case MSGC_SFX:
    case MSGC_MIDI:
    case MSGC_SPEED:
    case MSGC_GIVEITEM:
    case MSGC_TAKEITEM:
        return 1;
        
    default: // Shouldn't happen
        return 0;
    }
}

void Message::reset(const MsgStr* nextMsg)
{
    stream.reset(nextMsg->s);
    msgX=nextMsg->x;
    msgY=nextMsg->y;
    textSFX=nextMsg->sfx;
    wordWrappingEnabled=(nextMsg->stringflags&STRINGFLAG_WRAP)!=0;
    
    renderer.initialize(nextMsg);
}

void Message::continueTo(const MsgStr* nextMsg)
{
    stream.reset(nextMsg->s);
    textSFX=nextMsg->sfx;
    renderer.loadContinuationStyle(nextMsg);
}

void Message::draw(BITMAP* target, int yOffset)
{
    renderer.draw(target, msgX, msgY+yOffset);
}
