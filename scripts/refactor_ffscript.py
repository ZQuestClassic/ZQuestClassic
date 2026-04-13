# This script does 2 things:
#
# 1. A one-time migration of get/set_register to separate files.
# 2. Prints the switch case for getEngineSubsystemForRegister.

import os
import re

from collections import defaultdict
from pathlib import Path

# ==========================================
# Configuration
# ==========================================
GET_FILE = "get_register.txt"
SET_FILE = "set_register.txt"
OUTPUT_DIR = "src/zc/scripting/types"


subsystems = {}

for path in Path('resources/include/bindings').glob('*.zh'):
    subsystem = path.name.replace('.zh', '')
    content = path.read_text()
    matches = re.findall(r'@zasm_var ([^\s]*)', content)
    matches.extend(re.findall(r'SETR D2 ([^\s]{3,})', content))
    matches.extend(re.findall(r'SETR ([^\s]{3,})', content))
    matches = [m for m in matches if not m.startswith('REF')]
    subsystems[subsystem] = matches

subsystems['misc'] = [
    'CLASS_THISKEY',
    'CLASS_THISKEY2',
    'GDD',
    'PC',
    'REFBITMAP',
    'REFBOTTLESHOP',
    'REFBOTTLETYPE',
    'REFCOMBODATA',
    'REFCOMBOTRIGGER',
    'REFDIRECTORY',
    'REFDMAPDATA',
    'REFDROPSETDATA',
    'REFEWPN',
    'REFFFC',
    'REFFILE',
    'REFGENERICDATA',
    'REFITEM',
    'REFITEMDATA',
    'REFLWPN',
    'REFMAPDATA',
    'REFMSGDATA',
    'REFMUSIC',
    'REFNPC',
    'REFNPCDATA',
    'REFPALDATA',
    'REFPORTAL',
    'REFPORTAL',
    'REFRNG',
    'REFSAVEMENU',
    'REFSAVPORTAL',
    'REFSAVPORTAL',
    'REFSCREEN',
    'REFSHOPDATA',
    'REFSPRITE',
    'REFSPRITEDATA',
    'REFSTACK',
    'REFSUBSCREENDATA',
    'REFSUBSCREENPAGE',
    'REFSUBSCREENWIDG',
    'REFWEBSOCKET',
    'SP',
    'SP2',
    'SWITCHKEY',
]


