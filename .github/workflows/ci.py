import argparse
import json
import logging
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import tarfile
import zipfile

from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, Optional

import discord
import requests

script_dir = Path(__file__).parent.absolute()
root_dir = script_dir.parent.parent
tmp_dir = root_dir / ".tmp"


logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S',
)
logger = logging.getLogger(__name__)


def command(name: str, help: Optional[str] = None):
    """Decorator to register a function as a CLI command."""

    def decorator(func: Callable):
        func._cli_command = name
        func._cli_help = help
        return func

    return decorator


def argument(*name_or_flags, **kwargs):
    """Decorator to register an argument for a CLI command."""

    def decorator(func: Callable):
        if not hasattr(func, "_cli_args"):
            func._cli_args = []
        # Store in reverse order because decorators are applied bottom-to-top
        func._cli_args.insert(0, (name_or_flags, kwargs))
        return func

    return decorator


@dataclass
class CiContext:
    """Shared state and platform-specific configuration for CI operations."""

    arch: str
    compiler: str
    config: str
    release_version: str
    repo: str
    build_folder_base: Path

    system: str = platform.system()

    @property
    def is_windows(self) -> bool:
        return self.system == 'Windows'

    @property
    def is_mac(self) -> bool:
        return self.system == 'Darwin'

    @property
    def is_linux(self) -> bool:
        return self.system == 'Linux'

    @property
    def platform_label(self) -> str:
        if self.is_windows:
            arch_label = 'x86' if self.arch == 'win32' else 'x64'
            return f'windows-{arch_label}'
        return self.system.lower()

    @property
    def arch_label(self) -> str:
        if self.is_windows:
            return 'x86' if self.arch == 'win32' else 'x64'
        return self.arch

    def get_binary_dir(self) -> Path:
        """Determines the correct binary folder path based on OS extraction logic."""
        if self.is_windows:
            return self.build_folder_base
        elif self.is_mac:
            binary_dir = (
                self.build_folder_base / "ZQuest Classic.app" / "Contents" / "Resources"
            )
        elif self.is_linux:
            binary_dir = self.build_folder_base / "bin"

        if binary_dir.exists():
            return binary_dir

        return self.build_folder_base


def run_cmd(cmd, env=None, cwd=None, shell=False):
    """Helper to run a subprocess command with logging and error handling."""
    if isinstance(cmd, str) and not shell:
        cmd_str = cmd
        # posix=False helps preserve Windows paths if they contain backslashes
        is_posix = platform.system() != 'Windows'
        cmd = shlex.split(cmd, posix=is_posix)
    elif isinstance(cmd, list):
        cmd = [str(c) for c in cmd]
        cmd_str = shlex.join(cmd)
    else:
        cmd_str = str(cmd)

    logger.info(f"Running: {cmd_str}")
    try:
        subprocess.run(cmd, env=env, cwd=cwd, shell=shell, check=True)
    except subprocess.CalledProcessError as e:
        logger.error(f"Command failed with exit code {e.returncode}: {cmd_str}")
        sys.exit(e.returncode)
    except OSError as e:
        logger.error(f"Failed to execute command: {e}")
        sys.exit(1)


def run_python_cmd(cmd, env=None, cwd=None, shell=False):
    """Helper to run a python command with -Xutf8."""
    prefix = [sys.executable, "-Xutf8"]
    if shell:
        prefix_str = " ".join(shlex.quote(p) for p in prefix)
        if isinstance(cmd, list):
            cmd = f"{prefix_str} {shlex.join([str(c) for c in cmd])}"
        else:
            cmd = f"{prefix_str} {cmd}"
    else:
        if isinstance(cmd, str):
            is_posix = platform.system() != 'Windows'
            cmd = shlex.split(cmd, posix=is_posix)
        cmd = prefix + [str(c) for c in cmd]

    return run_cmd(cmd, env=env, cwd=cwd, shell=shell)


# -----------------------------------------------------------------------------
# build.yml
# -----------------------------------------------------------------------------


