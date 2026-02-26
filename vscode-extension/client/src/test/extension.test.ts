import * as vscode from 'vscode';
import * as assert from 'assert';
import { getDocUri, activate, setTestContent, executeDocumentSymbolProvider, executeHoverProvider, sleep, executeCompletionItemProvider } from './helper.js';
import { before } from 'mocha';
import { jestExpect as expect } from 'mocha-expect-snapshot';

function range(sLine: number, sChar: number, eLine: number, eChar: number) {
	const start = new vscode.Position(sLine, sChar);
	const end = new vscode.Position(eLine, eChar);
	return new vscode.Range(start, end);
}

async function testDiagnostics(uri: vscode.Uri, expectedDiagnostics: vscode.Diagnostic[]) {
	// Diagnostics are a push-model, so lets give the server a moment to process the script.
	await sleep(2000);
	const actualDiagnostics = vscode.languages.getDiagnostics(uri)
		.map(({ message, range, severity }) => ({ message, range, severity }));
	for (const diag of actualDiagnostics) {
		if (diag.message.startsWith('Error in temp file')) {
			diag.message = 'Error in temp file';
		}
	}
	assert.deepStrictEqual(actualDiagnostics, expectedDiagnostics);
}

suite('ZScript extension', function () {
	// Not ideal, but fixing flaky tests can be a full-time job.
	this.retries(3);

	before(async function () {
		if (!process.env.CI) {
			this.snapshotStateOptions = { updateSnapshot: 'all' };
		}
	});

	suite('Diagnoses errors and warnings', () => {
		const uri = getDocUri('diagnostics.zs');

		test('2.55', async () => {
			await activate('2.55', uri);
			await testDiagnostics(uri, [
				{ message: `Warning S094: Variable 'Screen->HasItem' is deprecated, and should not be used.`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
			]);
		});

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			await testDiagnostics(uri, [
				{ message: `Warning S094: Variable 'screendata->HasItem' is deprecated, and should not be used.`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
				{ message: `Error S102: Function 'fn' is not void, and must return a value!`, range: range(4, 4, 4, 6), severity: vscode.DiagnosticSeverity.Error },
			]);
		});

		test('latest', async () => {
			await activate('latest', uri);
			await testDiagnostics(uri, [
				{ message: `S094: Variable 'screendata->HasItem' is deprecated, and should not be used.\nCheck \`->Item > -1\` instead!`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
				{ message: `S102: Function 'fn' is not void, and must return a value!`, range: range(4, 4, 4, 6), severity: vscode.DiagnosticSeverity.Error },
			]);
		});
	});

	suite('Diagnoses errors and warnings - unsaved changes', () => {
		const uri = getDocUri('empty.zs');

		before(async () => {
			await setTestContent(uri, '#option WARN_DEPRECATED warn\n\nimport "std.zh"\n\nvoid fn() {\n\tScreen->HasItem;\n}\n');
		});

		test('2.55', async () => {
			await activate('2.55', uri);
			await testDiagnostics(uri, [
				{ message: `Warning S094: Variable 'Screen->HasItem' is deprecated, and should not be used.`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
			]);
		});

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			await testDiagnostics(uri, [
				{ message: `Warning S094: Variable 'screendata->HasItem' is deprecated, and should not be used.`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
			]);
		});

		test('latest', async () => {
			await activate('latest', uri);
			await testDiagnostics(uri, [
				{ message: `S094: Variable 'screendata->HasItem' is deprecated, and should not be used.\nCheck \`->Item > -1\` instead!`, range: range(5, 1, 5, 16), severity: vscode.DiagnosticSeverity.Warning },
			]);
		});
	});

	suite('Diagnoses parser errors', () => {
		const uri = getDocUri('parser_errors.zs');

		test('2.55', async () => {
			await activate('2.55', uri);
			await testDiagnostics(uri, [
				{ message: `unexpected LBRACE`, range: range(2, 22, 2, 23), severity: vscode.DiagnosticSeverity.Error },
				{ message: `Error in temp file`, range: range(0, 0, 0, 0), severity: vscode.DiagnosticSeverity.Error },
			]);
		});

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			await testDiagnostics(uri, [
				{ message: `unexpected LBRACE`, range: range(2, 22, 2, 23), severity: vscode.DiagnosticSeverity.Error },
				{ message: `Error in temp file`, range: range(0, 0, 0, 0), severity: vscode.DiagnosticSeverity.Error },
			]);
		});

		test('latest', async () => {
			await activate('latest', uri);
			await testDiagnostics(uri, [
				{ message: `syntax error, unexpected LBRACE`, range: range(2, 22, 2, 23), severity: vscode.DiagnosticSeverity.Error },
			]);
		});
	});

	suite('Default includes', () => {
		const uri = getDocUri('default_includes.zs');

		before(async () => {
			const config = vscode.workspace.getConfiguration('zscript');
			await config.update('defaultIncludeFiles', ['std.zh'], vscode.ConfigurationTarget.Global);
		});

		test('2.55', async () => {
			await activate('2.55', uri);
			await testDiagnostics(uri, []);
		});

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			await testDiagnostics(uri, []);
		});

		test('latest', async () => {
			await activate('latest', uri);
			await testDiagnostics(uri, []);
		});
	});

	suite('Hover tooltips', () => {
		const uri = getDocUri('symbols.zs');

		const getHovers = async (version: string) => {
			const positions = [
				new vscode.Position(27, 16),
				new vscode.Position(3, 20),
				new vscode.Position(36, 10),
				new vscode.Position(61, 8),
				version !== '3-no-json' ? new vscode.Position(67, 5) : null,
				new vscode.Position(82, 55),
				new vscode.Position(82, 78),
				version !== '3-no-json' ? new vscode.Position(97, 12) : null,
			].filter(Boolean);
			return (await Promise.all(positions.map(pos => executeHoverProvider(uri, pos))))
				.map((hovers, i) => {
					if (!hovers.length) {
						throw new Error(`empty hover for ${positions[i].line}, ${positions[i].character}`);
					}
					return hovers;
				})
				.flat()
				.map(hover => ({
					contents: hover.contents.map(c => c instanceof vscode.MarkdownString ? c.value : c),
					range: hover.range,
				}));
		};

		// Not supported in 2.55.

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			const hovers = await getHovers('3-no-json');
			expect(hovers).toMatchSnapshot();
		});

		test('latest', async () => {
			await activate('latest', uri);
			const hovers = await getHovers('latest');
			expect(hovers).toMatchSnapshot();
		});
	});

	suite('Document symbol outline', () => {
		const uri = getDocUri('symbols.zs');

		const getSymbols = async () => {
			const symbols = await executeDocumentSymbolProvider(uri);
			const flattened = [];
			const collect = (s: vscode.DocumentSymbol) => {
				const { name, kind, range, selectionRange } = s;
				flattened.push({ name, kind, range, selectionRange, children: s.children.length });
				s.children.forEach(collect);
			};
			symbols.forEach(collect);
			return flattened;
		};

		// Not supported in 2.55.

		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			expect(await getSymbols()).toMatchSnapshot();
		});

		test('latest', async () => {
			await activate('latest', uri);
			expect(await getSymbols()).toMatchSnapshot();
		});
	});

	suite('Autocomplete global constants', () => {
		const uri = getDocUri('empty.zs');

		const getCompletions = async () => {
			// The cache is populated on a configuration change/startup, so we give it an extra moment.
			await sleep(2000);

			const list = await executeCompletionItemProvider(uri, new vscode.Position(0, 0));
			const items = list.items.filter(i =>
				(i.label === 'DIR_UP' ||
					i.label === 'Trace' ||
					i.label === 'PI' ||
					i.label === 'Ghost_Dir') &&
				i.kind !== vscode.CompletionItemKind.Text // Ignore VS Code's default word suggestions
			);
			items.sort((a, b) => (a.label as string).localeCompare(b.label as string));
			return items.map(i => ({
				label: i.label,
				kind: i.kind,
				detail: i.detail,
				documentation: i.documentation instanceof vscode.MarkdownString ? i.documentation.value : i.documentation,
			}));
		};

		// Doesn't work, but shouldn't error.
		test('2.55', async () => {
			await activate('2.55', uri);
			const completions = await getCompletions();
			expect(completions).toHaveLength(0);
		});

		// Doesn't work, but shouldn't error.
		test('3-no-json', async () => {
			await activate('3-no-json', uri);
			const completions = await getCompletions();
			expect(completions).toHaveLength(0);
		});

		test('latest', async () => {
			await activate('latest', uri);
			const completions = await getCompletions();
			expect(completions).toMatchSnapshot();
			expect(completions).toHaveLength(4);
		});
	});

	suite('Autocomplete w/ context, member access', () => {
		const uri1 = getDocUri('autocomplete_1.zs');
		const uri2 = getDocUri('autocomplete_2.zs');

		before(async () => {
			// Activate the extension and open uri1.
			await activate('latest', uri1);

			// Open uri2 so the language server knows it exists in the workspace
			const doc2 = await vscode.workspace.openTextDocument(uri2);
			await vscode.window.showTextDocument(doc2);

			// Set the content for the multi-file test ahead of time
			await setTestContent(uri2, 'int GLOBAL_VAR = 1;');

			// Switch focus back to uri1
			const doc1 = await vscode.workspace.openTextDocument(uri1);
			await vscode.window.showTextDocument(doc1);

			// Wait for the initial server spin-up and global caching
			await sleep(2000);
		});

		test('Stale AST resilience (newline at bottom of function)', async () => {
			// Step 1: Provide a valid, compileable script so the AST successfully generates
			const validCode = 'void fn() {\n\tauto s = Screen;\n\tauto b = new bitmap(1, 1);\n}';
			await setTestContent(uri1, validCode);
			await sleep(2000); // Wait for the compiler job to succeed and cache metadata

			// Step 2: Simulate typing `s->` on a new line. 
			// We do NOT wait 2000ms here, because we want to test the LSP while the AST is theoretically stale/invalid.
			const doc = await vscode.workspace.openTextDocument(uri1);
			const editor = await vscode.window.showTextDocument(doc);
			await editor.edit(eb => {
				eb.insert(new vscode.Position(3, 0), '\n\n\ts->\n');
			});
			await sleep(500); // Brief pause to let VS Code sync the document change to the LSP

			// Step 3: Trigger completion right after `s->`
			const list = await executeCompletionItemProvider(uri1, new vscode.Position(5, 4));
			const hasLayerMap = list.items.some(i => i.label === 'LayerMap');
			expect(hasLayerMap).toBe(true);
		});

		test('Screen-> member access', async () => {
			// Even without compiling successfully, `Screen` is a global variable with a known type.
			await setTestContent(uri1, 'void fn() {\n\tScreen->\n}');
			await sleep(500);

			const list = await executeCompletionItemProvider(uri1, new vscode.Position(1, 9));
			const hasLayerMap = list.items.some(i => i.label === 'LayerMap');
			expect(hasLayerMap).toBe(true);
		});

		test('b-> (bitmap) member access', async () => {
			// Establish the local variable `b` in the AST
			const validCode = 'void fn() {\n\tauto b = new bitmap(1, 1);\n}';
			await setTestContent(uri1, validCode);
			await sleep(2000);

			const doc = await vscode.workspace.openTextDocument(uri1);
			const editor = await vscode.window.showTextDocument(doc);
			await editor.edit(eb => eb.insert(new vscode.Position(2, 0), '\tb->\n'));
			await sleep(500);

			const list = await executeCompletionItemProvider(uri1, new vscode.Position(2, 4));
			const hasCountColor = list.items.some(i => i.label === 'CountColor');
			expect(hasCountColor).toBe(true);
		});

		[
			'Game->CreateBitmap()->',
			'Game->CreateBitmap(100, 100)->',
			'Game->CreateBitmap(100, 100 + 100)->',
			'Game->CreateBitmap(Sqrt(12) + 1 + 123,\n\t1 + 333 + Sqrt(12))->',
		].forEach(variant => {
			test(`Function return value - ${variant.replace(/\n\t/g, ' ')}`, async () => {
				const validCode = 'void fn() {\n\n}';
				await setTestContent(uri1, validCode);
				await sleep(2000);

				const doc = await vscode.workspace.openTextDocument(uri1);
				const editor = await vscode.window.showTextDocument(doc);
				await editor.edit(eb => eb.insert(new vscode.Position(2, 0), `\t${variant}\n`));
				await sleep(500);

				// Calculate where the `->` actually ended up.
				const lines = variant.split('\n');
				const targetLine = 2 + (lines.length - 1); // Add line offset if variant has newlines.
				// If it's on the first line, add 1 for the '\t'. Otherwise, just take the length of the string.
				const targetChar = (lines.length === 1 ? 1 : 0) + lines[lines.length - 1].length;

				const list = await executeCompletionItemProvider(uri1, new vscode.Position(targetLine, targetChar));

				const hasCountColor = list.items.some(i => i.label === 'CountColor');
				expect(hasCountColor).toBe(true);

				const hasRectangle = list.items.some(i => i.label === 'Rectangle');
				expect(hasRectangle).toBe(true);
			});
		});

		test('Global constant suggestions (DIR_)', async () => {
			await setTestContent(uri1, 'void fn() {\n\tDIR_\n}');
			await sleep(500);

			const list = await executeCompletionItemProvider(uri1, new vscode.Position(1, 5));
			const hasDirUp = list.items.some(i => i.label === 'DIR_UP');
			expect(hasDirUp).toBe(true);
		});

		test('Cross-file global variables', async () => {
			// `autocomplete_2.zs` was already compiled in the `before` block with `GLOBAL_VAR`.
			// We just need to check if typing in uri1 can see it.
			await setTestContent(uri1, 'void fn() {\n\tGLOBAL_V\n}');
			await sleep(500);

			const list = await executeCompletionItemProvider(uri1, new vscode.Position(1, 9));

			// Filter out VS Code's default text/word suggestions just in case it grabs the literal text
			const hasGlobalVar = list.items.some(i =>
				i.label === 'GLOBAL_VAR' && i.kind !== vscode.CompletionItemKind.Text
			);
			expect(hasGlobalVar).toBe(true);
		});
	});
});
