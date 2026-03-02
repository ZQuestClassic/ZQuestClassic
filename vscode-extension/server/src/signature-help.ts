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
import { cleanDocString, docJobResults, DocSymbol, getMemberChain, globalClasses, globalVariables, SymbolResolver } from './helpers.js';

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
	const doc = opts.document;

	// Grab the active metadata so the SymbolResolver can check local scopes.
	const result = docJobResults.get(doc.uri);
	const metadata = result?.metadata ?? null;

	// Grab text preceding the cursor.
	const startLine = Math.max(0, params.position.line - 5);
	const text = doc.getText({
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

	// Look up the function.
	if (chain.length === 1) {
		// It's a global/local function call (e.g., `Trace(`)
		for (const sym of globalVariables.values()) {
			if (sym.name === targetFuncName && (sym.kind === SymbolKind.Function || sym.kind === SymbolKind.Method)) {
				signatures.push(createSignatureInfo(sym));
			}
		}
	} else {
		// It's a member access call (e.g., `Game->CreateBitmap(`).
		// Resolve the type of the object we are calling the method on.
		let currentType = SymbolResolver.getVariableType(chain[0], doc, params.position, metadata);

		// Resolve any intermediate chain links (e.g. `Game->GetScreen()->`).
		for (let i = 1; i < chain.length - 1; i++) {
			if (!currentType) break;

			const baseTypeName = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];
			currentType = SymbolResolver.getMemberType(baseTypeName, chain[i]);
		}

		// Now that we know the exact class type, look up the method!
		if (currentType) {
			const baseType = currentType.replace(/const\s+/, '').replace(/\[\]/g, '').trim().split(' ')[0];

			let currentClass = globalClasses.get(baseType);
			let inheritanceDepth = 0;

			// Follow the inheritance chain to find the target method.
			while (currentClass && inheritanceDepth < 10) {
				if (currentClass.children) {
					for (const child of currentClass.children) {
						if (child.name === targetFuncName && (child.kind === SymbolKind.Function || child.kind === SymbolKind.Method)) {
							signatures.push(createSignatureInfo(child));
						}
					}
				}

				currentClass = currentClass.parent ? globalClasses.get(currentClass.parent) : undefined;
				inheritanceDepth++;
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
