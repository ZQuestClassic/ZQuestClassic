import * as vscode from 'vscode';
import * as assert from 'assert';
import { getDocUri, activate, setTestContent, executeDocumentSymbolProvider, executeHoverProvider } from './helper.js';
import { before } from 'mocha';
import { jestExpect as expect } from 'mocha-expect-snapshot';

function range(sLine: number, sChar: number, eLine: number, eChar: number) {
	const start = new vscode.Position(sLine, sChar);
	const end = new vscode.Position(eLine, eChar);
	return new vscode.Range(start, end);
}

async function testDiagnostics(uri: vscode.Uri, expectedDiagnostics: vscode.Diagnostic[]) {
	const actualDiagnostics = vscode.languages.getDiagnostics(uri)
		.map(({ message, range, severity }) => ({ message, range, severity }));
	for (const diag of actualDiagnostics) {
		if (diag.message.startsWith('Error in temp file')) {
			diag.message = 'Error in temp file';
		}
	}
	assert.deepStrictEqual(actualDiagnostics, expectedDiagnostics);
}

suite('ZScript extension', () => {
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
});
