The script in this folder:

- takes the output of `build_emscripten.sh` and transforms the generated code into a nicer HTML structure
- adds a service worker capable of offline play
- provides a deployment mechanism to hoten.cc

# Deploying to hoten.cc/zc/

- (one-time) run `npm i` in `web/` folder
- build for the web (`sh build_emscripten.sh` or `DEBUG=1 sh build_emscripten.sh`)
- prepare the deployment: `python web/deploy/deploy.py --prepare --build_folder build_emscripten/Release`
- run `npx statikk --port 8001 --coi` in `dist` and confirm everything works
- deploy: `python web/deploy/deploy.py --deploy --prod` (drop `--prod` to deploy to `hoten.cc/zc-debug/`)

One-liner:

```
sh build_emscripten.sh && python web/deploy/deploy.py --prepare --build_folder build_emscripten/Release --deploy --prod
```