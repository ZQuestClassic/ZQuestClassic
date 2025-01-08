from docutils import nodes
from docutils.parsers.rst import Directive
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective, SphinxTranslator
from sphinx.util.typing import ExtensionMetadata
from docutils.parsers.rst.directives import unchanged
import html


class ZScriptNode(nodes.General, nodes.Element):
    url: str = ''
    data: str = ''
    fname: str = ''
    def __init__(self, url: str = '', data: str = '', fname: str = ''):
        super().__init__()
        self.url = url
        self.data = data
        self.fname = fname

class ZScriptDirective(SphinxDirective):
    has_content: bool = True
    option_spec: dict = {
        "url": unchanged,
        "fname": unchanged,
    }

    def run(self) -> list[nodes.Node]:
        return [ZScriptNode(
            url=self.options.get('url', '').strip(),
            data=''.join(self.content),
            fname=self.options.get('fname', '').strip(),
            )]


def visit_logo_node_html(translator: SphinxTranslator, node: ZScriptNode) -> None:
    html_str = '<iframe width=100%% height=800 src=https://web.zquestclassic.com/zscript/'
    if node.url != '':
        html_str += '?url=' + node.url
    elif node.data != '':
        html_str += '?data=' + html.escape(node.data)
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
