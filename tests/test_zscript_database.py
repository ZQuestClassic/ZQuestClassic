# Compiles every script in the PureZC database, and saves failures to a snapshot file.
#
# To update the snapshot at tests/snapshots/zscript_database_expected.txt:
#
#   python tests/test_zscript_database.py --update
#
# or:
#
#   python tests/update_snapshots.py
#
# To debug a particular script, just find it in `.tmp/test_zscript_database`.

# TODO failures worth investigating
#   00310-newbie-item/script.zs
#   00462-dark-souls-status-effects
#   00531-example-enemy/ExampleEnemyGhost.zs
#   00532-status-effects/statusEffects.zs

# TODO Errors in authored code, report to author
#   00096-shallow-water-splash-and-tall-grass-sfx
#   00431-no-cycle-spinning-tiles

import argparse
import os
import re
import subprocess
import sys
import unittest

from pathlib import Path

from common import ZCTestCase

ZSCRIPT_DATABASE_COMMIT = '8b448930ffa8665816114ad88bb61b38bdcfc4e5'

parser = argparse.ArgumentParser()
parser.add_argument(
    '--update', action='store_true', default=False, help='Update snapshot'
)
parser.add_argument('unittest_args', nargs='*')
args = parser.parse_args()

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
test_scripts_dir = root_dir / 'tests/scripts'
tmp_script_dir = root_dir / '.tmp/test_zscript_database'
tmp_script_dir.mkdir(parents=True, exist_ok=True)

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target

# Change include paths to use resources/ directly, instead of possibly-stale stuff inside a build folder.
def_include_paths = [
    str(root_dir / 'resources/include'),
    str(root_dir / 'resources/headers'),
]


def find_all_scripts(path: Path):
    script_paths = list(path.rglob('*.zs'))
    script_paths.extend(path.rglob('*.z'))
    script_paths.extend(path.rglob('*.zh'))
    return script_paths


