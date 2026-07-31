#!/usr/bin/env python3
"""CLI to query bug and feature request threads in a Discord server."""

import argparse
import asyncio
import json
import os
import sys
import time

from pathlib import Path

import discord

from discord.ext import commands

ZC_GUILD_ID = 876899628556091432
FORUM_CHANNELS = {
    1021382849603051571: "bugs",
    1021385902708248637: "features",
}
THIS_EMOJI_ID = 877358416992030731


# Bot token: ZC_DISCORD_API_KEY env var, or ~/.zc_discord_key (a file
# containing just the token) so no shell profile setup is needed.
def _get_api_key():
    if key := os.environ.get("ZC_DISCORD_API_KEY"):
        return key
    key_path = Path.home() / ".zc_discord_key"
    if key_path.exists():
        return key_path.read_text().strip()
    sys.exit(
        "error: no Discord bot token found.\n"
        "Set the ZC_DISCORD_API_KEY env var, or write the token to "
        f"{key_path}"
    )


API_KEY = None  # resolved by main() after arg parsing
CACHE_PATH = Path(__file__).parent.parent / ".tmp" / "threads" / ".this_counts.json"
CACHE_TTL = 86400  # 1 day

RESET = "\033[0m"
BOLD = "\033[1m"
DIM = "\033[2m"
RED = "\033[91m"
BLUE = "\033[94m"
YELLOW = "\033[93m"
GREEN = "\033[92m"
CYAN = "\033[96m"


def colored(text, *codes):
    if not sys.stdout.isatty():
        return text
    return "".join(codes) + text + RESET


def create_bot():
    intents = discord.Intents.default()
    intents.message_content = True
    intents.members = True
    return commands.Bot(".", intents=intents)


def load_cache() -> dict:
    if CACHE_PATH.exists():
        return json.loads(CACHE_PATH.read_text())
    return {}


def save_cache(cache: dict):
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    CACHE_PATH.write_text(json.dumps(cache, indent=2))


def get_this_count(message: discord.Message) -> int:
    for r in message.reactions:
        if not isinstance(r.emoji, str) and r.emoji.id == THIS_EMOJI_ID:
            return r.count
    return 0


async def cmd_list(bot: commands.Bot, args):
    guild = bot.get_guild(ZC_GUILD_ID)

    if args.channel == "bugs":
        channel_ids = [1021382849603051571]
    elif args.channel == "features":
        channel_ids = [1021385902708248637]
    else:
        channel_ids = list(FORUM_CHANNELS.keys())

    include_tags = {t.lower() for t in args.include_tag}
    exclude_tags = {t.lower() for t in args.exclude_tag}

    def tag_names(thread):
        return {t.name.lower() for t in thread.applied_tags}

    def is_filtered(thread):
        tags = tag_names(thread)
        if not args.include_closed and "closed" in tags:
            return True
        if exclude_tags and tags & exclude_tags:
            return True
        if include_tags and not (tags & include_tags):
            return True
        return False

    all_threads = []
    for cid in channel_ids:
        channel = guild.get_channel(cid)
        for thread in channel.threads:
            if not is_filtered(thread):
                all_threads.append((cid, thread))
        if args.include_archived:
            async for thread in channel.archived_threads(limit=None):
                if not is_filtered(thread):
                    all_threads.append((cid, thread))

    cache = load_cache()
    now = time.time()

    async def fetch_count(cid, thread):
        tid = str(thread.id)
        entry = cache.get(tid)
        if entry and now - entry["fetched_at"] < CACHE_TTL:
            return (cid, thread, entry["count"])
        starter = [m async for m in thread.history(oldest_first=True, limit=1)]
        count = get_this_count(starter[0]) if starter else 0
        cache[tid] = {"count": count, "fetched_at": now}
        return (cid, thread, count)

    threads_with_count = await asyncio.gather(
        *[fetch_count(cid, t) for cid, t in all_threads]
    )
    save_cache(cache)

    threads_with_count.sort(key=lambda x: x[2], reverse=True)

    if not threads_with_count:
        print("No threads found.")
        return

    print(colored(f'{"THIS":>4}  {"THREAD ID":<20}  {"TYPE":<10}  TITLE', BOLD))
    print(colored("─" * 76, DIM))

    for cid, thread, this_count in threads_with_count:
        name = FORUM_CHANNELS[cid]
        tag_str = " ".join(colored(f"[{t.name}]", YELLOW) for t in thread.applied_tags)
        type_str = colored(f"{name:<10}", RED if name == "bugs" else BLUE)
        title = colored(thread.name, DIM) if thread.archived else thread.name
        arc = colored(" (archived)", DIM) if thread.archived else ""
        count = colored(f"{this_count:>4}", GREEN if this_count else DIM)

        line = f"{count}  {thread.id:<20}  {type_str}  {title}"
        if tag_str:
            line += f"  {tag_str}"
        line += arc
        print(line)


