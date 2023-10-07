module.exports = {
	// globDirectory: './',
	// globPatterns: [
	// 	'{create,play}/**/*.{html,js,mjs}',
	// 	'zc.data.js',
	// ],
	runtimeCaching: [
		{
			urlPattern: /png|jpg|jpeg|svg|gif/,
			handler: 'CacheFirst',
		},
		{
			// Match everything except the wasm data file, which is cached in
			// IndexedDB by emscripten.
			urlPattern: ({ url }) => !url.pathname.endsWith('.data'),
			handler: 'NetworkFirst',
			options: {
				matchOptions: {
					// Otherwise the html page won't be cached (it can have query parameters).
					ignoreSearch: true,
				},
			},
		},
	],
	swDest: 'sw.js',
	skipWaiting: true,
	clientsClaim: true,
	offlineGoogleAnalytics: true,
	// ignoreURLParametersMatching: [
	// 	/^utm_/,
	// 	/^fbclid$/,
	// 	/^quest$/,
	// ],
};
