# This is to help provide comments and named arguments to bindings.zh generation
# TODO remove this after binding work is done

import html
import json
import os
import re

from pathlib import Path

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))

data = json.loads((script_dir / 'zsdocs_main.json').read_text())

classes = [
    'audio',
    'bitmap',
    'bottledata',
    'bottleshopdata',
    'combodata',
    'debug',
    'directory',
    'dmapdata',
    'dropsetdata',
    'eweapon',
    'ffc',
    'file',
    'filesystem',
    'game',
    'genericdata',
    'global',
    'graphics',
    'input',
    'item',
    'itemdata',
    'itemsprite',
    'lweapon',
    'mapdata',
    'messagedata',
    'npc',
    'npcdata',
    'paldata',
    'player',
    'portal',
    'randgen',
    'region',
    'savedportal',
    'screen',
    'screendata',
    'shopdata',
    'spritedata',
    'stack',
    'subscreendata',
    'subscreenpage',
    'subscreenwidget',
    'text',
    'websocket',
    'zinfo',
    'weapon',
]

sheets = []
for sheet in data['sheets']:
    cname = sheet['name'].lower()
    if cname in classes:
        if cname == 'global':
            cname = ''
        sheets.append((cname, sheet))

entries = []
for cname, sheet in sheets:
    for tab in sheet['tabs']:
        for line in tab.values():
            if isinstance(line, list):
                for entry in line:
                    if '-' in entry['name']:
                        continue
                    if entry['val'].startswith('$'):
                        continue
                    entries.append((cname, entry['val']))


def get_symbol_name(cname, sname):
    sname = re.sub(r'\[.*\]', '[]', sname)
    if cname == 'screen':
        cname = 'screendata'

    if cname and cname != 'global':
        return f'{cname}->{sname}'
    return sname


symbols = []


def process_entry(cname, entry):
    if not entry.startswith('<h3>'):
        return

    entry = entry.replace('[[filename|.zcsram]]', 'filename')

    entry = entry.replace('${constexpr} ', '')
    entry = entry.replace('...untyped', '')

    lines = entry.splitlines()
    symbols_pending_comments = []
    collecting_sigs = False

    for line in lines:
        if not line:
            continue
        if 'Pointers</h3>' in line:
            continue
        if line == '<h3>Note</h3>':
            break

        if line.startswith('<h3>') or collecting_sigs:
            collecting_sigs = True
            sig_text = line.replace(';', '').replace('<h3>', '').replace('</h3>', '')

            tokens = re.split(r'[ ,()]+', sig_text)
            tokens = [t for t in tokens if t]
            name = get_symbol_name(cname, tokens[1])

            if '(' not in sig_text:
                # print(line)
                symbol = {'name': name, 'comment': ''}
                symbols.append(symbol)
                symbols_pending_comments.append(symbol)
            else:
                # multiline sigs...
                if ')' not in sig_text:
                    break

                parens_text = sig_text[sig_text.index('(') + 1 : sig_text.index(')')]

                def filter_param(p: str):
                    p = p.split('=')[0].strip().split(' ')[1]
                    p = re.sub(r'\[.*\]', '', p)
                    return p

                params = [filter_param(p) for p in parens_text.split(',') if p]

                # TODO: mark params invalid for now. see Min/Max/Choose
                if any(p for p in params if '|' in p):
                    params = None
                symbol = {'name': name, 'params': params, 'comment': ''}
                symbols.append(symbol)
                symbols_pending_comments.append(symbol)
        else:
            comment = (
                html.unescape(line)
                .replace('<em>', '')
                .replace('</em>', '')
                .replace('<sub>', '\_')
                .replace('</sub>', '')
                .replace('<sup>', '**')
                .replace('</sup>', '')
                .replace('Read/Write;', '')
                .strip()
            )
            for symbol in symbols_pending_comments:
                symbol['comment'] = comment
            symbols_pending_comments = []

        if line.endswith('</h3>'):
            collecting_sigs = False


for cname, entry in entries:
    try:
        process_entry(cname, entry)
    except Exception as e:
        print('ERROR', cname, e)