async def cmd_show(bot: commands.Bot, args):
    thread_id = int(args.thread_id)

    try:
        thread = await bot.fetch_channel(thread_id)
    except discord.NotFound:
        print(f"Thread {thread_id} not found.", file=sys.stderr)
        return

    channel_name = FORUM_CHANNELS.get(thread.parent_id, "unknown")
    messages = [m async for m in thread.history(oldest_first=True, limit=None)]
    this_count = get_this_count(messages[0]) if messages else 0
    tags = [t.name for t in thread.applied_tags]

    type_color = RED if channel_name == "bugs" else BLUE
    print(colored(thread.name, BOLD))
    print(f"ID:     {thread.id}")
    print(f"Type:   {colored(channel_name, type_color)}")
    if tags:
        print(f'Tags:   {", ".join(tags)}')
    print(f"This:   {colored(str(this_count), GREEN if this_count else DIM)}")
    print(
        f'Status: {colored("archived", DIM) if thread.archived else colored("active", GREEN)}'
    )
    print()

    for i, msg in enumerate(messages):
        username = msg.author.display_name or msg.author.name
        timestamp = msg.created_at.strftime("%Y-%m-%d")
        content = msg.content.strip()

        if i == 0:
            print(colored("─── Original Post " + "─" * 52, DIM))
        else:
            if i == 1:
                print()
                print(colored("─── Replies " + "─" * 58, DIM))
            print()

        print(f"{colored(username, BOLD)}  {colored(timestamp, DIM)}")
        if content:
            print(content)
        for att in msg.attachments:
            print(colored(f"  ↳ {att.filename}  {att.url}", CYAN))
        for embed in msg.embeds:
            label = embed.title or (embed.description or "")[:80]
            if label:
                print(colored(f"  [embed: {label}]", YELLOW))


async def get_threads_async(channel_type=None, exclude_closed=True):
    """
    Fetch thread metadata (no descriptions — those require a separate API call per
    thread and are only fetched for threads actually assigned to an agent).
    Returns list of dicts: {id, title, url, type, tags, this_count}.
    """
    results = []
    bot = create_bot()

    @bot.event
    async def on_ready():
        guild = bot.get_guild(ZC_GUILD_ID)

        if channel_type == "bugs":
            cids = [1021382849603051571]
        elif channel_type == "features":
            cids = [1021385902708248637]
        else:
            cids = list(FORUM_CHANNELS.keys())

        raw = []
        for cid in cids:
            channel = guild.get_channel(cid)
            for thread in channel.threads:
                tags = {t.name.lower() for t in thread.applied_tags}
                if exclude_closed and "closed" in tags:
                    continue
                raw.append((cid, thread))

        cache = load_cache()
        now = time.time()

        async def fetch_count(thread):
            tid = str(thread.id)
            entry = cache.get(tid)
            if entry and now - entry["fetched_at"] < CACHE_TTL:
                return entry["count"]
            starter = [m async for m in thread.history(oldest_first=True, limit=1)]
            count = get_this_count(starter[0]) if starter else 0
            cache[tid] = {"count": count, "fetched_at": now}
            return count

        counts = await asyncio.gather(*[fetch_count(t) for _, t in raw])
        save_cache(cache)

        for (cid, thread), count in zip(raw, counts):
            results.append(
                {
                    "id": thread.id,
                    "title": thread.name,
                    "url": thread.jump_url,
                    "type": FORUM_CHANNELS[cid],
                    "tags": [t.name for t in thread.applied_tags],
                    "this_count": count,
                }
            )

        await bot.close()

    await bot.start(API_KEY)
    return results


