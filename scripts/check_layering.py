#!/usr/bin/env python3
"""Checks that #includes in src/ respect the module layering (see docs/project_structure.md).

A module is the first directory under src/ (core, zc, gui, ...), except that each component in
src/components is its own module. Files directly in src/ form the "src" module. The rules:

- Apps (zc, zq, parser, launcher, zupdater, standalone) must not include each other.
- Shared modules (core, dialog, and the top-level src/ files) must not include any app,
  and core must not include gui or dialog either.
- Low-level modules may only include the modules listed in LOW_LEVEL.
- Components must not include each other.
- Anything may include test_runner/ (for *_test.cpp files).

It also requires our own headers to be included with quotes, not angle brackets - except where
a file with the same name next to the including file would shadow it.

Existing violations are listed in EXCEPTIONS below. A violation not in that list fails the
check, and so does an exception that no longer exists - delete it, so the list only shrinks.

    python scripts/check_layering.py            # check
    python scripts/check_layering.py --update   # rewrite EXCEPTIONS to the current violations
"""

import argparse
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / 'src'

APPS = {'zc', 'zq', 'parser', 'launcher', 'zupdater', 'standalone'}
SHARED = {'core', 'dialog', 'src'}
# Modules each low-level module may include, besides itself.
LOW_LEVEL = {
    'base': set(),
    'zalleg': {'base'},
    'zconsole': {'base'},
    'metadata': {'base'},
    'al5_img': {'base'},
    'components': {'base', 'zalleg'},
    'gui': {'base', 'zalleg', 'core', 'components', 'src'},
}

