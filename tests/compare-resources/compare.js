const tracksEl = find('.snapshot-tracks');
const frameViewEl = find('.frame-view');
const titleEl = find('.replay-title');

/**
 * @typedef ReplayTestResults
 * @property {string} runs_on
 * @property {string} arch
 * @property {boolean} ci
 * @property {number=} workflow_run_id
 * @property {string=} git_ref
 * @property {string} zc_version
 * @property {string} time
 * @property {RunResult[]} runs Note: this differs from the Python type. We only have a single set of runs, not one for each attempt.
 */

/**
 * @typedef RunResult
 * @property {string} name
 * @property {string} directory
 * @property {boolean} success
 * @property {number=} exit_code
 * @property {string=} duration
 * @property {string} fps
 * @property {string} frame
 * @property {number} failing_frame
 * @property {number} unexpected_gfx_frames
 * @property {number} unexpected_gfx_segments
 * @property {number} unexpected_gfx_segments_limited
 * @property {Array<{path: string, frame: number, unexpected: boolean, hash: string}>} snapshots
 */

// `__TEST_RUNS__` has been defined by the python generation script.
/** @type {ReplayTestResults[]} */
const testRuns = __TEST_RUNS__;

/** @type {Array<RunResult & {source: string}>} */
let tracks = [];
/** @type {Array<{frame: number, snapshots: Array<{url: string, frame: number, unexpected: boolean}>, tracks: number[]}>} */
let trackFrames = [];
/** @type {Array<{start: number, end: number}>} */
let segments = [];
let frameViewFrameIndex = -1;
let frameViewTrackIndex = -1;
let trackViewDirty = false;

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

function setTitle(title) {
    titleEl.textContent = title;
}

function getOptions() {
    return {
        replay: find('.select__replays select').value,
        segment: Number(find('.select__segments select').value) || 0,
        showOnlyUnexpectedFrames: find('.label__expected-filter input').checked,
        showDiff: find('.label__diff input').checked,
    };
}

function showOptions(view) {
    for (let el of findAll('.toolbar__options > *')) {
        const forView = el.dataset['view'];
        if (forView !== undefined) el.classList.toggle('hidden', view !== forView);
    }
}

function findNextFrame(delta) {
    const currentIndex = trackFrames.findIndex(f => f.frame === frameViewFrameIndex);
    const nextIndex = currentIndex + delta;
    if (nextIndex < 0 || nextIndex >= trackFrames.length) return -1;

    return trackFrames[nextIndex].frame;
}

const observer = new IntersectionObserver((entries) => {
    const options = getOptions();
    for (const entry of entries) {
        if (!entry.isIntersecting) continue;

        // Without this, lazy images won't load on the left when scrolling until the leftmost
        // pixel is visible. Seems like a browser bug.
        for (const imgEl of findAll('img', entry.target)) {
            imgEl.loading = 'eager';
        }

        const frameIndex = Number(entry.target.dataset.frame);
        const trackFrameEls = findAll('.track-frame-present, .track-frame-missing', entry.target);

        if (options.showDiff) {
            for (let i = 1; i < tracks.length; i++) {
                (async function () {
                    const [img1, img2] = await Promise.all([
                        loadImageFromTrack(frameIndex, 0),
                        loadImageFromTrack(frameIndex, i),
                    ]);

                    if (!img1 || !img2) {
                        return;
                    }

                    const diffImg = await getDiffImage(img1, img2);
                    diffImg.classList.add('track-frame__image');
                    trackFrameEls[i].innerHTML = '';
                    trackFrameEls[i].append(diffImg);
                })();
            }
        }
    }
}, {
    root: null,
    rootMargin: '0px',
    threshold: 0
});