def intuit_imports(database_dir: Path, script_path: Path):
    code = script_path.read_text('utf-8', errors='ignore')
    imports = ['std.zh']
    include_paths = []

    if re.search(
        r'ghost|SetEnemyProperty|FindSpawnPoint|SetEWeaponmovement|EWF_|ENPROP_',
        code,
        re.IGNORECASE,
    ):
        except_for = [
            '00246-ghost-zh',
            '00258-gale-fruit/script.zs',
        ]
        if not any(e in script_path.as_posix() for e in except_for):
            imports.append('ghost.zh')

    if '00246-ghost-zh' in str(script_path):
        imports.append(database_dir / '00246-ghost-zh/header/ghost.zh')
        include_paths.append(database_dir / '00246-ghost-zh/header')

    if re.search(r'tango', code, re.IGNORECASE):
        imports.append('tango.zh')

    if re.search(r'moveLink|TRH_', code, re.IGNORECASE):
        imports.append('deprecated/theRandomHeader.zh')

    if re.search(r'classic_zh', code, re.IGNORECASE):
        imports.append(test_scripts_dir / 'compat/Classic.zh')
        include_paths.append(test_scripts_dir / 'compat')

    if re.search(r'CF_MATRIX', code, re.IGNORECASE):
        except_for = [
            '00397-matrix-password-puzzle',
            '00414-matrix-password-puzzle-up-and-down',
        ]
        if not any(e in script_path.as_posix() for e in except_for):
            imports.append(
                database_dir / '00414-matrix-password-puzzle-up-and-down/matrix50.z'
            )

    if re.search(r'LinkMovement', code, re.IGNORECASE):
        except_for = [
            '00284-linkmovement-zh/script.zs',
        ]
        if not any(e in script_path.as_posix() for e in except_for):
            imports.append('LinkMovement.zh')
            include_paths.append(database_dir / '00284-linkmovement-zh')

    if re.search(
        r'OnSameRank|GetComboRank|KnightMoveAdjacent|LeaperMoveAdjacent',
        code,
        re.IGNORECASE,
    ):
        imports.append(database_dir / '00391-chess-zh/script.zs')

    if re.search(r'SolidObjects_', code, re.IGNORECASE):
        imports.extend(
            [
                'ghost.zh',
                'LinkMovement.zh',
                database_dir / '00311-solid-ffcs-sideview-moving-platforms/script.zs',
            ]
        )
        include_paths.append(database_dir / '00284-linkmovement-zh')

    if re.search(
        r'GetCurrentItem|freezeScreen|debugValue|moveLink', code
    ):
        imports.append(test_scripts_dir / 'compat/stdExtra.zh')
        include_paths.append(test_scripts_dir / 'compat')

    if (
        re.search(
            r'CheckMaxLweapons|LweaponInit|PutFFCInFrontOfLink|LWF_|LW_GHOSTED',
            code,
            re.IGNORECASE,
        )
        or '00235-stdweapons-zh/stdWeapons' in script_path.as_posix()
    ):
        imports.append(database_dir / '00235-stdweapons-zh/stdWeapons.zh')
        include_paths.append(database_dir / '00235-stdweapons-zh')

    if re.search(
        r'stdExtra\.zh|SetSideviewFFCSolidity|Ghost_GSDCanMove|Ghost_WithinSolidFFC',
        code,
        re.IGNORECASE,
    ):
        imports.append('ghost.zh')
        imports.append(
            database_dir
            / '00415-grayswandir-sideview-engine/GSD_sideview/GSD_sideview.z'
        )
        include_paths.append(
            database_dir / '00415-grayswandir-sideview-engine/GSD_sideview'
        )
        imports.append(test_scripts_dir / 'compat/stdExtra.zh')
        include_paths.append(test_scripts_dir / 'compat')

    if 'tangoDemo' in str(script_path):
        imports.extend(
            [
                'tangoDemo/tango.zh',
                'tangoDemo/font/LttPOutline.zh',
                'tangoDemo/font/OracleExtended.zh',
                'tangoDemo/font/Japanese.zh',
                'tangoDemo/font/SmallExtended.zh',
                'tangoDemo/styles.zh',
                'tangoDemo/common.zh',
                'tangoDemo/itemObtained.z',
                'tangoDemo/longMessage.z',
                'tangoDemo/musicChanger.z',
                'tangoDemo/navi.z',
                'tangoDemo/npc.z',
                'tangoDemo/ojiisan.z',
                'tangoDemo/pickAColor.z',
                'tangoDemo/zelda.z',
                'tangoDemo/global.z',
            ]
        )
        include_paths.append(database_dir / 'scripts/00247-tango-zh/demo')

    if re.search(
        r'NPC_MISC_ORIGTILE',
        code,
        re.IGNORECASE,
    ):
        imports.append(database_dir / '00113-bigenemydx/script.zs')

    imports = [p.as_posix() if isinstance(p, Path) else p for p in imports]
    include_paths = [p.as_posix() if isinstance(p, Path) else p for p in include_paths]
    return list(dict.fromkeys(imports)), list(dict.fromkeys(include_paths))


