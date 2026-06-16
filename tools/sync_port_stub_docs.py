#!/usr/bin/env python3
"""Generate/update CanMV port //| stub docs from the C API analysis."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Any

TOOLS_DIR = Path(__file__).resolve().parent
CANMV_DIR = TOOLS_DIR.parents[0]
PORT_DIR = CANMV_DIR / "port"
THIRD_PARTY_DIR = PORT_DIR / "3rd-party"
AUTO_MARK = "# Auto-generated CanMV stub docs. Edit the signatures/docstrings here."

sys.path.insert(0, str(TOOLS_DIR))
import gen_canmv_stubs as stubs  # noqa: E402


def is_port_source(path: Path, include_third_party: bool = False) -> bool:
    try:
        path.resolve().relative_to(PORT_DIR.resolve())
    except ValueError:
        return False
    if not include_third_party:
        try:
            path.resolve().relative_to(THIRD_PARTY_DIR.resolve())
            return False
        except ValueError:
            pass
    return path.suffix in {".c", ".cpp", ".h", ".hpp"}


def remove_auto_blocks(text: str) -> str:
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    i = 0
    while i < len(lines):
        m = stubs.STUB_DOC_LINE_RE.match(lines[i])
        if m and AUTO_MARK in m.group("text"):
            i += 1
            while i < len(lines) and stubs.STUB_DOC_LINE_RE.match(lines[i]):
                i += 1
            if i < len(lines) and not lines[i].strip():
                i += 1
            continue
        out.append(lines[i])
        i += 1
    return "".join(out)


def doc_overrides_from_text(text: str, default_module: str | None = None) -> stubs.OverrideMap:
    overrides: stubs.OverrideMap = {}
    block: list[str] = []
    for line in text.splitlines():
        match = stubs.STUB_DOC_LINE_RE.match(line)
        if match:
            block.append(match.group("text"))
            continue
        if block:
            overrides = stubs.merge_overrides(overrides, stubs.stub_doc_block_to_override(default_module, block))
            block = []
    if block:
        overrides = stubs.merge_overrides(overrides, stubs.stub_doc_block_to_override(default_module, block))
    return overrides


def comment_block(lines: list[str]) -> str:
    return "".join(f"//| {line}\n" if line else "//|\n" for line in lines) + "\n"


def clean_doc(text: str) -> str:
    return " ".join(text.strip().split()).replace('"""', "'''")


def phrase(name: str) -> str:
    return name.strip("_").replace("_", " ")


def default_doc(name: str, owner: str) -> str:
    words = phrase(name)
    target = owner.split(".")[-1] if owner else "object"
    if name in {"__init__", "init"}:
        return f"Initialize {owner}."
    if name in {"deinit", "close", "release", "destroy"} or name.endswith("_destroy"):
        return f"Release resources held by {owner}."
    if name in {"start", "stop", "reset", "sync", "seek", "flush", "feed", "open"}:
        return f"{words.capitalize()} {owner}."
    if name == "run":
        return f"Run {owner}."
    if name == "build":
        return f"Build {owner}."
    if name.startswith("set_"):
        return f"Set {phrase(name[4:])} for {owner}."
    if name.startswith("get_"):
        return f"Return {phrase(name[4:])} for {owner}."
    if name.startswith("is_") or name.startswith("has_") or name in {"inited", "isenabled", "is_closed", "txdone", "any"}:
        return f"Return the {words} state for {owner}."
    if name.startswith("read"):
        return f"Read data from {owner}."
    if name.startswith("write") or name.startswith("send"):
        return f"Write or send data using {owner}."
    if name.startswith("load_") or name == "load":
        return f"Load {phrase(name[5:] if name.startswith('load_') else target)} for {owner}."
    if name.startswith("save_") or name == "save":
        return f"Save {phrase(name[5:] if name.startswith('save_') else target)} from {owner}."
    if name.startswith("find_"):
        return f"Find {phrase(name[5:])} in {owner}."
    if name.startswith("draw_"):
        return f"Draw {phrase(name[5:])} on {owner}."
    if name.startswith("to_"):
        return f"Convert {owner} to {phrase(name[3:])}."
    if name.startswith("from_"):
        return f"Create {owner} from {phrase(name[5:])}."
    if "post_process" in name or "postprocess" in name:
        subject = words.replace(" post process", "").replace(" postprocess", "")
        return f"Run {subject} post-processing for {owner}."
    if "preprocess" in name or "pre_process" in name:
        subject = words.replace(" pre process", "").replace(" preprocess", "")
        return f"Run {subject} preprocessing for {owner}."
    if name.endswith("_process"):
        return f"Process {phrase(name[:-8])} for {owner}."
    if name.startswith(("rgb", "grayscale", "binary", "lab", "yuv")) and "_to_" in name:
        src, dst = name.split("_to_", 1)
        return f"Convert {phrase(src)} values to {phrase(dst)}."
    if name in {"width", "height", "size", "count", "offset", "version", "format", "type", "poolid", "phyaddr", "virtaddr"}:
        return f"Return {words} for {owner}."
    if name in {"enable", "disable", "configure", "config_layer", "bind_layer", "unbind_layer"}:
        return f"{words.capitalize()} for {owner}."
    if name in {"convert", "copy", "copy_to", "copy_from", "resize", "compress", "blend", "crop", "replace", "assign"}:
        return f"{words.capitalize()} data for {owner}."
    return f"Perform {words} for {owner}."


