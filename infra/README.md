# ZQuest Classic infra

This folder holds all the server configuration files for self-hosted services needed for ZC.

Further, this README documents all the various services we maintain (self-hosted and hosted by third-parties).

## Domains

Registrar is Squarespace.

## https://zquestclassic.com

Source: https://github.com/ZQuestClassic/zquestclassic.com
Host: Netlify (free plan)

This is a simple static-site that hosts our news posts, release pages, and a homepage showcasing cool stuff made in ZC. It is automatically deployed on every push to the repo.

The ZC software updater fetches the latest download information from this site. Based on the channel, the following URL is fetched: `https://zquestclassic.com/releases/{channel}.json`. Valid channels are 2.55 and 3. This file is updated on every release: [release.yml](https://github.com/ZQuestClassic/ZQuestClassic/blob/main/.github/workflows/release.yml) initiates a workflow on the zquestclassic.com repo, which runs [update-releases.js](https://github.com/ZQuestClassic/zquestclassic.com/blob/main/scripts/update-releases.js) to fetch the latest versions from GitHub releases. Then the workflow commits the new information, pushes, and Netlify picks it up very soon after.

## https://web.zquestclassic.com

Repo: https://github.com/ZQuestClassic/web.zquestclassic.com
Host: Netlify (free plan)

This is a simple static-site that hosts the web build for ZC. It is automatically deployed on every push to the repo.

This also hosts the ZScript playground: https://web.zquestclassic.com/zscript (source `web/playground/playground.ts`).

Running `scripts/update.sh path/to/ZC/repo` will build the latest version. Commit, push, and Netlify will deploy soon.

## https://docs.zquestclassic.com

Host: DigitalOcean

This is a simple static-site that hosts the docs build for ZC. The deployed site is updated every 10 minutes via a cron.

The source code is at `./docs-www/`.

## https://data.zquestclassic.com

Host: DigitalOcean Spaces (S3)

This holds a structured database for all quests scraped into our database. [`manifest.json`](https://data.zquestclassic.com/manifest.json) is the index for all the data available. It includes versioned entries for everything, such that if a quest recieves an update we still have a reference to all older version. It also contains the enhanced music for quests that have it.

This data is utilized by many things:

* The web build uses it to build its quest list and serve quests, music
* `scripts/database.py` downloads a local copy of it for testing purposes
* The replay upload service only accepts replays for quests we have in this database

This [repo](https://github.com/ZQuestClassic/zc-database) handles scraping for new quests and uploading them to the database. It currently must be run manually.

## https://api.zquestclassic.com

Source: `api_server/`
Host: DigitalOcean

This is the production server used by ZC. It's currently only used to facilitate the opt-in replay upload feature.

## Uploaded replays

Host: DigitalOcean Spaces (S3)

`scripts/replay_uploads.py` downloads the uploaded replays for local testing.

## Archives

Host: DigitalOcean Spaces (S3)

Mostly every version of ZC is kept here, including many test builds for random commit SHAs. On every release, `scripts/upload_releases_to_s3.py` uploads the latest versions to this bucket.

`scripts/archives.py` provides a CLI for downloading and running any version of ZC.

`scripts/bisect_builds.py` uses this to speed up bisects, skipping local builds.

## Compare replay reports

When a replay fails in CI, an HTML compare replay report is generated via [compare.yml](https://github.com/ZQuestClassic/ZQuestClassic/blob/main/.github/workflows/compare.yml). The report is uploaded to [surge.sh](https://surge.sh/) for a short amount of time. It's also uploaded as an artifact in the GitHub workflow - if downloaded, run a local webserver (eg `python -m http.server`) to view it.

See `tests/compare_replays.py` for how this report is generated.

## Git LFS

We use LFS to host many huge replay/qst files that change frequently. The current provider for this is GitHub.
