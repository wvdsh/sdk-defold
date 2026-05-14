#!/usr/bin/env python3

from __future__ import annotations

import os
import argparse
import json
import re
import html
from pathlib import Path

# for mustache
import codecs
import pystache


CLASS_PATTERN = re.compile(r"declare\s+(?:abstract\s+)?class\s+([A-Za-z_$][\w$]*)[^{]*\{")
# CONST_PATTERN = re.compile(r"declare\s+const\s+([A-Za-z_$][\w$]*)\s*:\s*\{")
CONST_PATTERN = re.compile(r"declare\s+const\s+([A-Za-z_]*)[\w$]*\s*:\s*\{")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*", "", text)
    return text


def find_matching_brace(text: str, open_index: int) -> int:
    depth = 0
    in_string: str | None = None
    in_line_comment = False
    in_block_comment = False
    escape = False

    for index in range(open_index, len(text)):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""

        if in_line_comment:
            if char == "\n":
                in_line_comment = False
            continue

        if in_block_comment:
            if char == "*" and nxt == "/":
                in_block_comment = False
            continue

        if in_string:
            if escape:
                escape = False
            elif char == "\\":
                escape = True
            elif char == in_string:
                in_string = None
            continue

        if char in ("'", '"', "`"):
            in_string = char
            continue

        if char == "/" and nxt == "/":
            in_line_comment = True
            continue

        if char == "/" and nxt == "*":
            in_block_comment = True
            continue

        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index

    raise ValueError(f"Unmatched brace starting at offset {open_index}")


def split_top_level_statements(block: str) -> list[str]:
    statements: list[str] = []
    current: list[str] = []
    brace_depth = 0
    paren_depth = 0
    bracket_depth = 0
    angle_depth = 0
    in_string: str | None = None
    in_line_comment = False
    in_block_comment = False
    escape = False

    index = 0
    while index < len(block):
        char = block[index]
        nxt = block[index + 1] if index + 1 < len(block) else ""
        current.append(char)

        if in_line_comment:
            if char == "\n":
                in_line_comment = False
            index += 1
            continue

        if in_block_comment:
            if char == "*" and nxt == "/":
                current.append(nxt)
                in_block_comment = False
                index += 2
                continue
            index += 1
            continue

        if in_string:
            if escape:
                escape = False
            elif char == "\\":
                escape = True
            elif char == in_string:
                in_string = None
            index += 1
            continue

        if char in ("'", '"', "`"):
            in_string = char
            index += 1
            continue

        if char == "/" and nxt == "/":
            in_line_comment = True
            index += 1
            continue

        if char == "/" and nxt == "*":
            in_block_comment = True
            index += 1
            continue

        if char == "{":
            brace_depth += 1
        elif char == "}":
            brace_depth -= 1
        elif char == "(":
            paren_depth += 1
        elif char == ")":
            paren_depth -= 1
        elif char == "[":
            bracket_depth += 1
        elif char == "]":
            bracket_depth -= 1
        elif char == "<":
            angle_depth += 1
        elif char == ">":
            if angle_depth > 0:
                angle_depth -= 1
        elif (
            char == ";"
            and brace_depth == 0
            and paren_depth == 0
            and bracket_depth == 0
            and angle_depth == 0
        ):
            statement = "".join(current).strip()
            if statement:
                statements.append(statement)
            current = []
        index += 1

    tail = "".join(current).strip()
    if tail:
        statements.append(tail)

    return statements


def split_top_level_commas(text: str) -> list[str]:
    parts: list[str] = []
    current: list[str] = []
    brace_depth = 0
    paren_depth = 0
    bracket_depth = 0
    angle_depth = 0
    in_string: str | None = None
    escape = False

    for char in text:
        if in_string:
            current.append(char)
            if escape:
                escape = False
            elif char == "\\":
                escape = True
            elif char == in_string:
                in_string = None
            continue

        if char in ("'", '"', "`"):
            in_string = char
            current.append(char)
            continue

        if char == "{":
            brace_depth += 1
        elif char == "}":
            brace_depth -= 1
        elif char == "(":
            paren_depth += 1
        elif char == ")":
            paren_depth -= 1
        elif char == "[":
            bracket_depth += 1
        elif char == "]":
            bracket_depth -= 1
        elif char == "<":
            angle_depth += 1
        elif char == ">":
            if angle_depth > 0:
                angle_depth -= 1

        if (
            char == ","
            and brace_depth == 0
            and paren_depth == 0
            and bracket_depth == 0
            and angle_depth == 0
        ):
            part = "".join(current).strip()
            if part:
                parts.append(part)
            current = []
            continue

        current.append(char)

    tail = "".join(current).strip()
    if tail:
        parts.append(tail)

    return parts


