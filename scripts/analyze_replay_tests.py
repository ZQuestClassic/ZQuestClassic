# python -m pip install git+ssh://git@github.com/connorjclark/zquest-data.git@2c276f

import logging
import os
from pathlib import Path
from zquest.extract import ZeldaClassicReader
from zquest import constants

logging.getLogger('zquest').setLevel(logging.FATAL)

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent

qsts = [
    root_dir / 'resources/modules/classic/classic_1st.qst',
    *(root_dir / 'tests/replays').rglob('*.qst'),
]

counts = {qr: 0 for qr in constants.quest_rules}
for qst in qsts:
    reader = ZeldaClassicReader(str(qst))
    reader.read_qst()
    qrs = reader.get_quest_rules().get_values()
    print(f'{qst.name}: {len(qrs)} rules')
    for qr in qrs:
        counts[qr] += 1

print('')
covered_count = len([x for x in counts.values() if x > 0])
print(f'covered:  {covered_count} / {len(constants.quest_rules)}')
print('')

for qr, count in sorted(counts.items(), key=lambda x: -x[1]):
    print(qr, count)
