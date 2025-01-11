import html
import urllib

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.parsers.rst.directives import unchanged, flag
from docutils.parsers.rst.directives.admonitions import BaseAdmonition
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
        html_str = f'<pre class="hljs"><code class="language-zs">{html.escape(node.data)}</code></pre>'
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

class TodoNode(nodes.Admonition, nodes.Element):
    pass

def visit_todo_node_html(translator: SphinxTranslator, node: TodoNode) -> None:
    translator.visit_admonition(node)
def depart_todo_node_html(translator: SphinxTranslator, node: TodoNode) -> None:
    translator.depart_admonition(node)

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

def depart_ignored(translator: SphinxTranslator, node: nodes.Node) -> None:
    pass

def setup(app: Sphinx) -> ExtensionMetadata:
    app.add_directive('zscript', ZScriptDirective)
    app.add_directive('todo', TodoDirective)

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
        html=(visit_todo_node_html, depart_todo_node_html),
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
