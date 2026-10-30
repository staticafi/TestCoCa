#!/usr/bin/env python3
"""Convert Test-Comp XML testcases to TestCoCa's JSON format."""
import argparse
import json
import math
import sys
import zipfile
from pathlib import Path, PurePosixPath
from xml.etree import ElementTree


def parse_value(text):
    text = (text or "").strip()
    if not text:
        return 0
    if len(text) == 3 and text[0] == text[2] and text[0] in "'\"":
        text = str(ord(text[1]))
    elif text.startswith("0x"):
        try:
            text = str(int(text, 16))
        except ValueError:
            return 0

    try:
        return int(text, 10)
    except ValueError:
        pass

    try:
        value = float(text)
    except ValueError:
        return 0

    # Keep Inf/Nan as text so the JSON parser uses the same float parser.
    return value if math.isfinite(value) else text


def convert(data):
    root = ElementTree.fromstring(data)
    if root.tag != "testcase":
        return {"test-case": {"nondet": {"value": []}}}

    values = []
    for node in root:
        if node.tag != "input":
            continue
        item = {"value": parse_value(node.text)}
        type_name = (node.get("type") or "").strip()
        if type_name:
            item["type"] = type_name
        values.append(item)
    return {"test-case": {"nondet": {"value": values}}}


def load_sources(path):
    if path.is_dir():
        return [
            (p.stem, p.read_bytes())
            for p in path.rglob("*.xml")
            if p.name != "metadata.xml"
        ]

    if zipfile.is_zipfile(path):
        with zipfile.ZipFile(path) as archive:
            names = [
                name for name in archive.namelist()
                if name.endswith(".xml")
                and PurePosixPath(name).name != "metadata.xml"
            ]
            return [
                (PurePosixPath(name).stem, archive.read(name))
                for name in names
            ]

    return [(path.stem, path.read_bytes())]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("-o", "--output-dir")
    parser.add_argument("--zip", action="store_true")
    args = parser.parse_args()

    source = Path(args.input).resolve()
    output = (
        Path(args.output_dir).resolve()
        if args.output_dir
        else source.parent
        / f"{source.stem if source.is_file() else source.name}_converted"
    )

    cases = [
        (name + ".json", convert(data))
        for name, data in load_sources(source)
    ]
    if not cases:
        raise SystemExit(f"no XML tests found in {source}")

    if args.zip:
        with zipfile.ZipFile(f"{output}.zip", "w", zipfile.ZIP_DEFLATED) as archive:
            for name, test in cases:
                archive.writestr(
                    name,
                    json.dumps(test, indent=4, allow_nan=False) + "\n",
                )
    else:
        output.mkdir(parents=True, exist_ok=True)
        for old in output.glob("testcase-*.json"):
            old.unlink()
        for name, test in cases:
            (output / name).write_text(
                json.dumps(test, indent=4, allow_nan=False) + "\n"
            )


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        sys.exit(f"tc2json: {error}")
