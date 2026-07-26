import fs from 'node:fs/promises';
import http from 'http';
import path from 'path';
import {fileURLToPath} from 'url';

// Just enough to cover what the web package serves. Anything else (.qst, .zplay,
// .pat, ...) is fetched as bytes, so the octet-stream default is fine.
const mimeTypes = {
	'.bmp': 'image/bmp',
	'.css': 'text/css',
	'.gif': 'image/gif',
	'.html': 'text/html',
	'.ico': 'image/vnd.microsoft.icon',
	'.jpeg': 'image/jpeg',
	'.jpg': 'image/jpeg',
	'.js': 'text/javascript',
	'.json': 'application/json',
	'.map': 'application/json',
	'.md': 'text/markdown',
	'.mjs': 'text/javascript',
	'.png': 'image/png',
	'.svg': 'image/svg+xml',
	'.ttf': 'font/ttf',
	'.txt': 'text/plain',
	'.wasm': 'application/wasm',
};

const __dirname = path.posix.dirname(fileURLToPath(import.meta.url));
const projectRootDir = path.posix.normalize(path.posix.join(__dirname, '..'));
const hostname = 'localhost';
const port = 8000;
const rootDir = process.argv[2] || 'build_emscripten/Debug/packages/web';

const server = http.createServer(async (req, res) => {
	let file = new URL(`http://${hostname}${req.url}`).pathname;
	if (file.endsWith('/')) {
		file += 'index.html';
	} else if (!file.split('/').at(-1).includes('.')) {
		file += '/index.html';
	}

	if (file.startsWith('/host')) {
		if (!(file.endsWith('.qst') || file.endsWith('.zplay') || file.endsWith('.sav'))) {
			console.log(403, req.url);
			res.writeHead(403);
			res.end();
			return;
		}

		file = file.replace(/^\/host/, '');
	} else {
		file = path.join(rootDir, file);
	}

	let data;
	try {
		data = await fs.readFile(file);
	} catch (err) {
		console.log(404, req.url);
		res.writeHead(404);
		res.end();
		return;
	}

	console.log(200, req.url);
	res.writeHead(200, {
		'Content-Type': mimeTypes[path.extname(file).toLowerCase()] || 'application/octet-stream',
		'Content-Length': data.length,
		'Cross-Origin-Embedder-Policy': 'require-corp',
		'Cross-Origin-Opener-Policy': 'same-origin',
	});
	res.end(data);
});

server.listen(port, hostname, () => {
	console.log(`Server running at http://${hostname}:${port}/`);
	console.log(`root dir: ${rootDir}\n`);
	console.log('tip: can access files from host filesystem via /host/...');
	console.log(`for example: http://localhost:8000/play/?test=/host${projectRootDir}/tests/replays/playground/playground.qst&dmap=0&screen=0`);
	console.log(`or:          http://localhost:8000/play/?assert=/host${projectRootDir}/tests/replays/playground/circle.zplay`);
});
