import * as path from 'path';
import { ExtensionContext } from 'vscode';
import * as vscode from 'vscode';
import * as childProcess from 'child_process';
import {promisify} from 'util';

const execFile = promisify(childProcess.execFile);

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
	vscode.languages.registerDocumentFormattingEditProvider('zscript', {
        async provideDocumentFormattingEdits(document: vscode.TextDocument): Promise<vscode.TextEdit[]> {
			// const settings = vscode.workspace.getConfiguration('zscript', document.uri);

			try {
				const text = document.getText();
				const args = [
					'-style=file',
				];
				const parentDir = path.dirname(document.uri.fsPath);
				const output = await new Promise<string>(async (resolve, reject) => {
					const cp = childProcess.execFile('clang-format', args, {cwd: parentDir}, (err, stdout, stderr) => {
						if (err)
							return reject(err);

						resolve(stdout);
					});
					cp.stdin.write(text);
					cp.stdin.end();
				});
				if (output && text !== output) {
					const r = new vscode.Range(document.lineAt(0).range.start, document.lineAt(document.lineCount - 1).range.end);
					return [vscode.TextEdit.replace(r, output)];
				}
			} catch (e) {
				vscode.window.showErrorMessage(`Error while attempting to format script:\n${e.message}`);
				console.log(e);
			}
        }
    });

	// The server is implemented in node
	const serverModule = context.asAbsolutePath(
		path.join('server', 'out', 'server.js')
	);

	// If the extension is launched in debug mode then the debug server options are used
	// Otherwise the run options are used
	const serverOptions: ServerOptions = {
		run: { module: serverModule, transport: TransportKind.ipc },
		debug: {
			module: serverModule,
			transport: TransportKind.ipc,
		}
	};

	// Options to control the language client
	const clientOptions: LanguageClientOptions = {
		// Register the server for plain text documents
		documentSelector: [{ scheme: 'file', pattern: '**/*.{zs,zh,z}' }],
		synchronize: {
			// Notify the server about file changes to '.clientrc files contained in the workspace
			fileEvents: vscode.workspace.createFileSystemWatcher('**/.clientrc')
		}
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'zscriptLanguageServer',
		'ZScript Language Server',
		serverOptions,
		clientOptions
	);

	// Start the client. This will also launch the server
	client.start();
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
