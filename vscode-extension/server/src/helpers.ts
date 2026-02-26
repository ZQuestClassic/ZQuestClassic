import {
	CompletionItemKind,
	createConnection,
	Diagnostic,
	DiagnosticSeverity,
	DocumentSymbol,
	Position,
	SymbolKind,
} from 'vscode-languageserver/node';
import {
	Range,
	TextDocument
} from 'vscode-languageserver-textdocument';
import { URI } from 'vscode-uri';
import * as childProcess from 'child_process';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { promisify } from 'util';

const execFile = promisify(childProcess.execFile);

export type Connection = ReturnType<typeof createConnection>;

export interface Settings {
	installationFolder?: string;
	printCompilerOutput?: boolean;
	defaultIncludeFiles?: Array<string>;
	defaultIncludePaths?: Array<string>;
	ignoreConstAssert?: boolean;
}

export interface MetaDataSymbol {
	name?: string;
	type?: string;
	doc?: string;
	loc: { range: Range, uri: string };
	children?: MetaDataSymbol[];
}

export interface DocSymbol {
	kind: SymbolKind;
	name: string;
	type?: string;
	returnType?: string;
	parameters?: Array<{ name: string, type: string, default?: string }>;
	varargs?: boolean;
	parent?: string;
	comment: {
		tags: string[][];
		text: string;
	} | null,
	children?: DocSymbol[];
}

export interface SymbolPos {
	line: number;
	character: number;
	length: number;
}

export interface DocumentMetaData {
	currentFileSymbols: DocumentSymbol[];
	symbols: Record<number, MetaDataSymbol>;
	identifiers: Array<{
		loc: SymbolPos,
		symbol: number,
	}>;
}

export interface JobResult {
	diagnostics: Diagnostic[];
	metadata?: DocumentMetaData;
}

export const docJobResults = new Map<string, JobResult>();
export const globalTmpDir = os.tmpdir();

