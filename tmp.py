# turn the 100s of defines in std_constants.zh into enums

import re

from pathlib import Path

path = Path('resources/include/std_zh/std_constants.zh')

# TODO ! ctl+f "in std_constants.zh", delete.
# TODO ! ctl+f "<std>", delete.
# TODO ! ctl+f "constants", delete.
# TODO ! ctl+f "using the ", delete.
# TODO ! ctl+f "TODO: document values", delete.
# TODO ! ctl+f "To be used", delete.

# TODO !
#    CMB_QUADRANT - Screen->ComboS[]

name_map = {
    'AF': 'AnimationBitflags',
    'AT': 'AimType',
    'BITDX': 'BlitModeBitflags',
    'BLOCKFLAG': 'BlockBitflagsInt',
    'CB': 'ButtonIndex',
    'CF': 'ComboFlag',
    'CHRT': 'DMapChartedBitflags',
    'CIID': 'CurrentItemIDBitflags',
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
    'GOSTR': 'GameOverStringIndex',
    'HIT': 'HitIndex',
    'I': 'ItemID',
    'IMISC': 'ItemMiscBitflags',
    'IP': 'ItemPickupBitflags',
    'IPSTR': 'PickupStringBitflags',
    'ITM': 'ItemRequireBitflags',
    'L': 'BlockBitflags',
    'LI': 'LevelItemBitflags',
    'LW': 'LWeaponType',
    'MB': 'MouseButtonBitflags',
    'MOUSE': 'MouseIndex',
    'NOCARRY': 'NoCarryBitflags',
    'NORESET': 'NoResetBitflags',
    'NPC': 'EnemyID',
    'NPCMF': 'EnemyMiscBitflags',
    'NPCSF': 'EnemySpawnBitflags',
    'PT': 'PolygonRenderingMode',  # PolygonRenderMode ?
    'RT': 'RoomType',
    'RTC': 'RealTimeClockCategory',
    'SECCMB': 'SecretComboIndex',
    'SEF': 'ScreenEnemyFlagIndex',
    'SFX': 'SoundEffect',
    'SL': 'SpriteList',
    'SP': 'SpriteID',
    'SRAM': 'StoredMemoryBitflags',
    'SUBSEL': 'MoveSubscreenSelectorBitflags',
    'SZFLAG': 'SizeOverrideBitflags',
    'TF': 'TextFormatAlign',
    'UNBLOCK': 'UnblockableBitflags',
    'VOL': 'VolumeIndex',
    'WARP': 'WarpBitflags',
    'WARPEFFECT': 'WarpEffect',
}


def normalize_comment(comment: str):
    comment = comment.strip()
    comment = f'{comment[0].upper()}{comment[1:]}'
    if not comment.endswith('.') and not comment.endswith('!'):
        comment += '.'
    return comment


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

    if prefix in [
        'DEG',
        'DIR16',
        'HP',
        'INIT',
        'MAX',
        'MIN',
        'MISC',
        'MP',
        'NUM',
        'RAD',
        'SCREEN',
        'SFW',
        'SQRT',
        'SUBSCREEN',
        'TERMINAL',
    ]:
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
            enum_top_comment[prefix] = normalize_comment(current_comment)
            current_comment = ''

    value = re.search(r'=(.+);', line).group(1).strip()

    m = re.search(r';.*//(.+)', line)
    if m:
        comment = normalize_comment(m.group(1))
    else:
        comment = None

    enums[prefix].append((name, value, comment))
    lines_to_delete.append(line)

for enum_name, pairs in enums.items():
    if len(pairs) == 1:
        continue

    top_comment = enum_top_comment.get(enum_name, '')
    name = name_map.get(enum_name, enum_name) or enum_name

    if enum_name not in name_map:
        print(f'ERROR! missing in name map: {enum_name}')
        exit(1)

    lines = []
    if top_comment:
        comment_lines = [f'// {l.strip()}' for l in top_comment.splitlines()]
        lines.extend(comment_lines)
    lines.append(f'enum {name}')
    lines.append('{')

    max_val_len = max(len(p[1]) for p in pairs)

    for name, value, comment in pairs:
        lines.append(f'\t{name:30} = {value},')
        if comment:
            lines[-1] += (max_val_len - len(value)) * ' ' + f' // {comment}'

    lines.append('}')

    print('\n'.join(lines))
    print()

# TODO: delete stuff.
# lines_to_delete
