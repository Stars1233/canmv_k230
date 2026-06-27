#!/usr/bin/env python3
"""Package CanMV stubs for the VS Code extension CDN layout."""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import subprocess
import zipfile
from pathlib import Path
from typing import Any


def run_git(args: list[str], cwd: Path) -> str:
    try:
        return subprocess.check_output(args, cwd=str(cwd), stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return ""


def iter_files(root: Path) -> list[Path]:
    return sorted(path for path in root.rglob("*") if path.is_file())


def zip_tree(source_dir: Path, zip_path: Path) -> None:
    zip_path.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for file_path in iter_files(source_dir):
            rel = file_path.relative_to(source_dir).as_posix()
            info = zipfile.ZipInfo(rel)
            info.date_time = (1980, 1, 1, 0, 0, 0)
            info.external_attr = 0o644 << 16
            zipf.writestr(info, file_path.read_bytes())


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file_obj:
        for chunk in iter(lambda: file_obj.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}


def write_json(path: Path, data: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding="utf-8")


def canonical_commit(value: str, repo_dir: Path) -> str:
    commit = value.strip()
    if commit:
        return commit
    commit = os.environ.get("CI_COMMIT_SHA", "").strip()
    if commit:
        return commit
    return run_git(["git", "rev-parse", "HEAD"], repo_dir)


def require_full_commit(commit: str) -> str:
    if not re.fullmatch(r"[0-9a-fA-F]{40}", commit):
        raise SystemExit(f"Firmware commit must be a full 40-character SHA: {commit!r}")
    return commit.lower()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--stubs-dir", type=Path, required=True, help="generated stubs directory")
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("generated") / "canmv_vscode_extension",
        help="output root for the new CDN layout",
    )
    parser.add_argument("--firmware-commit", default="", help="canonical full firmware commit")
    parser.add_argument("--board", default=os.environ.get("STUB_BOARD", "k230_canmv_01studio"))
    parser.add_argument("--build-config", default="")
    parser.add_argument("--examples-id", default="", help="examples content id to link in firmware manifest")
    args = parser.parse_args(argv)

    stubs_dir = args.stubs_dir.resolve()
    if not stubs_dir.exists():
        raise SystemExit(f"Stubs directory not found: {stubs_dir}")

    repo_dir = Path(__file__).resolve().parents[1]
    firmware_commit = require_full_commit(canonical_commit(args.firmware_commit, repo_dir))
    if not firmware_commit:
        raise SystemExit("Could not determine firmware commit")

    out_root = args.out.resolve()
    stubs_out = out_root / "stubs"
    firmware_out = out_root / "firmware" / firmware_commit
    zip_path = stubs_out / f"{firmware_commit}.zip"
    zip_tree(stubs_dir, zip_path)

    source_metadata = read_json(stubs_dir / "canmv_stub_metadata.json")
    artifact = {
        "schema": 1,
        "artifact_type": "canmv-vscode-stubs",
        "firmware_commit": firmware_commit,
        "board": args.board,
        "build_config": args.build_config or f"{args.board}_defconfig",
        "created_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "zip": f"{firmware_commit}.zip",
        "zip_sha256": sha256_file(zip_path),
        "stub_file_count": len([path for path in iter_files(stubs_dir) if path.suffix == ".pyi"]),
        "source_metadata": source_metadata,
    }
    write_json(stubs_out / f"{firmware_commit}.json", artifact)
    write_text(stubs_out / "latest", firmware_commit + "\n")

    examples_id = args.examples_id.strip()
    manifest = {
        "schema": 1,
        "artifact_type": "canmv-vscode-firmware-manifest",
        "firmware_commit": firmware_commit,
        "board": args.board,
        "build_config": artifact["build_config"],
        "stubs": {
            "url": f"../../stubs/{firmware_commit}.zip",
            "metadata": f"../../stubs/{firmware_commit}.json",
            "sha256": artifact["zip_sha256"],
        },
        "examples": {
            "id": examples_id,
            "url": f"../../examples/{examples_id}.zip" if examples_id else "",
            "metadata": f"../../examples/{examples_id}.json" if examples_id else "",
        },
    }
    write_json(firmware_out / "manifest.json", manifest)
    write_text(out_root / "firmware" / "latest", firmware_commit + "\n")

    print(f"Packaged CanMV VS Code stubs: {zip_path}")
    print(f"  firmware_commit: {firmware_commit}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