// TODO: this should not be necessary. Get path in better OS-agnostic way.
export function cleanupFile(fname: string) {
	if (os.platform() !== 'win32')
		return fname.trim();
	return fname.replace(/\//g, '\\').trim();
}

export function cleanupFile2(fname: string) {
	if (os.platform() !== 'win32')
		return fname.trim();

	fname = URI.parse(fname).fsPath;
	if (fname.match(/[a-z]:\\.*/)) {
		// capitalize drive letters
		fname = fname[0].toUpperCase() + fname.slice(1);
	}
	return fname.replace(/\//g, '\\').trim();
}

export function fileMatches(f1: string, f2: string) {
	return cleanupFile(f1) == cleanupFile(f2);
}

export function cleanDocString(doc: string): string {
	return doc.replace(/\[@\d+\|(.+?)@\]/g, '`$1`');
}

export function getCompletionItemKind(symbolKind: SymbolKind): CompletionItemKind {
	switch (symbolKind) {
		case SymbolKind.Class: return CompletionItemKind.Class;
		case SymbolKind.Constant: return CompletionItemKind.Constant;
		case SymbolKind.EnumMember: return CompletionItemKind.EnumMember;
		case SymbolKind.Function: return CompletionItemKind.Function;
		case SymbolKind.Method: return CompletionItemKind.Method;
		case SymbolKind.Variable: return CompletionItemKind.Variable;
		default: return CompletionItemKind.Text;
	}
}

export function isPositionInRange(pos: Position, range: Range): boolean {
	if (pos.line < range.start.line || pos.line > range.end.line) {
		return false;
	}
	if (pos.line === range.start.line && pos.character < range.start.character) {
		return false;
	}
	if (pos.line === range.end.line && pos.character > range.end.character) {
		return false;
	}
	return true;
}

export function isPositionInScope(doc: TextDocument, pos: Position, sym: DocumentSymbol): boolean {
	// If it's before the start line/char, it's definitely false.
	if (pos.line < sym.range.start.line) {
		return false;
	}
	if (pos.line === sym.range.start.line && pos.character < sym.range.start.character) {
		return false;
	}

	// If it's strictly within the old AST range, we are good.
	if (pos.line <= sym.range.end.line) {
		return true;
	}

	// The cursor is past the old end line. The AST might be stale because
	// the user added newlines. Let's read the live text to verify.
	const text = doc.getText({ start: sym.range.start, end: pos });

	// Strip comments and strings to safely count actual braces.
	const cleanText = text.replace(/\/\/.*$/gm, '').replace(/\/\*[\s\S]*?\*\//g, '').replace(/"(?:\\.|[^"\\])*"/g, '');

	let balance = 0;
	let hasSeenBrace = false;

	for (let i = 0; i < cleanText.length; i++) {
		if (cleanText[i] === '{') {
			balance++;
			hasSeenBrace = true;
		} else if (cleanText[i] === '}') {
			balance--;
			// If the block officially closed before our cursor, we've exited the scope.
			if (hasSeenBrace && balance === 0) {
				return false;
			}
		}
	}

	// If we've opened a brace and haven't fully closed it yet, the block has been
	// pushed down and we are inside the extended bounds!
	return hasSeenBrace && balance > 0;
}

export function getBinAndResourcesFolders(settings: Settings) {
	const installFolder = settings.installationFolder;
	if (!installFolder) {
		throw new Error('missing installFolder');
	}

	let binFolder = installFolder;
	let resourcesFolder = installFolder;
	if (installFolder.endsWith('.app')) {
		resourcesFolder = path.join(installFolder, 'Contents/Resources');
		binFolder = resourcesFolder;
	} else if (fs.existsSync(path.join(installFolder, 'bin'))) {
		binFolder = path.join(installFolder, 'bin');
	}

	if (path.basename(binFolder) === 'bin') {
		resourcesFolder = path.join(binFolder, '../share/zquestclassic');
	}

	return { binFolder, resourcesFolder };
}

const pathInZCRepoCache = new Map<string, string | null>();
export function checkIfInZCRepo(file: string): string | null {
	if (process.env.TEST_ZSCRIPT) return null;

	let result = pathInZCRepoCache.get(file);
	if (result !== undefined) return result;

	result = null;

	const parts = file.split(path.sep);
	for (let i = parts.length - 1; i >= 0; i--) {
		const dir = parts.slice(0, i).join(path.sep);
		try {
			if (!fs.existsSync(`${dir}/.git`)) continue;
		} catch {
			break;
		}

		if (fs.existsSync(`${dir}/resources/include/std.zh`)) result = path.join(dir, 'resources');
		break;
	}

	pathInZCRepoCache.set(file, result);
	return result;
}

export async function runCompiler(opts: { settings: Settings, connection: Connection, uri?: string, cwd?: string, signal?: AbortSignal, args: string[] }) {
	const installFolder = opts.settings.installationFolder;
	if (!installFolder) {
		throw new Error('missing installFolder');
	}

	const exe = os.platform() === 'win32' ? './zscript.exe' : './zscript';
	const { binFolder, resourcesFolder } = getBinAndResourcesFolders(opts.settings);

	if (!fs.existsSync(`${binFolder}/${exe}`)) {
		if (opts.uri) {
			opts.connection.sendDiagnostics({
				uri: opts.uri,
				diagnostics: [{
					severity: DiagnosticSeverity.Error,
					range: {
						start: { line: 0, character: 0 },
						end: { line: 0, character: 0 },
					},
					message: 'Did not find zscript compiler - check setting zscript.installationFolder',
					source: 'zscript'
				}]
			});
		}
		return null;
	}

	// If in a checkout of the ZC repo, use the resources folder as cwd.
	const cwd = opts.cwd ?? checkIfInZCRepo(installFolder) ?? resourcesFolder;

	if (opts.settings.printCompilerOutput) {
		console.log([exe, ...opts.args].join(' '));
	}

	return await execFile(`${binFolder}/${exe}`, opts.args, {
		cwd,
		maxBuffer: 20_000_000,
		signal: opts.signal,
		env: { ...process.env, ZC_DISABLE_OSX_CHDIR: '1', ZC_DISABLE_CHDIR: '1' },
	});
}

// Parse the chain of member accesses, skipping over function arguments.
// E.g. "  auto x = Game->CreateBitmap(1, 1)->" becomes ["Game", "CreateBitmap"]
export function getMemberChain(text: string): string[] {
	const tokens: string[] = [];
	let current = '';
	let parenDepth = 0;

	text = text.replace(/(?:\.|->)\s*$/, '').trim();

	for (let i = text.length - 1; i >= 0; i--) {
		const char = text[i];
		if (char === ')') {
			parenDepth++;
		} else if (char === '(') {
			parenDepth--;
		} else if (parenDepth === 0) {
			if (char === '.') {
				if (current) tokens.unshift(current.trim());
				current = '';
			} else if (char === '>' && text[i - 1] === '-') {
				if (current) tokens.unshift(current.trim());
				current = '';
				i--; // skip '-'
			} else if (/[a-zA-Z0-9_]/.test(char)) {
				current = char + current;
			} else if (/\s/.test(char)) {
				// Ignore whitespace
			} else {
				break;
			}
		}
	}
	if (current) tokens.unshift(current.trim());
	return tokens;
}
