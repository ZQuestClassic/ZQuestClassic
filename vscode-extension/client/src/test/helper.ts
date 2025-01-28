import * as vscode from 'vscode';
import * as path from 'path';

export async function activate(version: string, uri: vscode.Uri) {
	const ext = vscode.extensions.getExtension('cjamcl.zquest-lsp')!;
	await ext.activate();
	await setVersion(version);
	const doc = await vscode.workspace.openTextDocument(uri);
	const editor = await vscode.window.showTextDocument(doc);
	await sleep(3000); // Wait for server activation
	return { doc, editor };
}

async function setVersion(version: string) {
	let zcPath: string;
	if (version === '2.55') {
		zcPath = process.env.ZC_PATH_255;
	} else if (version === '3-no-json') {
		zcPath = process.env.ZC_PATH_3_NO_JSON;
	} else if (version === 'latest') {
		zcPath = process.env.ZC_PATH_LATEST;
	}

	if (!zcPath) {
		throw new Error(`invalid version: ${version}`);
	}

	const config = vscode.workspace.getConfiguration('zscript');
	await config.update('installationFolder', zcPath, vscode.ConfigurationTarget.Global);
	await sleep(1000);
}

async function sleep(ms: number) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

export const getDocUri = (testFixtureName: string) => {
	const docPath = path.resolve(__dirname, '../../testFixture', testFixtureName);
	return vscode.Uri.file(docPath);
};

export async function setTestContent(uri: vscode.Uri, content: string): Promise<void> {
	const doc = await vscode.workspace.openTextDocument(uri);
	const editor = await vscode.window.showTextDocument(doc);
	const all = new vscode.Range(
		doc.positionAt(0),
		doc.positionAt(doc.getText().length)
	);
	await editor.edit(eb => eb.replace(all, content));
	await sleep(1000);
}

export async function executeDocumentSymbolProvider(uri: vscode.Uri): Promise<vscode.DocumentSymbol[]> {
	// TODO: fix so that `onDocumentSymbol` runs processScript if needed (or waits for ongoing processing).
	const doc = await vscode.workspace.openTextDocument(uri);
	await setTestContent(uri, doc.getText());

	while (true) {
		const symbols = await vscode.commands.executeCommand('vscode.executeDocumentSymbolProvider', uri) as any;
		if (symbols && symbols.length) return symbols;
		await sleep(1000);
	}
}

export async function executeHoverProvider(uri: vscode.Uri, pos: vscode.Position): Promise<vscode.Hover[]> {
	return await vscode.commands.executeCommand('vscode.executeHoverProvider', uri, pos);
}