# "file -> included file", both relative to src/.
EXCEPTIONS = """
advanced_music.cpp -> zc/zelda.h
advanced_music.cpp -> zq/zquest.h
base/process_management.cpp -> core/zdefs.h
base/util.cpp -> core/zdefs.h
base/zfix.cpp -> zc/replay.h
components/scc/scc.cpp -> core/flags.h
components/zasm/serialize.cpp -> core/zdefs.h
components/zasm/serialize.cpp -> parser/parserDefs.h
components/zasm/serialize.h -> core/zdefs.h
core/handles.cpp -> zc/combos.h
core/handles.cpp -> zc/maps.h
core/handles.cpp -> zc/zc_ffc.h
core/jwinfsel.h -> gui/jwin.h
core/qst.cpp -> dialog/info.h
core/qst.cpp -> zc/ffscript.h
core/qst_cheats.cpp -> zc/ffscript.h
core/qst_colors.cpp -> zc/ffscript.h
core/qst_comboaliases.cpp -> zc/ffscript.h
core/qst_combos.cpp -> zc/ffscript.h
core/qst_csets.cpp -> zc/ffscript.h
core/qst_dmaps.cpp -> zc/ffscript.h
core/qst_doors.cpp -> zc/ffscript.h
core/qst_favorites.cpp -> zc/ffscript.h
core/qst_ffscript.cpp -> zc/ffscript.h
core/qst_guys.cpp -> zc/ffscript.h
core/qst_header.cpp -> dialog/externs.h
core/qst_header.cpp -> zc/ffscript.h
core/qst_herosprites.cpp -> zc/ffscript.h
core/qst_icons.cpp -> zc/ffscript.h
core/qst_initdata.cpp -> zc/ffscript.h
core/qst_itemdropsets.cpp -> zc/ffscript.h
core/qst_items.cpp -> zc/ffscript.h
core/qst_maps.cpp -> zc/ffscript.h
core/qst_midis.cpp -> zc/ffscript.h
core/qst_misc.cpp -> zc/ffscript.h
core/qst_rules.cpp -> zc/ffscript.h
core/qst_sfx.cpp -> zc/ffscript.h
core/qst_strings.cpp -> zc/ffscript.h
core/qst_subscreen.cpp -> zc/ffscript.h
core/qst_tiles.cpp -> zc/ffscript.h
core/qst_weapons.cpp -> zc/ffscript.h
core/zdefs.cpp -> dialog/info.h
core/zdefs.cpp -> gui/jwin_a5.h
dialog/combo_trigger_editor.h -> zq/gui/cornerselect.h
dialog/combo_trigger_editor.h -> zq/gui/selcombo_swatch.h
dialog/combo_trigger_editor.h -> zq/gui/seltile_swatch.h
dialog/combo_trigger_editor.h -> zq/gui/tileanim_frame.h
dialog/comboeditor.h -> zq/gui/cornerselect.h
dialog/comboeditor.h -> zq/gui/selcombo_swatch.h
dialog/comboeditor.h -> zq/gui/seltile_swatch.h
dialog/comboeditor.h -> zq/gui/tileanim_frame.h
dialog/combopool.cpp -> zq/zq_tiles.h
dialog/combopool.h -> zq/gui/selcombo_swatch.h
dialog/combowizard.cpp -> zc/maps.h
dialog/common.cpp -> zq/zquest.h
dialog/compilesetting.cpp -> zc/ffscript.h
dialog/compilesetting.cpp -> zq/zquest.h
dialog/compilesetting.h -> parser/parserDefs.h
dialog/compilezscript.cpp -> zc/ffscript.h
dialog/control_binding.h -> zc/cheats.h
dialog/control_binding.h -> zc/control_scheme.h
dialog/control_schemes.h -> zc/control_scheme.h
dialog/door_sel.cpp -> zq/render.h
dialog/door_sel.cpp -> zq/zq_misc.h
dialog/door_sel.cpp -> zq/zquest.h
dialog/edit_dmap.cpp -> zq/zquest.h
dialog/edit_dmap.h -> zq/gui/dmap_mapgrid.h
dialog/edit_dmap.h -> zq/gui/dmap_minimap.h
dialog/edit_dmap.h -> zq/gui/regiongrid.h
dialog/edit_dmap.h -> zq/gui/selcombo_swatch.h
dialog/edit_music.cpp -> zq/zquest.h
dialog/edit_region.cpp -> zq/render.h
dialog/edit_region.cpp -> zq/zq_misc.h
dialog/edit_region.cpp -> zq/zquest.h
dialog/edit_region.h -> zq/gui/dmap_mapgrid.h
dialog/edit_region.h -> zq/gui/dmap_minimap.h
dialog/edit_region.h -> zq/gui/regiongrid.h
dialog/edit_region.h -> zq/gui/selcombo_swatch.h
dialog/enemyeditor.h -> zq/gui/seltile_swatch.h
dialog/ffc_editor.cpp -> zq/zquest.h
dialog/ffc_editor.h -> zq/gui/selcombo_swatch.h
dialog/heroeditor.h -> zq/gui/selherotile_swatch.h
dialog/info.cpp -> zq/zq_files.h
dialog/info_lister.cpp -> zq/zq_misc.h
dialog/info_lister.cpp -> zq/zquest.h
dialog/info_lister.h -> zq/gui/dmap_frame.h
dialog/info_lister.h -> zq/gui/tileanim_frame.h
dialog/init_data.cpp -> zc/ffscript.h
dialog/itemeditor.h -> zq/gui/tileanim_frame.h
dialog/layer_dialog.cpp -> zq/zquest.h
dialog/mapstyles.h -> zq/gui/seltile_swatch.h
dialog/midieditor.cpp -> zq/zquest.h
dialog/object_pages.cpp -> zq/zquest.h
dialog/paledit.cpp -> zq/zq_misc.h
dialog/paledit.cpp -> zq/zquest.h
dialog/paledit.h -> zq/gui/palette_frame.h
dialog/pickruleset.cpp -> zq/zq_files.h
dialog/pickruletemplate.h -> zq/zq_files.h
dialog/quest_browser.cpp -> zq/zq_files.h
dialog/quest_browser.cpp -> zq/zquest.h
dialog/quest_browser.h -> zq/gui/quest_list_view.h
dialog/quest_browser_data.cpp -> zq/zquest.h
dialog/quest_rules.cpp -> zq/zquest.h
dialog/save_menu_editor.cpp -> zq/zquest.h
dialog/screen_data.cpp -> zq/zquest.h
dialog/screen_data.h -> zq/gui/selcombo_swatch.h
dialog/screen_enemies.cpp -> zq/zquest.h
dialog/screen_enemies.h -> zq/gui/tileanim_frame.h
dialog/screen_notes.cpp -> zq/zquest.h
dialog/screen_notes.h -> zq/gui/tileanim_frame.h
dialog/script_data_editor.cpp -> zq/zquest.h
dialog/spritedata.h -> zq/gui/seltile_swatch.h
dialog/spritedata.h -> zq/gui/tileanim_frame.h
dialog/string_editor.h -> zq/gui/msgstr_preview.h
dialog/stringcontrolcode.h -> zq/gui/selcombo_swatch.h
dialog/subscr_lists_edit.cpp -> zq/zquest.h
dialog/subscr_props.h -> zq/gui/misc_color_sel.h
dialog/subscr_props.h -> zq/gui/misc_cset_sel.h
dialog/subscr_props.h -> zq/gui/seltile_swatch.h
dialog/subscreenwizard.cpp -> zq/zq_subscr.h
dialog/subscreenwizard.h -> zq/gui/misc_color_sel.h
dialog/subscreenwizard.h -> zq/gui/seltile_swatch.h
dialog/tilepicker.cpp -> zq/zq_tiles.h
dialog/tilesetwizard.cpp -> zq/zquest.h
dialog/volume_dlg.cpp -> zc/ffscript.h
dialog/warp_cheat_picker.cpp -> zc/cheats.h
dialog/weap_data_editor.h -> zq/gui/tileanim_frame.h
dialog/zc_midi_dlg.cpp -> zc/zelda.h
dialog/zc_screen_saver.cpp -> zc/matrix.h
dialog/zc_screen_saver.cpp -> zc/zelda.h
dialog/zinfodlg.cpp -> zq/zquest.h
dialog/zq_options.cpp -> zq/zq_misc.h
dialog/zq_options.cpp -> zq/zquest.h
dialog/zqhotkeys.h -> zq/zq_hotkey.h
drawing.cpp -> zc/zelda.h
ffc.cpp -> zc/hero.h
gamedata.cpp -> zc/scripting/script_object.h
gamedata.cpp -> zc/scripting/types/stack.h
gamedata.cpp -> zc/scripting/types/user_object.h
gui/builder.h -> zq/gui/cornerselect.h
gui/builder.h -> zq/gui/dmap_frame.h
gui/builder.h -> zq/gui/dmap_mapgrid.h
gui/builder.h -> zq/gui/dmap_minimap.h
gui/builder.h -> zq/gui/engraving.h
gui/builder.h -> zq/gui/misc_color_row.h
gui/builder.h -> zq/gui/misc_color_sel.h
gui/builder.h -> zq/gui/misc_cset_sel.h
gui/builder.h -> zq/gui/msgstr_preview.h
gui/builder.h -> zq/gui/palette_frame.h
gui/builder.h -> zq/gui/quest_list_view.h
gui/builder.h -> zq/gui/regiongrid.h
gui/builder.h -> zq/gui/selcombo_swatch.h
gui/builder.h -> zq/gui/selherotile_swatch.h
gui/builder.h -> zq/gui/seltile_swatch.h
gui/builder.h -> zq/gui/tileanim_frame.h
gui/ddpanel.cpp -> dialog/info.h
gui/dialog.h -> dialog/common.h
gui/editbox.cpp -> dialog/externs.h
gui/jwin.cpp -> dialog/info.h
gui/jwin_a5.cpp -> dialog/info.h
gui/qrpanel.cpp -> dialog/info.h
gui/top_level.cpp -> dialog/info.h
init.cpp -> zc/zelda.h
items.cpp -> zc/guys.h
items.cpp -> zc/zscriptversion.h
iter.h -> zc/maps.h
iter.h -> zc/zelda.h
music_playback.cpp -> zc/zc_sys.h
new_subscr.cpp -> zc/ffscript.h
pal.cpp -> zc/hero.h
parser/Compiler.h -> zq/ffasm.h
save_menu.cpp -> zc/ffscript.h
save_menu.cpp -> zc/title.h
save_menu.cpp -> zc/zc_sys.h
save_menu.cpp -> zq/render.h
save_menu.cpp -> zq/zquest.h
solidobject.cpp -> zc/hero.h
sprite.cpp -> zc/decorations.h
sprite.cpp -> zc/ffscript.h
sprite.cpp -> zc/guys.h
sprite.cpp -> zc/render.h
subscr.cpp -> zc/ffscript.h
tiles.cpp -> zc/maps.h
tiles.h -> zc/combos.h
user_object.cpp -> zc/scripting/arrays.h
user_object.cpp -> zc/scripting/script_object.h
user_object.cpp -> zc/zscriptversion.h
zalleg/files.cpp -> core/fonts.h
zalleg/files.h -> core/jwinfsel.h
zalleg/new_menu.cpp -> core/fonts.h
zalleg/new_menu.cpp -> core/zdefs.h
zalleg/new_menu.cpp -> gui/jwin.h
zalleg/render.cpp -> gui/jwin_a5.h
zalleg/render_tree.cpp -> core/fonts.h
zalleg/zalleg.cpp -> core/fonts.h
zalleg/zalleg.cpp -> sfx.h
zalleg/zalleg.h -> zsyssimple.h
zalleg/zqdialog.cpp -> core/zdefs.h
zalleg/zsys.cpp -> zconsole/ConsoleLogger.h
zalleg/zsys.h -> core/zdefs.h
zalleg/zsys.h -> gui/jwin.h
zc/ffscript.h -> parser/parserDefs.h
zc/zasm_optimize_internal.h -> parser/parserDefs.h
zc_list_data.cpp -> zc/zelda.h
zconsole/ConsoleLogger.h -> core/zdefs.h
zcsfx.cpp -> zc/ffscript.h
zq/commands.cpp -> zc/ffscript.h
zq/ffasmexport.h -> parser/Compiler.h
zq/zq_class.cpp -> zc/ffscript.h
zq/zq_class.cpp -> zc/zc_sys.h
zq/zq_class.cpp -> zc/zelda.h
zq/zq_custom.cpp -> zc/ffscript.h
zq/zq_doors.cpp -> zc/zc_sys.h
zq/zq_strings.cpp -> zc/ffscript.h
zq/zquest.cpp -> parser/Compiler.h
zq/zquest.cpp -> zc/ffscript.h
zq/zquest.h -> parser/parserDefs.h
zscrdata.h -> parser/Compiler.h
zscrdata.h -> zq/zquest.h
""".split('\n')

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*(["<])([^">]+)[">]')
SOURCE_EXTS = ('.cpp', '.c', '.h', '.hpp', '.inl', '.mm', '.ypp', '.lpp')


def module_of(rel):
    parts = rel.split('/')
    if parts[0] == 'components' and len(parts) > 2:
        return f'components/{parts[1]}'
    return parts[0] if len(parts) > 1 else 'src'


def violation(a, b):
    """Returns why module `a` may not include module `b`, or None if it may."""
    if a == b or b == 'test_runner':
        return None
    a, b_full, b = a.split('/')[0], b, b.split('/')[0]
    if a in APPS:
        return f'apps must not include other apps ({b}/)' if b in APPS else None
    if a in SHARED:
        if b in APPS:
            return f'shared code must not include an app ({b}/)'
        if a == 'core' and b in ('gui', 'dialog'):
            return f'core/ must not include {b}/'
        return None
    if a == 'components' and b == 'components':
        return f'components must not include each other ({b_full}/)'
    if a in LOW_LEVEL and b not in LOW_LEVEL[a]:
        if not LOW_LEVEL[a]:
            return f'{a}/ must not include other modules'
        return f'{a}/ may only include: {", ".join(sorted(LOW_LEVEL[a]))}'
    return None


def find_violations():
    """Returns ({"file -> target": (line, reason)}, [angle-bracket includes of our own headers])."""
    found = {}
    angled = []
    for dirpath, _, filenames in os.walk(SRC):
        for name in filenames:
            if not name.endswith(SOURCE_EXTS):
                continue
            path = Path(dirpath) / name
            rel = path.relative_to(SRC).as_posix()
            mod = module_of(rel)
            for lineno, line in enumerate(path.read_text(errors='replace').splitlines(), 1):
                m = INCLUDE_RE.match(line)
                if not m:
                    continue
                quoted, name = m.group(1) == '"', m.group(2)
                local = Path(os.path.normpath(path.parent / name))
                rooted = Path(os.path.normpath(SRC / name))
                if quoted and local.is_file():
                    candidate = local
                elif rooted.is_file():
                    candidate = rooted
                    if not quoted and (local == rooted or not local.is_file()):
                        angled.append(f'src/{rel}:{lineno}: use #include "{name}" for our own headers')
                else:
                    continue
                if SRC not in candidate.parents:
                    continue
                target = candidate.relative_to(SRC).as_posix()
                reason = violation(mod, module_of(target))
                if reason:
                    found.setdefault(f'{rel} -> {target}', (lineno, reason))
    return found, angled


def update_exceptions(violations):
    script = Path(__file__)
    text = script.read_text()
    body = ''.join(f'{v}\n' for v in sorted(violations))
    new_text, n = re.subn(r'(EXCEPTIONS = """\n).*?(""")', lambda m: m.group(1) + body + m.group(2), text, flags=re.S)
    assert n == 1
    script.write_text(new_text)
    print(f'wrote {len(violations)} exceptions to {script.relative_to(ROOT)}')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--update', action='store_true', help='rewrite EXCEPTIONS to the current violations')
    args = parser.parse_args()

    violations, angled = find_violations()
    if args.update:
        update_exceptions(violations)
        return 0

    exceptions = {e for e in EXCEPTIONS if e}
    new = sorted(set(violations) - exceptions)
    stale = sorted(exceptions - set(violations))
    for v in new:
        file, target = v.split(' -> ')
        lineno, reason = violations[v]
        print(f'src/{file}:{lineno}: includes {target}, but {reason}')
    if new:
        print(f'\n{len(new)} include(s) break the module layering (see the top of scripts/check_layering.py).')
        print('Fix them, or if they are really needed, run `python scripts/check_layering.py --update`')
        print('to add them as exceptions.')
    for a in angled:
        print(a)
    if stale:
        print('\nThese exceptions no longer apply:')
        for v in stale:
            print(f'    {v}')
        print('Run `python scripts/check_layering.py --update` to remove them.')
    return 1 if new or stale or angled else 0


if __name__ == '__main__':
    sys.exit(main())