def get_subsystem(register_name):
    """
    Map a register name to its corresponding subsystem.
    Expand this logic to cover all your engine's prefixes.
    """

    for subsystem, registers in subsystems.items():
        if register_name in registers:
            return subsystem

    # `subsystems` covers most stuff, but not everything is still in a bindings file.

    match register_name:
        case 'BITMAPHEIGHT' | 'BITMAPWIDTH':
            return 'bitmap'

        case 'BSHOPCOMBO' | 'BSHOPCSET' | 'BSHOPFILL' | 'BSHOPPRICE' | 'BSHOPSTR':
            return 'bottleshopdata'

        case (
            'BOTTLEAMOUNT'
            | 'BOTTLECOUNTER'
            | 'BOTTLEFLAGS'
            | 'BOTTLENEXT'
            | 'BOTTLEPERCENT'
        ):
            return 'bottledata'

        case (
            'COMBODACLK'
            | 'COMBODAKIMANIMY'
            | 'COMBODANIMFLAGS'
            | 'COMBODASPEED'
            | 'COMBODATAID'
            | 'COMBODATAINITD'
            | 'COMBODATASCRIPT'
            | 'COMBODATTRIBUTES'
            | 'COMBODATTRIBYTES'
            | 'COMBODATTRISHORTS'
            | 'COMBODBLOCKHOLE'
            | 'COMBODBLOCKNPC'
            | 'COMBODBLOCKTRIG'
            | 'COMBODBLOCKWEAPON'
            | 'COMBODBLOCKWPNLEVEL'
            | 'COMBODCONVXSPEED'
            | 'COMBODCONVYSPEED'
            | 'COMBODCSET'
            | 'COMBODCSET2FLAGS'
            | 'COMBODDIRCHANGETYPE'
            | 'COMBODDISTANCECHANGETILES'
            | 'COMBODDIVEITEM'
            | 'COMBODDOCK'
            | 'COMBODEFFECT'
            | 'COMBODFAIRY'
            | 'COMBODFFATTRCHANGE'
            | 'COMBODFLAG'
            | 'COMBODFLIP'
            | 'COMBODFOORDECOTILE'
            | 'COMBODFOORDECOTYPE'
            | 'COMBODFRAME'
            | 'COMBODFRAMES'
            | 'COMBODGENFLAGARR'
            | 'COMBODHOOKSHOTPOINT'
            | 'COMBODLADDERPASS'
            | 'COMBODLIFTBREAKSFX'
            | 'COMBODLIFTBREAKSPRITE'
            | 'COMBODLIFTDAMAGE'
            | 'COMBODLIFTFLAGS'
            | 'COMBODLIFTGFXCCSET'
            | 'COMBODLIFTGFXCOMBO'
            | 'COMBODLIFTGFXSPRITE'
            | 'COMBODLIFTGFXTYPE'
            | 'COMBODLIFTHEIGHT'
            | 'COMBODLIFTITEM'
            | 'COMBODLIFTLEVEL'
            | 'COMBODLIFTLIGHTRAD'
            | 'COMBODLIFTLIGHTSHAPE'
            | 'COMBODLIFTSFX'
            | 'COMBODLIFTTIME'
            | 'COMBODLIFTUNDERCMB'
            | 'COMBODLIFTUNDERCS'
            | 'COMBODLIFTWEAPONITEM'
            | 'COMBODLOCKBLOCK'
            | 'COMBODLOCKBLOCKCHANGE'
            | 'COMBODMAGICMIRROR'
            | 'COMBODMODHPAMOUNT'
            | 'COMBODMODHPDELAY'
            | 'COMBODMODHPTYPE'
            | 'COMBODMODMPDELAY'
            | 'COMBODMODMPTYPE'
            | 'COMBODNEXTC'
            | 'COMBODNEXTD'
            | 'COMBODNEXTTIMER'
            | 'COMBODNMODMPAMOUNT'
            | 'COMBODNOPUSHBLOCK'
            | 'COMBODNUMTRIGGERS'
            | 'COMBODONLYGEN'
            | 'COMBODOTILE'
            | 'COMBODOVERHEAD'
            | 'COMBODPLACENPC'
            | 'COMBODPUSHDIR'
            | 'COMBODPUSHED'
            | 'COMBODPUSHHEAVY'
            | 'COMBODPUSHWAIT'
            | 'COMBODRAFT'
            | 'COMBODRESETROOM'
            | 'COMBODSAVEPOINTTYPE'
            | 'COMBODSCREENFREEZETYPE'
            | 'COMBODSECRETCOMBO'
            | 'COMBODSINGULAR'
            | 'COMBODSKIPANIM'
            | 'COMBODSLOWWALK'
            | 'COMBODSPAWNNPC'
            | 'COMBODSPAWNNPCCHANGE'
            | 'COMBODSPAWNNPCWHEN'
            | 'COMBODSTATUETYPE'
            | 'COMBODSTEPCHANGEINTO'
            | 'COMBODSTEPTYPE'
            | 'COMBODSTRIKECHANGE'
            | 'COMBODSTRIKEITEM'
            | 'COMBODSTRIKEREMNANTS'
            | 'COMBODSTRIKEREMNANTSTYPE'
            | 'COMBODSTRIKEWEAPONS'
            | 'COMBODTILE'
            | 'COMBODTOUCHITEM'
            | 'COMBODTOUCHSTAIRS'
            | 'COMBODTRIGBOSSPAL'
            | 'COMBODTRIGBUNNY'
            | 'COMBODTRIGCSETCHANGE'
            | 'COMBODTRIGDMAPLVL'
            | 'COMBODTRIGEXDOORDIR'
            | 'COMBODTRIGEXDOORIND'
            | 'COMBODTRIGEXSTATE'
            | 'COMBODTRIGGERBUTTON'
            | 'COMBODTRIGGERCHANGECMB'
            | 'COMBODTRIGGERCOOLDOWN'
            | 'COMBODTRIGGERCOPYCAT'
            | 'COMBODTRIGGERCTR'
            | 'COMBODTRIGGERCTRAMNT'
            | 'COMBODTRIGGERFLAGS'
            | 'COMBODTRIGGERFLAGS2'
            | 'COMBODTRIGGERGENSCRIPT'
            | 'COMBODTRIGGERGROUP'
            | 'COMBODTRIGGERGROUPVAL'
            | 'COMBODTRIGGERGSTATE'
            | 'COMBODTRIGGERGTIMER'
            | 'COMBODTRIGGERITEM'
            | 'COMBODTRIGGERLEVEL'
            | 'COMBODTRIGGERLIGHTBEAM'
            | 'COMBODTRIGGERLSTATE'
            | 'COMBODTRIGGERPROX'
            | 'COMBODTRIGGERSENS'
            | 'COMBODTRIGGERSFX'
            | 'COMBODTRIGGERTIMER'
            | 'COMBODTRIGGERTYPE'
            | 'COMBODTRIGITEMJINX'
            | 'COMBODTRIGITEMPICKUP'
            | 'COMBODTRIGLITEMS'
            | 'COMBODTRIGLVLPAL'
            | 'COMBODTRIGPUSHTIME'
            | 'COMBODTRIGQUAKETIME'
            | 'COMBODTRIGSHIELDJINX'
            | 'COMBODTRIGSPAWNENEMY'
            | 'COMBODTRIGSPAWNITEM'
            | 'COMBODTRIGSTUN'
            | 'COMBODTRIGSWORDJINX'
            | 'COMBODTRIGTINTB'
            | 'COMBODTRIGTINTG'
            | 'COMBODTRIGTINTR'
            | 'COMBODTRIGWAVYTIME'
            | 'COMBODTYPE'
            | 'COMBODUSRFLAGARR'
            | 'COMBODUSRFLAGS'
            | 'COMBODWALK'
            | 'COMBODWARPDIRECT'
            | 'COMBODWARPLOCATION'
            | 'COMBODWARPSENS'
            | 'COMBODWARPTYPE'
            | 'COMBODWATER'
            | 'COMBODWHISTLE'
            | 'COMBODWINGAME'
            | 'COMBOD_DIVE_UNDER_LEVEL'
            | 'COMBOD_Z_HEIGHT'
            | 'COMBOD_Z_STEP_HEIGHT'
        ):
            return 'combodata'

        case (
            'CMBTRIGBOSSPAL'
            | 'CMBTRIGBUNNY'
            | 'CMBTRIGBUTTON'
            | 'CMBTRIGCHANGECMB'
            | 'CMBTRIGCOOLDOWN'
            | 'CMBTRIGCOPYCAT'
            | 'CMBTRIGCSETCHANGE'
            | 'CMBTRIGCTR'
            | 'CMBTRIGCTRAMNT'
            | 'CMBTRIGDMAPLVL'
            | 'CMBTRIGEXDOORDIR'
            | 'CMBTRIGEXDOORIND'
            | 'CMBTRIGEXSTATE'
            | 'CMBTRIGFLAGS'
            | 'CMBTRIGGENSCRIPT'
            | 'CMBTRIGGERDESTHEROX'
            | 'CMBTRIGGERDESTHEROY'
            | 'CMBTRIGGERDESTHEROZ'
            | 'CMBTRIGGERFAILPROMPTCID'
            | 'CMBTRIGGERFAILPROMPTCS'
            | 'CMBTRIGGERFAILSTR'
            | 'CMBTRIGGERFORCEPLAYERDIR'
            | 'CMBTRIGGERICECOMBO'
            | 'CMBTRIGGERICEVX'
            | 'CMBTRIGGERICEVY'
            | 'CMBTRIGGERPLAYERBOUNCE'
            | 'CMBTRIGGERPROMPTCID'
            | 'CMBTRIGGERPROMPTCS'
            | 'CMBTRIGGERPROMPTX'
            | 'CMBTRIGGERPROMPTY'
            | 'CMBTRIGGERREQPLAYERDIR'
            | 'CMBTRIGGERREQPLAYERJUMP'
            | 'CMBTRIGGERREQPLAYERX'
            | 'CMBTRIGGERREQPLAYERY'
            | 'CMBTRIGGERREQPLAYERZ'
            | 'CMBTRIGGERTRIGSTR'
            | 'CMBTRIGGER_GRAVITY'
            | 'CMBTRIGGER_TERMINAL_VELOCITY'
            | 'CMBTRIGGROUP'
            | 'CMBTRIGGROUPVAL'
            | 'CMBTRIGGSTATE'
            | 'CMBTRIGGTIMER'
            | 'CMBTRIGITEMJINX'
            | 'CMBTRIGITEMPICKUP'
            | 'CMBTRIGLIGHTBEAM'
            | 'CMBTRIGLITEMS'
            | 'CMBTRIGLSTATE'
            | 'CMBTRIGLVLPAL'
            | 'CMBTRIGPROX'
            | 'CMBTRIGPUSHTIME'
            | 'CMBTRIGQUAKETIME'
            | 'CMBTRIGREQGLOBALSTATE'
            | 'CMBTRIGREQITEM'
            | 'CMBTRIGREQLVLSTATE'
            | 'CMBTRIGSFX'
            | 'CMBTRIGSHIELDJINX'
            | 'CMBTRIGSPAWNENEMY'
            | 'CMBTRIGSPAWNITEM'
            | 'CMBTRIGSTUN'
            | 'CMBTRIGSWORDJINX'
            | 'CMBTRIGTIMER'
            | 'CMBTRIGTINTB'
            | 'CMBTRIGTINTG'
            | 'CMBTRIGTINTR'
            | 'CMBTRIGUNREQGLOBALSTATE'
            | 'CMBTRIGUNREQLVLSTATE'
            | 'CMBTRIGREQSCRSTATE'
            | 'CMBTRIGUNREQSCRSTATE'
            | 'CMBTRIGREQSCREXSTATE'
            | 'CMBTRIGUNREQSCREXSTATE'
            | 'CMBTRIGREQSTATEMAP'
            | 'CMBTRIGREQSTATESCREEN'
            | 'CMBTRIGWAVYTIME'
            | 'CMBTRIGWPNLEVEL'
            | 'CMBTRIG_PLAY_MUSIC'
            | 'CMBTRIG_MUSIC_REFRESH'
        ):
            return 'combotrigger'

        case 'DIRECTORYSIZE':
            return 'directory'

        case (
            'DMAPDATAASUBSCRIPT'
            | 'DMAPDATACHARTED'
            | 'DMAPDATACOMPASS'
            | 'DMAPDATACONTINUE'
            | 'DMAPDATADISABLEDITEMS'
            | 'DMAPDATAFLAGARR'
            | 'DMAPDATAFLAGS'
            | 'DMAPDATAGRID'
            | 'DMAPDATAINTROSTRINGID'
            | 'DMAPDATALARGEMAPCSET'
            | 'DMAPDATALARGEMAPTILE'
            | 'DMAPDATAFLOOR'
            | 'DMAPDATALEVEL'
            | 'DMAPDATALOOPEND'
            | 'DMAPDATALOOPSTART'
            | 'DMAPDATAMAP'
            | 'DMAPINITD'
            | 'DMAPDATAMAPINITD'
            | 'DMAPDATAMAPSCRIPT'
            | 'DMAPDATAMIDI'
            | 'DMAPDATAMINIMAPCSET'
            | 'DMAPDATAMINIMAPTILE'
            | 'DMAPDATAMIRRDMAP'
            | 'DMAPDATAMUISCTRACK'
            | 'DMAPDATAOFFSET'
            | 'DMAPDATAPALETTE'
            | 'DMAPDATAPSUBSCRIPT'
            | 'DMAPDATASIDEVIEW'
            | 'DMAPDATASUBINITD'
            | 'DMAPDATASUBSCRA'
            | 'DMAPDATASUBSCRO'
            | 'DMAPDATASUBSCRP'
            | 'DMAPDATATYPE'
            | 'DMAPDATAXFADEIN'
            | 'DMAPDATAXFADEOUT'
            | 'DMAPDATA_GRAVITY_STRENGTH'
            | 'DMAPDATA_TERMINAL_VELOCITY'
            | 'DMAPSCRIPT'
        ):
            return 'dmapdata'

        case 'DROPSETCHANCES' | 'DROPSETCHOOSE' | 'DROPSETITEMS' | 'DROPSETNULLCHANCE':
            return 'dropsetdata'

        case (
            'EWEAPONSCRIPTUID'
            | 'EWPNANGLE'
            | 'EWPNANGULAR'
            | 'EWPNASPEED'
            | 'EWPNAUTOROTATE'
            | 'EWPNBEHIND'
            | 'EWPNBURNLIGHTRADIUS'
            | 'EWPNCOLLDET'
            | 'EWPNCSET'
            | 'EWPNDEAD'
            | 'EWPNDEATHDROPSET'
            | 'EWPNDEATHIPICKUP'
            | 'EWPNDEATHITEM'
            | 'EWPNDEATHSFX'
            | 'EWPNDEATHSPRITE'
            | 'EWPNDEGANGLE'
            | 'EWPNDIR'
            | 'EWPNDRAWTYPE'
            | 'EWPNDROWNCLK'
            | 'EWPNDROWNCMB'
            | 'EWPNENGINEANIMATE'
            | 'EWPNEXTEND'
            | 'EWPNFAKEJUMP'
            | 'EWPNFAKEZ'
            | 'EWPNFALLCLK'
            | 'EWPNFALLCMB'
            | 'EWPNFLAGS'
            | 'EWPNFLASH'
            | 'EWPNFLASHCSET'
            | 'EWPNFLIP'
            | 'EWPNFRAME'
            | 'EWPNFRAMES'
            | 'EWPNGLOWRAD'
            | 'EWPNGLOWSHP'
            | 'EWPNGRAVITY'
            | 'EWPNHXOFS'
            | 'EWPNHXSZ'
            | 'EWPNHYOFS'
            | 'EWPNHYSZ'
            | 'EWPNHZSZ'
            | 'EWPNINITD'
            | 'EWPNJUMP'
            | 'EWPNLEVEL'
            | 'EWPNLIFTHEIGHT'
            | 'EWPNLIFTLEVEL'
            | 'EWPNLIFTTIME'
            | 'EWPNMISCD'
            | 'EWPNMOVEFLAGS'
            | 'EWPNOCSET'
            | 'EWPNOTILE'
            | 'EWPNPARENT'
            | 'EWPNPARENTUID'
            | 'EWPNPOWER'
            | 'EWPNROTATION'
            | 'EWPNSCALE'
            | 'EWPNSCRIPT'
            | 'EWPNSCRIPTFLIP'
            | 'EWPNSCRIPTTILE'
            | 'EWPNSHADOWSPR'
            | 'EWPNSHADOWXOFS'
            | 'EWPNSHADOWYOFS'
            | 'EWPNSPRITES'
            | 'EWPNSTEP'
            | 'EWPNTILE'
            | 'EWPNTIMEOUT'
            | 'EWPNPIERCE'
            | 'EWPNTOTALDYOFFS'
            | 'EWPNTXSZ'
            | 'EWPNTYPE'
            | 'EWPNTYSZ'
            | 'EWPNUNBL'
            | 'EWPNVX'
            | 'EWPNVY'
            | 'EWPNX'
            | 'EWPNXOFS'
            | 'EWPNY'
            | 'EWPNYOFS'
            | 'EWPNZ'
            | 'EWPNZOFS'
            | 'EWSWHOOKED'
        ):
            return 'eweapon'

        case (
            'DATA'
            | 'DELAY'
            | 'FCSET'
            | 'FFCHEIGHT'
            | 'FFCID'
            | 'FFCINDEX'
            | 'FFCLAYER'
            | 'FFCWIDTH'
            | 'FFFLAGSD'
            | 'FFINITDD'
            | 'FFLINK'
            | 'FFMISCD'
            | 'FFSCRIPT'
            | 'FFTHEIGHT'
            | 'FFTWIDTH'
            | 'FX'
            | 'FY'
            | 'XD'
            | 'XD2'
            | 'YD'
            | 'YD2'
        ):
            return 'ffc'

        case 'FILEEOF' | 'FILEERR' | 'FILEPOS':
            return 'file'

        case (
            'GENDATADATA'
            | 'GENDATAEVENTSTATE'
            | 'GENDATAEXITSTATE'
            | 'GENDATAINITD'
            | 'GENDATARELOADSTATE'
            | 'GENDATARUNNING'
            | 'GENDATASIZE'
        ):
            return 'genericdata'

        case (
            'ITEMACLK'
            | 'ITEMASPEED'
            | 'ITEMCSET'
            | 'ITEMDELAY'
            | 'ITEMDIR'
            | 'ITEMDRAWTYPE'
            | 'ITEMDROPPEDBY'
            | 'ITEMDROWNCLK'
            | 'ITEMDROWNCMB'
            | 'ITEMENGINEANIMATE'
            | 'ITEMEXTEND'
            | 'ITEMFAKEJUMP'
            | 'ITEMFAKEZ'
            | 'ITEMFALLCLK'
            | 'ITEMFALLCMB'
            | 'ITEMFLASH'
            | 'ITEMFLASHCSET'
            | 'ITEMFLIP'
            | 'ITEMFORCEGRAB'
            | 'ITEMFRAME'
            | 'ITEMFRAMES'
            | 'ITEMGLOWRAD'
            | 'ITEMGLOWSHP'
            | 'ITEMGRAVITY'
            | 'ITEMHXOFS'
            | 'ITEMHXSZ'
            | 'ITEMHYOFS'
            | 'ITEMHYSZ'
            | 'ITEMHZSZ'
            | 'ITEMID'
            | 'ITEMJUMP'
            | 'ITEMLEVEL'
            | 'ITEMMISCD'
            | 'ITEMMOVEFLAGS'
            | 'ITEMNOHOLDSOUND'
            | 'ITEMNOSOUND'
            | 'ITEMOTILE'
            | 'ITEMPICKUP'
            | 'ITEMPSTRING'
            | 'ITEMPSTRINGFLAGS'
            | 'ITEMROTATION'
            | 'ITEMSCALE'
            | 'ITEMSCRIPTFLIP'
            | 'ITEMSCRIPTTILE'
            | 'ITEMSCRIPTUID'
            | 'ITEMSHADOWSPR'
            | 'ITEMSHADOWXOFS'
            | 'ITEMSHADOWYOFS'
            | 'ITEMSPRITEINITD'
            | 'ITEMSPRITESCRIPT'
            | 'ITEMTILE'
            | 'ITEMTXSZ'
            | 'ITEMTYPE'
            | 'ITEMTYSZ'
            | 'ITEMX'
            | 'ITEMXOFS'
            | 'ITEMY'
            | 'ITEMYOFS'
            | 'ITEMZ'
            | 'ITEMZOFS'
            | 'ITMSWHOOKED'
        ):
            return 'item'

        case (
            'IDATAAMOUNT'
            | 'IDATAASPEED'
            | 'IDATAATTRIB'
            | 'IDATAATTRIB_L'
            | 'IDATABUNNYABLE'
            | 'IDATABURNINGLIGHTRAD'
            | 'IDATABURNINGSPR'
            | 'IDATACOLLECTFLAGS'
            | 'IDATACOMBINE'
            | 'IDATACONSTSCRIPT'
            | 'IDATACOOLDOWN'
            | 'IDATACOST2'
            | 'IDATACOSTCOUNTER'
            | 'IDATACOSTCOUNTER2'
            | 'IDATACOUNTER'
            | 'IDATACSET'
            | 'IDATADELAY'
            | 'IDATADOWNGRADE'
            | 'IDATADRAWLAYER'
            | 'IDATADUPLICATES'
            | 'IDATADURATION'
            | 'IDATADXOFS'
            | 'IDATADYOFS'
            | 'IDATAEDIBLE'
            | 'IDATAFLAGS'
            | 'IDATAFLAGUNUSED'
            | 'IDATAFLASHCSET'
            | 'IDATAFRAMES'
            | 'IDATAGAINLOWER'
            | 'IDATAGRADUAL'
            | 'IDATAHXOFS'
            | 'IDATAHXSZ'
            | 'IDATAHYOFS'
            | 'IDATAHYSZ'
            | 'IDATAHZSZ'
            | 'IDATAID'
            | 'IDATAINITDD'
            | 'IDATAJINXIMMUNE'
            | 'IDATAJINXSWAP'
            | 'IDATAKEEP'
            | 'IDATAKEEPOLD'
            | 'IDATALEVEL'
            | 'IDATALTM'
            | 'IDATAMAGCOST'
            | 'IDATAMAGICTIMER'
            | 'IDATAMAGICTIMER2'
            | 'IDATAMAX'
            | 'IDATAMINHEARTS'
            | 'IDATAMISC'
            | 'IDATAMISCD'
            | 'IDATAMOVEFLAGS'
            | 'IDATAOVERRIDEFL'
            | 'IDATAOVERRIDEFLWEAP'
            | 'IDATAPFLAGS'
            | 'IDATAPICKUP'
            | 'IDATAPICKUPLITEMLEVEL'
            | 'IDATAPICKUPLITEMS'
            | 'IDATAPOWER'
            | 'IDATAPSCRIPT'
            | 'IDATAPSOUND'
            | 'IDATAPSTRING'
            | 'IDATARUPEECOST'
            | 'IDATASCRIPT'
            | 'IDATASETMAX'
            | 'IDATASPRITE'
            | 'IDATASPRSCRIPT'
            | 'IDATASSWIMDISABLED'
            | 'IDATATILE'
            | 'IDATATILEH'
            | 'IDATATILEHWEAP'
            | 'IDATATILEW'
            | 'IDATATILEWWEAP'
            | 'IDATATYPE'
            | 'IDATAUSEBURNSPR'
            | 'IDATAUSEDEF'
            | 'IDATAUSESOUND'
            | 'IDATAUSESOUND2'
            | 'IDATAUSEWPN'
            | 'IDATAVALIDATE'
            | 'IDATAVALIDATE2'
            | 'IDATAWEAPHXOFS'
            | 'IDATAWEAPHXSZ'
            | 'IDATAWEAPHYOFS'
            | 'IDATAWEAPHYSZ'
            | 'IDATAWEAPHZSZ'
            | 'IDATAWEAPONSCRIPT'
            | 'IDATAWEAPXOFS'
            | 'IDATAWEAPYOFS'
            | 'IDATAWMOVEFLAGS'
            | 'IDATAWPNINITD'
            | 'IDATAWRANGE'
        ):
            return 'itemdata'

        case (
            'LWEAPONSCRIPTUID'
            | 'LWPNANGLE'
            | 'LWPNANGULAR'
            | 'LWPNASPEED'
            | 'LWPNAUTOROTATE'
            | 'LWPNBEHIND'
            | 'LWPNBURNLIGHTRADIUS'
            | 'LWPNCOLLDET'
            | 'LWPNCSET'
            | 'LWPNDEAD'
            | 'LWPNDEATHDROPSET'
            | 'LWPNDEATHIPICKUP'
            | 'LWPNDEATHITEM'
            | 'LWPNDEATHSFX'
            | 'LWPNDEATHSPRITE'
            | 'LWPNDEGANGLE'
            | 'LWPNDIR'
            | 'LWPNDRAWTYPE'
            | 'LWPNDROWNCLK'
            | 'LWPNDROWNCMB'
            | 'LWPNENGINEANIMATE'
            | 'LWPNEXTEND'
            | 'LWPNFAKEJUMP'
            | 'LWPNFAKEZ'
            | 'LWPNFALLCLK'
            | 'LWPNFALLCMB'
            | 'LWPNFLAGS'
            | 'LWPNFLASH'
            | 'LWPNFLASHCSET'
            | 'LWPNFLIP'
            | 'LWPNFRAME'
            | 'LWPNFRAMES'
            | 'LWPNGLOWRAD'
            | 'LWPNGLOWSHP'
            | 'LWPNGRAVITY'
            | 'LWPNHXOFS'
            | 'LWPNHXSZ'
            | 'LWPNHYOFS'
            | 'LWPNHYSZ'
            | 'LWPNHZSZ'
            | 'LWPNINITD'
            | 'LWPNJUMP'
            | 'LWPNLEVEL'
            | 'LWPNLIFTHEIGHT'
            | 'LWPNLIFTLEVEL'
            | 'LWPNLIFTTIME'
            | 'LWPNMISCD'
            | 'LWPNMOVEFLAGS'
            | 'LWPNOCSET'
            | 'LWPNOTILE'
            | 'LWPNPARENT'
            | 'LWPNPOWER'
            | 'LWPNROTATION'
            | 'LWPNSCALE'
            | 'LWPNSCRIPT'
            | 'LWPNSCRIPTFLIP'
            | 'LWPNSCRIPTTILE'
            | 'LWPNSHADOWSPR'
            | 'LWPNSHADOWXOFS'
            | 'LWPNSHADOWYOFS'
            | 'LWPNSPECIAL'
            | 'LWPNSPRITES'
            | 'LWPNSTEP'
            | 'LWPNTILE'
            | 'LWPNTIMEOUT'
            | 'LWPNPIERCE'
            | 'LWPNTOTALDYOFFS'
            | 'LWPNTXSZ'
            | 'LWPNTYPE'
            | 'LWPNTYSZ'
            | 'LWPNUNBL'
            | 'LWPNUSEDEFENCE'
            | 'LWPNUSEWEAPON'
            | 'LWPNVX'
            | 'LWPNVY'
            | 'LWPNX'
            | 'LWPNXOFS'
            | 'LWPNY'
            | 'LWPNYOFS'
            | 'LWPNZ'
            | 'LWPNZOFS'
            | 'LWSWHOOKED'
        ):
            return 'lweapon'

        case (
            'MAPDATABOSSSFX'
            | 'MAPDATACATCHALL'
            | 'MAPDATACOLOUR'
            | 'MAPDATACOMBOCD'
            | 'MAPDATACOMBODATAD'
            | 'MAPDATACOMBODD'
            | 'MAPDATACOMBOED'
            | 'MAPDATACOMBOFD'
            | 'MAPDATACOMBOID'
            | 'MAPDATACOMBOSD'
            | 'MAPDATACOMBOTD'
            | 'MAPDATACSENSITIVE'
            | 'MAPDATADOOR'
            | 'MAPDATADOORCOMBOSET'
            | 'MAPDATAENEMY'
            | 'MAPDATAENEMYFLAGS'
            | 'MAPDATAENTRYX'
            | 'MAPDATAENTRYY'
            | 'MAPDATAEXCARRY'
            | 'MAPDATAEXDOOR'
            | 'MAPDATAEXITDIR'
            | 'MAPDATAEXRESET'
            | 'MAPDATAEXSTATED'
            | 'MAPDATAFFCSET'
            | 'MAPDATAFFDATA'
            | 'MAPDATAFFDELAY'
            | 'MAPDATAFFEFFECTHEIGHT'
            | 'MAPDATAFFEFFECTWIDTH'
            | 'MAPDATAFFFLAGS'
            | 'MAPDATAFFHEIGHT'
            | 'MAPDATAFFINITIALISED'
            | 'MAPDATAFFLINK'
            | 'MAPDATAFFSCRIPT'
            | 'MAPDATAFFWIDTH'
            | 'MAPDATAFFX'
            | 'MAPDATAFFXDELTA'
            | 'MAPDATAFFXDELTA2'
            | 'MAPDATAFFY'
            | 'MAPDATAFFYDELTA'
            | 'MAPDATAFFYDELTA2'
            | 'MAPDATAFLAGS'
            | 'MAPDATAGUY'
            | 'MAPDATAGUYCOUNT'
            | 'MAPDATAHASITEM'
            | 'MAPDATAHOLDUPSFX'
            | 'MAPDATAINITDARRAY'
            | 'MAPDATAITEM'
            | 'MAPDATAITEMX'
            | 'MAPDATAITEMY'
            | 'MAPDATALAYERINVIS'
            | 'MAPDATALAYERMAP'
            | 'MAPDATALAYEROPACITY'
            | 'MAPDATALAYERSCREEN'
            | 'MAPDATALENSHIDES'
            | 'MAPDATALENSLAYER'
            | 'MAPDATALENSSHOWS'
            | 'MAPDATAMAP'
            | 'MAPDATAMISCD'
            | 'MAPDATANEXTMAP'
            | 'MAPDATANEXTSCREEN'
            | 'MAPDATANOCARRY'
            | 'MAPDATANOCARRYARR'
            | 'MAPDATANORESET'
            | 'MAPDATANORESETARR'
            | 'MAPDATAOCEANSFX'
            | 'MAPDATAPATH'
            | 'MAPDATAPATTERN'
            | 'MAPDATAREGIONID'
            | 'MAPDATAROOM'
            | 'MAPDATASCRDATA'
            | 'MAPDATASCRDATASIZE'
            | 'MAPDATASCREEN'
            | 'MAPDATASCREENEFLAGSD'
            | 'MAPDATASCREENFLAGSD'
            | 'MAPDATASCREENMIDI'
            | 'MAPDATASCREENSTATED'
            | 'MAPDATASCRIPT'
            | 'MAPDATASCRIPTDRAWS'
            | 'MAPDATASECRETCOMBO'
            | 'MAPDATASECRETCSET'
            | 'MAPDATASECRETFLAG'
            | 'MAPDATASECRETSFX'
            | 'MAPDATASIDEWARPDMAP'
            | 'MAPDATASIDEWARPID'
            | 'MAPDATASIDEWARPINDEX'
            | 'MAPDATASIDEWARPOVFLAGS'
            | 'MAPDATASIDEWARPSC'
            | 'MAPDATASIDEWARPTYPE'
            | 'MAPDATASTAIRX'
            | 'MAPDATASTAIRY'
            | 'MAPDATASTRING'
            | 'MAPDATASWARPRETSQR'
            | 'MAPDATATILEWARPDMAP'
            | 'MAPDATATILEWARPOVFLAGS'
            | 'MAPDATATILEWARPSCREEN'
            | 'MAPDATATILEWARPTYPE'
            | 'MAPDATATIMEDWARPTICS'
            | 'MAPDATATWARPRETSQR'
            | 'MAPDATAUNDERCOMBO'
            | 'MAPDATAUNDERCSET'
            | 'MAPDATAVALID'
            | 'MAPDATAWARPARRIVALX'
            | 'MAPDATAWARPARRIVALY'
            | 'MAPDATAWARPRETURNC'
            | 'MAPDATAWARPRETX'
            | 'MAPDATAWARPRETY'
            | 'MAPDATA_FLAG'
            | 'MAPDATA_GRAVITY_STRENGTH'
            | 'MAPDATA_TERMINAL_VELOCITY'
            | 'MAPDATA_MUSIC'
        ):
            return 'mapdata'

        case (
            'MESSAGEDATACSET'
            | 'MESSAGEDATAFLAGS'
            | 'MESSAGEDATAFONT'
            | 'MESSAGEDATAH'
            | 'MESSAGEDATAHSPACE'
            | 'MESSAGEDATALISTPOS'
            | 'MESSAGEDATANEXT'
            | 'MESSAGEDATAPORTCSET'
            | 'MESSAGEDATAPORTHEI'
            | 'MESSAGEDATAPORTTILE'
            | 'MESSAGEDATAPORTWID'
            | 'MESSAGEDATAPORTX'
            | 'MESSAGEDATAPORTY'
            | 'MESSAGEDATASEGMENTS'
            | 'MESSAGEDATASFX'
            | 'MESSAGEDATATEXTHEI'
            | 'MESSAGEDATATEXTLEN'
            | 'MESSAGEDATATEXTWID'
            | 'MESSAGEDATATILE'
            | 'MESSAGEDATATRANS'
            | 'MESSAGEDATAVSPACE'
            | 'MESSAGEDATAW'
            | 'MESSAGEDATAX'
            | 'MESSAGEDATAY'
        ):
            return 'msgdata'

        case (
            'NPCBEHAVIOUR'
            | 'NPCBGSFX'
            | 'NPCBOSSPAL'
            | 'NPCCANFLICKER'
            | 'NPCCOLLDET'
            | 'NPCCOLLISION'
            | 'NPCCSET'
            | 'NPCDD'
            | 'NPCDEATHSPR'
            | 'NPCDEFENSED'
            | 'NPCDIR'
            | 'NPCDP'
            | 'NPCDRAWTYPE'
            | 'NPCDROWNCLK'
            | 'NPCDROWNCMB'
            | 'NPCENGINEANIMATE'
            | 'NPCEXTEND'
            | 'NPCFADING'
            | 'NPCFAKEJUMP'
            | 'NPCFAKEZ'
            | 'NPCFALLCLK'
            | 'NPCFALLCMB'
            | 'NPCFIRESFX'
            | 'NPCFLAGS'
            | 'NPCFLASHINGCSET'
            | 'NPCFLICKERCOLOR'
            | 'NPCFLICKERTRANSP'
            | 'NPCFRAME'
            | 'NPCFRAMERATE'
            | 'NPCFROZEN'
            | 'NPCFROZENCSET'
            | 'NPCFROZENTILE'
            | 'NPCGLOWRAD'
            | 'NPCGLOWSHP'
            | 'NPCGRAVITY'
            | 'NPCHALTCLK'
            | 'NPCHALTRATE'
            | 'NPCHASITEM'
            | 'NPCHITBY'
            | 'NPCHITDIR'
            | 'NPCHOMING'
            | 'NPCHP'
            | 'NPCHUNGER'
            | 'NPCHXOFS'
            | 'NPCHXSZ'
            | 'NPCHYOFS'
            | 'NPCHYSZ'
            | 'NPCHZSZ'
            | 'NPCID'
            | 'NPCIMMORTAL'
            | 'NPCINITD'
            | 'NPCINVINC'
            | 'NPCISCORE'
            | 'NPCITEMSET'
            | 'NPCJUMP'
            | 'NPCKNOCKBACKSPEED'
            | 'NPCLINEDUP'
            | 'NPCMFLAGS'
            | 'NPCMISCD'
            | 'NPCMOVEFLAGS'
            | 'NPCMOVESTATUS'
            | 'NPCNOSCRIPTKB'
            | 'NPCNOSLIDE'
            | 'NPCORIGINALHP'
            | 'NPCOTILE'
            | 'NPCPARENTUID'
            | 'NPCRANDOM'
            | 'NPCRATE'
            | 'NPCRINGLEAD'
            | 'NPCROTATION'
            | 'NPCSCALE'
            | 'NPCSCRDEFENSED'
            | 'NPCSCRIPT'
            | 'NPCSCRIPTFLIP'
            | 'NPCSCRIPTTILE'
            | 'NPCSCRIPTUID'
            | 'NPCSHADOWSPR'
            | 'NPCSHADOWXOFS'
            | 'NPCSHADOWYOFS'
            | 'NPCSHIELD'
            | 'NPCSLIDECLK'
            | 'NPCSPAWNSPR'
            | 'NPCSTEP'
            | 'NPCSTUN'
            | 'NPCSUBMERGED'
            | 'NPCSUPERMAN'
            | 'NPCSWHOOKED'
            | 'NPCTILE'
            | 'NPCTOTALDYOFFS'
            | 'NPCTXSZ'
            | 'NPCTYPE'
            | 'NPCTYSZ'
            | 'NPCWDP'
            | 'NPCWEAPON'
            | 'NPCWEAPSPRITE'
            | 'NPCX'
            | 'NPCXOFS'
            | 'NPCY'
            | 'NPCYOFS'
            | 'NPCZ'
            | 'NPCZOFS'
        ):
            return 'npc'

        case (
            'NPCDATAANIM'
            | 'NPCDATABGSFX'
            | 'NPCDATACSET'
            | 'NPCDATADEATHSFX'
            | 'NPCDATADROPSET'
            | 'NPCDATAEANIM'
            | 'NPCDATAEFRAMERATE'
            | 'NPCDATAEHEIGHT'
            | 'NPCDATAETILE'
            | 'NPCDATAEWIDTH'
            | 'NPCDATAFIRESFX'
            | 'NPCDATAFLAGS1'
            | 'NPCDATAFLAGS2'
            | 'NPCDATAFRAMERATE'
            | 'NPCDATAFROZENCSET'
            | 'NPCDATAFROZENTILE'
            | 'NPCDATAHALT'
            | 'NPCDATAHEIGHT'
            | 'NPCDATAHITHEIGHT'
            | 'NPCDATAHITSFX'
            | 'NPCDATAHITWIDTH'
            | 'NPCDATAHITZ'
            | 'NPCDATAHOMING'
            | 'NPCDATAHP'
            | 'NPCDATAHUNGER'
            | 'NPCDATAHXOFS'
            | 'NPCDATAHYOFS'
            | 'NPCDATARANDOM'
            | 'NPCDATASCRIPT'
            | 'NPCDATASHEIGHT'
            | 'NPCDATASIZEFLAG'
            | 'NPCDATASTEP'
            | 'NPCDATASTILE'
            | 'NPCDATASWIDTH'
            | 'NPCDATATILE'
            | 'NPCDATATILEHEIGHT'
            | 'NPCDATATILEWIDTH'
            | 'NPCDATATOUCHDAMAGE'
            | 'NPCDATATYPE'
            | 'NPCDATAWEAPON'
            | 'NPCDATAWEAPONDAMAGE'
            | 'NPCDATAWEAPONSCRIPT'
            | 'NPCDATAWIDTH'
            | 'NPCDATAWPNSPRITE'
            | 'NPCDATAXOFS'
            | 'NPCDATAYOFS'
            | 'NPCDATAZOFS'
            | 'NPCDDEATHSPR'
            | 'NPCDSHADOWSPR'
            | 'NPCDSPAWNSPR'
            | 'NPCMATCHINITDLABEL'
        ):
            return 'npcdata'

        case 'PALDATAB' | 'PALDATACOLOR' | 'PALDATAG' | 'PALDATAR':
            return 'paldata'

        case (
            'PORTALACLK'
            | 'PORTALAFRM'
            | 'PORTALASPD'
            | 'PORTALCLOSEDIS'
            | 'PORTALDMAP'
            | 'PORTALFRAMES'
            | 'PORTALOTILE'
            | 'PORTALSAVED'
            | 'PORTALSCREEN'
            | 'PORTALWARPSFX'
            | 'PORTALWARPVFX'
            | 'PORTALX'
            | 'PORTALY'
        ):
            return 'portal'

        case (
            'SAVEDPORTALDESTDMAP'
            | 'SAVEDPORTALDSTSCREEN'
            | 'SAVEDPORTALPORTAL'
            | 'SAVEDPORTALSPRITE'
            | 'SAVEDPORTALSRCDMAP'
            | 'SAVEDPORTALSRCSCREEN'
            | 'SAVEDPORTALWARPSFX'
            | 'SAVEDPORTALWARPVFX'
            | 'SAVEDPORTALX'
            | 'SAVEDPORTALY'
        ):
            return 'savedportal'

        case (
            'ROOMDATA'
            | 'ROOMTYPE'
            | 'SCRDOORD'
            | 'SCREENDATABOSSSFX'
            | 'SCREENDATACATCHALL'
            | 'SCREENDATACOLOUR'
            | 'SCREENDATACSENSITIVE'
            | 'SCREENDATADOOR'
            | 'SCREENDATADOORCOMBOSET'
            | 'SCREENDATAENEMY'
            | 'SCREENDATAENEMYFLAGS'
            | 'SCREENDATAENTRYX'
            | 'SCREENDATAENTRYY'
            | 'SCREENDATAEXCARRY'
            | 'SCREENDATAEXDOOR'
            | 'SCREENDATAEXITDIR'
            | 'SCREENDATAEXRESET'
            | 'SCREENDATAFLAGS'
            | 'SCREENDATAGUY'
            | 'SCREENDATAGUYCOUNT'
            | 'SCREENDATAHASITEM'
            | 'SCREENDATAHOLDUPSFX'
            | 'SCREENDATAITEM'
            | 'SCREENDATAITEMX'
            | 'SCREENDATAITEMY'
            | 'SCREENDATALAYERINVIS'
            | 'SCREENDATALAYERMAP'
            | 'SCREENDATALAYEROPACITY'
            | 'SCREENDATALAYERSCREEN'
            | 'SCREENDATALENSLAYER'
            | 'SCREENDATANEXTMAP'
            | 'SCREENDATANEXTSCREEN'
            | 'SCREENDATANOCARRY'
            | 'SCREENDATANOCARRYARR'
            | 'SCREENDATANORESET'
            | 'SCREENDATANORESETARR'
            | 'SCREENDATAOCEANSFX'
            | 'SCREENDATAPATH'
            | 'SCREENDATAPATTERN'
            | 'SCREENDATAROOM'
            | 'SCREENDATASCREENMIDI'
            | 'SCREENDATASCRIPTDRAWS'
            | 'SCREENDATASECRETCOMBO'
            | 'SCREENDATASECRETCSET'
            | 'SCREENDATASECRETFLAG'
            | 'SCREENDATASECRETSFX'
            | 'SCREENDATASIDEWARPDMAP'
            | 'SCREENDATASIDEWARPINDEX'
            | 'SCREENDATASIDEWARPOVFLAGS'
            | 'SCREENDATASIDEWARPSC'
            | 'SCREENDATASIDEWARPTYPE'
            | 'SCREENDATASTAIRX'
            | 'SCREENDATASTAIRY'
            | 'SCREENDATASTRING'
            | 'SCREENDATASWARPRETSQR'
            | 'SCREENDATATILEWARPDMAP'
            | 'SCREENDATATILEWARPOVFLAGS'
            | 'SCREENDATATILEWARPSCREEN'
            | 'SCREENDATATILEWARPTYPE'
            | 'SCREENDATATIMEDWARPTICS'
            | 'SCREENDATATWARPRETSQR'
            | 'SCREENDATAVALID'
            | 'SCREENDATAWARPARRIVALX'
            | 'SCREENDATAWARPARRIVALY'
            | 'SCREENDATAWARPRETURNC'
            | 'SCREENDATAWARPRETX'
            | 'SCREENDATAWARPRETY'
            | 'SCREENDATA_GRAVITY_STRENGTH'
            | 'SCREENDATA_TERMINAL_VELOCITY'
            | 'SCREENEFLAGSD'
            | 'SCREENEXSTATED'
            | 'SCREENFLAGSD'
            | 'SCREENINITD'
            | 'SCREENLENSHIDES'
            | 'SCREENLENSSHOWS'
            | 'SCREENSCRDATA'
            | 'SCREENSCRIPT'
            | 'SCREENSECRETSTRIGGERED'
            | 'SCREENSIDEWARPID'
            | 'SCREENSTATED'
            | 'SCREEN_FLAG'
            | 'SDD'
            | 'UNDERCOMBO'
            | 'UNDERCSET'
            | 'SCREENDATA_MUSIC'
        ):
            return 'screendata'

        case 'SHOPDATAHASITEM' | 'SHOPDATAITEM' | 'SHOPDATAPRICE' | 'SHOPDATASTRING':
            return 'shopdata'

        case (
            'SPRITE_CSET'
            | 'SPRITE_CURRENT_SCREEN'
            | 'SPRITE_CUSTOM_GRAVITY_STRENGTH'
            | 'SPRITE_CUSTOM_TERMINAL_VELOCITY'
            | 'SPRITE_DIR'
            | 'SPRITE_DRAW_STYLE'
            | 'SPRITE_DROWN_CLK'
            | 'SPRITE_DROWN_CMB'
            | 'SPRITE_ENGINE_ANIMATE'
            | 'SPRITE_EXTEND'
            | 'SPRITE_FAKE_JUMP'
            | 'SPRITE_FAKE_Z'
            | 'SPRITE_FALL_CLK'
            | 'SPRITE_FALL_CMB'
            | 'SPRITE_FLIP'
            | 'SPRITE_GRAVITY'
            | 'SPRITE_GRAVITY_STRENGTH'
            | 'SPRITE_HIT_HEIGHT'
            | 'SPRITE_HIT_OFFSET_X'
            | 'SPRITE_HIT_OFFSET_Y'
            | 'SPRITE_HIT_WIDTH'
            | 'SPRITE_HIT_ZHEIGHT'
            | 'SPRITE_JUMP'
            | 'SPRITE_LIGHT_RADIUS'
            | 'SPRITE_LIGHT_SHAPE'
            | 'SPRITE_MISCD'
            | 'SPRITE_MOVE_FLAGS'
            | 'SPRITE_ROTATION'
            | 'SPRITE_SCALE'
            | 'SPRITE_SCRIPT_FLIP'
            | 'SPRITE_SCRIPT_TILE'
            | 'SPRITE_SHADOW_SPR'
            | 'SPRITE_SHADOW_XOFS'
            | 'SPRITE_SHADOW_YOFS'
            | 'SPRITE_SPAWN_SCREEN'
            | 'SPRITE_SWHOOKED'
            | 'SPRITE_TERMINAL_VELOCITY'
            | 'SPRITE_TILE'
            | 'SPRITE_TILE_H'
            | 'SPRITE_TILE_W'
            | 'SPRITE_X'
            | 'SPRITE_X_OFFSET'
            | 'SPRITE_Y'
            | 'SPRITE_Y_OFFSET'
            | 'SPRITE_Z'
            | 'SPRITE_Z_OFFSET'
        ):
            return 'sprite'

        case (
            'SPRITEDATACSETS'
            | 'SPRITEDATAFLAGS'
            | 'SPRITEDATAFLCSET'
            | 'SPRITEDATAFRAMES'
            | 'SPRITEDATAID'
            | 'SPRITEDATAMISC'
            | 'SPRITEDATASPEED'
            | 'SPRITEDATATILE'
            | 'SPRITEDATATYPE'
        ):
            return 'spritedata'

        case 'STACKFULL' | 'STACKSIZE':
            return 'stack'

        case (
            'SUBDATABTNLEFT'
            | 'SUBDATABTNRIGHT'
            | 'SUBDATACURPG'
            | 'SUBDATACURSORPOS'
            | 'SUBDATAFLAGS'
            | 'SUBDATAINITD'
            | 'SUBDATANUMPG'
            | 'SUBDATAPAGES'
            | 'SUBDATASCRIPT'
            | 'SUBDATASELECTORASPD'
            | 'SUBDATASELECTORCSET'
            | 'SUBDATASELECTORDELAY'
            | 'SUBDATASELECTORDSTH'
            | 'SUBDATASELECTORDSTW'
            | 'SUBDATASELECTORDSTX'
            | 'SUBDATASELECTORDSTY'
            | 'SUBDATASELECTORFLASHCSET'
            | 'SUBDATASELECTORFRM'
            | 'SUBDATASELECTORHEI'
            | 'SUBDATASELECTORTILE'
            | 'SUBDATASELECTORWID'
            | 'SUBDATATRANSARGS'
            | 'SUBDATATRANSCLK'
            | 'SUBDATATRANSFLAGS'
            | 'SUBDATATRANSFROMPG'
            | 'SUBDATATRANSLEFTARGS'
            | 'SUBDATATRANSLEFTFLAGS'
            | 'SUBDATATRANSLEFTSFX'
            | 'SUBDATATRANSLEFTTY'
            | 'SUBDATATRANSRIGHTARGS'
            | 'SUBDATATRANSRIGHTFLAGS'
            | 'SUBDATATRANSRIGHTSFX'
            | 'SUBDATATRANSRIGHTTY'
            | 'SUBDATATRANSTOPG'
            | 'SUBDATATRANSTY'
            | 'SUBDATATYPE'
        ):
            return 'subscreendata'

        case (
            'SUBPGCURSORPOS'
            | 'SUBPGINDEX'
            | 'SUBPGNUMWIDG'
            | 'SUBPGSUBDATA'
            | 'SUBPGWIDGETS'
        ):
            return 'subscreenpage'

        case (
            'SUBWIDGBTNPG'
            | 'SUBWIDGBTNPRESS'
            | 'SUBWIDGDISPITM'
            | 'SUBWIDGEQPITM'
            | 'SUBWIDGFLAG'
            | 'SUBWIDGGENFLAG'
            | 'SUBWIDGH'
            | 'SUBWIDGINDEX'
            | 'SUBWIDGPAGE'
            | 'SUBWIDGPGMODE'
            | 'SUBWIDGPGTARG'
            | 'SUBWIDGPOS'
            | 'SUBWIDGPOSES'
            | 'SUBWIDGPOSFLAG'
            | 'SUBWIDGPRESSINITD'
            | 'SUBWIDGPRESSSCRIPT'
            | 'SUBWIDGREQCOUNTER'
            | 'SUBWIDGREQCOUNTERCOND'
            | 'SUBWIDGREQCOUNTERVAL'
            | 'SUBWIDGREQLITEMLEVEL'
            | 'SUBWIDGREQLITEMS'
            | 'SUBWIDGREQOWNITEMS'
            | 'SUBWIDGREQSCRIPTDISABLED'
            | 'SUBWIDGREQUNOWNITEMS'
            | 'SUBWIDGREQ_DMAP_FLOOR'
            | 'SUBWIDGREQ_LEVEL_STATE_LEVEL'
            | 'SUBWIDGREQ_SCRSTATE_MAP'
            | 'SUBWIDGREQ_SCRSTATE_SCREEN'
            | 'SUBWIDGREQ_SCRSTATE_STATE'
            | 'SUBWIDGREQ_SCRSTATE_EXSTATE'
            | 'SUBWIDGREQ_LEVEL_STATE'
            | 'SUBWIDGREQ_DMAP_LEVEL'
            | 'SUBWIDGREQ_DMAP'
            | 'SUBWIDGREQ_MAP'
            | 'SUBWIDGREQ_SCREEN'
            | 'SUBWIDGSELECTORASPD'
            | 'SUBWIDGSELECTORCSET'
            | 'SUBWIDGSELECTORDELAY'
            | 'SUBWIDGSELECTORDSTH'
            | 'SUBWIDGSELECTORDSTW'
            | 'SUBWIDGSELECTORDSTX'
            | 'SUBWIDGSELECTORDSTY'
            | 'SUBWIDGSELECTORFLASHCSET'
            | 'SUBWIDGSELECTORFRM'
            | 'SUBWIDGSELECTORHEI'
            | 'SUBWIDGSELECTORTILE'
            | 'SUBWIDGSELECTORWID'
            | 'SUBWIDGTRANSPGARGS'
            | 'SUBWIDGTRANSPGFLAGS'
            | 'SUBWIDGTRANSPGSFX'
            | 'SUBWIDGTRANSPGTY'
            | 'SUBWIDGTYPE'
            | 'SUBWIDGTY_ALIGN'
            | 'SUBWIDGTY_ANIMVAL'
            | 'SUBWIDGTY_BUTTON'
            | 'SUBWIDGTY_COLOR_BG'
            | 'SUBWIDGTY_COLOR_BG2'
            | 'SUBWIDGTY_COLOR_VISITED'
            | 'SUBWIDGTY_COLOR_CMPBLNK'
            | 'SUBWIDGTY_COLOR_CMPOFF'
            | 'SUBWIDGTY_COLOR_FILL'
            | 'SUBWIDGTY_COLOR_OLINE'
            | 'SUBWIDGTY_COLOR_PLAYER'
            | 'SUBWIDGTY_COLOR_ROOM'
            | 'SUBWIDGTY_COLOR_SHD'
            | 'SUBWIDGTY_COLOR_SHD2'
            | 'SUBWIDGTY_COLOR_TXT'
            | 'SUBWIDGTY_COLOR_TXT2'
            | 'SUBWIDGTY_CONTAINER'
            | 'SUBWIDGTY_CORNER'
            | 'SUBWIDGTY_COSTIND'
            | 'SUBWIDGTY_COUNTERS'
            | 'SUBWIDGTY_CSET'
            | 'SUBWIDGTY_DELAY'
            | 'SUBWIDGTY_FLIP'
            | 'SUBWIDGTY_FONT'
            | 'SUBWIDGTY_FRAMECSET'
            | 'SUBWIDGTY_FRAMES'
            | 'SUBWIDGTY_FRAMETILE'
            | 'SUBWIDGTY_GAUGE_HEI'
            | 'SUBWIDGTY_GAUGE_WID'
            | 'SUBWIDGTY_GRIDX'
            | 'SUBWIDGTY_GRIDY'
            | 'SUBWIDGTY_HSPACE'
            | 'SUBWIDGTY_INFCHAR'
            | 'SUBWIDGTY_INFITM'
            | 'SUBWIDGTY_ITEMCLASS'
            | 'SUBWIDGTY_ITEMID'
            | 'SUBWIDGTY_LITEMS'
            | 'SUBWIDGTY_MAXDIG'
            | 'SUBWIDGTY_MINDIG'
            | 'SUBWIDGTY_NUMBER'
            | 'SUBWIDGTY_PERCONTAINER'
            | 'SUBWIDGTY_PIECECSET'
            | 'SUBWIDGTY_PIECETILE'
            | 'SUBWIDGTY_SHADOWTY'
            | 'SUBWIDGTY_SHOWDRAIN'
            | 'SUBWIDGTY_SPEED'
            | 'SUBWIDGTY_TABSIZE'
            | 'SUBWIDGTY_TILE'
            | 'SUBWIDGTY_TOTAL'
            | 'SUBWIDGTY_UNITS'
            | 'SUBWIDGTY_VSPACE'
            | 'SUBWIDGW'
            | 'SUBWIDGX'
            | 'SUBWIDGY'
            | 'SUBWIDG_DISPH'
            | 'SUBWIDG_DISPW'
            | 'SUBWIDG_DISPX'
            | 'SUBWIDG_DISPY'
        ):
            return 'subscreenwidget'

        case (
            'SAVEMENU_CURSORTILE'
            | 'SAVEMENU_CURSORCSET'
            | 'SAVEMENU_BGCOLOR'
            | 'SAVEMENU_SFX_CURSOR'
            | 'SAVEMENU_SFX_CHOOSE'
            | 'SAVEMENU_HSPACE'
            | 'SAVEMENU_VSPACE'
            | 'SAVEMENU_OPTX'
            | 'SAVEMENU_OPTY'
            | 'SAVEMENU_TEXT_ALIGN'
            | 'SAVEMENU_TEXT_BOX_ALIGN'
            | 'SAVEMENU_CLOSE_FRAMES'
            | 'SAVEMENU_CLOSE_FLASH_RATE'
            | 'SAVEMENU_MUSIC'
            | 'SAVEMENU_MIDI'
            | 'SAVEMENU_NUM_OPTIONS'
            | 'SAVEMENU_FLAGS'
            | 'SAVEMENU_OPT_FLAGS'
            | 'SAVEMENU_OPT_COLOR_TEXT'
            | 'SAVEMENU_OPT_COLOR_PICKED'
            | 'SAVEMENU_OPT_FONTS'
            | 'SAVEMENU_OPT_FRZSCR'
            | 'SAVEMENU_ID'
            | 'SAVEMENU_BGTILE'
            | 'SAVEMENU_BGCSET'
            | 'SAVEMENU_BGTW'
            | 'SAVEMENU_BGTH'
        ):
            return 'save_menu'

        case (
            'MUSICDATA_ID'
            | 'MUSICDATA_MIDI'
            | 'MUSICDATA_IS_ENHANCED'
            | 'MUSICDATA_TRACK'
            | 'MUSICDATA_LOOPSTART'
            | 'MUSICDATA_LOOPEND'
            | 'MUSICDATA_XFADEIN'
            | 'MUSICDATA_XFADEOUT'
            | 'MUSICDATA_IS_ACTIVE'
        ):
            return 'music'

        case (
            'WEBSOCKET_HAS_MESSAGE'
            | 'WEBSOCKET_MESSAGE_TYPE'
            | 'WEBSOCKET_STATE'
            | 'WEBSOCKET_URL'
        ):
            return 'websocket'

    if register_name in [
        'SIMULATEKEYPRESS',
    ]:
        return 'input'

    if register_name in [
        'SCRIPTRAM',
        'GLOBALRAM',
    ]:
        return 'game'

    if register_name.startswith('SCREEN'):
        return 'screendata'
    if register_name.startswith('HERO'):
        return 'hero'
    if register_name.startswith('LINK'):
        return 'hero'
    if register_name.startswith('MAPDATA'):
        return 'mapdata'

    print(f'? {register_name}')
    return "misc"


