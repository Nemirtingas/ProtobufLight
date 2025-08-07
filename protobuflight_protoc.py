#!/usr/bin/env python3
import re
import sys
from pathlib import Path
from collections import OrderedDict

# ---- Config / mapping .proto -> C++ ----------------------------------------
PROTO_TO_CPP = {
    "double": "double",
    "float": "float",
    "int32": "int32_t",
    "int64": "int64_t",
    "uint32": "uint32_t",
    "uint64": "uint64_t",
    "sint32": "int32_t",
    "sint64": "int64_t",
    "fixed32": "uint32_t",
    "fixed64": "uint64_t",
    "sfixed32": "int32_t",
    "sfixed64": "int64_t",
    "bool": "bool",
    "string": "std::string",
    "bytes": "std::string",  # could be specialized
}

SCALAR_PROTOS = {
    "double", "float", "int32", "int64", "uint32", "uint64",
    "sint32", "sint64", "fixed32", "fixed64", "sfixed32", "sfixed64",
    "bool"
}

# ---- regex patterns --------------------------------------------------------
COMMENT_RE = re.compile(r'//.*?$|/\*.*?\*/', re.DOTALL | re.MULTILINE)
IMPORT_RE = re.compile(r'import\s+"([^"]+)";')
PACKAGE_RE = re.compile(r'package\s+([\w\.]+);')
MESSAGE_START_RE = re.compile(r'\bmessage\s+(\w+)\s*\{')
ONEOF_START_RE = re.compile(r'\boneof\s+(\w+)\s*\{')
FIELD_RE = re.compile(r'''
    (?P<label>optional|required|repeated)?\s*
    (?:
        map<\s*(?P<map_key>\w+)\s*,\s*(?P<map_value>\w+)\s*>|
        (?P<type>\w+)
    )\s+
    (?P<name>\w+)\s*=\s*(?P<number>\d+)
''', re.VERBOSE)
CLOSE_BRACE_RE = re.compile(r'\}')

# ---- AST classes -----------------------------------------------------------
class Field:
    def __init__(self, name, proto_type=None, number=None, label=None, map_key=None, map_value=None):
        self.name = name
        self.proto_type = proto_type  # for map, proto_type is None
        self.number = int(number) if number is not None else None
        self.label = label  # repeated / optional / required
        self.map_key = map_key
        self.map_value = map_value

    def cpp_type(self):
        if self.map_key and self.map_value:
            key = PROTO_TO_CPP.get(self.map_key, self.map_key)
            val = PROTO_TO_CPP.get(self.map_value, self.map_value)
            return f"std::map<{key}, {val}>"
        base = None
        if self.proto_type in PROTO_TO_CPP:
            base = PROTO_TO_CPP[self.proto_type]
        else:
            base = self.proto_type  # message or user-defined

        if self.label == "repeated":
            return f"std::vector<{base}>"
        if self.label == "optional":
            return f"std::optional<{base}>"
        return base

    def member_decl(self):
        return f"{self.cpp_type()} {self.name};"

class Oneof:
    def __init__(self, name):
        self.name = name
        self.fields = []  # Field list

    def variant_type_list(self):
        types = []
        for f in self.fields:
            if f.map_key and f.map_value:
                key = PROTO_TO_CPP.get(f.map_key, f.map_key)
                val = PROTO_TO_CPP.get(f.map_value, f.map_value)
                types.append(f"std::map<{key}, {val}>")
            else:
                if f.proto_type in PROTO_TO_CPP:
                    base = PROTO_TO_CPP[f.proto_type]
                else:
                    base = f.proto_type
                if f.label == "repeated":
                    types.append(f"std::vector<{base}>")
                else:
                    types.append(base)
        return types

class Message:
    def __init__(self, name):
        self.name = name
        self.fields = []      # regular fields (outside oneof)
        self.oneofs = []      # Oneof objects
        self.nested = []      # nested Message
        self.package = None

    def cpp_struct(self):
        lines = []
        lines.append(f"struct {self.name}")
        lines.append("{")
        for f in self.fields:
            lines.append(f"    {f.member_decl()}")
        for oneof in self.oneofs:
            variant_types = oneof.variant_type_list()
            if variant_types:
                joined = ", ".join(variant_types)
                lines.append(f"    std::variant<std::monostate, {joined}> {oneof.name};")
        lines.append("")
        lines.append("    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }")
        lines.append("    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }")
        lines.append("    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }")
        
        lines.append("};")
        return "\n".join(lines)

    def trait_specialization(self):
        lines = []
        lines.append("template<>")
        lines.append(f"struct ProtobufLight::Reflection::ProtobufTrait<{self.name}>")
        lines.append("{")
        lines.append("    template<typename Obj, typename Callback>")
        lines.append("    static void ForEachField(Obj& obj, Callback&& cb) {")
        for f in self.fields:
            if f.number is None:
                continue
            lines.append(f'        cb(obj.{f.name}, FieldMeta<{f.number}>{{"{f.name}"}});')
        for oneof in self.oneofs:
            if not oneof.fields:
                continue
            nums = ",".join(str(f.number) for f in oneof.fields)
            lines.append(f'        cb(obj.{oneof.name}, FieldMeta<{nums}>{{"{oneof.name}"}});')
        lines.append("    }")
        lines.append("};")
        return "\n".join(lines)

