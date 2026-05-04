import os
import re

root = 'build/html'

# Lowercase all files and directories
for dirpath, dirnames, filenames in os.walk(root, topdown=False):
    for name in filenames + dirnames:
        old_path = os.path.join(dirpath, name)
        new_path = os.path.join(dirpath, name.lower())
        if old_path != new_path:
            os.rename(old_path, new_path)

# Rewrite internal links in HTML files
# This regex looks for internal .html links and lowercases them while removing the extension.
# It handles fragments (#) and query strings (?) correctly.
# It avoids links starting with a protocol (http://, https://, etc.) but includes absolute internal paths.
link_re = re.compile(r'href=\"(?![a-z]+://)([^\"]+)\.html([#?][^\"]*)?\"')
for dirpath, _, filenames in os.walk(root):
    for name in filenames:
        if name.endswith('.html'):
            path = os.path.join(dirpath, name)
            with open(path, 'r', encoding='utf-8') as f:
                content = f.read()

            def rewrite_link(m):
                path_part = m.group(1).lower()
                extra_part = m.group(2) if m.group(2) else ''
                return f'href=\"{path_part}{extra_part}\"'

            new_content = link_re.sub(rewrite_link, content)

            if content != new_content:
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(new_content)
