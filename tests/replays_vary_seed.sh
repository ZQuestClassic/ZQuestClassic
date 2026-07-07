#!/usr/bin/env bash
#
# Run run_replay_tests.py in --batch mode N times, each with a
# different (random, printed) seed, forwarding any extra args. This hunts for
# global state that isn't reset between quests: a leak tends to only show
# up under specific batch orderings, so varying the seed increases coverage.
#
# Usage:
#   tests/replays_vary_seed.sh N [extra run_replay_tests.py args...]
#
# Examples:
#   tests/replays_vary_seed.sh 10 --filter playground
#   tests/replays_vary_seed.sh 5 --batch-size 6 --filter playground --filter misc
#
# Stops at the first run that fails and exits non-zero. Reproduce that run with:
#   python tests/run_replay_tests.py --batch --seed <SEED> ...

set -uo pipefail

if [ "$#" -lt 1 ]; then
  echo "usage: $0 N [extra run_replay_tests.py args...]" >&2
  exit 2
fi

n="$1"
shift

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runner="$script_dir/run_replay_tests.py"

for ((i = 1; i <= n; i++)); do
  seed=$RANDOM
  echo ""
  echo "=================== run $i/$n (seed $seed) ==================="
  if ! python "$runner" --batch --seed "$seed" "$@"; then
    echo ""
    echo "run $i/$n failed (seed $seed); stopping."
    echo "reproduce with: python tests/run_replay_tests.py --batch --seed $seed $*"
    exit 1
  fi
done

echo ""
echo "all $n runs passed"
