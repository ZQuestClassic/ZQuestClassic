document.addEventListener('DOMContentLoaded', () => {
	for (const el of [...document.querySelectorAll('a.external')]) {
		el.target = '_blank';
	}
});