# ==========================================
# Parsing Logic
# ==========================================
def parse_switch_cases(filepath):
    if not os.path.exists(filepath):
        print(f"Warning: {filepath} not found. Skipping.")
        return []

    with open(filepath, 'r') as f:
        lines = f.readlines()

    # Added lowercase a-z just in case your registers aren't strictly uppercase
    case_pattern = re.compile(r'^\s*case\s+([a-zA-Z0-9_]+)\s*:')

    parsed_blocks = []
    current_registers = []
    current_body = []

    brace_depth = 0
    base_depth = None  # We will dynamically determine the top-level depth

    for line in lines:
        # Strip comments so we don't count braces inside them
        clean_line = line.split('//')[0].split('/*')[0]

        # If there are closing braces on this line before a case, process them first
        # (Though rare for a case statement to share a line with a closing brace)

        match = case_pattern.match(line)

        if match:
            # Lock in the base depth on the very first case we find
            if base_depth is None:
                base_depth = brace_depth

            # Only treat it as a new block if we are at the locked top-level depth
            if brace_depth == base_depth:
                if current_registers and any(
                    l.strip() and not l.strip().startswith('//') for l in current_body
                ):
                    parsed_blocks.append(
                        {'registers': current_registers, 'body': "".join(current_body)}
                    )
                    current_registers = []
                    current_body = []

                current_registers.append(match.group(1))
                current_body.append(line)
            else:
                # This is a nested case
                if current_registers:
                    current_body.append(line)
        else:
            # Not a case line
            if current_registers:
                current_body.append(line)

        # Update brace depth *after* evaluating the line so the opening
        # brace of a nested block doesn't incorrectly shift the depth of the `case` line itself
        brace_depth += clean_line.count('{')
        brace_depth -= clean_line.count('}')

    # Append the final block
    if current_registers:
        parsed_blocks.append(
            {'registers': current_registers, 'body': "".join(current_body)}
        )

    for b in parsed_blocks:
        assert len(b['registers']) == 1

    parsed_blocks.sort(key=lambda x: x['registers'][0])

    for b in parsed_blocks:
        if 'Dropset Variables' in b['body']:
            print(b)

    return parsed_blocks


