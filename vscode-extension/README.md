# VS Code extension for ZScript

## For users

Setup:

1. Get the extension [here](https://marketplace.visualstudio.com/items?itemName=cjamcl.zquest-lsp) or by searching "zquest lsp" in the VS Code extensions panel
1. Set the "Installation Folder" setting of the extension to where you keep ZC. Open the settings with `Ctrl/CMD + ,` and search for "zscript"

### Features

* Error and warning diagnostics
* Syntax highlighting
* "Go to Definition" support for variables/functions/other symbols (`Ctrl/CMD + click` or right-click)
* Document outline for all the symbols in a script
* Hover tooltips to show code documentation
* Some support for autoformatting


### Hover tooltips

When a symbol has a comment near it like this, hovering over any usage displays a tooltip:

```c++
// Some comment
// Can be longer than one line
int counter1;
int counter2; // Some comment
/** Some comment */
int counter3;
/**
 * Some comment
 */
int counter4;
```

Comments are rendered as markdown.

Comments may contain inline links to other symbols with the [`@link`](https://jsdoc.app/tags-inline-link) tag:

```c++
// Better than {@link counter2} ... {@link counter2|that counter is lame}
int counter1;
// You can also use a shorthand for links like this: [counter1]
int counter2;
```


### Autoformatting

If you want formatting, you'll need `clang-format` installed and accessible on your PATH. See [this page](https://www.dynamsoft.com/codepool/vscode-format-c-code-windows-linux.html) for more.

You'll need to wrap invalid-C++ code like so:
```cpp
// clang-format off
@Author("Connor")
ffc script MyScript {
  // clang-format on

  void run(int a) {

  }
}
```

You can change the format rules used by creating a `.clang-format` file in your script directory.

If you wish to further customize the code theme used (beyond what some theme you can download does), see:

- Select `inspect editor tokens` in command palette (ctrl+shit+p) to inspect what the token types are
- read https://medium.com/@danromans/how-to-customize-semantic-token-colorization-with-visual-studio-code-ac3eab96141b#:~:text=Now%2C%20with%20the%20needed%20information to understand how to make a token bold, italic, colored, whatever


## For developing

```sh
# open this folder in terminal
# install things
npm install
# open _this folder_ (not repo root) in vscode
# in Run and Debug panel, run the client
# This will open up a new vscode extension debug window, where the code in this
# folder is used.
# To re-update the extension, hit the "restart" thing in the first VS Code's debug pane
```

Most of the interesting code is in `server.ts`. `syntaxes/` has grammar stuff.

Read this:
- https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide#scope-inspector
- https://macromates.com/manual/en/language_grammars (standard grammar scopes at bottom)
