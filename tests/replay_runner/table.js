function createTable(selector, data) {
	const columns = [
		{ data: 'replay' },
		// {data: 'qst'},
		// {data: 'author'},
		{ data: 'frames' },
		// {data: 'length'},
		// {data: 'debug'},
		{ data: 'starting_dmap' },
		{ data: 'starting_scr' },
		// {data: 'starting_retsqr'},
		// {data: 'sync_rng'},
		// {data: 'test_mode'},
		{ data: 'time_created' },
		// {data: 'time_updated'},
		{ data: 'version' },
		// {data: 'zc_version_created'},
		// {data: 'zc_version_updated'},
	];

	const trEl = find(`${selector} thead tr`);
	trEl.textContent = '';
	for (const column of columns) {
		const el = document.createElement('th');
		el.textContent = column.data;
		trEl.append(el);
	}

	const realData = [];
	for (const row of data) {
		const d = {};
		for (const column of columns) {
			d[column.data] = row[column.data] ?? '';
		}
		realData.push(d);
	}

	DataTable.ext.errMode = 'none';
	const table = new DataTable(selector, {
		data: realData,
		columns,
		paging: false,
		rowId: 'replay',
		rowReorder: true,
		select: {
			style: 'multi+shift',
		},
		layout: {
			topStart: {
				buttons: [
					{
						text: 'Run',
						action: function () {
							const rows = table.rows({ selected: true });
							const filter = [];
							rows.data().each(d => filter.push(d.replay));
							if (filter.length) {
								startRun(filter);
							}
						},
					},
					{
						text: 'Select All',
						action: function () {
							table.rows({ search: 'applied' }).select();
						},
					},
					{
						text: 'Unselect All',
						action: function () {
							table.rows({ search: 'applied' }).deselect();
						},
					},
					'createState',
					{
						extend: 'savedStates',
						config: {
							preDefined: {
								'Playground': {
									order: [[1, 'asc']],
									search: {
										search: 'playground',
									},
								},
							},
						},
					},
				],
			},
			topEnd: ['search', 'info'],
		},
	});
	table.on('error', console.error);
	return table;
}
