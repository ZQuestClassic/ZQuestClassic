#include "fontClass.h"
#include "zelda.h"

Font Font::getByID(int id)
{
    switch(id)
    {
    case font_z3font:
        return Font(z3font);
        
    case font_z3smallfont:
        return Font(z3smallfont);
        
    case font_deffont:
        return Font(deffont);
        
    case font_lfont:
        return Font(lfont);
        
    case font_lfont_l:
        return Font(lfont_l);
        
    case font_pfont:
        return Font(pfont);
        
    case font_mfont:
        return Font(mfont);
        
    case font_ztfont:
        return Font(ztfont);
        
    case font_sfont:
        return Font(sfont);
        
    case font_sfont2:
        return Font(sfont2);
        
    case font_spfont:
        return Font(spfont);
        
    case font_ssfont1:
        return Font(ssfont1);
        
    case font_ssfont2:
        return Font(ssfont2);
        
    case font_ssfont3:
        return Font(ssfont3);
        
    case font_ssfont4:
        return Font(ssfont4);
        
    case font_gblafont:
        return Font(gblafont);
        
    case font_goronfont:
        return Font(goronfont);
        
    case font_zoranfont:
        return Font(zoranfont);
        
    case font_hylian1font:
        return Font(hylian1font);
        
    case font_hylian2font:
        return Font(hylian2font);
        
    case font_hylian3font:
        return Font(hylian3font);
        
    case font_hylian4font:
        return Font(hylian4font);
        
    case font_gboraclefont:
        return Font(gboraclefont);
        
    case font_gboraclepfont:
        return Font(gboraclepfont);
        
    case font_dsphantomfont:
        return Font(dsphantomfont);
        
    case font_dsphantompfont:
        return Font(dsphantompfont);
        
    case font_zfont:
    default:
        return Font(zfont);
    }
}

Font::Font():
    allegroFont(zfont)
{
}


Font::Font(FONT* f):
    allegroFont(f)
{
}