# ---- Parsing logic ---------------------------------------------------------
def parse_proto_file(path: Path, visited=None):
    if visited is None:
        visited = set()
    path = path.resolve()
    if path in visited:
        return OrderedDict()
    visited.add(path)
    text = path.read_text(encoding="utf-8")
    # strip comments
    text = COMMENT_RE.sub("", text)

    # handle imports recursively
    imports = IMPORT_RE.findall(text)
    messages = OrderedDict()

    package_match = PACKAGE_RE.search(text)
    pkg = package_match.group(1) if package_match else None

    # naive tokenization by braces to track nesting
    idx = 0
    length = len(text)
    stack = []  # stack of Message or Oneof context
    current_oneof = None

    # iterate line-wise for simplicity
    lines = text.splitlines()
    for line in lines:
        # detect message start
        mmsg = MESSAGE_START_RE.search(line)
        if mmsg:
            name = mmsg.group(1)
            msg = Message(name)
            msg.package = pkg
            if stack and isinstance(stack[-1], Message):
                stack[-1].nested.append(msg)
            else:
                messages[name] = msg
            stack.append(msg)
            current_oneof = None
            continue

        # detect oneof start
        o = ONEOF_START_RE.search(line)
        if o and stack and isinstance(stack[-1], Message):
            oneof = Oneof(o.group(1))
            stack[-1].oneofs.append(oneof)
            stack.append(oneof)
            current_oneof = oneof
            continue

        # detect closing brace
        if "}" in line:
            if stack:
                stack.pop()
                if stack and isinstance(stack[-1], Oneof):
                    current_oneof = stack[-1]
                else:
                    current_oneof = None
            continue

        # field detection in current context
        if stack:
            context = stack[-1]
            for fm in FIELD_RE.finditer(line):
                label = fm.group("label")
                map_key = fm.group("map_key")
                map_value = fm.group("map_value")
                typ = fm.group("type")
                name = fm.group("name")
                number = fm.group("number")

                if isinstance(context, Oneof):
                    if map_key and map_value:
                        fld = Field(name=name, proto_type=None, number=number, label=None,
                                    map_key=map_key, map_value=map_value)
                    else:
                        fld = Field(name=name, proto_type=typ, number=number, label=label)
                    context.fields.append(fld)
                elif isinstance(context, Message):
                    if map_key and map_value:
                        fld = Field(name=name, proto_type=None, number=number, label=None,
                                    map_key=map_key, map_value=map_value)
                    else:
                        fld = Field(name=name, proto_type=typ, number=number, label=label)
                    context.fields.append(fld)
    # recursively merge imported files
    for imp in imports:
        imported_path = (path.parent / imp).resolve()
        if imported_path.exists():
            sub = parse_proto_file(imported_path, visited)
            for k, v in sub.items():
                if k not in messages:
                    messages[k] = v

    return messages

# ---- Code generation -------------------------------------------------------
def generate_header(messages: dict, out_path: Path):
    with out_path.open("w", encoding="utf-8") as f:
        f.write("// Auto-generated from .proto\n")
        f.write("#pragma once\n\n")
        f.write("#include <ProtobufLight/ProtobufLightReflection.hpp>\n\n")
        for name, msg in messages.items():
            f.write(msg.cpp_struct() + "\n\n")
        for name, msg in messages.items():
            f.write(msg.trait_specialization() + "\n\n")

def main():
    if len(sys.argv) != 3:
        print("Usage: protobuflight_protoc.py <input.proto> <output.hpp>")
        sys.exit(1)
    proto_file = Path(sys.argv[1])
    out_file = Path(sys.argv[2])
    if not proto_file.exists():
        print(f"Input file not found: {proto_file}")
        sys.exit(1)

    messages = parse_proto_file(proto_file)
    generate_header(messages, out_file)
    print(f"Generated {out_file} with messages: {', '.join(messages.keys())}")

if __name__ == "__main__":
    main()