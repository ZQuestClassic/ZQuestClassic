# TODO: support --frame and other replay options
# TODO: cancel replay runs
# TODO: pretty ui
# TODO: see info about baseline replay run
# TODO: refactor replay test runner as library

import asyncio
import json
import os
import platform
import shutil
import subprocess
import sys

from dataclasses import dataclass
from pathlib import Path

import aiohttp

from aiohttp import web

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))
root_dir = (script_dir / '../..').resolve()
sys.path.append(str((script_dir / '..').absolute()))

from common import get_recent_release_tag
from lib.replay_helpers import read_replay_meta
from run_test_workflow import get_args_for_collect_baseline_from_test_results

compare_reports_dir = root_dir / f'.tmp/compare_reports'
compare_reports_dir.mkdir(parents=True, exist_ok=True)


@dataclass
class Client:
    ws: web.WebSocketResponse
    view: str

    async def send_msg(self, msg):
        try:
            await self.ws.send_json(msg)
        except ConnectionResetError as e:
            pass

    async def send_view_msg(self, view, data):
        await self.send_msg({'type': 'view', 'view': view, 'data': data})


clients: list[Client] = []
view_store = {}
test_results_last_update = {}

for compare_report_dir in compare_reports_dir.glob('*'):
    view_store[f'/compare/{compare_report_dir.name}'] = {'status': 'done'}


async def update_client(client: Client):
    if client.view not in view_store:
        return

    await client.send_view_msg(client.view, view_store[client.view])


async def update_view(view: str, data: dict):
    view_store[view] = data
    for client in clients:
        if client.ws.status and client.view == view:
            await client.send_view_msg(view, data)


def try_parse_json(text: str):
    try:
        return json.loads(text)
    except:
        print('failed to parse json, ignoring')
        return None


def create_proc(program: str, args: list[str]):
    return asyncio.create_subprocess_exec(
        program,
        *args,
        cwd=root_dir,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
    )


async def update_views():
    views = set()
    for client in clients:
        if client.view and client.view != '/':
            views.add(client.view)

    for view in views:
        parts = [x for x in client.view.split('/') if x]
        type = parts[0]

        if type == 'results':
            test_results_name = parts[1]
            test_results_path = (
                root_dir / '.tmp/test_results' / test_results_name / 'test_results.json'
            )
            if not test_results_path.exists():
                continue

            data = None
            if test_results_path in view_store:
                data = view_store[test_results_path]

            mtime = os.path.getmtime(test_results_path)
            if test_results_last_update.get(test_results_path, None) != mtime:
                # It's possible that the json file is in the middle of being written to,
                # so check for failure.
                results = try_parse_json(test_results_path.read_text())
                if results:
                    data = {
                        'name': test_results_path.parent.name,
                        'results': results,
                    }

            if not data:
                continue

            test_results_last_update[test_results_path] = mtime
            await update_view(view, data)


def handle_initial_req(request):
    replays = []
    replays_path = script_dir / '../replays'
    for replay_path in replays_path.rglob('*.zplay'):
        meta = read_replay_meta(replay_path)
        replays.append(
            {
                'replay': str(replay_path.relative_to(replays_path)),
                'meta': meta,
            }
        )

    result_paths = []
    results_path = root_dir / '.tmp/test_results'
    for result_path in results_path.rglob('test_results.json'):
        result_paths.append(
            {
                'name': result_path.parent.name,
                'date': os.path.getmtime(result_path),
            }
        )
    results_path = result_paths.sort(key=lambda x: x['date'], reverse=True)

    data = {
        'replays': replays,
        'testResults': result_paths,
    }

    return web.json_response(data)


def handle_test_results_list_req(request):
    results_path = root_dir / '.tmp/test_results'
    for result_path in results_path.glob('*'):
        results_path.append(
            {
                'name': result_path.name,
            }
        )
    data = {'testResults': results_path}
    return web.json_response(data)


async def handle_update_view_command(client: Client, params):
    client.view = params['view']
    await update_client(client)


async def handle_run_command(client: Client, params):
    test_results_name = params['name']
    del params['name']
    test_results_dir = root_dir / f'.tmp/test_results/{test_results_name}'
    if test_results_dir.exists():
        return

    build_folder = os.environ.get('BUILD_FOLDER', None)
    args = [
        root_dir / 'tests/run_replay_tests.py',
        '--test_results',
        test_results_dir,
        '--for_dev_server',
    ]
    if build_folder:
        args.extend(['--build_folder', build_folder])
    for key, value in params.items():
        if type(value) is not list:
            value = [value]
        for v in value:
            args.extend([f'--{key}', str(v)])
    p = await create_proc(sys.executable, args)
    exit_code = await p.wait()
    # Exit code of 2 is "replays failed, but there is still a final test_results.json".
    # Ignore that here because update_views will handle it.
    if exit_code != 0 and exit_code != 2:
        stderr = (await p.stderr.read()).decode('utf-8')
        view = f'/results/{test_results_name}'
        print(view, stderr)
        await update_view(
            view, {'error': f'[FAILED] error running replays:\n' + stderr}
        )