def configure_signatures(ctx: CiContext):
    """Generates the metadata.h C++ header file based on the target compiler."""
    logger.info(
        f"Configuring signatures for compiler: {ctx.compiler}, version: {ctx.release_version}"
    )

    metadata_dir = root_dir / "src/metadata"
    metadata_dir.mkdir(parents=True, exist_ok=True)
    metadata_file = metadata_dir / "metadata.h"

    lines = ['#define __TIMEZONE__ "UTC"', '']

    if ctx.compiler == 'msvc':
        lines.extend(
            [
                '#define COMPILER_V_FIRST (_MSC_VER/100)',
                '#define COMPILER_V_SECOND (_MSC_VER%100)',
                '#define COMPILER_V_THIRD (_MSC_FULL_VER%100000)',
                '#define COMPILER_V_FOURTH _MSC_BUILD',
                '#define COMPILER_NAME "MSVC"',
                '#define METADATA_IMPL_STRINGIFY(x) #x',
                '#define COMPILER_VERSION METADATA_IMPL_STRINGIFY(_MSC_FULL_VER)',
                '',
            ]
        )

        # Parse version for MSVC (.rc files)
        version_match = re.match(r"(\d+)\.(\d+)\.(\d+)", ctx.release_version)
        if version_match:
            lines.append(f'#define V_ZC_FIRST {version_match.group(1)}')
            lines.append(f'#define V_ZC_SECOND {version_match.group(2)}')
            lines.append(f'#define V_ZC_THIRD {version_match.group(3)}')
        else:
            logger.warning(
                f"Could not parse major.minor.patch from '{ctx.release_version}'"
            )
            lines.extend(
                [
                    '#define V_ZC_FIRST 0',
                    '#define V_ZC_SECOND 0',
                    '#define V_ZC_THIRD 0',
                ]
            )

        prerelease_match = re.match(r".*prerelease\.(\d+)", ctx.release_version)
        if prerelease_match:
            lines.append(f'#define V_ZC_FOURTH {prerelease_match.group(1)}')
        else:
            lines.append('#define V_ZC_FOURTH 0')

    elif ctx.compiler == 'clang':
        lines.extend(
            [
                '#define COMPILER_V_FIRST __clang_major__',
                '#define COMPILER_V_SECOND __clang_minor__',
                '#define COMPILER_V_THIRD __clang_patchlevel__',
                '#define COMPILER_V_FOURTH 0',
                '#define COMPILER_NAME "clang"',
                '#define COMPILER_VERSION __clang_version__',
            ]
        )

    elif ctx.compiler == 'gcc':
        lines.extend(
            [
                '#define COMPILER_V_FIRST __GNUC__',
                '#define COMPILER_V_SECOND __GNUC_MINOR__',
                '#define COMPILER_V_THIRD __GNUC_PATCHLEVEL__',
                '#define COMPILER_V_FOURTH 0',
                '#define COMPILER_NAME "gcc"',
                '#define COMPILER_VERSION __VERSION__',
            ]
        )

    else:
        logger.error(f"Unknown compiler: {ctx.compiler}")
        sys.exit(1)

    lines.append('')

    metadata_file.write_text('\n'.join(lines) + '\n')

    logger.info(f"Successfully generated {metadata_file}")


