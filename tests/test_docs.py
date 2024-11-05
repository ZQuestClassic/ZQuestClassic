import os
import platform
import shutil
import subprocess
import sys
import unittest

from pathlib import Path

is_windows = platform.system() == 'Windows'
script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = script_dir.parent
zscript_generated_sources_path = root_dir / 'docs-www/source/zscript'
docs_generated_html_path = root_dir / 'docs-www/build/html'

sys.path.append(str((root_dir / 'scripts').absolute()))
import run_target


class TestDocs(unittest.TestCase):

    def setUp(self):
        self.maxDiff = None

    def test_generate_docs(self):
        if 'emscripten' in str(run_target.get_build_folder()):
            raise unittest.SkipTest('unsupported platform')

        if docs_generated_html_path.exists():
            shutil.rmtree(docs_generated_html_path)
        if zscript_generated_sources_path.exists():
            shutil.rmtree(zscript_generated_sources_path)

        # Ensure there are no issues building the docs site, and no Sphinx warnings.
        subprocess.check_call(['bash', root_dir / 'docs-www/update.sh'], shell=is_windows)

        # Very basic check that files were generated as expected.

        n = len(list(zscript_generated_sources_path.rglob('*.rst')))
        self.assertGreater(n, 100)

        n = len(list(docs_generated_html_path.rglob('*.html')))
        self.assertGreater(n, 100)


if __name__ == '__main__':
    unittest.main()