class TestZScriptDatabase(ZCTestCase):
    def setUp(self):
        self.maxDiff = None

    def test_zscript_database(self):
        database_dir = root_dir / '.tmp/zscript-database'
        if not database_dir.exists():
            print('Cloning zscript-database repo, this will take a moment')
            subprocess.check_call(
                [
                    'git',
                    'clone',
                    'https://github.com/ZQuestClassic/zscript-database.git',
                    database_dir,
                ],
                stderr=subprocess.DEVNULL,
            )
        subprocess.check_call(
            ['git', 'fetch'], cwd=database_dir, stderr=subprocess.DEVNULL
        )
        subprocess.check_call(
            ['git', 'checkout', ZSCRIPT_DATABASE_COMMIT],
            cwd=database_dir,
            stderr=subprocess.DEVNULL,
        )
        database_dir = database_dir / 'scripts'

        script_paths = set(find_all_scripts(database_dir))
        ignore = [
            # ZASM.
            '00240-dynamic-tile-overlay-zasm',
            # Can't get this working.
            '00247-tango-zh/demo/tangoDemo',
            # Tested in others.
            '00247-tango-zh/header',
            # These fail in different ways across platforms.
            '00341-wind-waker-stealth',
            '00462-dark-souls-status-effects',
            '00531-example-enemy',
            # Casing issue in imports (ex: FFCscript.zh instead of ffcscript.zh)
            '00078-lttp-style-map-display',
            '00210-magic-clock',
        ]
        for dir_path in ignore:
            script_paths -= set(find_all_scripts(database_dir / dir_path))
        script_paths = list(script_paths)
        script_paths.sort()

        result = ''
        failures = []
        passed = 0

        num_workers = max(1, os.cpu_count() - 1)
        workers = [None] * num_workers

        result_dict = {}
        next_worker_index = 0

        for script_path in script_paths:
            worker_index = next_worker_index
            data = workers[worker_index]
            if data is not None:
                name, p = data
                output = p.communicate()[0]
                result_dict[name] = (p.returncode, output)

            next_worker_index = (next_worker_index + 1) % num_workers
            rel_name = script_path.relative_to(database_dir).as_posix()
            tmp_script_path = tmp_script_dir / rel_name
            tmp_script_path.parent.mkdir(parents=True, exist_ok=True)

            # In general, scripts are not isolated / do not import what they use.
            # So let's lend a hand.
            imports, include_paths = intuit_imports(database_dir, script_path)
            include_paths.append(database_dir)
            imports.append(rel_name)
            lines = [
                '#option ON_MISSING_RETURN warn',
                '#option WARN_DEPRECATED off',
                '',
                *(f'#includepath "{path}"' for path in include_paths),
                '',
                *(f'#include "{path}"' for path in imports),
            ]
            tmp_script_path.write_text('\n'.join(lines), 'utf-8')

            exe_name = run_target.get_exe_name('zscript')
            p = subprocess.Popen(
                [
                    run_target.get_build_folder() / exe_name,
                    '-input',
                    tmp_script_path,
                    '-include',
                    ';'.join(def_include_paths),
                    '-unlinked',
                    '-delay_cassert',
                ],
                env={**os.environ, 'TEST_ZSCRIPT': '1', 'ZC_DISABLE_DEBUG': '1'},
                cwd=run_target.get_build_folder(),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )
            workers[worker_index] = (script_path, p)

        for i, data in enumerate(workers):
            if data == None:
                continue

            name, p = data
            output = p.communicate()[0]
            result_dict[name] = (p.returncode, output)
            workers[i] = None

        result_dict = dict(sorted(result_dict.items(), key=lambda x: x[0].as_posix()))
        for script_path, (returncode, output) in result_dict.items():
            success = returncode == 0 and 'Success!' in output
            if success:
                passed += 1
                continue

            output = output.replace('\\', '/')
            output = output.replace(database_dir.as_posix() + '/', '')
            output = output.replace(root_dir.as_posix() + '/', '')
            output = re.sub(r'Compiling \'.*\'\n', '', output)
            output = re.sub(r'find label \d+', 'find label', output)
            output = re.sub(r' @ Columns \d+-\d+', '', output)

            rel_name = script_path.relative_to(database_dir).as_posix()
            result += f'=== {rel_name}\n'
            result += output + '\n'

            if success:
                passed += 1
            else:
                failures.append(str(rel_name))

        total = len(script_paths)
        header = f'{passed} / {total} ({int(passed/total*100)}%) passed\n\n'
        header += f'Failures:\n'
        header += '\t' + '\n\t'.join(failures) + '\n\n'
        result = header + result

        # print('failing w/ missing return value:')
        # missing_returns = []
        # for script_path, (returncode, output) in result_dict.items():
        #     success = returncode == 0 and 'Success!' in output
        #     if not success and 'must return a value' in output:
        #         import json

        #         meta_path = script_path.with_name('meta.json')
        #         while not meta_path.exists():
        #             meta_path = meta_path.parent.with_name('meta.json')
        #         meta = json.loads(meta_path.read_text())
        #         downloads = meta.get('downloads') or 'fail'
        #         if not downloads[0].isdigit():
        #             meta['downloads'] = 0
        #         missing_returns.append((script_path, meta))

        # missing_returns.sort(key=lambda x: -int(x[1].get('downloads', 0)))

        # for script_path, meta in missing_returns:
        #     url = meta.get('source')
        #     downloads = meta.get('downloads', 0)
        #     author = meta.get('author', '?')
        #     path = script_path.relative_to(database_dir)
        #     print(f'(url: {url}, downloads: {downloads}, author: {author}) {path}')

        expected_path = root_dir / 'tests/snapshots/zscript_database_expected.txt'
        self.expect_snapshot(expected_path, result, args.update)


if __name__ == '__main__':
    sys.argv[1:] = args.unittest_args
    unittest.main()
