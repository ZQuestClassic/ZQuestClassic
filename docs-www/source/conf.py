# TODO: apply for algolia license https://github.com/godotengine/godot-docs/issues/4461

import os
import sys

sys.path.append(os.path.abspath('extensions'))

project = "ZQuest Classic"
copyright = "2024, ZQuest Classic Foundation"
author = "ZQuest Classic Developers"

extensions = ['zscript']

templates_path = ["_templates"]
exclude_patterns = []

html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
html_logo = "_static/ZC_Logo.png"
html_favicon = "_static/favicon.png"
html_js_files = ["main.js"]

html_theme_options = {
    # Collapse navigation (False makes it tree-like)
    "collapse_navigation": False,
}
html_show_sourcelink = False

rst_prolog = """
.. |const| replace:: :abbr:`const (This method has no side effects. It doesn't modify any of the instance's member variables.)`
.. |varargs| replace:: :abbr:`varargs (This method accepts any number of parameters after the ones described here.)`
.. |void| replace:: :abbr:`void (No return value.)`
.. |untyped| replace:: :abbr:`untyped (Could be any type.)`
.. |int| replace:: :abbr:`int (Not really an integer. In ZScript, int stores a fixed-position decimal number. For real integers, use the 'long' type.)`
.. |long| replace:: :abbr:`long (ZScript's real integer type.)`
.. |T| replace:: :abbr:`T (Generic type, can bound to any type.)`
.. |T1| replace:: :abbr:`T1 (Generic type, can bound to any type, as long as all generic types match.)`
.. |T2| replace:: :abbr:`T2 (Generic type, can bound to any type, as long as all generic types match.)`
.. |T3| replace:: :abbr:`T3 (Generic type, can bound to any type, as long as all generic types match.)`

.. role:: del
"""

# pdf
rinoh_documents = [
    dict(doc='index', target='zquestclassic', template='pdf.rtt'),
]
