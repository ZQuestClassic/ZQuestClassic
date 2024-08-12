# monaco's built-in themes aren't powereful enough to handle TM tokens
# so this script grabs some VS Code themes and converts them
# https://github.com/Nishkalkashyap/monaco-vscode-textmate-theme-converter#monaco-vscode-textmate-theme-converter

import json
import os
import subprocess

from pathlib import Path

import json5
import requests

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
themes_dir = script_dir / 'themes'
root_dir = script_dir.parent.parent
tmp_dir = root_dir / '.tmp/playground_generate_themes'
tmp_dir.mkdir(exist_ok=True, parents=True)


def download_theme(url: str):
    response = requests.get(url)
    content = json5.loads(response.text)
    name = Path(url).name
    if 'tokenColors' not in content:
        content['tokenColors'] = []
    (tmp_dir / name).write_text(json.dumps(content, indent=2))


def download_themes_metadata(url: str):
    pkg = json.loads(requests.get(url).text)
    return pkg['contributes']['themes']


gh_url = 'https://raw.githubusercontent.com/microsoft/vscode/main/extensions'
themes_metadata_dict = {
    'defaults': download_themes_metadata(f'{gh_url}/theme-defaults/package.json'),
    'tomorrow-night-blue': download_themes_metadata(
        f'{gh_url}/theme-tomorrow-night-blue/package.json'
    ),
    'solarized-light': download_themes_metadata(
        f'{gh_url}/theme-solarized-light/package.json'
    ),
    'solarized-dark': download_themes_metadata(
        f'{gh_url}/theme-solarized-dark/package.json'
    ),
}


def get_theme_metadata(theme_path: Path):
    if 'solarized-light' in theme_path.name:
        themes_metadata = themes_metadata_dict['solarized-light']
    elif 'solarized-dark' in theme_path.name:
        themes_metadata = themes_metadata_dict['solarized-dark']
    elif 'tomorrow-night-blue' in theme_path.name:
        themes_metadata = themes_metadata_dict['tomorrow-night-blue']
    else:
        themes_metadata = themes_metadata_dict['defaults']
    return next(t for t in themes_metadata if Path(t['path']).name == f.name)


download_theme(f'{gh_url}/theme-defaults/themes/dark_modern.json')
download_theme(f'{gh_url}/theme-defaults/themes/dark_plus.json')
download_theme(f'{gh_url}/theme-defaults/themes/dark_vs.json')
download_theme(f'{gh_url}/theme-defaults/themes/hc_black.json')
download_theme(f'{gh_url}/theme-defaults/themes/hc_light.json')
download_theme(f'{gh_url}/theme-defaults/themes/light_modern.json')
download_theme(f'{gh_url}/theme-defaults/themes/light_plus.json')
download_theme(f'{gh_url}/theme-defaults/themes/light_vs.json')
download_theme(
    f'{gh_url}/theme-tomorrow-night-blue/themes/tomorrow-night-blue-color-theme.json'
)

download_theme(
    f'{gh_url}/theme-solarized-light/themes/solarized-light-color-theme.json'
)
download_theme(f'{gh_url}/theme-solarized-dark/themes/solarized-dark-color-theme.json')


def normalize_theme(path: Path):
    theme = json.loads(path.read_text())
    if 'include' in theme:
        parent_theme = normalize_theme(path.parent / theme['include'])
        theme['colors'] = {
            **parent_theme.get('colors', {}),
            **theme.get('colors', {}),
        }
        theme['tokenColors'] = [
            *parent_theme.get('tokenColors', []),
            *theme.get('tokenColors', []),
        ]
        theme['rules'] = [
            *parent_theme.get('rules', []),
            *theme.get('rules', []),
        ]

    return theme


for f in tmp_dir.glob('*.json'):
    theme = normalize_theme(f)
    f.write_text(json.dumps(theme, indent=2))


for f in tmp_dir.glob('*.json'):
    print(f)
    dest = tmp_dir / f.name
    subprocess.check_call(
        [
            'npx',
            'monaco-vscode-textmate-theme-converter',
            '-i',
            str(f),
            '-o',
            str(dest),
        ]
    )
    content = json.loads(dest.read_text())
    theme_metdata = get_theme_metadata(dest)
    content['base'] = theme_metdata['uiTheme']
    content['inherit'] = True

    dest = themes_dir / f.name.replace('-color-theme', '')
    dest.write_text(json.dumps(content, indent=2))
