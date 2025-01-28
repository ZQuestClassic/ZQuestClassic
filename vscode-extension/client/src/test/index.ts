import * as path from 'path';
import * as Mocha from 'mocha';
import { glob } from 'glob';
import * as mochaExpect from 'mocha-expect-snapshot';

export async function run(): Promise<void> {
	// Create the mocha test
	const mocha = new Mocha({
		ui: 'tdd',
		color: true,
		require: ['mocha-expect-snapshot'],
		// @ts-expect-error
		rootHooks: mochaExpect.mochaHooks,
	});
	mocha.timeout(100000);

	// @ts-expect-error
	mochaExpect.setSnapshotResolver({resolveSnapshotPath(name: string) {
		return `${__dirname}/../../src/test/__snapshots__/${path.basename(name)}.snap`;
	}});

	const testsRoot = __dirname;

	const files = await glob.glob('**.test.js', { cwd: testsRoot });
	// Add files to the test suite
	files.forEach(f => mocha.addFile(path.resolve(testsRoot, f)));
	try {
		// Run the mocha test
		await new Promise<void>((resolve, reject) => {
			mocha.run(failures => {
				if (failures > 0) {
					reject(`${failures} tests failed.`);
				} else {
					resolve();
				}
			});
		});
	} catch (err) {
		console.error(err);
		throw err;
	}
}