function init() {
    console.log({ testRuns });
    for (const testResults of testRuns) {
        // We only have a single set of runs for each test replay.
        testResults.runs = testResults.runs[0];
    }

    function onToggleUnexpectedFilter() {
        renderTracks(getOptions());
    }

    function onToggleDiff(value) {
        if (value !== undefined) find('.label__diff input').checked = value;
        if (frameViewFrameIndex !== -1) {
            showFrameView(frameViewFrameIndex, frameViewTrackIndex, getOptions());
            trackViewDirty = true;
        } else {
            renderTracks(getOptions());
        }
    }

    function onSwitchTrack(delta) {
        let nextTrackIndex = frameViewTrackIndex + delta;
        if (nextTrackIndex < 0) nextTrackIndex = tracks.length - 1;
        if (nextTrackIndex >= tracks.length) nextTrackIndex = 0;
        showFrameView(frameViewFrameIndex, nextTrackIndex, getOptions());
    }

    function onSwitchFrame(delta) {
        const nextFrameIndex = findNextFrame(delta);
        if (nextFrameIndex === -1) return;

        showFrameView(nextFrameIndex, frameViewTrackIndex, getOptions());
    }

    find('.label__expected-filter input').addEventListener('change', onToggleUnexpectedFilter);
    find('.label__diff input').addEventListener('change', () => onToggleDiff());

    document.addEventListener('keypress', (e) => {
        if (frameViewFrameIndex !== -1) {
            if (e.key === ',') {
                onSwitchTrack(-1);
            }
            if (e.key === '.') {
                onSwitchTrack(1);
            }
        }

        if (e.key === 'd') {
            onToggleDiff(!find('.label__diff input').checked);
        }
    });

    document.addEventListener('keydown', (e) => {
        if (frameViewFrameIndex !== -1) {
            if (e.code === 'ArrowLeft') {
                onSwitchFrame(-1);
            }
            if (e.code === 'ArrowRight') {
                onSwitchFrame(1);
            }
            if (e.code === 'ArrowUp') {
                onSwitchTrack(-1);
            }
            if (e.code === 'ArrowDown') {
                onSwitchTrack(1);
            }
            if (e.code === 'Escape') {
                hideFrameView();
            }
        }
    });

    tracksEl.addEventListener('click', (e) => {
        if (!e.target.classList.contains('track-frame__image')) return;

        const containerEl = e.target.closest('.track-frame-container');
        const trackFrameEl = e.target.closest('.track-frame');
        const frameIndex = Number(containerEl.dataset['frame']);
        const trackIndex = Number(trackFrameEl.dataset['track']);
        showFrameView(frameIndex, trackIndex, getOptions());
    });

    find('.frame-view').addEventListener('click', () => {
        hideFrameView();
    });

    const replays = [...new Set(testRuns.map(r => r.runs.map(r => r.name)).flat())];
    const selectEl = find('.select__replays select');
    for (const replay of replays) {
        const el = document.createElement('option');
        el.value = el.textContent = replay;
        selectEl.append(el);
    }

    selectEl.addEventListener('change', () => renderTracks(getOptions()));
    renderTracks(getOptions());

    const segmentsEl = find('.select__segments select');
    segmentsEl.addEventListener('change', () => {
        const options = getOptions();
        scrollToSegment(options.segment);
    });
}

/**
 * @param {ReturnType<typeof getOptions>} options
 */
