from docutils import nodes
from docutils.parsers.rst import Directive
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective, SphinxTranslator
from sphinx.util.typing import ExtensionMetadata
from docutils.parsers.rst.directives import unchanged
import urllib
import html


class ZScriptNode(nodes.General, nodes.Element):
    url: str = ''
    data: str = ''
    fname: str = ''
    height: int = 800
    def __init__(self, url: str = '', data: str = '', fname: str = '', height = 800):
        super().__init__()
        self.url = url
        self.data = data
        self.fname = fname
        self.height = height

class ZScriptDirective(SphinxDirective):
    has_content: bool = True
    option_spec: dict = {
        "url": unchanged,
        "fname": unchanged,
        "height": int,
    }

    def run(self) -> list[nodes.Node]:
        return [ZScriptNode(
            url=self.options.get('url', '').strip(),
            data='\n'.join(self.content),
            fname=self.options.get('fname', '').strip(),
            height=self.options.get('height', 800),
            )]


def visit_logo_node_html(translator: SphinxTranslator, node: ZScriptNode) -> None:
    if node.data:
        html_str = f'<pre class="hljs"><code class="language-zs">{html.escape(node.data)}</code></pre>'
        translator.body.append(html_str)
    else:
        html_str = f'<iframe width=100%% height={node.height} src=https://web.zquestclassic.com/zscript/'
        first = False
        if node.url != '':
            html_str += '?url=' + node.url
        elif node.data != '':
            html_str += '?data=' + urllib.parse.quote(node.data)
        else: first = True
        if node.fname != '':
            html_str += '?' if first else '&'
            html_str += 'fname=' + urllib.parse.quote(node.fname)
        html_str += '></iframe>\n'
        translator.body.append(html_str)


def depart_logo_node_html(translator: SphinxTranslator, node: ZScriptNode) -> None:
    pass


def visit_logo_node_unsuported(translator: SphinxTranslator, node: ZScriptNode) -> None:
    logger.warning("zscript: unsupported output format (node skipped)")
    raise nodes.SkipNode


def setup(app: Sphinx) -> ExtensionMetadata:
    app.add_directive('zscript', ZScriptDirective)

    app.add_node(
        ZScriptNode,
        html=(visit_logo_node_html, depart_logo_node_html),
        epub=(visit_logo_node_unsuported, None),
        latex=(visit_logo_node_unsuported, None),
        man=(visit_logo_node_unsuported, None),
        texinfo=(visit_logo_node_unsuported, None),
        text=(visit_logo_node_unsuported, None),
        rinoh=(visit_logo_node_unsuported, None),
    )

    return {
        'version': '0.1',
        'env_version': 1,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
