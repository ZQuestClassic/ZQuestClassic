import re

import requests


def _list_bucket_impl(bucket_url: str, marker: str, keys: list[str]):
    url = f'{bucket_url}?max-keys=1000'
    if marker:
        url += f'&marker={marker}'
    xml = requests.get(url).text
    keys.extend(re.compile(r'<Key>(.*?)</Key>').findall(xml))
    match = re.compile(r'<NextMarker>(.*?)</NextMarker>').search(xml)
    if match:
        return match.group(1)
    return None


def list_bucket(bucket_url: str):
    keys = []
    marker = ''

    while True:
        marker = _list_bucket_impl(bucket_url, marker, keys)
        if not marker:
            break

    return keys