def _message_to_dict(m: discord.Message) -> dict:
    attachments = [{"filename": a.filename, "url": a.url} for a in m.attachments]
    embeds = []
    for e in m.embeds:
        label = e.title or (e.description or "")[:80]
        if label:
            embeds.append({"label": label})
    return {
        "author": m.author.display_name or m.author.name,
        "date": m.created_at.strftime("%Y-%m-%d"),
        "content": m.content.strip(),
        "attachments": attachments,
        "embeds": embeds,
    }


async def enrich_threads_async(threads: list[dict]) -> list[dict]:
    """
    Fetch the starter-message description for a list of thread dicts (in-place).
    Only call this for threads that have actually been assigned to agents.
    """
    bot = create_bot()

    @bot.event
    async def on_ready():
        for t in threads:
            try:
                ch = await bot.fetch_channel(t["id"])
                messages = [m async for m in ch.history(oldest_first=True, limit=None)]
                t["description"] = messages[0].content if messages else ""
                t["history"] = [
                    _message_to_dict(m) for m in messages if not m.is_system()
                ]
            except discord.NotFound:
                t["description"] = ""
                t["history"] = []
        await bot.close()

    await bot.start(API_KEY)
    return threads


def get_threads(channel_type=None, exclude_closed=True):
    """Synchronous wrapper — only call this outside of an async context."""
    return asyncio.run(get_threads_async(channel_type, exclude_closed))


async def get_thread_async(thread_id: int):
    """
    Fetch a single thread by ID. Much faster than get_threads_async when you
    know the thread ID. Returns a plain dict or None if not found.
    """
    result = []
    bot = create_bot()

    @bot.event
    async def on_ready():
        try:
            thread = await bot.fetch_channel(thread_id)
            messages = [m async for m in thread.history(oldest_first=True, limit=None)]
            description = messages[0].content if messages else ""
            history = [_message_to_dict(m) for m in messages if not m.is_system()]
            result.append(
                {
                    "id": thread.id,
                    "title": thread.name,
                    "url": thread.jump_url,
                    "type": FORUM_CHANNELS.get(thread.parent_id, "unknown"),
                    "tags": [t.name for t in thread.applied_tags],
                    "description": description,
                    "history": history,
                }
            )
        except discord.NotFound:
            pass
        await bot.close()

    await bot.start(API_KEY)
    return result[0] if result else None


def main():
    parser = argparse.ArgumentParser(
        description="Query Discord bug/feature threads",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
examples:
  %(prog)s list                       list active threads (both channels)
  %(prog)s list bugs                  list active bug threads
  %(prog)s list features              list active feature requests
  %(prog)s list --include-archived    include archived threads
  %(prog)s show 1234567890123456789   show full thread with replies\
""",
    )

    sub = parser.add_subparsers(dest="command", required=True)

    lp = sub.add_parser("list", help="list threads")
    lp.add_argument(
        "channel",
        nargs="?",
        choices=["bugs", "features"],
        help="filter by channel type",
    )
    lp.add_argument(
        "--include-archived",
        action="store_true",
        help="include archived (resolved) threads",
    )
    lp.add_argument(
        "--include-closed", action="store_true", help="include threads tagged Closed"
    )
    lp.add_argument(
        "--include-tag",
        metavar="TAG",
        action="append",
        default=[],
        help="only show threads with this tag (repeatable)",
    )
    lp.add_argument(
        "--exclude-tag",
        metavar="TAG",
        action="append",
        default=[],
        help="hide threads with this tag (repeatable)",
    )

    sp = sub.add_parser("show", help="show thread details and replies")
    sp.add_argument("thread_id", help="Discord thread ID")

    args = parser.parse_args()

    global API_KEY
    API_KEY = _get_api_key()

    bot = create_bot()

    @bot.event
    async def on_ready():
        if args.command == "list":
            await cmd_list(bot, args)
        elif args.command == "show":
            await cmd_show(bot, args)
        await bot.close()

    bot.run(API_KEY)


if __name__ == "__main__":
    main()
