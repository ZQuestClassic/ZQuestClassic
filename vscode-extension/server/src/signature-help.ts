import {
	MarkupKind,
	SignatureHelpParams,
	SignatureHelp,
	SignatureInformation,
	SymbolKind,
} from 'vscode-languageserver/node';
import {
	TextDocument
} from 'vscode-languageserver-textdocument';
import { cleanDocString, DocSymbol, getMemberChain } from './helpers';
import { globalClasses, globalVariables } from './completion';

interface SignatureHelpOpts {
	document: TextDocument;
	params: SignatureHelpParams;
}

function createSignatureInfo(symbol: DocSymbol): SignatureInformation {
	const params = symbol.parameters ?? [];
	const paramsString = params.map(p => `${p.type} ${p.name}` + (p.default ? ` = ${p.default}` : '')).join(', ');

	return {
		label: `${symbol.returnType ?? 'void'} ${symbol.name}(${paramsString})`,
		documentation: symbol.comment ? {
			kind: MarkupKind.Markdown,
			value: cleanDocString(symbol.comment.text),
		} : undefined,
		parameters: params.map(p => ({
			label: `${p.type} ${p.name}`
		}))
	};
}

// Note: this only supports functions defined in bindings / std / packaged libraries, not anything else,
// since the data comes from the cached `-doc` execution done in `updateCompletionItems`.
export async function onSignatureHelp(opts: SignatureHelpOpts): Promise<SignatureHelp | null> {
	const params = opts.params;

	// Grab text preceding the cursor.
	const startLine = Math.max(0, params.position.line - 5);
	const text = opts.document.getText({
		start: { line: startLine, character: 0 },
		end: params.position
	});

	// Scan backward to find the active parameter (comma count) and the opening `(`.
	let activeParameter = 0;
	let parenDepth = 0;
	let callStartIndex = -1;

	for (let i = text.length - 1; i >= 0; i--) {
		const char = text[i];
		if (char === ')') {
			parenDepth++;
		} else if (char === '(') {
			if (parenDepth === 0) {
				callStartIndex = i;
				break; // Found the start of the function call!
			}
			parenDepth--;
		} else if (char === ',' && parenDepth === 0) {
			activeParameter++; // Found a comma at the root level of the function call
		}
	}

	if (callStartIndex === -1) return null;

	// Extract the chain right before the '('
	const beforeParen = text.slice(0, callStartIndex).trim();
	const chain = getMemberChain(beforeParen);
	if (chain.length === 0) return null;

	const targetFuncName = chain[chain.length - 1];
	const signatures: SignatureInformation[] = [];

	// Look up the function (simplified: checks globals and class methods)
	if (chain.length === 1) {
		// It's a global/local function call (e.g., `Trace(`)
		for (const sym of globalVariables.values()) {
			if (sym.name === targetFuncName && (sym.kind === SymbolKind.Function || sym.kind === SymbolKind.Method)) {
				signatures.push(createSignatureInfo(sym));
			}
		}
	} else {
		// It's a member access call (e.g., `Game->CreateBitmap(`)
		for (const cls of globalClasses.values()) {
			if (cls.children) {
				for (const child of cls.children) {
					if (child.name === targetFuncName && (child.kind === SymbolKind.Function || child.kind === SymbolKind.Method)) {
						signatures.push(createSignatureInfo(child));
					}
				}
			}
		}
	}

	if (signatures.length === 0) return null;

	return {
		signatures,
		activeSignature: 0, // VS Code will auto-adjust this based on parameter count
		activeParameter
	};
}
