#!/usr/bin/env python
from __future__ import print_function

import subprocess
import sys
import os


def _get_output(command, repo_path):
    return subprocess.check_output(
        command,
        cwd=repo_path,
        stderr=subprocess.STDOUT,
        universal_newlines=True,
    ).strip()


def get_git_info(repo_path):
    # Python 2.6 doesn't have check_output, so check for that
    try:
        subprocess.check_output
        subprocess.check_call
    except AttributeError:
        return None

    try:
        git_hash = _get_output(["git", "rev-parse", "--short", "HEAD"], repo_path)
    except subprocess.CalledProcessError as er:
        if er.returncode == 128:
            # git exit code of 128 means no repository found
            return None
        git_hash = "unknown"
    except OSError:
        return None

    try:
        with open(os.devnull, "wb") as devnull:
            subprocess.check_call(
                ["git", "describe", "--tags", "--exact-match"],
                cwd=repo_path,
                stdout=devnull,
                stderr=devnull,
            )
        git_tag = _get_output(
            ["git", "describe", "--long", "--tags", "--dirty", "--always"],
            repo_path,
        )
    except subprocess.CalledProcessError:
        try:
            tag_list = _get_output(["git", "tag", "--sort=-v:refname"], repo_path)
            latest_tag = tag_list.splitlines()[0] if tag_list else ""
            if latest_tag:
                commit_count = _get_output(
                    ["git", "rev-list", "--count", "%s..HEAD" % latest_tag],
                    repo_path,
                )
                git_tag = "%s-%s-g%s" % (latest_tag, commit_count, git_hash)
            else:
                git_tag = git_hash
        except subprocess.CalledProcessError:
            git_tag = git_hash
        except OSError:
            return None

    try:
        # Check if there are any modified files.
        subprocess.check_call(
            ["git", "diff", "--no-ext-diff", "--quiet", "--exit-code"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
        )
        # Check if there are any staged files.
        subprocess.check_call(
            ["git", "diff-index", "--cached", "--quiet", "HEAD", "--"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
        )
    except subprocess.CalledProcessError:
        git_hash += "-dirty"
    except OSError:
        return None

    return git_tag, git_hash


def main():
    repo_path = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else "."
    info = get_git_info(repo_path)
    if info is None:
        info = ("", "unknown")
    tag, commit = info
    print("MICROPY_GIT_TAG=" + tag)
    print("MICROPY_GIT_HASH=" + commit)

if __name__ == "__main__":
    main()
