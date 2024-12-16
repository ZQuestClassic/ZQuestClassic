# ZC API Server

Provides the following services for ZC:

* Uploading replays (download and run locally via `scripts/replay_uploads.py`)

## Run locally

To test locally, set this in your `zc.cfg`:

```ini
[zeldadx]
api_endpoint = http://127.0.0.1:5000
```

and run `bash start.sh` from this folder. You'll find the server data at `.tmp/api_server`.
