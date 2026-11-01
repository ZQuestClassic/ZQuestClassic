document.addEventListener('DOMContentLoaded', () => {
	for (const el of [...document.querySelectorAll('a.external')]) {
		el.target = '_blank';
	}
	setupLightbox();
});

// Images with the 'lightbox' class (`:class: lightbox` on an image directive) open larger when
// clicked. Click anywhere or press Escape to close.
function setupLightbox() {
	const images = [...document.querySelectorAll('img.lightbox')];
	if (!images.length) return;

	const dialog = document.createElement('dialog');
	dialog.className = 'lightbox-dialog';
	const big = document.createElement('img');
	dialog.append(big);
	document.body.append(dialog);
	dialog.addEventListener('click', () => dialog.close());
	// The theme's own Escape handler cancels the key, which stops the dialog closing by itself.
	dialog.addEventListener('keydown', (e) => {
		if (e.key === 'Escape') dialog.close();
	});

	// The largest size that fits the window. From 2x up it's a whole multiple, so pixel art stays
	// sharp; a small screen gets whatever fits.
	const fit = () => {
		if (!big.naturalWidth) return;
		let scale = Math.min(window.innerWidth * 0.95 / big.naturalWidth,
			window.innerHeight * 0.95 / big.naturalHeight);
		if (scale >= 2) scale = Math.floor(scale);
		big.style.width = `${big.naturalWidth * scale}px`;
		big.style.height = `${big.naturalHeight * scale}px`;
	};
	big.addEventListener('load', fit);
	window.addEventListener('resize', () => {
		if (dialog.open) fit();
	});

	const open = (img) => {
		big.src = img.currentSrc || img.src;
		big.alt = img.alt;
		// Keep the image's other classes, like 'dark-light'.
		big.className = img.className.replace(/\blightbox\b/, '');
		fit();
		dialog.showModal();
	};

	for (const img of images) {
		img.tabIndex = 0;
		img.setAttribute('role', 'button');
		img.title = 'Click to enlarge';
		img.addEventListener('click', () => open(img));
		img.addEventListener('keydown', (e) => {
			if (e.key === 'Enter' || e.key === ' ') {
				e.preventDefault();
				open(img);
			}
		});
	}
}