@command("install-deps", help="Install OS-level dependencies")
def install_deps(ctx: CiContext, args):
    logger.info("Starting dependency installation...")

    if ctx.is_mac:
        logger.info("macOS detected. Installing via Homebrew.")
        run_cmd("brew install ninja ccache")

        logger.info("Building custom Bison 3.6...")
        run_cmd("wget http://ftp.gnu.org/gnu/bison/bison-3.6.tar.gz")
        run_cmd("tar -zxvf bison-3.6.tar.gz")

        env = os.environ.copy()
        env['CFLAGS'] = '-Wno-error=incompatible-pointer-types'
        run_cmd("./configure", cwd="bison-3.6", env=env)
        run_cmd("make", cwd="bison-3.6")
        run_cmd("sudo make install", cwd="bison-3.6")

    elif ctx.is_linux:
        logger.info("Linux detected. Installing via apt-get.")
        run_cmd("sudo apt-get update")
        packages = [
            "ccache",
            "ninja-build",
            "build-essential",
            "libx11-dev",
            "libglu1-mesa-dev",
            "freeglut3-dev",
            "mesa-common-dev",
            "libssl-dev",
            "libcurl4-openssl-dev",
            "libxcursor1",
            "libasound2-dev",
            "libgtk-3-dev",
            "flex",
            "bison",
            "elfutils",
            "libpulse-dev",
            "libfreetype6-dev",
        ]
        # multilib is x86-only; these packages don't exist on arm64 Ubuntu.
        if platform.machine() in ('x86_64', 'AMD64'):
            packages += ["gcc-multilib", "g++-multilib"]
        packages_str = " ".join(packages)
        run_cmd(f"sudo apt-get install -y {packages_str}")

    elif ctx.is_windows:
        logger.info("Windows detected. Installing via Chocolatey and vcpkg.")
        run_cmd("choco install .github/dependencies.config -y --allow-downgrade")

        vcpkg_dir = root_dir / "vcpkg"
        vcpkg_exe = vcpkg_dir / "vcpkg.exe"
        triplet = 'x64-windows' if ctx.arch_label == 'x64' else 'x86-windows'
        run_cmd(f"{vcpkg_exe} install --triplet {triplet} curl openssl freetype")

    logger.info("Dependency installation complete.")


@command("build", help="Configure and build the project")
@argument("--no-cache", dest="cache", action="store_false", help="Disable ccache")
@argument("--official", action="store_true", help="Mark as an official build")
@argument("--test-runner", action="store_true", help="Include base_test_runner target")
@argument("--sentry", action="store_true", help="Build with Sentry support")
def build(ctx: CiContext, args):
    logger.info(
        f"Starting build process (Config: {ctx.config}, Compiler: {ctx.compiler})..."
    )

    env = os.environ.copy()

    if args.cache:
        env['CCACHE_BASEDIR'] = str(root_dir)
        env['CCACHE_DIR'] = str(root_dir / '.ccache')
        env['CCACHE_MAXSIZE'] = '400M'
        env['CCACHE_SLOPPINESS'] = 'time_macros'
    else:
        logger.info("Cache disabled.")
        env['CCACHE_DISABLE'] = '1'

    if ctx.compiler == 'gcc':
        logger.info("Forcing GCC compiler paths.")
        env['CC'] = 'gcc'
        env['CXX'] = 'g++'

    cmake_toolchain = None
    if ctx.is_windows:
        triplet = 'x64-windows' if ctx.arch_label == 'x64' else 'x86-windows'
        cmake_toolchain = root_dir / 'vcpkg/scripts/buildsystems/vcpkg.cmake'

    targets = ['zplayer', 'zeditor', 'zscript', 'zlauncher']
    if ctx.is_windows:
        targets.append('zupdater')
    if args.test_runner:
        targets.append('base_test_runner')
    targets_str = " ".join(targets)

    configure_signatures(ctx)

    logger.info("Running CMake configure...")
    is_official = str(args.official).lower()
    wants_tests = str(args.test_runner).lower()
    sentry = str(args.sentry).lower()

    cmake_config_cmd = [
        "cmake",
        "-S",
        ".",
        "-B",
        "build",
        "-G",
        "Ninja Multi-Config",
        "-DCOPY_RESOURCES=OFF",
        f"-DZC_OFFICIAL={is_official}",
        f"-DZC_VERSION={ctx.release_version}",
        f"-DRELEASE_PLATFORM={ctx.platform_label}",
        "-DRELEASE_CHANNEL=3",
        f"-DREPO={ctx.repo}",
        f"-DWANT_SENTRY={sentry}",
        f"-DWANT_ZC_TESTS={wants_tests}",
    ]
    if ctx.is_windows:
        cmake_config_cmd.append("-DCMAKE_WIN32_EXECUTABLE=1")
    if ctx.is_linux and not args.official:
        # Enable DCHECK in CI on Linux for testing.
        cmake_config_cmd.append("-DCMAKE_CXX_FLAGS=-DDCHECK_IS_ON")
    if cmake_toolchain:
        cmake_config_cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={cmake_toolchain}")
        cmake_config_cmd.append(f"-DVCPKG_TARGET_TRIPLET={triplet}")
    if args.cache:
        cmake_config_cmd.extend(
            [
                "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
            ]
        )

    run_cmd(cmake_config_cmd, env=env)

    if args.cache:
        run_cmd("ccache -z", env=env)

    logger.info("Running CMake build...")
    run_cmd(
        f"cmake --build build --config {ctx.config} --target {targets_str} -- -k 0",
        env=env,
    )

    if args.cache:
        run_cmd("ccache -s", env=env)

    logger.info("Verifying source tree integrity...")
    run_cmd("git diff --cached --exit-code")
    logger.info("Build completed successfully.")