def table_position(text: str, table_name: str | None) -> int | None:
    if not table_name:
        return None
    pattern = re.compile(
        r"(?m)^\s*(?:STATIC\s+|static\s+)?(?:const\s+)?mp_rom_map_elem_t\s+"
        + re.escape(table_name)
        + r"\s*\[\]\s*=\s*\{"
    )
    m = pattern.search(text)
    return m.start() if m else None


def source_for_type(analysis: stubs.CAnalysis, source_text: dict[Path, str]) -> dict[str, Path]:
    result: dict[str, Path] = {}
    for type_var in analysis.types:
        needle = f"MP_DEFINE_CONST_OBJ_TYPE(\n    {type_var}"
        for path, text in source_text.items():
            if needle in text or re.search(r"MP_DEFINE_CONST_OBJ_TYPE\(\s*" + re.escape(type_var) + r"\b", text):
                result[type_var] = path
                break
    return result


def type_modules(analysis: stubs.CAnalysis) -> dict[str, set[tuple[str, str]]]:
    result: dict[str, set[tuple[str, str]]] = {}
    qstr_types = stubs.types_by_qstr(analysis)
    for module_name, module in analysis.modules.items():
        for name, (kind, ref) in stubs.module_entries(analysis, module).items():
            if kind == "class" and ref in analysis.types:
                result.setdefault(ref, set()).add((module_name, name))
            elif kind in {"function", "object"} and name in qstr_types:
                result.setdefault(qstr_types[name].type_var, set()).add((module_name, name))
    return result


def symbol_override(overrides: stubs.OverrideMap, module: str, name: str) -> dict[str, Any]:
    value = stubs.module_symbols(overrides, module).get(name, {})
    return value if isinstance(value, dict) else {}


def method_override(class_override: dict[str, Any], name: str) -> dict[str, Any]:
    methods = class_override.get("methods", {})
    value = methods.get(name, {}) if isinstance(methods, dict) else {}
    return value if isinstance(value, dict) else {}


def class_block(
    module: str,
    ctype: stubs.CType,
    existing: stubs.OverrideMap,
    preferred: stubs.OverrideMap,
    class_name: str | None = None,
) -> str | None:
    class_name = class_name or ctype.qstr
    existing_class = symbol_override(existing, module, class_name)
    preferred_class = symbol_override(preferred, module, class_name)
    existing_methods = existing_class.get("methods", {}) if isinstance(existing_class.get("methods", {}), dict) else {}

    lines = [AUTO_MARK, f"module: {module}", f"class {class_name}:"]
    added = False
    if not existing_class.get("doc"):
        lines.append(f'    """{clean_doc(preferred_class.get("doc") or f"{module}.{class_name} object.")}"""')
        added = True
    if ctype.make_new is not None and not existing_class.get("init"):
        init_sig = preferred_class.get("init") or stubs.signature_from_args(["self"] + ctype.init_args, "None")
        lines.append(f"    def __init__{stubs.ensure_signature(init_sig, '(self, *args: Any, **kwargs: Any) -> None')}:")
        lines.append(f'        """{clean_doc(preferred_class.get("init_doc") or f"Create a {module}.{class_name} object.")}"""')
        added = True
    for method_name, signature in sorted(ctype.methods.items()):
        if method_name in existing_methods or not stubs.valid_stub_identifier(method_name):
            continue
        pref = method_override(preferred_class, method_name)
        final_sig = stubs.ensure_signature(pref.get("signature"), signature)
        lines.append(f"    def {method_name}{final_sig}:")
        lines.append(f'        """{clean_doc(pref.get("doc") or default_doc(method_name, f"{module}.{class_name}"))}"""')
        added = True
    return comment_block(lines) if added else None