def starts_with_symbol_decl(token: str):
    if '=' in token:
        if '(' not in token:
            return False

        a = token.index('=')
        b = token.index('(')
        if a < b:
            return False

    m = re.match(r'^\s*(\w+)(\[\])? ', line)
    if not m:
        return False
    if m.group(1) in classes:
        return True
    if m.group(1) in [
        'bool',
        'char32',
        'constexpr',
        'float',
        'int',
        'long',
        'rgb',
        'untyped',
        'untyped',
        'void',
        'void',
    ]:
        return True
    return False


def filter_param(p: str):
    if '...' in p:
        return None

    if '=' in p:
        p = p.split('=')[0]
    p = p.strip()

    p = p.split(' ')[1].strip()
    p = re.sub(r'\[.*\]', '', p)
    return p


def find_symbol(name, params=None):
    pl = len(params) if params else 0
    return next(
        (
            s
            for s in symbols
            if s['name'] == name
            and (
                (pl == (len(s['params']) if 'params' in s and s['params'] else 0))
                or 'params' in s
                and s['params'] == None
            )
        ),
        None,
    )


def process_entry_2(section, symbol_texts, comment):
    symbols_cleaned = []
    for txt in symbol_texts:
        if '(' in txt:
            name = get_symbol_name(
                section,
                txt.replace('constexpr ', '').split('(')[0].split(' ')[1].strip(),
            )
            parens_text = txt[txt.index('(') + 1 : txt.index(')')]
            params = [filter_param(p) for p in parens_text.split(',') if p]
            existing = find_symbol(name, params)
            params = [p for p in params if p]
            if existing:
                if not existing['params'] and params:
                    existing['params'] = params
                if not existing['comment'] and comment:
                    existing['comment'] = comment
            else:
                symbols.append({'name': name, 'params': params, 'comment': comment})
        else:
            name = get_symbol_name(
                section,
                txt.replace(';', '').split(' ')[1].split('->')[-1].strip(),
            )
            existing = next((s for s in symbols if s['name'] == name), None)
            if existing:
                if not existing['comment'] and comment:
                    existing['comment'] = comment
            else:
                symbols.append({'name': name, 'comment': comment})


old_zscript = Path(script_dir / '../resources/docs/ZScript_Additions.txt').read_text()
old_zscript = old_zscript[old_zscript.index('//  Global  //') :]
cur_section = None
active_symbols = []
active_doc = ''
in_middle_of_sig = False
results = []
for line in old_zscript.splitlines():
    if re.match(r'^//(.+)//$', line):
        section = line.replace('/', '').replace('->', '').strip().lower()
        if section == '*weapon':
            section = 'weapon'
        if section == 'hero':
            section = 'player'
        if section in classes or section == 'global':
            cur_section = section
            active_symbols = []
            active_doc = ''
            in_middle_of_sig = False
            continue

    if in_middle_of_sig:
        active_symbols[-1] += ' ' + line
        if ')' in active_symbols[-1]:
            in_middle_of_sig = False
        continue

    line = line.strip()

    if re.match(r'^\s*\*.*', line):
        active_doc += line[1:].strip() + '\n'
    elif starts_with_symbol_decl(line):
        if active_doc:
            if active_symbols:
                process_entry_2(cur_section, active_symbols, active_doc.strip())
            active_symbols = []
            active_doc = ''

        if '(' in line and ')' not in line:
            in_middle_of_sig = True
        active_symbols.append(line)
    else:
        if active_symbols:
            process_entry_2(cur_section, active_symbols, active_doc.strip())
        active_symbols = []
        active_doc = ''


old_zscript = Path(script_dir / '../resources/docs/zscript.txt').read_text()
cur_section = None
for line in old_zscript.splitlines():
    if line.startswith('//---'):
        cur_section = line.replace('//---', '').strip().split(' ')[0].lower()
        # if 'Weapon Functions and Variables' in line:
        #     cur_section = 'weapon'
        continue

    if not starts_with_symbol_decl(line):
        continue

    m = re.match(r'\t(\w+) (\w+)\((.*)\)', line)
    if not m:
        continue

    name = get_symbol_name(cur_section, m[2])
    params_txt = m[3]
    if not m[3]:
        continue

    existing = next((s for s in symbols if s['name'] == name), None)
    if existing:
        continue

    params = []
    for param_txt in params_txt.strip().split(', '):
        t, param = param_txt.split(' ')
        params.append(param.replace('[]', ''))

    symbol = {'name': name, 'comment': '', 'params': params}
    symbols.append(symbol)