@command("package", help="Package the built artifacts")
@argument("--packages-dir")
@argument("--debug-info", action="store_true", help="Include debug info")
def package(ctx: CiContext, args):
    logger.info(f"Starting packaging process (Config: {ctx.config})...")

    if ctx.is_windows:
        package_name = f"{ctx.release_version}-windows-{ctx.arch_label}.zip"
    elif ctx.is_mac:
        package_name = f"{ctx.release_version}-mac.dmg"
    elif ctx.is_linux:
        package_name = f"{ctx.release_version}-linux.tar.gz"
    else:
        package_name = f"{ctx.release_version}-unknown.zip"

    logger.info(f"Target package name: {package_name}")

    packages_dir = Path(args.packages_dir)
    packages_dir.mkdir(exist_ok=True)

    if ctx.config == 'Coverage':
        logger.info("Packaging Coverage build...")
        build_dir = root_dir / "build"
        tmp_src_dir = build_dir / "tmp-src/parser"
        tmp_src_dir.mkdir(parents=True, exist_ok=True)

        shutil.copy(root_dir / "src/parser/lex.yy.cpp", tmp_src_dir)
        shutil.copy(root_dir / "src/parser/y.tab.cpp", tmp_src_dir)
        shutil.copy(root_dir / "src/parser/y.tab.hpp", tmp_src_dir)

        run_cmd(
            "tar czpvf build.tar.gz * --preserve-permissions", cwd=build_dir, shell=True
        )
        shutil.move(build_dir / "build.tar.gz", packages_dir / "build.tar.gz")

    build_dir = root_dir / "build"
    package_output_dir = build_dir / "package_output"
    package_output_dir.mkdir(parents=True, exist_ok=True)

    cpack_cmd = [
        "cpack",
        "--verbose",
        "-C",
        ctx.config,
        "-D",
        f"CPACK_PACKAGE_DIRECTORY={package_output_dir.absolute()}",
    ]
    if args.debug_info:
        cpack_cmd.extend(
            [
                "-D",
                "CPACK_CUSTOM_INSTALL_VARIABLES=MAC_CREATE_DEBUG_SYMBOLS_BUNDLE=TRUE",
            ]
        )

    run_cmd(cpack_cmd, cwd=build_dir)

    # Move and rename the generated artifact.
    files = [f for f in package_output_dir.iterdir() if f.is_file()]
    if not files:
        logger.error(f"No package found in {package_output_dir}")
        sys.exit(1)

    ext = Path(package_name).suffix
    target_file = next((f for f in files if f.name.endswith(ext)), files[0])
    dest_path = packages_dir / package_name
    shutil.move(target_file, dest_path)
    logger.info(f"Moved {target_file.name} to {dest_path}")

    logger.info("Packaging complete.")


# -----------------------------------------------------------------------------
# test.yml
# -----------------------------------------------------------------------------


