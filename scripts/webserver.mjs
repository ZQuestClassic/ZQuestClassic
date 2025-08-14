import fs from 'node:fs/promises';
import http from 'http';
import mime from 'mime-types';
import path from 'path';

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
		'Content-Type': mime.lookup(path.extname(file)),
		'Content-Length': data.length,
		'Cross-Origin-Embedder-Policy': 'require-corp',
		'Cross-Origin-Opener-Policy': 'same-origin',
	});
	res.end(data);
});

server.listen(port, hostname, () => {
	console.log(`Server running at http://${hostname}:${port}/`);
});