# ==========================================
# Generation Logic
# ==========================================
def format_cases(blocks, tabs=2):
    output = ""
    base_indent = "\t" * tabs

    for block in blocks:
        raw_lines = block['body'].splitlines()

        # Remove trailing empty lines AND trailing comment-only lines
        while raw_lines:
            last_line = raw_lines[-1].strip()
            if not last_line or last_line.startswith('//'):
                raw_lines.pop()
            else:
                break

        if not raw_lines:
            continue

        # Grab the exact leading whitespace of the 'case' line to act as our baseline
        first_line = raw_lines[0]
        baseline_ws = first_line[: len(first_line) - len(first_line.lstrip())]

        for line in raw_lines:
            if not line.strip():
                output += "\n"
                continue

            # Strip only the baseline whitespace, preserving relative internal indentation
            if line.startswith(baseline_ws):
                cleaned = line[len(baseline_ws) :]
            else:
                cleaned = line.lstrip()

            cleaned = cleaned.replace("    ", "\t")
            output += f"{base_indent}{cleaned}\n"

    return output.rstrip()


def generate_cpp_files():
    # 1. Parse both files
    get_blocks = parse_switch_cases(GET_FILE)
    set_blocks = parse_switch_cases(SET_FILE)

    # 2. Group by subsystem
    subsystem_get_map = defaultdict(list)
    subsystem_set_map = defaultdict(list)

    for block in get_blocks:
        # Use the first register to determine the subsystem for the whole block
        sub = get_subsystem(block['registers'][0])
        subsystem_get_map[sub].append(block)

    for block in set_blocks:
        sub = get_subsystem(block['registers'][0])
        subsystem_set_map[sub].append(block)

    all_subsystems = set(subsystem_get_map.keys()).union(subsystem_set_map.keys())

    # 3. Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # 4. Generate files
    for sub in all_subsystems:
        get_code = format_cases(subsystem_get_map.get(sub, []), tabs=2)
        set_code = format_cases(subsystem_set_map.get(sub, []), tabs=2)

        # C++ Template formatted strictly with tabs and braces on separate lines
        cpp_content = f"""#include "zc/scripting/types/{sub}.h"\n
#include "base/check.h"
#include "components/zasm/defines.h"

int32_t {sub}_get_register(int32_t reg)
{{
\tint32_t ret = 0;

\tswitch (reg)
\t{{
{get_code}

\t\tdefault:
\t\t\tNOTREACHED();
\t}}

\treturn ret;
}}

void {sub}_set_register(int32_t reg, int32_t value)
{{
\tswitch (reg)
\t{{
{set_code}

\t\tdefault:
\t\t\tNOTREACHED();
\t}}
}}
"""
        out_path = os.path.join(OUTPUT_DIR, f"{sub}-TEMP.cpp")
        with open(out_path, "w") as f:
            f.write(cpp_content)

        print(f"Generated: {out_path}")

        header = (
            f'''
#ifndef ZC_SCRIPTING_{sub.upper()}_H_
#define ZC_SCRIPTING_{sub.upper()}_H_

#include <cstdint>

int32_t {sub}_get_register(int32_t reg);
void {sub}_set_register(int32_t reg, int32_t value);

#endif
        '''.strip()
            + '\n'
        )
        out_path = Path(os.path.join(OUTPUT_DIR, f"{sub}.h"))
        if not out_path.exists():
            out_path.write_text(header)
            print(f"Generated: {out_path}")


