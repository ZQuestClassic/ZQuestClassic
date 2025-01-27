import {
	createConnection,
	TextDocuments,
	Diagnostic,
	DiagnosticSeverity,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	CompletionItem,
	TextDocumentPositionParams,
	TextDocumentSyncKind,
	InitializeResult,
	HoverParams,
	Hover,
	DocumentSymbolParams,
	DocumentSymbol,
	MarkupKind,
	DefinitionParams,
	Position,
	Location,
	WorkspaceFolder,
} from 'vscode-languageserver/node';
import { URI } from 'vscode-uri';
import {
	Range,
	TextDocument
} from 'vscode-languageserver-textdocument';
import * as childProcess from 'child_process';
import { promisify } from 'util';
import * as os from 'os';
import * as fs from 'fs';
import path = require('path');
import * as glob from 'glob';

const execFile = promisify(childProcess.execFile);

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);

// Create a simple text document manager.
const documents: TextDocuments<TextDocument> = new TextDocuments(TextDocument);

interface SymbolPos {
	line: number;
	character: number;
	length: number;
}

interface DocumentMetaData {
	currentFileSymbols: DocumentSymbol[];
	symbols: Record<number, { doc?: string, loc: { range: Range, uri: string } }>;
	identifiers: Array<{
		loc: SymbolPos,
		symbol: number,
	}>;
}
const docMetadataMap = new Map<string, DocumentMetaData>();

let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;

let workspaceFolders: WorkspaceFolder[] | null | undefined;

async function initWorkspace() {
	docMetadataMap.clear();

	const uris: URI[] = [];
	for (const workspaceFolder of workspaceFolders || []) {
		const uri = URI.parse(workspaceFolder.uri);
		const files = glob.sync('**/*.{zs,zh,z}', { root: uri.fsPath });
		for (const file of files) {
			uris.push(URI.file(path.join(uri.fsPath, file)));
		}
	}


	for (const uri of uris) {
		await processScript(uri.toString(), fs.readFileSync(uri.fsPath, 'utf8'));
	}
}

connection.onInitialize(async (params: InitializeParams) => {
	const capabilities = params.capabilities;

	// Does the client support the `workspace/configuration` request?
	// If not, we fall back using global settings.
	hasConfigurationCapability = !!(
		capabilities.workspace && !!capabilities.workspace.configuration
	);
	hasWorkspaceFolderCapability = !!(
		capabilities.workspace && !!capabilities.workspace.workspaceFolders
	);
	hasDiagnosticRelatedInformationCapability = !!(
		capabilities.textDocument &&
		capabilities.textDocument.publishDiagnostics &&
		capabilities.textDocument.publishDiagnostics.relatedInformation
	);

	const result: InitializeResult = {
		capabilities: {
			textDocumentSync: TextDocumentSyncKind.Incremental,
			// Tell the client that this server supports code completion.
			completionProvider: {
				resolveProvider: true
			},
			hoverProvider: true,
			definitionProvider: true,
			documentSymbolProvider: {
				label: 'ZScript',
			},
			referencesProvider: true,
		}
	};
	if (hasWorkspaceFolderCapability) {
		result.capabilities.workspace = {
			workspaceFolders: {
				supported: true
			}
		};
	}

	workspaceFolders = params.workspaceFolders;

	return result;
});

connection.onInitialized(async () => {
	if (hasConfigurationCapability) {
		// Register for all configuration changes.
		connection.client.register(DidChangeConfigurationNotification.type, undefined);
	}
	if (hasWorkspaceFolderCapability) {
		// TODO: doesn't really work well yet.
		// connection.workspace.onDidChangeWorkspaceFolders(async () => {
		// 	await initWorkspace();
		// });

		// await initWorkspace();
	}
});

// The example settings
interface Settings {
	installationFolder?: string;
	printCompilerOutput?: boolean;
	defaultIncludeFiles?: Array<string>;
	defaultIncludePaths?: Array<string>;
	ignoreConstAssert?: boolean;
}

// The global settings, used when the `workspace/configuration` request is not supported by the client.
// Please note that this is not the case when using this server with the client provided in this example
// but could happen with other clients.
const defaultSettings: Settings = {};
let globalSettings: Settings = defaultSettings;

// Cache the settings of all open documents
const documentSettings: Map<string, Thenable<Settings>> = new Map();

connection.onDidChangeConfiguration(change => {
	if (hasConfigurationCapability) {
		// Reset all cached document settings
		documentSettings.clear();
	} else {
		globalSettings = <Settings>(
			(change.settings.zscriptLanguageServer || defaultSettings)
		);
	}

	// Revalidate all open text documents
	documents.all().forEach(document => processScript(document.uri, document.getText()));
});

