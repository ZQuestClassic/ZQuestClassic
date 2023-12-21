import argparse
from argparse import ArgumentTypeError
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from pathlib import Path
import os
import shutil
import urllib.parse
import mimetypes
import subprocess

def dir_path(path):
    if os.path.isdir(path):
        return Path(path)
    else:
        raise ArgumentTypeError(f'{path} is not a valid directory')

def start_webserver(out_dir: Path):
    class Server(BaseHTTPRequestHandler):
        def do_GET(self):
            path = urllib.parse.unquote(urllib.parse.urlparse(self.path).path)
            if path.endswith('/'):
                path += 'index.html'
            elif '.' not in path.split('/')[-1]:
                path += '/index.html'
            path = out_dir / path[1:]

            if path.exists():
                mimetype, _ = mimetypes.guess_type(path)
                self.send_response(200)
                self.send_header('Content-Type', mimetype)
                self.send_header('Content-Length', os.path.getsize(path))
                self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
                self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
                self.end_headers()
                with open(path, 'rb') as f:
                    shutil.copyfileobj(f, self.wfile)
            else:
                self.send_response(404)
                self.end_headers()

    port = 8000
    httpd = ThreadingHTTPServer(('localhost', port), Server)
    print(f'Open: http://localhost:{port}')
    httpd.serve_forever()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dir', type=dir_path)
    args = parser.parse_args()

    # Doesn't work so well - too many connection reset when running replays in parallel.
    # start_webserver(args.dir)
    subprocess.check_call(['npx', '--prefer-offline', 'statikk', '--port', '8000', '--coi'], cwd=args.dir)
