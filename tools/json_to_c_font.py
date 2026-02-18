#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
import unicodedata


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert JSON font map into C designated initializers"
    )
    parser.add_argument("input", type=Path, help="Path to source JSON")
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Write result to file (default: stdout)",
    )
    parser.add_argument(
        "--height",
        type=int,
        default=12,
        help="Number of rows per glyph in generated C array",
    )
    parser.add_argument(
        "--trim-top",
        type=int,
        default=None,
        help="How many rows to trim from top (default: auto-center trim)",
    )
    parser.add_argument(
        "--index-width",
        type=int,
        default=2,
        help="Minimum hex digits for index: [0x00], [0x0021], ...",
    )
    parser.add_argument(
        "--start-codepoint",
        type=int,
        default=0,
        help="Skip glyphs below this codepoint",
    )
    return parser.parse_args()


def format_value(value: int) -> str:
    if value < 0:
        raise ValueError(f"Negative glyph row value is not supported: {value}")
    if value <= 0xFF:
        return f"0x{value:02X}"
    return f"0x{value:X}"


def comment_for_codepoint(codepoint: int) -> str:
    if codepoint == 0x20:
        return "space"

    ch = chr(codepoint)
    category = unicodedata.category(ch)

    # Control, format, surrogate, private-use, unassigned.
    if category.startswith("C"):
        return f"U+{codepoint:04X}"

    if ch == "\\":
        return "\\\\"

    if ch.isspace() and ch != " ":
        return f"U+{codepoint:04X}"

    return ch


def trim_rows(rows: list[int], height: int, trim_top: int | None) -> list[int]:
    if len(rows) == height:
        return rows

    if len(rows) < height:
        return rows + [0] * (height - len(rows))

    # len(rows) > height
    if trim_top is None:
        diff = len(rows) - height
        trim_top = diff // 2
    trim_bottom = len(rows) - height - trim_top

    if trim_top < 0 or trim_bottom < 0:
        raise ValueError(
            f"Invalid trim values for len={len(rows)}, height={height}, trim_top={trim_top}"
        )

    return rows[trim_top : len(rows) - trim_bottom]


def render_glyph(codepoint: int, rows: list[int], index_width: int, height: int) -> str:
    index_digits = max(index_width, len(f"{codepoint:X}"))
    values = ", ".join(format_value(v) for v in rows)
    comment = comment_for_codepoint(codepoint)
    return f"    [0x{codepoint:0{index_digits}X}] = {{ {values} }},    // {comment}"


def main() -> None:
    args = parse_args()
    data = json.loads(args.input.read_text(encoding="utf-8"))

    if not isinstance(data, dict):
        raise ValueError("JSON root must be an object: {\"33\": [...], ...}")

    glyphs: list[tuple[int, list[int]]] = []
    for key, value in data.items():
        codepoint = int(key)
        if codepoint < args.start_codepoint:
            continue
        if not isinstance(value, list) or not all(isinstance(v, int) for v in value):
            raise ValueError(f"Glyph {key} must be an array of integers")
        glyph_rows = trim_rows(value, args.height, args.trim_top)
        glyphs.append((codepoint, glyph_rows))

    glyphs.sort(key=lambda x: x[0])

    lines = [
        f"// Generated from {args.input.name}",
        f"// Glyph rows: {args.height}",
    ]
    lines.extend(
        render_glyph(codepoint, rows, args.index_width, args.height)
        for codepoint, rows in glyphs
    )

    output_text = "\n".join(lines) + "\n"

    if args.output:
        args.output.write_text(output_text, encoding="utf-8")
    else:
        print(output_text, end="")


if __name__ == "__main__":
    main()
