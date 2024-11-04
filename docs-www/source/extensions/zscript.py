from docutils import nodes
from docutils.parsers.rst import Directive
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective, SphinxTranslator
from sphinx.util.typing import ExtensionMetadata


class ZScriptNode(nodes.General, nodes.Element):
    pass


class ZScriptDirective(SphinxDirective):
    required_arguments: int = 1

    def run(self) -> list[nodes.Node]:
        return [ZScriptNode(path=self.arguments[0])]


def visit_logo_node_html(translator: SphinxTranslator, node: ZScriptNode) -> None:
    html = ''
    html += (
        '<iframe width=100%% height=800 src=https://web.zquestclassic.com/zscript/?url='
    )
    html += node['path']
    html += '></iframe>\n'
    translator.body.append(html)


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