def parse_arguments(parameters_text: str) -> list[dict[str, object]]:
    if not parameters_text.strip():
        return []

    arguments: list[dict[str, object]] = []
    for raw_param in split_top_level_commas(parameters_text):
        param = " ".join(raw_param.split())
        if not param:
            continue

        rest = param.startswith("...")
        if rest:
            param = param[3:].strip()

        initializer = None
        if "=" in param:
            left, right = param.split("=", 1)
            param = left.strip()
            initializer = right.strip()

        if ":" in param:
            left, param_type = param.split(":", 1)
            param_type = param_type.strip()
        else:
            left, param_type = param, None

        left = left.strip()
        optional = left.endswith("?")
        if optional:
            left = left[:-1].strip()

        if param_type.startswith("GenericId"):
            param_type = "string"

        if param_type.startswith("Array"):
            param_type = "array"

        arguments.append(
            {
                "argument_name": left,
                "type": param_type,
                "optional": optional,
                "rest": rest,
                "default": initializer,
            }
        )

    if len(arguments) > 0:
        arguments[len(arguments) - 1]["last"] = True

    return arguments


def parse_method(statement: str) -> dict[str, object] | None:
    compact = " ".join(statement.split())
    if not compact or "private " in compact:
        return None
    if compact.startswith("private "):
        return None
    if compact.startswith("protected ") and "(" not in compact:
        return None
    if "(" not in compact or ")" not in compact:
        return None

    method_match = re.match(
        r"^(?:public\s+|protected\s+|static\s+|readonly\s+|abstract\s+|declare\s+)*([A-Za-z_$][\w$]*)\s*(?:<.*?>)?\s*\((.*)\)\s*(?::\s*(.+))?;$",
        compact,
    )
    if not method_match:
        return None

    name = method_match.group(1)
    arguments = parse_arguments(method_match.group(2))
    return_type = method_match.group(3).strip() if method_match.group(3) else None

    return {
        "method_name": name,
        "arguments": arguments,
        "returnType": return_type,
        "signature": compact,
    }


def parse_class_body(body: str) -> list[dict[str, str]]:
    methods: list[dict[str, str]] = []
    clean_body = strip_comments(body)

    for statement in split_top_level_statements(clean_body):
        print("STATEMENT", statement)
        method = parse_method(statement)
        if method is not None:
            methods.append(method)

    return methods


def parse_const_body(body: str) -> list[dict[str, str]]:
    fields: list[dict[str, str]] = []
    clean_body = strip_comments(body)

    for statement in split_top_level_statements(clean_body):
        compact = " ".join(statement.split())
        match = re.match(r"^readonly\s+([A-Za-z_$][\w$]*)\s*:\s*(.+);$", compact)
        if not match:
            continue
        fields.append(
            {
                "field_name": match.group(1),
                "field_type": match.group(2),
            }
        )

    return fields


def extract_classes(text: str) -> list[dict[str, object]]:
    classes: list[dict[str, object]] = []

    for match in CLASS_PATTERN.finditer(text):
        name = match.group(1)
        open_brace = match.end() - 1
        close_brace = find_matching_brace(text, open_brace)
        body = text[open_brace + 1 : close_brace]
        classes.append(
            {
                "class_name": name,
                "methods": parse_class_body(body),
            }
        )

    return classes


def extract_consts(text: str) -> list[dict[str, object]]:
    consts: list[dict[str, object]] = []

    for match in CONST_PATTERN.finditer(text):
        name = match.group(1)
        open_brace = match.end() - 1
        close_brace = find_matching_brace(text, open_brace)
        body = text[open_brace + 1 : close_brace]
        consts.append(
            {
                "name": name,
                "readonlyFields": parse_const_body(body),
            }
        )

    return consts


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Parse a TypeScript declaration file into a JSON summary.",
    )
    parser.add_argument("input", type=Path, help="Path to the input .d.ts file")
    parser.add_argument("output", type=Path, help="Path to the output JSON file")
    return parser.parse_args()


def render(data, templatefile, outfile) -> None:
    with open(templatefile, 'r') as f:
        template = f.read()
        result = pystache.render(template, data)
        with codecs.open(outfile, "w") as f:
            # f.write(result)
            f.write(html.unescape(result))

def main() -> None:
    args = parse_args()
    input_path = args.input.resolve()
    output_path = args.output.resolve()
    script_path = os.path.dirname(os.path.realpath(__file__))

    print(f"Parsing {input_path}")

    # read the input file, parse it
    text = input_path.read_text(encoding="utf-8")
    payload = {
        "source": str(input_path),
        "classes": extract_classes(text),
        "consts": extract_consts(text),
    }

    # write data into index.json (for debugging)
    # with codecs.open(os.path.join(script_path, "index.json"), "wb", encoding="utf-8") as f:
    #     json.dump(payload, f, indent=2)
    with codecs.open(os.path.join(script_path, "index.json"), "w") as f:
        json.dump(payload, f, indent=2)
    
    # render wavedash.lua from extracted data
    render(payload, os.path.join(script_path, "wavedash_lua.mtl"), output_path)

    print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
