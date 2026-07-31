// Tokenizes syntaxes/zscript-grammar-test.zs with the grammar and checks
// each `//! valid` / `//! invalid` marker against what the real TextMate
// engine produces. Run from vscode-extension/: npm run check-zscript-grammar
import fs from 'fs';
import { createHighlighter } from 'shiki';

// Optional argument lets you point at another grammar, e.g. to confirm the
// fixture actually fails against an unpatched upstream copy.
const GRAMMAR = process.argv[2] ?? 'syntaxes/zscript.tmLanguage.json';
const FIXTURE = 'syntaxes/zscript-grammar-test.zs';

const grammar = JSON.parse(fs.readFileSync(GRAMMAR, 'utf-8'));
const source = fs.readFileSync(FIXTURE, 'utf-8');

const highlighter = await createHighlighter({
	themes: ['monokai'],
	langs: [{ ...grammar, name: 'zscript' }],
});

// Tokenize the file as a whole so multi-line state (block comments, the
// preprocessor begin/end rule) is exercised the way an editor would.
const { tokens } = highlighter.codeToTokens(source, {
	lang: 'zscript', theme: 'monokai', includeExplanation: true,
});

const lines = source.split('\n');
const isInvalid = (t) => (t.explanation ?? [])
	.some(e => e.scopes.some(s => s.scopeName.startsWith('invalid')));

let checked = 0;
const failures = [];

for (const [i, line] of lines.entries()) {
	// `invalid-single` additionally asserts the whole construct flagged as ONE
	// token. Plain `invalid` can't tell `[@Flag16]` from `@Flag1` + `[6]`.
	const marker = line.match(/^\s*\/\/!\s*(valid|invalid|invalid-single)\s*$/);
	if (!marker) continue;

	// The marker describes the next non-blank, non-marker line.
	let target = i + 1;
	while (target < lines.length &&
		(!lines[target].trim() || /^\s*\/\/!/.test(lines[target]))) target++;
	if (target >= lines.length) continue;

	const wantInvalid = marker[1] !== 'valid';
	const wantSingle = marker[1] === 'invalid-single';
	// Ignore comment tokens so a `// note` on the line can't skew the result.
	const real = (tokens[target] ?? [])
		.filter(t => t.content.trim())
		.filter(t => !(t.explanation ?? [])
			.some(e => e.scopes.some(s => s.scopeName.startsWith('comment'))));
	const gotInvalid = real.some(isInvalid);
	const gotSingle = real.length === 1;

	checked++;
	if (gotInvalid !== wantInvalid || (wantSingle && !gotSingle)) {
		failures.push({
			line: target + 1,
			text: lines[target].trim(),
			expected: marker[1],
			actual: `${gotInvalid ? 'invalid' : 'valid'}, ${real.length} token(s)`,
			tokens: (tokens[target] ?? [])
				.filter(t => t.content.trim())
				.map(t => (isInvalid(t) ? `[${t.content.trim()}]` : t.content.trim()))
				.join(' '),
		});
	}
}

for (const f of failures) {
	console.error(`FAIL ${FIXTURE}:${f.line}  ${JSON.stringify(f.text)}`);
	console.error(`     expected ${f.expected}, got ${f.actual}  ->  ${f.tokens}`);
}

console.log(`\n${checked - failures.length}/${checked} grammar expectations met` +
	(failures.length ? ` (${failures.length} failing)` : ''));
process.exit(failures.length ? 1 : 0);
