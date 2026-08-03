# git log upstream/main --since 'May 1'  --oneline

import re
import subprocess
import sys
import tempfile

# Everything in this script is relative to this branch, never the current
# HEAD - running from the main checkout must give the same answers as
# running from the 2.55 worktree.
TARGET_BRANCH = "releases/2.55"


def get_cherrypicked_shas():
    """Return the set of full SHAs already on releases/2.55 (directly or as cherry-pick sources)."""
    excluded = set()
    try:
        output = subprocess.check_output(
            ["git", "log", f"2.55.0..{TARGET_BRANCH}", "--format=%H%n%b"],
            text=True,
            stderr=subprocess.DEVNULL,
        )
    except subprocess.CalledProcessError:
        return excluded

    for line in output.splitlines():
        line = line.strip()
        if not line:
            continue
        # Full SHA of a commit directly on releases/2.55
        if re.fullmatch(r"[0-9a-f]{40}", line):
            excluded.add(line)
            continue
        # Original SHA referenced by a cherry-pick
        m = re.search(r"\(cherry picked from commit ([0-9a-f]{40})\)", line)
        if m:
            excluded.add(m.group(1))

    return excluded


def get_missing_paths(sha):
    """Paths touched by `sha` that don't exist on TARGET_BRANCH. A cherry-pick
    can still apply "cleanly" around these (silently creating the file, or
    patching lookalike context), so they deserve a loud warning."""
    try:
        paths = subprocess.check_output(
            ["git", "show", "--format=", "--name-only", sha],
            text=True,
            stderr=subprocess.DEVNULL,
        ).split()
    except subprocess.CalledProcessError:
        return []

    missing = []
    for path in paths:
        result = subprocess.run(
            ["git", "cat-file", "-e", f"{TARGET_BRANCH}:{path}"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode != 0:
            missing.append(path)
    return missing


def check_cherrypick_conflicts(full_shas):
    """
    Return the subset of full_shas that can be cherry-picked onto the tip of
    TARGET_BRANCH without conflicts. Uses a single temporary worktree so the
    real index/worktree are never touched.
    """
    clean = set()
    if not full_shas:
        return clean

    with tempfile.TemporaryDirectory() as tmpdir:
        try:
            subprocess.check_call(
                ["git", "worktree", "add", "--detach", tmpdir, TARGET_BRANCH],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        except subprocess.CalledProcessError:
            print(
                f"Error: could not create a worktree for {TARGET_BRANCH}; "
                "conflict checking skipped.",
                file=sys.stderr,
            )
            return clean

        try:
            for sha in full_shas:
                result = subprocess.run(
                    ["git", "cherry-pick", "--no-commit", sha],
                    cwd=tmpdir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                if result.returncode == 0:
                    clean.add(sha)

                # Reset the worktree back to HEAD for the next iteration.
                # cherry-pick --abort handles both the in-progress and clean cases.
                subprocess.run(
                    ["git", "cherry-pick", "--abort"],
                    cwd=tmpdir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                subprocess.run(
                    ["git", "reset", "--hard", "HEAD"],
                    cwd=tmpdir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
                subprocess.run(
                    ["git", "clean", "-fdq"],
                    cwd=tmpdir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                )
        finally:
            subprocess.run(
                ["git", "worktree", "remove", "--force", tmpdir],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )

    return clean


def main():
    if len(sys.argv) != 2:
        print("Usage: python manage_commits.py <filename>", file=sys.stderr)
        sys.exit(1)

    filename = sys.argv[1]
    seen_shas = set()
    excluded_shas = get_cherrypicked_shas()
    commits = []

    try:
        with open(filename, "r") as f:
            for line in f:
                parts = line.strip().split(maxsplit=1)
                if not parts:
                    continue

                raw_sha = parts[0]

                try:
                    # Request Unix timestamp, FULL 40-char hash, 12-char hash, and subject
                    cmd = [
                        "git",
                        "show",
                        "-s",
                        "--format=%ct %H %h %s",
                        "--abbrev=12",
                        raw_sha,
                    ]
                    output = subprocess.check_output(
                        cmd, text=True, stderr=subprocess.DEVNULL
                    ).strip()

                    git_parts = output.split(maxsplit=3)
                    if len(git_parts) < 3:
                        continue  # Failsafe for unexpected empty git output

                    time_str = git_parts[0]
                    full_sha = git_parts[1]
                    abbrev_sha = git_parts[2]
                    subject = git_parts[3] if len(git_parts) == 4 else ""

                    # Skip commits already on releases/2.55 (directly or via cherry-pick)
                    if full_sha in excluded_shas:
                        continue

                    # Deduplicate using the definitive full Git SHA, not the raw text input
                    if full_sha in seen_shas:
                        continue
                    seen_shas.add(full_sha)

                    commits.append(
                        (int(time_str), full_sha, f"{abbrev_sha} {subject}".strip())
                    )

                except subprocess.CalledProcessError:
                    print(
                        f"Warning: Could not process SHA {raw_sha} (invalid or missing in this repo)",
                        file=sys.stderr,
                    )

    except FileNotFoundError:
        print(f"Error: Could not find file '{filename}'", file=sys.stderr)
        sys.exit(1)

    # Sort by timestamp (newest first = largest timestamp)
    commits.sort(key=lambda x: x[0], reverse=True)

    for _, full_sha, message in commits:
        print(message)

    clean_shas = check_cherrypick_conflicts([sha for _, sha, _ in commits])

    clean_commits = [(sha, msg) for _, sha, msg in commits if sha in clean_shas]
    if clean_commits:
        print(f"\n--- conflict-free cherry-picks (onto {TARGET_BRANCH}) ---")
        print("--- NOTE: 'applies cleanly' is not 'compiles'; build each pick ---")
        clean_commits.reverse()
        for full_sha, message in clean_commits:
            print(f"git cherry-pick -x {full_sha}  # {message}")
            missing = get_missing_paths(full_sha)
            if missing:
                print(
                    f"#   WARNING: touches paths missing on {TARGET_BRANCH}: "
                    f"{', '.join(missing)}. The pick may create them or remap "
                    "them via rename detection - inspect what it actually did."
                )


if __name__ == "__main__":
    main()
