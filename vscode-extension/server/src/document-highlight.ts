import {
	DocumentHighlightParams,
	DocumentHighlight,
	DocumentHighlightKind,
} from 'vscode-languageserver/node';
import {
	TextDocument
} from 'vscode-languageserver-textdocument';
import { DocumentMetaData } from './helpers';

interface DocumentHighlightOpts {
	document: TextDocument;
	metadata: DocumentMetaData;
	params: DocumentHighlightParams;
}

export function onDocumentHighlight(opts: DocumentHighlightOpts): DocumentHighlight[] | null {
	const params = opts.params;
	const identifiers = opts.metadata.identifiers;

	// 1. Find the identifier exactly under the user's cursor
	const targetId = identifiers.find(id =>
		id.loc.line === params.position.line &&
		params.position.character >= id.loc.character &&
		params.position.character <= (id.loc.character + id.loc.length)
	);

	if (!targetId) return null;

	// 2. Collect every other identifier in the file that shares this symbol ID
	const highlights: DocumentHighlight[] = [];
	for (const id of identifiers) {
		if (id.symbol === targetId.symbol) {
			highlights.push({
				range: {
					start: { line: id.loc.line, character: id.loc.character },
					end: { line: id.loc.line, character: id.loc.character + id.loc.length }
				},
				// You can use .Read or .Write if your compiler distinguishes them,
				// but .Text is the standard default for basic highlights.
				kind: DocumentHighlightKind.Text
			});
		}
	}

	return highlights;
}