function renderTracks(options) {
    trackViewDirty = false;
    showOptions('tracks');

    tracks = [];
    for (const testResults of testRuns) {
        for (const run of testResults.runs) {
            if (run.name === options.replay) {
                tracks.push({
                    source: testResults.label,
                    ...run,
                });
            }
        }
    }
    console.log({ tracks });

    let minFrame = tracks[0].snapshots[0].frame;
    let maxFrame = tracks[0].snapshots[0].frame;
    for (const track of tracks) {
        minFrame = Math.min(minFrame, track.snapshots[track.snapshots.length - 1].frame);
        maxFrame = Math.max(maxFrame, track.snapshots[track.snapshots.length - 1].frame);
    }

    tracksEl.textContent = '';
    tracksEl.classList.remove('hidden');
    frameViewEl.classList.add('hidden');
    setTitle(options.replay);

    const trackIndicies = [];
    for (const _ of tracks) {
        trackIndicies.push(0);
    }

    trackFrames = [];

    let frame = -1;
    while (frame <= maxFrame) {
        let nextFrame = Number.POSITIVE_INFINITY;
        for (let i = 0; i < tracks.length; i++) {
            if (trackIndicies[i] >= tracks[i].snapshots.length) continue;

            if (nextFrame > tracks[i].snapshots[trackIndicies[i]].frame) {
                nextFrame = tracks[i].snapshots[trackIndicies[i]].frame;
            }
        }

        if (!Number.isFinite(nextFrame)) break;

        // At least one track has a snapshot for this frame.
        frame = nextFrame;

        trackFrames.push({ frame, snapshots: [], tracks: [] });

        for (let i = 0; i < tracks.length; i++) {
            if (trackIndicies[i] === tracks[i].snapshots.length) continue;

            if (tracks[i].snapshots[trackIndicies[i]].frame === nextFrame) {
                trackFrames[trackFrames.length - 1].snapshots.push(tracks[i].snapshots[trackIndicies[i]]);
                trackFrames[trackFrames.length - 1].tracks.push(i);
            }
        }

        // Increment track indicies.
        for (let i = 0; i < tracks.length; i++) {
            if (trackIndicies[i] === tracks[i].snapshots.length) continue;

            if (tracks[i].snapshots[trackIndicies[i]].frame === nextFrame) {
                trackIndicies[i] += 1;
            }
        }
    }

    // Render track sources.
    {
        const containerEl = document.createElement('div');
        containerEl.className = 'track-frame-container';
        tracksEl.append(containerEl);
        for (const track of tracks) {
            let el = document.createElement('div');
            el.className = 'track-frame--source';
            el.innerHTML = '&nbsp;';
            containerEl.append(el);

            el = document.createElement('div');
            el.className = 'track-frame track-frame--source';
            el.innerHTML = track.source.split(' ').join('<br>');
            containerEl.append(el);
        }
    }

    if (options.showOnlyUnexpectedFrames) {
        trackFrames = trackFrames.filter(trackFrame => {
            if (tracks.length === 1) return trackFrame.snapshots[0].unexpected;

            const baselineMissing = !trackFrame.tracks.includes(0);
            return baselineMissing || trackFrame.snapshots.some((snapshot, i) => i > 0 && snapshot.unexpected);
        });
    }

    segments = [];
    let startSegment = true;

    for (let i = 0; i < trackFrames.length; i++) {
        const trackFrame = trackFrames[i];

        if (startSegment) {
            segments.push({start: i, end: null});
            startSegment = false;
        }

        const containerEl = document.createElement('div');
        containerEl.className = 'track-frame-container';
        containerEl.dataset['frame'] = trackFrame.frame;
        containerEl.innerHTML = `<div class='track-frame-number'>${trackFrame.frame}</div>`;
        tracksEl.append(containerEl);
        observer.observe(containerEl);

        for (let j = 0; j < tracks.length; j++) {
            const index = trackFrame.tracks.indexOf(j);
            const snapshot = trackFrame.snapshots[index];
            const el = document.createElement('div');
            el.className = 'track-frame ' + (snapshot ? 'track-frame-present' : 'track-frame-missing');
            el.dataset['track'] = j;
            containerEl.append(el);

            if (snapshot) {
                el.innerHTML = `<img loading=lazy class='track-frame__image' src='${snapshot.path}'>`;
            }

            const unexpected = j > 0 ?
                (snapshot?.unexpected || (!!snapshot !== trackFrame.tracks.includes(0))) :
                snapshot?.unexpected;
            {
                const statusEl = document.createElement('div');
                statusEl.className = 'track-frame ' + (unexpected ? 'track-frame__unexpected' : 'track-frame__ok');
                containerEl.append(statusEl);
            }
            if (unexpected) {
                find('.track-frame-number', containerEl).classList.add('track-frame-number--unexpected')
            }
        }

        if (i + 1 < trackFrames.length && trackFrames[i].frame + 60 < trackFrames[i + 1].frame) {
            const el = document.createElement('div');
            el.className = 'track-frame-container track-frame-container--skip';
            el.innerHTML = '<div>.</div>';
            tracksEl.append(el);
            startSegment = true;
            segments[segments.length - 1].end = i;
        }
    }
    segments[segments.length - 1].end = trackFrames.length - 1;

    console.log({ trackFrames, segments });

    const segmentsEl = find('.select__segments select');
    segmentsEl.innerHTML = '';
    for (let i = 0; i < segments.length; i++) {
        const el = document.createElement('option');
        el.value = i;
        const {start, end} = segments[i];
        const length = trackFrames[end].frame - trackFrames[start].frame + 1;
        el.textContent = `Segment ${i + 1} Frame ${trackFrames[start].frame} length: ${length}`;
        segmentsEl.append(el);
    }
    segmentsEl.value = options.segment;
}

function scrollToSegment(index) {
    const selectedSegmentFrame = trackFrames[segments[index].start].frame;
    find(`.track-frame-container[data-frame="${selectedSegmentFrame}"]`).scrollIntoView({inline: 'start'});
}