@command("extract-package", help="Unpack build artifact for testing")
@argument("--package", required=True)
@argument("--artifact-name")
def extract_package(ctx: CiContext, args):
    logger.info(f"Extracting package for testing (Config: {ctx.config})...")

    if ctx.config == 'Coverage':
        logger.info("Extracting Coverage tarball...")
        (root_dir / "build").mkdir(exist_ok=True)
        run_cmd(f'tar -xvzf "{Path(args.artifact_name) / "build.tar.gz"}" -C build')
        shutil.copytree(
            root_dir / "build/tmp-src", root_dir / "src", dirs_exist_ok=True
        )

    extract_dir = ctx.build_folder_base
    extract_dir.mkdir(parents=True, exist_ok=True)
    package_path = Path(args.package)

    if ctx.is_windows:
        logger.info("Windows detected. Extracting via 7z.")
        run_cmd(['7z', 'x', package_path.absolute()], cwd=extract_dir)

    elif ctx.is_mac:
        logger.info("macOS detected. Mounting dmg and copying app contents.")
        run_cmd(f"hdiutil attach -mountpoint .tmp/zc-mounted '{package_path}'")
        shutil.copytree(
            ".tmp/zc-mounted/ZQuest Classic.app",
            extract_dir / "ZQuest Classic.app",
            dirs_exist_ok=True,
        )
        if Path(".tmp/zc-mounted/ZQuest Classic.app.dSYM").exists():
            shutil.copytree(
                ".tmp/zc-mounted/ZQuest Classic.app.dSYM",
                extract_dir / "ZQuest Classic.app.dSYM",
                dirs_exist_ok=True,
            )
        else:
            logger.warning("Missing ZQuest Classic.app.dSYM")
        run_cmd("hdiutil unmount .tmp/zc-mounted")

    elif ctx.is_linux:
        logger.info("Linux detected. Extracting tarball.")
        run_cmd(f'tar -xvzf "{package_path}" -C "{extract_dir}"')

    logger.info(f"Package ready. Target binary dir is: {ctx.get_binary_dir()}")


@command("replay-tests", help="Run ZC replay tests")
@argument("--runs-on", required=True)
@argument("--run-id", required=True)
@argument("--shard")
@argument("--extra-args", default="")
@argument("--no-jit", action="store_true")
@argument("--no-optimize-zasm", action="store_true")
def replay_tests(ctx: CiContext, args):
    logger.info("Preparing to run replay tests...")

    binary_dir = ctx.get_binary_dir()
    test_results_dir = tmp_dir / f"test_results/{args.run_id}"
    if args.no_jit:
        test_results_dir = test_results_dir.with_name(f"{test_results_dir.name}-nojit")
    if args.no_optimize_zasm:
        test_results_dir = test_results_dir.with_name(
            f"{test_results_dir.name}-nooptimizezasm"
        )
    test_results_dir.mkdir(parents=True, exist_ok=True)

    base_cmd = [
        'tests/run_replay_tests.py',
        f'--build_folder={binary_dir}',
        f'--build_type={ctx.config}',
        f'--ci={args.runs_on}_{ctx.arch}',
        f'--test_results_folder={test_results_dir}',
        '--retries=1',
    ]

    if args.shard:
        base_cmd.append(f"--shard={args.shard}")

    if args.no_jit:
        logger.info("Running some replays with JIT disabled.")
        base_cmd.extend(
            [
                "--filter=playground",
                "--filter=keys.zplay",
                "--filter=yuurand_riviere.zplay",
                "--filter=crucible_quest_short",
                "--no-jit",
                "--no-headless",
            ]
        )

    if args.no_optimize_zasm:
        logger.info("Running some replays with the ZASM optimizer disabled.")
        base_cmd.extend(
            [
                "--filter=playground",
                "--filter=keys.zplay",
                "--filter=yuurand_riviere.zplay",
                "--filter=crucible_quest_short",
                "--no-optimize-zasm",
            ]
        )

    # TODO: not working for some reason
    if ctx.compiler == 'gcc' and (args.no_jit or args.no_optimize_zasm):
        base_cmd = [arg for arg in base_cmd if arg != "--filter=yuurand_riviere.zplay"]
        base_cmd = [arg for arg in base_cmd if arg != "--filter=crucible_quest_short"]

    if args.extra_args:
        logger.info(f"Applying extra arguments: {args.extra_args}")
        extra_args = shlex.split(args.extra_args)

        # Ignore any extra filter args if a fixed replay subset was selected.
        if args.no_jit or args.no_optimize_zasm:
            while "--filter" in extra_args:
                idx = extra_args.index("--filter")
                del extra_args[idx : idx + 2]

        base_cmd.extend(extra_args)

    run_python_cmd(base_cmd)


