const viewStates = new Map();
let ws;
let wsConnected;
let data;
let table;

const Views = {
	load: '/load',
	new: '/new',
	results: (testResultsName) => `/results/${testResultsName}`,
	compare: (testResultsName) => `/compare/${testResultsName}`,
};

const socket = {
	updateView(view) {
		ws.send(JSON.stringify({
			command: 'update-view',
			params: { view },
		}));
	},
	run(params) {
		ws.send(JSON.stringify({
			command: 'run',
			params,
		}));
	},
	compare(params) {
		ws.send(JSON.stringify({
			command: 'compare',
			params,
		}));
	},
};

function startRun(filter) {
	const date = new Date().toLocaleDateString(undefined, {
		year: 'numeric',
		month: 'short',
		day: '2-digit',
		hour: 'numeric',
		minute: 'numeric',
		second: 'numeric',
	}).replace(/[\s,:]+/g, '_');
	const randomWords1 = ['artifical', 'busy', 'clever', 'dynamic', 'easy', 'fun'];
	const randomWords2 = ['ant', 'beaver', 'cat', 'dog', 'ent', 'fae'];
	const pick = (arr) => arr[Math.floor(Math.random() * arr.length)];
	const name = `${pick(randomWords1)}-${pick(randomWords2)}-${date}`;
	socket.run({ name, filter });
	show(Views.results(name));
}

/**
 * @param {string} query
 * @param {HTMLElement=} rootEl 
 * @return {HTMLElement}
 */
function find(query, rootEl = document.documentElement) {
	const el = rootEl.querySelector(query);
	if (!el) throw new Error(`missing element ${query}`);
	return el;
}

/**
 * @param {string} query
 * @param {HTMLElement=} rootEl 
 * @return {HTMLElement[]}
 */
function findAll(query, rootEl = document.documentElement) {
	return [...rootEl.querySelectorAll(query)];
}

/**
 * @param {string} templateName
 * @param {any} data
 * @return {HTMLElement}
 */
function createComponent(templateName, data) {
	const tmplEl = find(`#tmpl-${templateName}`);
	const el = tmplEl.content.cloneNode(true).firstElementChild;
	for (const [key, value] of Object.entries(data)) {
		const subEl = find(`.${templateName}__${key}`, el);
		if (!subEl) {
			throw new Error('missing: ' + key);
		}
		subEl.textContent = value;
	}
	return el;
}

function renderNewSection(data) {
	const replays = data.replays.map(r => {
		return {
			replay: r.replay,
			...r.meta,
		};
	});
	table = createTable('.section.new table', replays);
}

async function main() {
	const response = await fetch('./data.json');
	data = await response.json();

	renderNewSection(data);

	await new Promise((resolve, reject) => {
		const url = new URL('/ws', location.href);
		url.protocol = 'ws:';
		ws = new WebSocket(url);
		ws.addEventListener('open', () => {
			wsConnected = true;
			resolve();
		});
		ws.addEventListener('error', reject);
		ws.onmessage = (e) => {
			const msg = JSON.parse(e.data);
			if (msg.type === 'view') {
				viewStates.set(msg.view, msg.data);
				if (location.pathname === msg.view) {
					render(msg.view, msg.data);
				}
			}
		}
	});

	if (location.pathname) {
		show(location.pathname);
	}
}

function getSection(name) {
	return find(`.section.${name} .content`);
}

function openSection(name) {
	const sectionEl = find(`.section.${name}`);

	for (const el of findAll('.section')) {
		el.classList.toggle('hidden', el !== sectionEl);
	}

	return find('.content', sectionEl);
}

function show(view) {
	if (location.href === view)
		return;

	history.pushState({ view }, '', view);
	socket.updateView(view);
	render(view, viewStates.get(view) ?? {});
}

function getViewParts(view) {
	return view.split('/').filter(Boolean);
}

function render(view, state) {
	if (view === '/') {
		for (const el of findAll('.section')) {
			el.classList.toggle('hidden', true);
		}
		return;
	}

	const parts = getViewParts(view);
	const type = parts[0];
	const sectionEl = openSection(type);

	switch (type) {
		case 'new': {
			// nothing
			break;
		}
		case 'load': {
			sectionEl.textContent = '';
			for (const testResult of data.testResults) {
				const el = document.createElement('div');
				// const numReplays = testResult.results.runs[0].length;
				const date = new Date(testResult.date * 1000);
				el.textContent = `${testResult.name} - ${date.toLocaleString()}`;
				const viewEl = document.createElement('button');
				viewEl.textContent = 'View';
				el.prepend(viewEl);
				sectionEl.append(el);
				viewEl.addEventListener('click', () => {
					show(Views.results(testResult.name));
				});
			}
			break;
		}
		case 'results': {
			if (state.error) {
				find('.results__error', sectionEl).textContent = state.error;
				return;
			}

			find('.results__error', sectionEl).textContent = '';
			find('.results__name', sectionEl).textContent = state.name;

			const listEl = find('.replays-list', sectionEl);
			listEl.textContent = '';
			const { results } = state || {};
			if (!results) {
				listEl.textContent = 'loading ... (this should only take a few seconds)';
				return;
			}

			let failed = false;
			let finished = true;
			for (const runs of results.runs) {
				for (const run of runs) {
					let status = run.success ? 'success' : (run.stopped ? 'failed' : 'running');
					if (run.fps === null) status = 'pending';
					if (status === 'failed') failed = true;
					if (status === 'pending' || status === 'running') finished = false;
					const statusClass = `replay--${status}`;
					if (status === 'failed') {
						status += ` frame ${run.failing_frame}`;
					}
					if (status === 'running') {
						status += ` ${run.frame} / ${run.num_frames}`;
					}
					const el = createComponent('replay', {
						name: run.name,
						status,
						fps: run.fps,
					});
					el.classList.add(statusClass);
					listEl.append(el);
				}
			}
			if (results.runs.length === 0) finished = false;

			const compareEl = find('.btn-compare');
			compareEl.classList.toggle('hidden', !failed || !finished);
			break;
		}
		case 'compare': {
			find('.status', sectionEl).textContent = state.status;
			const showEl = find('.show-link', sectionEl);
			showEl.classList.toggle('hidden', state.status !== 'done');
			showEl.href = `/compare/show/${parts[1]}`;

			const generateEl = find('.btn-compare-generate', sectionEl);
			if (state.status === 'done') {
				generateEl.textContent = 'Regenerate';
				generateEl.disabled = false;
			} else if (state.status) {
				generateEl.textContent = 'Generating ...';
				generateEl.disabled = true;
			} else {
				generateEl.textContent = 'Generate';
				generateEl.disabled = false;
			}
			break;
		}
	}
}

window.addEventListener('popstate', (e) => {
	const view = e.state.view;
	render(view, viewStates.get(view) ?? {});
	socket.updateView(view);
});

find('.btn-new').addEventListener('click', () => {
	show(Views.new);
});

find('.btn-load').addEventListener('click', async () => {
	if (!wsConnected) return;

	const response = await fetch('./data.json');
	data = await response.json();
	show(Views.load);
});

find('.btn-compare').addEventListener('click', () => {
	const name = getViewParts(location.pathname)[1];
	show(Views.compare(name));
});

find('.btn-compare-generate').addEventListener('click', () => {
	const name = getViewParts(location.pathname)[1];
	socket.compare({ name });
});

main();
