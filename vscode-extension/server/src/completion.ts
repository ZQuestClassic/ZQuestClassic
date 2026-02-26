import {
	CompletionItem,
	CompletionItemTag,
	DocumentSymbol,
	InsertTextFormat,
	MarkupKind,
	SymbolKind,
	TextDocumentPositionParams,
} from 'vscode-languageserver/node';
import {
	TextDocument
} from 'vscode-languageserver-textdocument';
import { cleanDocString, cleanupFile, Connection, docJobResults, DocSymbol, DocumentMetaData, getBinAndResourcesFolders, getCompletionItemKind, getMemberChain, globalTmpDir, isPositionInScope, runCompiler, Settings } from './helpers.js';
import * as fs from 'fs';

interface OnCompletionOpts {
	document: TextDocument;
	metadata: DocumentMetaData;
	params: TextDocumentPositionParams;
}

export const cachedGlobalCompletionItems: CompletionItem[] = [];
export const globalClasses = new Map<string, DocSymbol>();
export const globalVariables = new Map<string, DocSymbol>();

function createCompletionItem(symbol: DocSymbol): CompletionItem {
	const deprecated = symbol.comment?.tags.some(t => t[0] === 'deprecated');

	let detail;
	let insertText;
	let insertTextFormat: InsertTextFormat = InsertTextFormat.PlainText;
	let command;

	if (symbol.kind === SymbolKind.Function || symbol.kind === SymbolKind.Method) {
		const params = (symbol.parameters ?? []).map(p => `${p.type} ${p.name}` + (p.default ? ` = ${p.default}` : ''));
		if (symbol.varargs) {
			params.push('…');
		}
		const paramsString = params.join(', ');
		detail = `${symbol.returnType} ${symbol.name}(${paramsString})`;

		// Build the snippet syntax: FunctionName(${1:param1}, ${2:param2}).
		const snippetParams = (symbol.parameters ?? []).map((p, index) => `\${${index + 1}:${p.name}}`);
		if (symbol.varargs) {
			snippetParams.push(`\${${(symbol.parameters ?? []).length + 1}:...}`);
		}

		insertText = `${symbol.name}(${snippetParams.join(', ')})`;
		insertTextFormat = InsertTextFormat.Snippet;

		command = {
			title: 'Trigger Parameter Hints',
			command: 'editor.action.triggerParameterHints',
		};
	} else {
		detail = symbol.type;
	}

	return {
		label: symbol.name,
		kind: getCompletionItemKind(symbol.kind),
		insertText,
		insertTextFormat,
		command,
		tags: deprecated ? [CompletionItemTag.Deprecated] : undefined,
		documentation: symbol.comment ? {
			kind: MarkupKind.Markdown,
			value: cleanDocString(symbol.comment.text),
		} : undefined,
		detail,
	};
}

export async function updateCompletionItems(settings: Settings, connection: Connection, signal: AbortSignal): Promise<void> {
	const { resourcesFolder } = getBinAndResourcesFolders(settings);

	const tmpFile = cleanupFile(`${globalTmpDir}/tmp-autocomplete-cache.zs`);
	const headers = [
		'ghost.zh',
		'gui.zh',
		'std.zh',
		'tango.zh',
	].filter(header => fs.existsSync(`${resourcesFolder}/include/${header}`) || fs.existsSync(`${resourcesFolder}/headers/${header}`));
	fs.writeFileSync(tmpFile, headers.map(header => `#include "${header}"`).join('\n'));

	let cp;
	try {
		cp = await runCompiler({
			settings,
			connection,
			signal,
			args: [
				'-input', tmpFile,
				'-unlinked',
				'-doc',
				'-no-codegen',
			],
		});
	} catch (err: any) {
		if (!signal.aborted) {
			connection.console.error(`Error updating completion items: ${err}`);
		}
		return;
	}

	if (signal.aborted || !cp || !cp.stdout.startsWith('{')) {
		return;
	}

	const seenSymbolNames = new Set();
	function processSymbol(symbol: DocSymbol, isClassMember: boolean = false) {
		if (symbol.kind === SymbolKind.Class) {
			globalClasses.set(symbol.name, symbol);
		}

		// Only cache actual global variables/constants, not class properties.
		if (!isClassMember && (symbol.kind === SymbolKind.Variable || symbol.kind === SymbolKind.Constant)) {
			globalVariables.set(symbol.name, symbol);
		}

		const kindsToAutocomplete: SymbolKind[] = [
			SymbolKind.Class,
			SymbolKind.Constant,
			SymbolKind.EnumMember,
			SymbolKind.Function,
			SymbolKind.Method,
			SymbolKind.Variable,
		];

		// Only push to the global autocomplete list if it's not a class member.
		if (!isClassMember && kindsToAutocomplete.includes(symbol.kind) && !seenSymbolNames.has(symbol.name)) {
			seenSymbolNames.add(symbol.name);
			cachedGlobalCompletionItems.push(createCompletionItem(symbol));
		}

		if (symbol.children) {
			// If the current symbol is a class (or we are already inside one), mark children as class members.
			const childIsClassMember = isClassMember || symbol.kind === SymbolKind.Class;
			symbol.children.forEach(child => processSymbol(child, childIsClassMember));
		}
	}

	cachedGlobalCompletionItems.length = 0;
	globalClasses.clear();
	globalVariables.clear();
	const data = JSON.parse(cp.stdout);
	for (const file of data.files) {
		file.symbols.forEach((symbol: DocSymbol) => processSymbol(symbol));
	}
}

