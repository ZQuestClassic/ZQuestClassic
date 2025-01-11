import html
import urllib

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.parsers.rst.directives import unchanged, flag
from docutils.parsers.rst.directives.admonitions import BaseAdmonition
from sphinx import addnodes
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective, SphinxTranslator
from sphinx.util.typing import ExtensionMetadata


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

def visit_zscript_node_html(translator: SphinxTranslator, node: ZScriptNode) -> None:
    if node.data:
        html_str = f'<pre class="hljs language-zs"><code>{html.escape(node.data)}</code></pre>'
        html_str += f'''
        <script>
            {{
                const el = document.currentScript.previousElementSibling;
                requestAnimationFrame(() => hljs.highlightElement(el));
            }}
        </script>
        '''
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

def visit_zscript_node_unsupported(translator: SphinxTranslator, node: ZScriptNode) -> None:
    logger.warning("zscript: unsupported output format (node skipped)")
    raise nodes.SkipNode

class AdmonitionNode(nodes.Admonition, nodes.Element):
    pass

class TodoNode(AdmonitionNode):
    pass

class TodoDirective(BaseAdmonition, SphinxDirective):
    node_class = TodoNode
    optional_arguments: int = 1
    final_argument_whitespace: bool = True
    option_spec: dict = {
        'nowarn': flag
    }
    
    def run(self) -> list[nodes.Node]:
        self.options['class'] = ['admonition-todo']
        
        (node,) = super().run()
        title = 'TODO'
        if len(self.arguments) and len(self.arguments[0]):
            title += f': {self.arguments[0]}'
        node.insert(0, nodes.title(text=_(title)))
        node['docname'] = self.env.docname
        self.add_name(node)
        self.set_source_info(node)
        self.state.document.note_explicit_target(node)
        
        if not 'nowarn' in self.options:
            self.reporter.warning(f'{title}: {"\n".join(self.content)}')
        return [node]

class PlansNode(AdmonitionNode):
    pass

class PlansDirective(BaseAdmonition, SphinxDirective):
    node_class = PlansNode
    optional_arguments: int = 1
    final_argument_whitespace: bool = True
    
    def run(self) -> list[nodes.Node]:
        self.options['class'] = ['admonition-plans']
        
        (node,) = super().run()
        title = 'Not Yet Implemented'
        if len(self.arguments) and len(self.arguments[0]):
            title = f': {self.arguments[0]}'
        node.insert(0, nodes.title(text=_(title)))
        node['docname'] = self.env.docname
        self.add_name(node)
        self.set_source_info(node)
        self.state.document.note_explicit_target(node)
        
        return [node]

def visit_admonition_node_html(translator: SphinxTranslator, node: AdmonitionNode) -> None:
    translator.visit_admonition(node)
def depart_admonition_node_html(translator: SphinxTranslator, node: AdmonitionNode) -> None:
    translator.depart_admonition(node)


class ZSDeprecated(SphinxDirective):
    """
    Directive to describe a deprecation in a specific version.
    Adapted from 'sphinx.domains.changeset's 'VersionChange'
    """

    has_content = True
    required_arguments = 0
    optional_arguments = 1
    final_argument_whitespace = True

    def run(self) -> list[nodes.Node]:
        node = addnodes.versionmodified()
        node.document = self.state.document
        self.set_source_info(node)
        node['type'] = 'deprecated'
        has_version = len(self.arguments) > 0
        if has_version:
            node['version'] = self.arguments[0]
            text = f'Deprecated since version {self.arguments[0]}'
        else:
            text = 'Deprecated'
        if self.content:
            node += self.parse_content_to_nodes()
        classes = ['versionmodified', 'deprecated']
        if len(node) > 0 and isinstance(node[0], nodes.paragraph):
            # the contents start with a paragraph
            if node[0].rawsource:
                # make the first paragraph translatable
                content = nodes.inline(node[0].rawsource, translatable=True)
                content.source = node[0].source
                content.line = node[0].line
                content += node[0].children
                node[0].replace_self(
                    nodes.paragraph('', '', content, translatable=False)
                )

            para = node[0]
            para.insert(0, nodes.inline('', '%s: ' % text, classes=classes))
        elif len(node) > 0:
            # the contents do not starts with a paragraph
            para = nodes.paragraph(
                '',
                '',
                nodes.inline('', '%s: ' % text, classes=classes),
                translatable=False,
            )
            node.insert(0, para)
        else:
            # the contents are empty
            para = nodes.paragraph(
                '',
                '',
                nodes.inline('', '%s.' % text, classes=classes),
                translatable=False,
            )
            node.append(para)
        
        if has_version:
            domain = self.env.domains.changeset_domain
            domain.note_changeset(node)

        return [node]


def depart_ignored(translator: SphinxTranslator, node: nodes.Node) -> None:
    pass

def setup(app: Sphinx) -> ExtensionMetadata:
    app.add_directive('zscript', ZScriptDirective)
    app.add_directive('todo', TodoDirective)
    app.add_directive('plans', PlansDirective)
    app.add_directive('deprecated', ZSDeprecated, True)
    
    app.add_node(
        ZScriptNode,
        html=(visit_zscript_node_html, depart_ignored),
        epub=(visit_zscript_node_unsupported, None),
        latex=(visit_zscript_node_unsupported, None),
        man=(visit_zscript_node_unsupported, None),
        texinfo=(visit_zscript_node_unsupported, None),
        text=(visit_zscript_node_unsupported, None),
        rinoh=(visit_zscript_node_unsupported, None),
    )
    app.add_node(
        TodoNode,
        html=(visit_admonition_node_html, depart_admonition_node_html),
        epub=(None, None),
        latex=(None, None),
        man=(None, None),
        texinfo=(None, None),
        text=(None, None),
        rinoh=(None, None),
    )
    app.add_node(
        PlansNode,
        html=(visit_admonition_node_html, depart_admonition_node_html),
        epub=(None, None),
        latex=(None, None),
        man=(None, None),
        texinfo=(None, None),
        text=(None, None),
        rinoh=(None, None),
    )

    return {
        'version': '0.1',
        'env_version': 1,
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
