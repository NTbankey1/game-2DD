#!/usr/bin/env python3
"""Verify source files don't include from forbidden layers.

Layer rules:
core/ → can ONLY include core/ headers (no engine, no game)
engine/ → can include core/ + engine/ (no game)
game/ → can include core/ + engine/ + game/
tests/ → can include anything
main.cpp → can include anything

Usage: python3 scripts/check_layer_deps.py
Return code 0 = clean, 1 = violations found.
"""
import re
import sys
from pathlib import Path

SRC = Path("src")
INCLUDE_RE = re.compile(r'#include\s+"[^"]+"|#include\s+<[^>]+>')
VIOLATIONS = []

def extracted_layer(path: Path) -> int:
    """Return layer depth: 0=core, 1=engine, 2=game."""
    parts = path.relative_to(SRC).parts
    if len(parts) >= 2:
        return {"core": 0, "engine": 1, "game": 2}.get(parts[0], -1)
    return -1

def target_layer(include: str) -> int | None:
    """Extract target layer from include path."""
    path = include.strip('#include "').rstrip('"')
    parts = path.split("/")
    if len(parts) >= 1:
        return {"core": 0, "engine": 1, "game": 2}.get(parts[0], None)
    return None

for cpp_file in sorted(SRC.rglob("*.[ch]pp")):
    source = extracted_layer(cpp_file)
    if source < 0:
        continue
    for i, line in enumerate(cpp_file.read_text().splitlines(), 1):
        m = INCLUDE_RE.search(line)
        if not m:
            continue
        target = target_layer(m.group(0))
        if target is not None and target > source:
            VIOLATIONS.append(f"{cpp_file}:{i}: includes from layer {target} > source layer {source}")

if VIOLATIONS:
    for v in VIOLATIONS:
        print(f"LAYER VIOLATION: {v}")
    sys.exit(1)

print("✅ All layer dependencies valid")