export async function onCompletion(opts: OnCompletionOpts): Promise<CompletionItem[]> {
	const p = opts.params;
	const doc = opts.document;
	if (!doc) {
		return cachedGlobalCompletionItems;
	}

	if (!opts.metadata.currentFileSymbols) {
		return cachedGlobalCompletionItems;
	}

	// Grab the text from a few lines up to catch multi-line statements.
	const startLine = Math.max(0, p.position.line - 5);
	const precedingText = doc.getText({
		start: { line: startLine, character: 0 },
		end: p.position
	});

	// Allow optional word characters after the arrow so autocomplete works while typing.
	const memberAccessMatch = precedingText.match(/(?:->|\.)([a-zA-Z0-9_]*)$/);

	if (memberAccessMatch) {
		// Strip off the partially typed member name and the `->`/`.`
		// e.g., "Game->CreateBitmap(1,1)->Co" becomes "Game->CreateBitmap(1,1)".
		const baseExpression = precedingText.slice(0, memberAccessMatch.index);
		// Now we just evaluate the chain leading up to the arrow.
		const chain = getMemberChain(baseExpression);

		if (chain.length > 0) {
			const rootName = chain[0];
			let currentType: string | undefined;

			// Safe variable lookup for the root of the chain (e.g., "Game").
			function findLocalType(symbols: DocumentSymbol[], targetName: string): string | undefined {
				for (const sym of symbols) {
					if (sym.name === targetName) {
						// Look up the exact symbol ID using the AST's coordinate mapping.
						const ident = opts.metadata.identifiers.find(id =>
							id.loc.line === sym.selectionRange.start.line &&
							sym.selectionRange.start.character >= id.loc.character &&
							sym.selectionRange.start.character <= id.loc.character + id.loc.length
						);

						if (ident) {
							const metaSym = opts.metadata.symbols[ident.symbol];
							if (metaSym && metaSym.type) return metaSym.type;
						}
					}
					if (sym.children && sym.children.length > 0 && isPositionInScope(doc!, p.position, sym)) {
						const found = findLocalType(sym.children, targetName);
						if (found) return found;
					}
				}

				return undefined;
			}

			currentType = findLocalType(opts.metadata.currentFileSymbols, rootName);

			if (!currentType) {
				const globalVar = globalVariables.get(rootName);
				if (globalVar) {
					currentType = globalVar.type;
				}
			}

			// Resolve the rest of the chain (e.g., step into "CreateBitmap").
			for (let i = 1; i < chain.length; i++) {
				if (!currentType) break;

				const baseTypeName = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];
				let currentClass = globalClasses.get(baseTypeName);
				let foundMemberType: string | undefined;

				let inheritanceDepth = 0;
				while (currentClass && inheritanceDepth < 10) {
					if (currentClass.children) {
						const member = currentClass.children.find(c => c.name === chain[i]);
						if (member && (member.type || member.returnType)) {
							foundMemberType = member.type || member.returnType; // E.g., The return type of CreateBitmap
							break;
						}
					}
					currentClass = currentClass.parent ? globalClasses.get(currentClass.parent) : undefined;
					inheritanceDepth++;
				}

				currentType = foundMemberType;
			}

			// Return the members of whatever type we ended up with.
			if (currentType) {
				const baseType = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];

				const members: CompletionItem[] = [];
				let currentClass = globalClasses.get(baseType);
				let inheritanceDepth = 0;

				while (currentClass && inheritanceDepth < 10) {
					if (currentClass.children) {
						for (const child of currentClass.children) {
							members.push(createCompletionItem(child));
						}
					}

					currentClass = currentClass.parent ? globalClasses.get(currentClass.parent) : undefined;
					inheritanceDepth++;
				}

				return members;
			}
		}

		// If we couldn't resolve the chain, return nothing.
		return [];
	}

	const localItems: CompletionItem[] = [];

	// Recursively extract symbols, honoring strict scopes and namespace qualifiers
	function extractLocalSymbols(symbols: DocumentSymbol[], prefix: string = '', isInsideParent: boolean = true, isCurrentDoc: boolean = true) {
		for (const sym of symbols) {
			// Are we physically inside this specific block?
			const isInsideThis = isCurrentDoc && doc && isPositionInScope(doc, p.position, sym);

			// Add the item if it's visible in the current scope OR if we are digging into a namespace
			if (isInsideParent || prefix) {
				localItems.push({
					label: prefix + sym.name,
					data: sym.name,
					kind: getCompletionItemKind(sym.kind),
				});
			}

			if (sym.children && sym.children.length > 0) {
				if (isInsideThis) {
					// We are inside this block (like a function). Its local variables are directly accessible.
					extractLocalSymbols(sym.children, '', true, isCurrentDoc);
				} else if (sym.kind === SymbolKind.Namespace) {
					// We are outside, but it's a namespace. We can access members using a qualifier.
					extractLocalSymbols(sym.children, prefix + sym.name + '::', false, isCurrentDoc);
				}
				// Note: If it's a function/method and `isInsideThis` is false, do not recurse.
				// This keeps its local variables completely hidden.
			}
		}
	}

	// Process all compiled files in the workspace.
	for (const [uri, result] of docJobResults.entries()) {
		if (result.metadata) {
			const isCurrentDoc = uri === p.textDocument.uri;
			extractLocalSymbols(result.metadata.currentFileSymbols, '', true, isCurrentDoc);
		}
	}

	return [...cachedGlobalCompletionItems, ...localItems];
}