async def handle_compare_command(client: Client, params):
    test_results_name = params['name']
    test_results_dir = root_dir / '.tmp/test_results' / test_results_name
    test_results_path = test_results_dir / 'test_results.json'
    view = f'/compare/{test_results_name}'

    tag = get_recent_release_tag(['--match', '3.*.*'])
    await update_view(view, {'status': f'downloading baseline build {tag}'})
    args = [
        root_dir / 'scripts/archives.py',
        'download',
        tag,
    ]
    p = await create_proc(sys.executable, args)
    if await p.wait() != 0:
        stderr = (await p.stderr.read()).decode('utf-8')
        await update_view(
            view, {'status': f'[FAILED] downloading baseline build {tag}\n' + stderr}
        )
        return

    stdout = (await p.stdout.read()).decode('utf-8')
    build_dir = Path(stdout.strip())
    if platform.system() == 'Darwin':
        zc_app_path = next(build_dir.glob('*.app'))
        build_dir = zc_app_path / 'Contents/Resources'

    await update_view(view, {'status': 'running baseline replays'})
    baseline_test_results_dir = (
        root_dir / '.tmp/baseline_test_results' / f'{test_results_name}-baseline'
    )
    if baseline_test_results_dir.exists():
        shutil.rmtree(baseline_test_results_dir)
    baseline_test_results_dir.parent.mkdir(exist_ok=True, parents=True)
    args = [
        str(root_dir / 'tests/run_replay_tests.py'),
        '--replay',
        '--build_folder',
        str(build_dir),
        '--test_results_folder',
        str(baseline_test_results_dir),
        '--retries=2',
        '--no_report_on_failure',
        '--not_interactive',
        *get_args_for_collect_baseline_from_test_results([test_results_path]),
    ]
    p = await create_proc(sys.executable, args)
    if await p.wait() != 0:
        stderr = (await p.stderr.read()).decode('utf-8')
        await update_view(
            view, {'status': f'[FAILED] running baseline replays\n{stderr}'}
        )
        return

    await update_view(view, {'status': 'creating report'})
    dest = compare_reports_dir / test_results_name
    if dest.exists():
        shutil.rmtree(dest)
    dest.mkdir()
    args = [
        root_dir / 'tests/compare_replays.py',
        '--local',
        baseline_test_results_dir,
        '--local',
        test_results_dir,
        '--dest',
        dest,
        '--no-start-server',
    ]
    p = await create_proc(sys.executable, args)
    if await p.wait() != 0:
        stderr = (await p.stderr.read()).decode('utf-8')
        await update_view(view, {'status': f'[FAILED] creating report\n{stderr}'})
        return

    await update_view(view, {'status': 'done'})


async def websocket_handler(request):
    ws = web.WebSocketResponse()
    await ws.prepare(request)

    client = Client(ws=ws, view=None)
    clients.append(client)

    async for msg in ws:
        if msg.type == aiohttp.WSMsgType.TEXT:
            if msg.data == 'close':
                await ws.close()
            else:
                data = json.loads(msg.data)
                co = None
                if data['command'] == 'update-view':
                    co = handle_update_view_command
                elif data['command'] == 'run':
                    co = handle_run_command
                elif data['command'] == 'compare':
                    co = handle_compare_command
                if co:
                    asyncio.run_coroutine_threadsafe(
                        co(client, data['params']), asyncio.get_running_loop()
                    )
        elif msg.type == aiohttp.WSMsgType.ERROR:
            print('ws connection closed with exception %s' % ws.exception())

    clients.remove(client)

    return ws


def create_runner():
    @web.middleware
    async def cache_control(request: web.Request, handler):
        response: web.Response = await handler(request)
        resource_name = request.match_info.route.name
        if resource_name and resource_name.startswith('static'):
            response.headers.setdefault('Cache-Control', 'no-cache')
        return response

    def static_handler(dir: Path):
        def handler(request):
            tail = request.match_info['tail']
            file_path = dir / tail
            if not file_path.exists():
                return web.HTTPNotFound()
            if file_path.is_dir():
                if not tail.endswith('/'):
                    return web.HTTPPermanentRedirect(tail + '/')
                file_path /= 'index.html'
                if not file_path.exists():
                    return web.HTTPNotFound()
            return web.FileResponse(file_path)

        return handler

    routes = web.RouteTableDef()

    @routes.get('/')
    @routes.get('/new')
    @routes.get('/load')
    @routes.get('/results/{results}')
    @routes.get('/compare/{compare}')
    async def get_html(request):
        return web.FileResponse(script_dir / 'index.html')

    app = web.Application(middlewares=[cache_control])
    app.add_routes(routes)
    app.add_routes(
        [
            web.get('/ws', websocket_handler),
            web.get('/data.json', handle_initial_req),
            web.get('/compare/show/{tail:.*}', static_handler(compare_reports_dir)),
            web.static('/', script_dir, name='static'),
        ]
    )
    return web.AppRunner(app)


async def start_server(host='localhost', port=8009):
    runner = create_runner()
    await runner.setup()
    site = web.TCPSite(runner, host, port)
    await site.start()
    print(f'server started at http://{host}:{port}')
    while True:
        await update_views()
        await asyncio.sleep(0.25)


async def main():
    tasks = [start_server()]
    await asyncio.gather(*tasks)


asyncio.run(main())