@command("unit-tests", help="Run Python unit test suite")
def unit_tests(ctx: CiContext, args):
    logger.info("Running Python unit test suite...")

    env = os.environ.copy()
    env['BUILD_FOLDER'] = str(ctx.get_binary_dir())
    # Tests that shell out to run_replay_tests.py need this to pick the right
    # timeouts and duration estimates (the build folder name alone doesn't
    # reveal the config in CI).
    env['BUILD_TYPE'] = ctx.config
    env['CXX'] = ctx.compiler

    if ctx.config != 'Coverage':
        logger.info("Updating auto script tests...")
        run_python_cmd(
            "tests/update_auto_script_tests.py",
            env=env,
        )

    logger.info("Discovering and running unittests...")
    run_python_cmd("-m unittest discover tests", env=env)

    logger.info("Running pytest unit tests...")
    run_python_cmd("-m pytest tests/test_replay_harness.py", env=env)


# -----------------------------------------------------------------------------
# coverage.yml
# -----------------------------------------------------------------------------


@command("coverage", help="Generate coverage report")
def coverage(ctx: CiContext, args):
    logger.info("Generating coverage report...")
    env = os.environ.copy()
    env['GCOV'] = "llvm-cov-14 gcov"
    run_cmd("bash tests/generate_coverage_report.sh", env=env)


# -----------------------------------------------------------------------------
# compare.yml
# -----------------------------------------------------------------------------


def extract_nested_archives(src_dir: Path):
    """Extracts downloaded zips, and then any inner tar files."""
    for filepath in src_dir.rglob("*.zip"):
        with zipfile.ZipFile(filepath, 'r') as zip_ref:
            # Extract to a folder with the same name as the zip (minus extension).
            extract_dir = filepath.parent / filepath.stem
            zip_ref.extractall(extract_dir)

    for tar_path in src_dir.rglob("*.tar"):
        with tarfile.open(tar_path, 'r') as tar_ref:
            tar_ref.extractall(tar_path.parent)