def write_register_routing_table_switch():
    paths = list(Path('src/zc/scripting/types').glob('*.cpp'))
    paths.sort()
    cases = []
    for path in paths:
        content = path.read_text()
        content = content.split('_run_command')[0]
        matches = re.findall(r'case ([A-Z].*):', content)
        matches.extend(re.findall(r'if\s?\(reg == ([A-Z].*)\)', content))
        matches = list(set(matches))
        matches.sort()
        if not matches:
            continue

        for m in matches:
            cases.append(f'\t\tcase {m}:')
        cases.append(
            f'\t\t\treturn EngineSubsystem::{path.name.replace('.cpp', '')};\n'
        )

    code = (
        '''// Generated by scripts/refactor_ffscript.py.

#include "zc/scripting/types.h"
#include "components/zasm/defines.h"
#include <array>

namespace {

constexpr EngineSubsystem getEngineSubsystemForRegister(int reg)
{
	switch (reg)
	{
'''
        + '\n'.join(cases)
        + '''
	}

	return EngineSubsystem::none;
}

} // end namespace

extern const std::array<EngineSubsystem, MAX_REGISTER_ID + 1> register_routing_table = []() constexpr {
	std::array<EngineSubsystem, MAX_REGISTER_ID + 1> data{};

	for (int i = 0; i <= MAX_REGISTER_ID; i++)
		data[i] = getEngineSubsystemForRegister(i);

	return data;
}();
    '''.rstrip()
        + '\n'
    )

    Path('src/zc/scripting/types.cpp').write_text(code)


if __name__ == "__main__":
    # generate_cpp_files()
    write_register_routing_table_switch()
