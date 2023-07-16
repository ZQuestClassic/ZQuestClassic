# VS Code extension for ZScript

## For users

Get the extension [here](https://marketplace.visualstudio.com/items?itemName=cjamcl.zquest-lsp).

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

The rest of this README is left unmodified from the example project this was taken from.

_______

Heavily documented sample code for https://code.visualstudio.com/api/language-extensions/language-server-extension-guide

## Functionality

This Language Server works for plain text file. It has the following language features:
- Completions
- Diagnostics regenerated on each file change or configuration change

It also includes an End-to-End test.

## Structure

```
.
├── client // Language Client
│   ├── src
│   │   ├── test // End to End tests for Language Client / Server
│   │   └── extension.ts // Language Client entry point
├── package.json // The extension manifest.
└── server // Language Server
    └── src
        └── server.ts // Language Server entry point
```

## Running the Sample

- Run `npm install` in this folder. This installs all necessary npm modules in both the client and server folder
- Open VS Code on this folder.
- Press Ctrl+Shift+B to start compiling the client and server in [watch mode](https://code.visualstudio.com/docs/editor/tasks#:~:text=The%20first%20entry%20executes,the%20HelloWorld.js%20file.).
- Switch to the Run and Debug View in the Sidebar (Ctrl+Shift+D).
- Select `Launch Client` from the drop down (if it is not already).
- Press ▷ to run the launch config (F5).
- In the [Extension Development Host](https://code.visualstudio.com/api/get-started/your-first-extension#:~:text=Then%2C%20inside%20the%20editor%2C%20press%20F5.%20This%20will%20compile%20and%20run%20the%20extension%20in%20a%20new%20Extension%20Development%20Host%20window.) instance of VSCode, open a document in 'plain text' language mode.
  - Type `j` or `t` to see `Javascript` and `TypeScript` completion.
  - Enter text content such as `AAA aaa BBB`. The extension will emit diagnostics for all words in all-uppercase.
