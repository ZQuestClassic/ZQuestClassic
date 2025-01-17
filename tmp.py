# turn the 100s of defines in std_constants.zh into enums

import re

from pathlib import Path

path = Path('resources/include/std_zh/std_constants.zh')

# TODO ! ctl+f "in std_constants.zh", delete.
# TODO ! ctl+f "<std>", delete.
# TODO ! ctl+f "constants", delete.
# TODO ! ctl+f "TODO: document values", delete.
# TODO ! ctl+f "To be used", delete.

# TODO ! 
#    CMB_QUADRANT - Screen->ComboS[]

# TODO ! rm:
#    ..

name_map = {
    'AF': 'AnimationFlag',
    'AT': 'AimType',
    'BITDX': 'BitmapDrawBitflags',
    'BLOCKFLAG': 'BlockBitflagsInt',
    'CB': 'ControllerButton',
    'CF': 'ComboFlag',
    'CHRT': 'DMapChartedBitflags',
    'CIID': 'CurrentItemIDFlag',
    'CMB': 'ComboSolidityBitflags',
    'D': 'DoorType',
    'DIR2': 'Dir2',
    'DMF': 'DMapBitflags',
    'DOOR': 'DoorIndex',
    'EW': 'EWeaponType',
    'EXT': 'ExtendMode',
    'FFCBF': 'FFCBitflags',
    'FFCF': 'FFCFlag',
    'FONT': 'FontType',
    'GOS': 'GameOverScreenElementIndex',
    'I': 'ItemID',
    'IMISC': 'ItemMiscBitflags',
    'IP': 'ItemPickupBitflags',
    'IPSTR': 'PickupStringBitflags',
    'ITM': 'ItemRequireBitflags',
    'L': 'BlockBitflags',
    'LI': 'LevelItemBitflags',
    'LW': 'LWeaponType',
    'MOUSE': 'MouseIndex',
    'NOCARRY': 'NoCarryBitflags',
    'NORESET': 'NoResetBitflags',
    'NPC': 'EnemyID',
    'NPCMF': 'EnemyMiscBitflags',
    'NPCSF': 'EnemySpawnFlags',
    'PT': 'PolygonRenderingMode',
    'RT': 'RenderTarget',
    'RTC': 'RealTimeClockCategory',
    'SEF': 'ScreenEnemyFlagIndex',
    'SFX': 'SoundEffect',
    'SL': 'SpriteList',
    'SRAM': 'StoredMemoryBitflags',
    'SUBSEL': 'MoveSubscreenSelectorBitflags',
    'SZFLAG': 'SizeOverrideBitflags',
    'TF': 'TextFormatAlign',
    'UNBLOCK': 'UnblockableBitflags',
    'VOL': 'VolumeIndex',
    'WARP': 'WarpBitflags',
    'WARPEFFECT': 'WarpEffect',
    'HIT': 'HitIndex',

    # TODO !
    'INTBTN': '', # ??
    'MB': 'MouseButtonIndex', # naming ??? drop index?? maybe yes..
    'SECCMB': 'SecretComboIndex',  # naming ??? drop index?? maybe yes..
    'SP': 'SpriteID', # SpriteType?
}

current_comment = ''
enum_top_comment = {}
enums = {}
lines_to_delete = []

for line in path.read_text().splitlines():
    if line.startswith('//'):
        current_comment += line[2:] + '\n'
        continue

    if not line.lower().startswith('define'):
        current_comment = ''
        continue

    result = re.search(r'[a-zA-Z\d]+_[a-zA-Z\d_]+', line)
    if not result:
        current_comment = ''
        continue

    name = result.group(0)
    prefix = name.split('_')[0]

    if prefix == 'FONT' and name.endswith('_HEIGHT'):
        current_comment = ''
        continue

    if name.endswith('_SIZE'):
        current_comment = ''
        continue

    if prefix in ['NUM', 'MIN', 'MAX', 'TERMINAL', 'SCREEN', 'INIT', 'MISC', 'HP', 'MP', 'SQRT', 'RAD', 'DEG', 'SUBSCREEN', 'DIR16']:
        current_comment = ''
        continue

    # gonna delete
    if prefix in ['CSET']:
        current_comment = ''
        continue

    # confusing... ignore.
    if prefix in ['NPCA', 'BIT']:
        current_comment = ''
        continue

    if prefix not in enums:
        enums[prefix] = []
        if current_comment:
            enum_top_comment[prefix] = current_comment.strip()
            if not enum_top_comment[prefix].endswith('.'):
                enum_top_comment[prefix] += '.'
            current_comment = ''

    value = re.search(r'=(.+);', line).group(1).strip()

    m = re.search(r';.*//(.+)', line)
    if m:
        comment = m.group(1).strip()
        if not comment.endswith('.'):
            comment += '.'
    else:
        comment = None

    enums[prefix].append((name, value, comment))
    lines_to_delete.append(line)

for enum_name, pairs in enums.items():
    if len(pairs) == 1:
        continue

    top_comment = enum_top_comment.get(enum_name, '')
    name = name_map.get(enum_name, enum_name) or enum_name

    lines = []
    if top_comment:
        comment_lines = [f'// {l}' for l in top_comment.splitlines()]
        lines.extend(comment_lines)
    lines.append(f'enum {name}')
    lines.append('{')

    max_val_len = max(len(p[1]) for p in pairs)

    for name, value, comment in pairs:
        lines.append(f'\t{name:30} = {value},')
        if comment:
            lines[-1] += (max_val_len-len(value))*' ' + f' // {comment}'
            
    lines.append('}')

    print('\n'.join(lines))
    print()

# TODO: delete stuff.
# lines_to_delete
