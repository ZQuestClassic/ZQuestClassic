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
import { cachedGlobalCompletionItems, cleanDocString, cleanupFile, Connection, docJobResults, DocSymbol, DocumentMetaData, getBinAndResourcesFolders, getCompletionItemKind, getMemberChain, globalClasses, globalTmpDir, globalVariables, isPositionInScope, runCompiler, Settings, SymbolResolver } from './helpers.js';
import * as fs from 'fs';

interface OnCompletionOpts {
	document: TextDocument;
	metadata: DocumentMetaData;
	params: TextDocumentPositionParams;
}

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

/**
 * Gathers all autocomplete completion items for a specific class.
 */
function getClassMembers(className: string): CompletionItem[] {
	const members: CompletionItem[] = [];
	const seen = new Set<string>();

	let currentClass = className;
	let depth = 0;

	while (currentClass && depth < 10) {
		const builtIn = globalClasses.get(currentClass);
		if (builtIn && builtIn.children) {
			for (const child of builtIn.children) {
				if (!seen.has(child.name)) {
					seen.add(child.name);
					members.push(createCompletionItem(child));
				}
			}
		}

		const wsClass = SymbolResolver.getWorkspaceClass(currentClass);
		if (wsClass && wsClass.cls.children) {
			for (const child of wsClass.cls.children) {
				if (!seen.has(child.name)) {
					seen.add(child.name);
					const ident = wsClass.metadata.identifiers.find(id =>
						id.loc.line === child.selectionRange.start.line &&
						child.selectionRange.start.character >= id.loc.character &&
						child.selectionRange.start.character <= id.loc.character + id.loc.length
					);
					let metaSym;
					if (ident) metaSym = wsClass.metadata.symbols[ident.symbol];

					members.push({
						label: child.name,
						kind: getCompletionItemKind(child.kind),
						insertText: child.name,
						insertTextFormat: InsertTextFormat.PlainText,
						detail: metaSym?.type,
					});
				}
			}
		}

		currentClass = builtIn?.parent || "";
		depth++;
	}

	return members;
}

export async function onCompletion(opts: OnCompletionOpts): Promise<CompletionItem[]> {
	const p = opts.params;
	const doc = opts.document;
	if (!doc || !opts.metadata.currentFileSymbols) {
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
			// Get the base variable type.
			let currentType = SymbolResolver.getVariableType(chain[0], doc, p.position, opts.metadata);

			// Resolve the rest of the chain (e.g., step into "CreateBitmap").
			for (let i = 1; i < chain.length; i++) {
				if (!currentType) break;

				const baseTypeName = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];
				currentType = SymbolResolver.getMemberType(baseTypeName, chain[i]);
			}

			// 3. Return the class members of the final resolved type
			if (currentType) {
				const baseType = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];
				return getClassMembers(baseType);
			}
		}

		return [];
	}

	const localItems: CompletionItem[] = [];
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
					// We are inside this block. Its local variables are directly accessible.
					extractLocalSymbols(sym.children, '', true, isCurrentDoc);
				} else if (sym.kind === SymbolKind.Namespace || sym.kind === SymbolKind.Class) {
					// We are outside, but it's a namespace. We can access members using a qualifier.
					extractLocalSymbols(sym.children, prefix + sym.name + '::', false, isCurrentDoc);
				} else if (sym.kind === SymbolKind.Enum) {
					// Enum members leak into the surrounding scope, so they should be
					// visible anytime the Enum itself is visible.
					extractLocalSymbols(sym.children, prefix, isInsideParent, isCurrentDoc);
				}
				// Note: If it's a function/method/class and `isInsideThis` is false, we DO NOT recurse. 
				// This keeps local variables and class properties completely hidden.
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

	const filteredLocalItems = localItems.filter(item => !cachedGlobalCompletionItems.some(g => g.label === item.data));
	return [...cachedGlobalCompletionItems, ...filteredLocalItems];
}