@command("compare-replays", help="Compare replays and notify Discord")
@argument("--run-id", required=True, help="Current GitHub Run ID")
@argument("--ref-name", default="unknown", help="Branch name for discord thread")
@argument("--actor", default="unknown", help="GitHub Actor for discord notification")
def compare_replays(ctx: CiContext, args):
    logger.info("Starting replay comparison process...")
    token = os.environ.get("GITHUB_TOKEN")
    surge_token = os.environ.get("SURGE_TOKEN")
    discord_webhook = os.environ.get("REPLAY_FAILURE_DISCORD_WEBHOOK")

    if not token:
        logger.error("GITHUB_TOKEN environment variable is required.")
        sys.exit(1)

    results_dir = root_dir / "test-results"
    results_dir.mkdir(exist_ok=True)

    # Fetch current run artifacts from test.yml
    logger.info(f"Fetching artifacts for current run: {args.run_id}")
    url = (
        f"https://api.github.com/repos/{ctx.repo}/actions/runs/{args.run_id}/artifacts"
    )
    headers = {
        'Authorization': f'Bearer {token}',
        'Accept': 'application/vnd.github.v3+json',
    }

    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()
        data = response.json()
    except requests.RequestException as e:
        logger.error(f"GitHub API Error: {e}")
        sys.exit(1)

    artifacts = [
        a for a in data.get('artifacts', []) if a['name'].startswith('replays-')
    ]
    if not artifacts:
        logger.error("No test_results found. Exiting.")
        sys.exit(1)

    for artifact in artifacts:
        logger.info(f"Downloading {artifact['name']}...")
        dest = results_dir / f"{artifact['name']}.zip"
        try:
            with requests.get(
                artifact['archive_download_url'], headers=headers, stream=True
            ) as r:
                r.raise_for_status()
                with open(dest, 'wb') as f:
                    shutil.copyfileobj(r.raw, f)
        except requests.RequestException as e:
            logger.error(f"Failed to download artifact {artifact['name']}: {e}")
            sys.exit(1)

    extract_nested_archives(results_dir)

    # Collect baseline via run_test_workflow.py
    gh_out_file = root_dir / ".tmp/github_output.txt"
    gh_out_file.parent.mkdir(parents=True, exist_ok=True)
    gh_out_file.write_text("")

    env = os.environ.copy()
    env['GITHUB_OUTPUT'] = str(gh_out_file.absolute())

    run_python_cmd(
        f"tests/run_test_workflow.py --test_results {results_dir} --repo {ctx.repo} --token {token}",
        env=env,
    )

    baseline_run_id = None
    for line in gh_out_file.read_text().splitlines():
        if "baseline_run_id=" in line:
            baseline_run_id = line.strip().split("=", 1)[1]

    if not baseline_run_id:
        logger.error("Failed to capture baseline_run_id")
        sys.exit(1)

    # Generate Report via compare_replays.py
    run_python_cmd(
        f"tests/compare_replays.py --workflow_run {baseline_run_id} --local {results_dir} --repo {ctx.repo} --token {token}"
    )

    # Upload to surge.sh
    report_dir = root_dir / "tests/compare-report"
    domain = f"zc-replay-compare-{args.run_id}.surge.sh"

    if surge_token:
        logger.info("Deploying report to Surge...")
        env['SURGE_TOKEN'] = surge_token
        run_cmd(f"npx surge@0.23.1 {report_dir} {domain}", env=env)
        report_url = f"https://{domain}"
    else:
        report_url = "No deployment (missing SURGE_TOKEN)"

    # Notify via Discord
    if discord_webhook:
        try:
            webhook = discord.SyncWebhook.from_url(discord_webhook)
            webhook.send(
                f"Report: {report_url}\nCI: https://github.com/{ctx.repo}/actions/runs/{args.run_id}\nActor: {args.actor}",
                thread_name=f"Replay tests failed for {args.ref_name}",
                wait=True,
            )
        except ImportError:
            logger.error("The 'discord' python package is missing.")


# -----------------------------------------------------------------------------
# CLI Entry Point
# -----------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description="ZQuest Classic CI Script")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Global/Shared arguments for all subparsers
    parent = argparse.ArgumentParser(add_help=False)
    parent.add_argument(
        "--arch", default="x64", help="Architecture (x64, win32, intel, aarch64)"
    )
    parent.add_argument("--compiler", default="gcc", help="Compiler (gcc, clang, msvc)")
    parent.add_argument(
        "--config", default="Release", help="Release, RelWithDebInfo, etc."
    )
    parent.add_argument("--release-version", help="Version string")
    parent.add_argument("--repo", required=True, help="Repo name")
    parent.add_argument("--build-folder", help="Target extraction directory")

    # Collect all functions decorated with @command
    commands = [
        obj
        for obj in globals().values()
        if callable(obj) and hasattr(obj, "_cli_command")
    ]

    # Register each command and its specific arguments
    for func in commands:
        sub = subparsers.add_parser(
            func._cli_command, help=func._cli_help, parents=[parent]
        )
        for name_or_flags, kwargs in getattr(func, "_cli_args", []):
            sub.add_argument(*name_or_flags, **kwargs)

    args = parser.parse_args()

    # Initialize the high-level context
    root = Path(__file__).parent.parent.parent.absolute()
    ctx = CiContext(
        arch=args.arch,
        compiler=args.compiler,
        config=args.config,
        release_version=args.release_version or "3.0.0",
        repo=args.repo,
        build_folder_base=Path(args.build_folder or "zc-extracted").absolute(),
    )

    # Dispatch to the appropriate function
    cmd_map = {func._cli_command: func for func in commands}
    cmd_map[args.command](ctx, args)


if __name__ == "__main__":
    main()
