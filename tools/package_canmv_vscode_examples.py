#!/usr/bin/env python3
"""Package CanMV examples for the VS Code extension CDN layout.

The output is content-addressed so CI can publish examples only when example
content changes, while firmware/stub manifests can point at the matching
examples package.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import subprocess
import zipfile
from pathlib import Path


EXAMPLE_DIR_PATTERN = re.compile(r"\d{2}-.+")


def run_git(args: list[str], cwd: Path) -> str:
    try:
        return subprocess.check_output(args, cwd=str(cwd), stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return ""


def copy_examples(resource_dir: Path, stage_dir: Path) -> None:
    src = resource_dir / "examples"
    if not src.is_dir():
        raise SystemExit(f"Examples directory not found: {src}")

    example_dirs = sorted(
        path for path in src.iterdir()
        if path.is_dir() and EXAMPLE_DIR_PATTERN.fullmatch(path.name)
    )
    if not example_dirs:
        raise SystemExit(f"No numbered example directories found in: {src}")
    if not any(
        path.is_file()
        for directory in example_dirs
        for path in directory.rglob("*.py")
    ):
        raise SystemExit(f"No Python examples found in numbered directories under: {src}")

    examples_out = stage_dir / "examples"
    examples_out.mkdir(parents=True, exist_ok=True)
    for directory in example_dirs:
        for source in sorted(directory.rglob("*.py")):
            if not source.is_file():
                continue
            destination = examples_out / directory.name / source.relative_to(directory)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, destination)


def copy_models(resource_dir: Path, stage_dir: Path) -> None:
    src = resource_dir / "models"
    if src.exists():
        shutil.copytree(src, stage_dir / "models")


def iter_files(root: Path) -> list[Path]:
    return sorted(path for path in root.rglob("*") if path.is_file())


def tree_hash(root: Path) -> str:
    digest = hashlib.sha256()
    for file_path in iter_files(root):
        rel = file_path.relative_to(root).as_posix()
        digest.update(rel.encode("utf-8"))
        digest.update(b"\0")
        digest.update(file_path.read_bytes())
        digest.update(b"\0")
    return digest.hexdigest()


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


def write_text(path: Path, value: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("resource_dir", type=Path, help="CanMV resources directory")
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("generated") / "canmv_vscode_extension",
        help="output root for the new CDN layout",
    )
    parser.add_argument("--commit", default=os.environ.get("CI_COMMIT_SHA", ""), help="source commit for metadata")
    args = parser.parse_args(argv)

    resource_dir = args.resource_dir.resolve()
    if not resource_dir.exists():
        raise SystemExit(f"Resources directory not found: {resource_dir}")

    out_root = args.out.resolve()
    stage_dir = out_root / "_stage_examples"
    if stage_dir.exists():
        shutil.rmtree(stage_dir)
    stage_dir.mkdir(parents=True, exist_ok=True)

    copy_examples(resource_dir, stage_dir)
    copy_models(resource_dir, stage_dir)

    content_id = tree_hash(stage_dir)
    examples_dir = out_root / "examples"
    zip_path = examples_dir / f"{content_id}.zip"
    json_path = examples_dir / f"{content_id}.json"
    zip_tree(stage_dir, zip_path)

    commit = args.commit or run_git(["git", "rev-parse", "HEAD"], resource_dir.parent)
    metadata = {
        "schema": 1,
        "artifact_type": "canmv-vscode-examples",
        "id": content_id,
        "source_commit": commit,
        "created_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "zip": f"{content_id}.zip",
        "zip_sha256": sha256_file(zip_path),
        "file_count": len(iter_files(stage_dir)),
        "contains": {
            "examples": (stage_dir / "examples").exists(),
            "models": (stage_dir / "models").exists(),
        },
    }
    write_text(json_path, json.dumps(metadata, indent=2, sort_keys=True) + "\n")
    write_text(examples_dir / "latest", content_id + "\n")

    shutil.rmtree(stage_dir)
    print(f"Packaged CanMV VS Code examples: {zip_path}")
    print(f"  id: {content_id}")
    print(f"  files: {metadata['file_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