def module_block(module_name: str, module: stubs.CModule, analysis: stubs.CAnalysis, existing: stubs.OverrideMap, preferred: stubs.OverrideMap) -> str | None:
    existing_module = stubs.module_override(existing, module_name)
    existing_symbols = existing_module.get("symbols", {}) if isinstance(existing_module.get("symbols", {}), dict) else {}
    lines = [AUTO_MARK, f"module: {module_name}"]
    added = False
    if not existing_module.get("doc"):
        lines.append(f'"""{clean_doc(stubs.module_override(preferred, module_name).get("doc") or f"CanMV {module_name} module.")}"""')
        added = True
    for name, (kind, ref) in sorted(stubs.module_entries(analysis, module).items()):
        if kind != "function" or ref not in analysis.functions or name in existing_symbols or not stubs.valid_stub_identifier(name):
            continue
        pref = symbol_override(preferred, module_name, name)
        sig = stubs.ensure_signature(pref.get("signature"), stubs.function_signature(analysis.functions[ref], include_self=False, name=name))
        lines.append(f"def {name}{sig}:")
        lines.append(f'    """{clean_doc(pref.get("doc") or default_doc(name, module_name))}"""')
        added = True
    return comment_block(lines) if added else None


def insert_blocks(text: str, blocks: list[tuple[int, str]]) -> str:
    grouped: dict[int, list[str]] = {}
    for pos, block in blocks:
        grouped.setdefault(pos, []).append(block)
    for pos in sorted(grouped, reverse=True):
        text = text[:pos] + "".join(grouped[pos]) + text[pos:]
    return text


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=None)
    parser.add_argument("--out", type=Path, default=None)
    parser.add_argument("--overrides", type=Path, default=stubs.DEFAULT_OVERRIDES)
    parser.add_argument("--include-third-party", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args(argv)

    build_dir = args.build_dir or stubs.default_build_dir()
    analysis = stubs.analyze_c_sources(stubs.discover_qstr_sources(build_dir) or stubs.fallback_sources())
    preferred = stubs.load_overrides(args.overrides)
    source_paths = sorted({p.resolve() for p in analysis.sources if is_port_source(p, args.include_third_party)})
    original = {p: p.read_text(encoding="utf-8", errors="ignore") for p in source_paths}
    cleaned = {p: remove_auto_blocks(t) for p, t in original.items()}
    type_source = source_for_type(analysis, cleaned)
    modules_for_type = type_modules(analysis)

    default_module_by_source: dict[Path, str | None] = {}
    for module_name, module in analysis.modules.items():
        if module.source:
            p = Path(module.source).resolve()
            default_module_by_source[p] = module_name if p not in default_module_by_source else None

    existing_by_source = {p: doc_overrides_from_text(cleaned[p], default_module_by_source.get(p)) for p in source_paths}
    blocks_by_source: dict[Path, list[tuple[int, str]]] = {p: [] for p in source_paths}
    block_count = 0

    for type_var, ctype in sorted(analysis.types.items(), key=lambda item: item[1].qstr):
        p = type_source.get(type_var)
        if p not in cleaned:
            continue
        table = analysis.dict_tables.get(ctype.locals_dict or "")
        pos = table_position(cleaned[p], table)
        if pos is None:
            continue
        for module_name, class_name in sorted(modules_for_type.get(type_var, set())):
            block = class_block(module_name, ctype, existing_by_source[p], preferred, class_name)
            if block:
                blocks_by_source[p].append((pos, block))
                block_count += 1

    for module_name, module in sorted(analysis.modules.items()):
        if not module.source:
            continue
        p = Path(module.source).resolve()
        if p not in cleaned:
            continue
        pos = table_position(cleaned[p], module.table)
        if pos is None:
            continue
        block = module_block(module_name, module, analysis, existing_by_source[p], preferred)
        if block:
            blocks_by_source[p].append((pos, block))
            block_count += 1

    changed = 0
    for p in source_paths:
        new_text = insert_blocks(cleaned[p], blocks_by_source[p])
        if new_text != original[p]:
            changed += 1
            if not args.dry_run:
                p.write_text(new_text, encoding="utf-8")

    print(f"Port sources scanned: {len(source_paths)}")
    print(f"Doc blocks generated: {block_count}")
    print(f"Files changed: {changed}{' (dry run)' if args.dry_run else ''}")

    if args.out and not args.dry_run:
        stubs.main(["--build-dir", str(build_dir), "--out", str(args.out), "--overrides", str(args.overrides)])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
