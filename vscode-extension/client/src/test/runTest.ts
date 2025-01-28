import * as path from 'path';
import * as fs from 'fs';

import { runTests } from '@vscode/test-electron';
import { execFileSync } from 'child_process';

const ROOT = path.join(__dirname, '../../../..');

function getZcPath(version: string): string {
	const zcPath = execFileSync('python', ['scripts/archives.py', 'download', version], {
		cwd: ROOT,
		encoding: 'utf-8',
	}).trim();

	if (fs.existsSync(`${zcPath}/ZQuest Classic.app`)) {
		return `${zcPath}/ZQuest Classic.app`;
	}

	return zcPath;
}

function findZcPath(paths: string[]): string {
	return paths.find(p => fs.existsSync(p));
}

async function main() {
	try {
		// The folder containing the Extension Manifest package.json
		// Passed to `--extensionDevelopmentPath`
		const extensionDevelopmentPath = path.resolve(__dirname, '../../../');

		// The path to test runner
		// Passed to --extensionTestsPath
		const extensionTestsPath = path.resolve(__dirname, './index');

		const extensionTestsEnv: Record<string, string> = {
			ZC_PATH_255: getZcPath('2.55.7'),
			ZC_PATH_3_NO_JSON: getZcPath('3.0.0-prerelease.60+2024-08-04'),
			ZC_PATH_LATEST: findZcPath([process.env.BUILD_FOLDER, `${ROOT}/build/Release`, `${ROOT}/build/RelWithDebInfo`]),
		};
		console.log('Using these binaries for extension test:', extensionTestsEnv);

		extensionTestsEnv.TEST_ZSCRIPT = '1';

		// Download VS Code, unzip it and run the integration test
		await runTests({ extensionDevelopmentPath, extensionTestsPath, extensionTestsEnv });
	} catch (err) {
		console.error('Failed to run tests');
		process.exit(1);
	}
}

main();
