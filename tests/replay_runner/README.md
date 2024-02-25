# Replay Runner webapp

Start with `python tests/replay_runner/server.py`

Open up a terminal, start this, and minimize the window. Keep it running while running any replays. If it closes, you'll lose whatever run it was doing. It can't resume.

If you need to cancel a replay run, for now you just need to restart the server.

You can filter what replays run. And save that selection in the "Create State" thing. No other options (like --frame) are supported right now.

When the replay finishes, there's a button for generating a compare report. just click it and wait.

Press "Load" to view previous runs. Restarting the server doesn't lose anything here (or generated compare reports).

Defaults to using build/Release to run replays. Can change by setting `BUILD_FOLDER` environment variable, like `BUILD_FOLDER=build/Debug python ...`