async function loadImageFromTrack(frameIndex, trackIndex) {
    const track = tracks[trackIndex];
    const snapshot = track.snapshots.find(s => s.frame === frameIndex);
    if (!snapshot) return;

    return new Promise((resolve) => {
        const img = document.createElement('img');
        img.dataset['frame'] = frameIndex;
        img.dataset['track'] = trackIndex;
        img.addEventListener('load', () => resolve(img));
        img.src = snapshot.path;
        if (img.complete) resolve(img);
    });
}

const blobUrlPromiseCache = new Map();
async function getDiffImage(img1, img2) {
    function getImgData(img) {
        const canvas = document.createElement('canvas');
        const context = canvas.getContext('2d');
        canvas.width = img.width;
        canvas.height = img.height;
        context.drawImage(img, 0, 0);
        return context.getImageData(0, 0, img.width, img.height);
    }

    async function getBlobUrlFromImageData(imgData) {
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        canvas.width = imgData.width;
        canvas.height = imgData.height;
        ctx.putImageData(imgData, 0, 0);
        const blob = await new Promise(resolve => canvas.toBlob(resolve));
        return URL.createObjectURL(blob);
    }

    const key = [new URL(img1.src).pathname, new URL(img2.src).pathname].join(';');
    let blobUrlPromise = blobUrlPromiseCache.get(key);
    if (!blobUrlPromise) {
        const imgData1 = getImgData(img1);
        const imgData2 = getImgData(img2);
        const diffImgData = getImgData(new Image(imgData1.width, imgData2.height));
        pixelmatch(imgData1.data, imgData2.data, diffImgData.data, imgData1.width, imgData1.height, { threshold: 0 });
        blobUrlPromise = getBlobUrlFromImageData(diffImgData);
        blobUrlPromiseCache.set(key, blobUrlPromise);
    }

    const diffImg = document.createElement('img');
    diffImg.src = await blobUrlPromise;
    diffImg.classList = 'diff-image';
    return diffImg;
}

/**
 * @param {number} frameIndex
 * @param {number} trackIndex
 * @param {ReturnType<typeof getOptions>} options
 */
async function showFrameView(frameIndex, trackIndex, options) {
    showOptions('frame');

    const selectedImg = await loadImageFromTrack(frameIndex, trackIndex);

    if (frameViewFrameIndex !== frameIndex) {
        frameViewFrameIndex = frameIndex;
        frameViewEl.innerHTML = '';

        // Only wait for the current track, but preload the rest.
        tracks.forEach((_, i) => loadImageFromTrack(frameIndex, i));

        // Also preload the neighboring frames.
        const prevFrame = findNextFrame(-1);
        if (prevFrame !== -1) tracks.forEach((_, i) => loadImageFromTrack(prevFrame, i));
        const nextFrame = findNextFrame(1);
        if (nextFrame !== -1) tracks.forEach((_, i) => loadImageFromTrack(nextFrame, i));
    }

    frameViewTrackIndex = trackIndex;

    if (selectedImg) frameViewEl.append(selectedImg);
    tracksEl.classList.add('hidden');
    frameViewEl.classList.remove('hidden');

    const parts = [
        options.replay,
        frameViewFrameIndex,
        tracks[frameViewTrackIndex].source,
        options.showDiff ? 'diff' : '',
    ];
    setTitle(parts.filter(Boolean).join(' - '));

    for (const img of findAll('img', frameViewEl)) {
        img.classList.add('hidden');
        if (img.classList.contains('diff-image')) img.remove();
    }

    if (options.showDiff) {
        const [img1, img2] = await Promise.all([
            loadImageFromTrack(frameIndex, 0),
            loadImageFromTrack(frameIndex, trackIndex),
        ]);

        if (!img1 || !img2) {
            setTitle(`${titleEl.textContent} (missing)`);
            return;
        }

        const diffImg = await getDiffImage(img1, img2);
        frameViewEl.append(diffImg);
    } else {
        if (selectedImg) {
            selectedImg.classList.remove('hidden');
        } else {
            setTitle(`${titleEl.textContent} (missing)`);
        }
    }
}

async function hideFrameView() {
    tracksEl.classList.remove('hidden');
    frameViewEl.classList.add('hidden');
    frameViewFrameIndex = -1;
    frameViewTrackIndex = -1;
    setTitle(getOptions().replay);
    showOptions('tracks');
    if (trackViewDirty) renderTracks(getOptions());
}

init();