for symbol in symbols:
    cname = None
    if '->' in symbol['name']:
        cname = symbol['name'].split('->')[0]

    if 'params' in symbol and symbol['params']:
        for i, param in enumerate(symbol['params']):
            if param == 'loop':
                symbol['params'][i] = 'loop_sfx'

    if 'comment' in symbol:
        comment = symbol['comment']
        while True:
            m = re.search(r'\$\{([^|]+)(\|[^|]+)\}', comment)
            if not m:
                break

            if len(m.groups()) == 2:
                link = m.group(1)
                text = m.group(2)[1:]
            else:
                text = None
                link = m.group(1)

            if link == 'Input::button':
                link = 'Input::Button'
            if link == 'Input::press':
                link = 'Input::Press'
            if link == 'itemclass':
                link = 'itemdata'
            if link == 'bitmapptr':
                link = 'bitmap'

            is_valid_symbol = False
            if re.match(r'^[->a-zA-Z0-9:()\[\]]+$', link):
                symbol_name = (
                    link.replace('::', '->').replace('()', '').replace('[]', '')
                )
                if '->' in symbol_name:
                    l, r = symbol_name.split('->')
                    symbol_name = get_symbol_name(l.lower(), r)
                link_symbol = next(
                    (s for s in symbols if s['name'].replace('[]', '') == symbol_name),
                    None,
                )
                is_valid_symbol = link_symbol != None
                if symbol_name in classes:
                    is_valid_symbol = True
                if symbol_name in [
                    'screendata->FastTile',
                    'itemsprite->Pickup',
                    'lweapon->Animation',
                    'paldata->rgb',
                    'input->button',
                    'input->press',
                    'screendata->ComboS',
                ]:
                    is_valid_symbol = True
                if not is_valid_symbol:
                    print('REJECT', symbol_name, comment)

            if is_valid_symbol:
                if '::' in link:
                    l, r = link.split('::')
                    if l == 'Input':
                        l = 'input'
                    if l == 'Screen':
                        l = 'screendata'
                    if l == 'Graphics':
                        l = 'graphics'
                    if l == cname:
                        link = r
                link = link.replace('Screen::', 'Screen->')
                link = link.replace('Input::', 'Input->')
                link = link.replace('Screen::', 'Screen->')
                link = link.replace('Graphics::', 'Graphics->')
                link = link.replace('FFC::', 'ffc::')
                if text and text in ['Generic Script']:
                    doc_link = '{@link %s|%s}' % (link, text)
                else:
                    doc_link = f'[{link}]'
                # print(doc_link, comment)
            elif text:
                doc_link = text
            else:
                doc_link = link

            comment = comment[0 : m.start(0)] + doc_link + comment[m.end(0) :]

        comment = comment.replace('${message string}', 'message string')

        symbol['comment'] = comment

    if 'comment' in symbol:
        comment = symbol['comment']

        if 'VALID WIDGETS:' in comment:
            lines = comment.splitlines()
            idx = lines.index('VALID WIDGETS:')
            lines[idx + 2] = '- ' + '\n- '.join(lines[idx + 2].split(', '))
            comment = '\n'.join(lines).replace('VALID WIDGETS:', '')

        comment = comment.replace('READ-ONLY:', '')
        comment = comment.replace('Read-only. ', '')
        comment = comment.replace('Read-only; ', '')
        comment = comment.replace('Read-only: ', '')
        comment = comment.replace('Read-only, ', '')
        comment = comment.replace('Read-only', '')
        comment = comment.replace(', read-only', '')
        comment = comment.replace('Read-Only. ', '')
        comment = comment.replace('READ-ONLY', '')

        if comment == '?':
            comment = ''

        lines = comment.splitlines()
        in_code_block = False
        in_list = False
        comment = ''
        for line in lines:
            if not line.strip():
                continue

            if line == '```':
                in_code_block = not in_code_block

            in_list = line.startswith('-')
            comment += line
            if in_code_block or in_list:
                comment += '\n'
            else:
                comment += '\n\n'

        symbol['comment'] = comment.strip()


symbols.sort(key=lambda s: s['name'])

(script_dir / '../resources/zscript_symbols.json').write_text(
    json.dumps(symbols, indent=2)
)