connection.onDidChangeWatchedFiles(e => {
	// TODO: probably have to re-init?
	e.changes;
});

function getDocumentSettings(uri: string): Thenable<Settings> {
	if (!hasConfigurationCapability) {
		return Promise.resolve(globalSettings);
	}
	let result = documentSettings.get(uri);
	if (!result) {
		result = connection.workspace.getConfiguration({
			scopeUri: uri,
			section: 'zscript'
		});
		documentSettings.set(uri, result);
	}
	return result;
}

documents.onDidOpen(e => {
	processScript(e.document.uri, e.document.getText());
});

// Only keep settings for open documents
documents.onDidClose(e => {
	documentSettings.delete(e.document.uri);
	docMetadataMap.delete(e.document.uri);
});

// The content of a text document has changed. This event is emitted
// when the text document first opened or when its content has changed.
documents.onDidChangeContent(e => {
	processScript(e.document.uri, e.document.getText());
});

// TODO: this should not be necessary. Get path in better OS-agnostic way.
function cleanupFile(fname: string) {
	if (os.platform() !== 'win32')
		return fname.trim();
	return fname.replace(/\//g, '\\').trim();
}
function cleanupFile2(fname: string) {
	if (os.platform() !== 'win32')
		return fname.trim();

	fname = URI.parse(fname).fsPath;
	if (fname.match(/[a-z]:\\.*/)) {
		// capitalize drive letters
		fname = fname[0].toUpperCase() + fname.slice(1);
	}
	return fname.replace(/\//g, '\\').trim();
}
function fileMatches(f1: string, f2: string) {
	return cleanupFile(f1) == cleanupFile(f2);
}

function parseOutput(settings: Settings, stdout: string, stderr: string): { diagnostics: Diagnostic[], metadata?: DocumentMetaData } {
	if (stdout.startsWith('{')) {
		if (settings.printCompilerOutput) {
			console.log(stderr);
		}
		const result = JSON.parse(stdout) as { diagnostics: Diagnostic[], metadata?: DocumentMetaData };
		return result;
	}

	// For older releases, diagnostics and metadata must be parsed from stdout.

	const [compilerOutput, metadataStr] = stdout.split('=== METADATA', 2);
	if (settings.printCompilerOutput) {
		console.log(compilerOutput);
	}

	const diagnostics: Diagnostic[] = [];
	for (const line of compilerOutput.split('\n')) {
		if (line.includes('syntax error')) {
			const m = line.match(/syntax error, (.*) \[(.*) Line (\d+) Column (\d+).*\].*/);
			let message = '';
			let lineNum = 0;
			let colNum = 0;
			let fname = '';
			if (m) {
				fname = cleanupFile(m[2]);
				message = m[1].trim();
				if (fileMatches(fname, "ZQ_BUFFER") || fileMatches(fname, tmpInput)) {
					lineNum = 0;
					colNum = 0;
					message = `Syntax error in temp file (check your ZScript Extension settings):\n${message}`;
				}
				else if (fileMatches(fname, tmpScript)) {
					lineNum = Number(m[3]) - 1;
					colNum = Number(m[4]);
				}
				else {
					lineNum = 0;
					colNum = 0;
					message = `Syntax error in "${fname}":\n${message}`;
				}
			} else {
				message = line.split('syntax error, ', 2)[1].trim();
			}

			const diagnostic: Diagnostic = {
				severity: DiagnosticSeverity.Error,
				range: {
					start: { line: lineNum, character: 0 },
					end: { line: lineNum, character: colNum },
				},
				message,
				source: 'zscript',
			};
			diagnostics.push(diagnostic);
		} else if (line.includes('Error') || line.includes('Warning')) {
			const m = line.match(/(.*)Line (\d+).*Columns (\d+)-(\d+) - (.*)/);
			let message = '';
			let lineNum = 0;
			let colStartNum = 0;
			let colEndNum = 0;
			let sev = 'Warning';
			let fname = '';
			if (m) {
				fname = cleanupFile(m[1]);
				message = m[5].trim();
				if (message.startsWith('Error'))
					sev = 'Error';
				if (fname.length == 0 || fileMatches(fname, "ZQ_BUFFER") || fileMatches(fname, tmpInput)) //error in temp file
				{
					lineNum = 0;
					colStartNum = 0;
					colEndNum = 0;
					message = `${sev} in temp file (check your ZScript Extension settings):\n${message}`;
				}
				else if (fileMatches(fname, tmpScript)) {
					lineNum = Number(m[2]) - 1;
					colStartNum = Number(m[3]) - 1;
					colEndNum = Number(m[4]) - 1;
				}
				else {
					lineNum = 0;
					colStartNum = 0;
					colEndNum = 0;
					message = `${sev} in "${fname}":\n${message}`;
				}
			} else {
				message = line.trim();
			}

			const diagnostic: Diagnostic = {
				severity: sev == 'Error' ? DiagnosticSeverity.Error : DiagnosticSeverity.Warning,
				range: {
					start: { line: lineNum, character: colStartNum },
					end: { line: lineNum, character: colEndNum },
				},
				message,
				source: 'zscript',
			};
			diagnostics.push(diagnostic);
		} else if (line.includes('ERROR:') || line.includes('WARNING:')) {
			const m = line.match(/(.*) \[(.*) Line (\d+) Column (\d+).*/);
			let message = '';
			let lineNum = 0;
			let colNum = 0;
			let sev = 'Warning';
			let fname = '';
			if (m) {
				fname = cleanupFile(m[2]);
				message = m[1].trim();
				if (message.startsWith('ERROR:'))
					sev = 'Error';
				if (fileMatches(fname, "ZQ_BUFFER") || fileMatches(fname, tmpInput)) {
					lineNum = 0;
					colNum = 0;
					message = `${sev} in temp file (check your ZScript Extension settings):\n${message}`;
				}
				else if (fileMatches(fname, tmpScript)) {
					lineNum = Number(m[3]) - 1;
					colNum = Number(m[4]);
				}
				else {
					lineNum = 0;
					colNum = 0;
					message = `${sev} in "${fname}":\n${message}`;
				}
			} else {
				message = line.trim();
			}

			const diagnostic: Diagnostic = {
				severity: sev == 'Error' ? DiagnosticSeverity.Error : DiagnosticSeverity.Warning,
				range: {
					start: { line: lineNum, character: 0 },
					end: { line: lineNum, character: colNum },
				},
				message,
				source: 'zscript',
			};
			diagnostics.push(diagnostic);
		}
	}

	let metadata;
	try {
		metadata = JSON.parse(metadataStr);
	} catch (e: any) {
		connection.console.error(e.toString());
	}

	return { diagnostics, metadata };
}

const globalTmpDir = os.tmpdir();
const tmpInput = cleanupFile(`${globalTmpDir}/tmp2.zs`);
const tmpScript = cleanupFile(`${globalTmpDir}/tmp.zs`);

async function processScript(uri: string, content: string): Promise<void> {
	const settings = await getDocumentSettings(uri);
	let includeText = "#option NO_ERROR_HALT on\n#option HEADER_GUARD on\n";

	if (!settings.installationFolder) {
		connection.sendDiagnostics({
			uri, diagnostics: [{
				severity: DiagnosticSeverity.Error,
				range: {
					start: { line: 0, character: 0 },
					end: { line: 0, character: 0 },
				},
				message: 'Must set zscript.installationFolder setting',
				source: 'extension'
			}]
		});
		return;
	}

	if (settings.defaultIncludePaths) {
		settings.defaultIncludePaths.forEach(str => {
			includeText += `#includepath "${str}"\n`;
		});
	}
	if (settings.defaultIncludeFiles) {
		settings.defaultIncludeFiles.forEach(str => {
			includeText += `#include "${str}"\n`;
		});
	}

	includeText += `#include "${tmpScript}"\n`;
	let stdout = '';
	let stderr = '';
	let success = false;
	fs.writeFileSync(tmpInput, includeText);
	fs.writeFileSync(tmpScript, content);
	const exe = os.platform() === 'win32' ? './zscript.exe' : './zscript';
	if (settings.printCompilerOutput) {
		console.log(`Attempting to compile buffer:\n-----\n${includeText}\n-----`);
	}
	try {
		let originPath = URI.parse(uri).fsPath;
		if (originPath.match(/[a-z]:\\.*/)) {
			const letter = originPath.at(0);
			if (letter) //capitalize drive letters
				originPath = letter.toUpperCase() + originPath.slice(1);
		}
		const args = [
			'-unlinked',
			'-delay_cassert',
			'-input', tmpInput,
			'-force_ignore', originPath,
			'-metadata',
			'-metadata-tmp-path', tmpScript,
			'-metadata-orig-path', originPath,
			'-json',
		];
		if (settings.ignoreConstAssert)
			args.push('-ignore_cassert');
		const cp = await execFile(exe, args, {
			cwd: settings.installationFolder,
			maxBuffer: 20_000_000,
		});
		success = true;
		stdout = cp.stdout;
		stderr = cp.stderr;
	} catch (e: any) {
		if (e.code === undefined) throw e;
		stdout = e.stdout || e.toString();
	}

	const { diagnostics, metadata } = parseOutput(settings, stdout, stderr);

	// Fallback, incase compiling failed but we failed to parse out an error.
	if (!success && diagnostics.length === 0) {
		diagnostics.push({
			severity: DiagnosticSeverity.Error,
			range: {
				start: { line: 0, character: 0 },
				end: { line: 0, character: 0 },
			},
			message: [stdout, stderr].join('\n\n'),
			source: 'zscript',
		});
	}

	// Send the computed diagnostics to VSCode.
	connection.sendDiagnostics({ uri: uri, diagnostics });

	if (!metadata) return;

	docMetadataMap.set(uri, metadata);
}

connection.onDidChangeWatchedFiles(_change => {
	// Monitored files have change in VSCode
	connection.console.log('We received an file change event');
});

// This handler provides the initial list of the completion items.
connection.onCompletion(
	(_textDocumentPosition: TextDocumentPositionParams): CompletionItem[] => {
		return [];
		// The pass parameter contains the position of the text document in
		// which code complete got requested. For the example we ignore this
		// info and always provide the same completion items.
		// return [
		// 	{
		// 		label: 'TypeScript',
		// 		kind: CompletionItemKind.Text,
		// 		data: 1
		// 	},
		// 	{
		// 		label: 'JavaScript',
		// 		kind: CompletionItemKind.Text,
		// 		data: 2
		// 	}
		// ];
	}
);

// This handler resolves additional information for the item selected in
// the completion list.
connection.onCompletionResolve(
	(item: CompletionItem): CompletionItem => {
		// if (item.data === 1) {
		// 	item.detail = 'TypeScript details';
		// 	item.documentation = 'TypeScript documentation';
		// } else if (item.data === 2) {
		// 	item.detail = 'JavaScript details';
		// 	item.documentation = 'JavaScript documentation';
		// }
		return item;
	}
);

function resolvePosition(uri: string, pos: Position) {
	const metadata = docMetadataMap.get(uri);
	if (!metadata)
		return null;

	const identifier = metadata.identifiers.find(ident =>
		ident.loc.line == pos.line &&
		pos.character >= ident.loc.character &&
		pos.character < ident.loc.character + ident.loc.length);
	if (!identifier) {
		return null;
	}

	return {metadata, identifier};
}

connection.onHover((p: HoverParams): Hover | null => {
	const result = resolvePosition(p.textDocument.uri, p.position);
	if (!result)
		return null;

	const {metadata, identifier} = result;
	const value = metadata.symbols[identifier.symbol].doc;
	if (!value)
		return null;

	return {
		contents: { kind: MarkupKind.Markdown, value },
	};
});

connection.onDocumentSymbol((p: DocumentSymbolParams) => {
	return docMetadataMap.get(p.textDocument.uri)?.currentFileSymbols;
});

connection.onDefinition((p: DefinitionParams) => {
	const result = resolvePosition(p.textDocument.uri, p.position);
	if (!result)
		return null;

	const {metadata, identifier} = result;
	const symbol = metadata.symbols[identifier.symbol];
	if (cleanupFile2(symbol.loc.uri) === tmpScript || symbol.loc.uri === 'file://' + tmpScript)
		symbol.loc.uri = p.textDocument.uri;
	return symbol.loc;
});

connection.onReferences((p) => {
	const result = resolvePosition(p.textDocument.uri, p.position);
	if (!result)
		return null;

	const {identifier} = result;
	const locations: Location[] = [];
	for (const [uri, metadata] of docMetadataMap) {
		// TODO: currently, symbol ids are not the same across multiple compilations, so can
		// only find references within the same document.
		// Idea for how to fix:
		//    1. Compile all scripts found in `initWorkspace` in one go
		//    2. Will help to support relative imports
		//    3. Will help for an error within an import to not end the compilation (for purposes of getting metadata)
		if (uri !== p.textDocument.uri) {
			continue;
		}

		locations.push(...metadata.identifiers
			.filter(ident => ident.symbol === identifier.symbol)
			.map(ident => {
				return {
					uri,
					range: {
						start: ident.loc,
						end: { line: ident.loc.line, character: ident.loc.character + ident.loc.length },
					},
				};
			}));
	}

	return locations;
});

// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

// Listen on the connection
connection.listen();
