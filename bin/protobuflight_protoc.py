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
    "bytes": "std::string",
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
ENUM_START_RE = re.compile(r'\benum\s+(\w+)\s*\{')
FIELD_RE = re.compile(r'''
    (?P<label>optional|required|repeated)?\s*
    (?:
        map<\s*(?P<map_key>\w+)\s*,\s*(?P<map_value>\w+)\s*>|
        (?P<type>\w+)
    )\s+
    (?P<name>\w+)\s*=\s*(?P<number>\d+)
''', re.VERBOSE)
ENUM_FIELD_RE = re.compile(r'(?P<name>\w+)\s*=\s*(?P<num>\d+)\s*;?')

# ---- AST classes -----------------------------------------------------------
class Field:
    def __init__(self, name, proto_type=None, number=None, label=None, map_key=None, map_value=None):
        self.name = name
        self.proto_type = proto_type
        self.number = int(number) if number is not None else None
        self.label = label
        self.map_key = map_key
        self.map_value = map_value

    def cpp_type(self):
        if self.map_key and self.map_value:
            key = PROTO_TO_CPP.get(self.map_key, self.map_key)
            val = PROTO_TO_CPP.get(self.map_value, self.map_value)
            return f"std::map<{key}, {val}>"
        base = PROTO_TO_CPP.get(self.proto_type, self.proto_type)
        if self.label == "repeated":
            return f"std::vector<{base}>"
        if self.label == "optional":
            return f"std::optional<{base}>"
        return base

    def member_decl(self):
        return f"{self.cpp_type()} {self.name}{{}};"

class Oneof:
    def __init__(self, name):
        self.name = name
        self.fields = []

    def variant_type_list(self):
        types = []
        for f in self.fields:
            if f.map_key and f.map_value:
                key = PROTO_TO_CPP.get(f.map_key, f.map_key)
                val = PROTO_TO_CPP.get(f.map_value, f.map_value)
                types.append(f"std::map<{key}, {val}>")
            else:
                base = PROTO_TO_CPP.get(f.proto_type, f.proto_type)
                if f.label == "repeated":
                    types.append(f"std::vector<{base}>")
                else:
                    types.append(base)
        return types

class Enum:
    def __init__(self, name):
        self.name = name
        self.values = []  # list of (name, number)

    def cpp_enum(self, indent=0):
        sp = " " * indent
        lines = []
        lines.append(f"{sp}enum class {self.name} : int32_t")
        lines.append(f"{sp}" + "{")
        for val_name, val_num in self.values:
            lines.append(f"{sp}    {val_name} = {val_num},")
        lines.append(f"{sp}" + "};")
        return "\n".join(lines)

class Message:
    def __init__(self, name):
        self.name = name
        self.fields = []
        self.oneofs = []
        self.nested = []      # can contain Enum and Message
        self.package = None

# ---- Parsing logic ---------------------------------------------------------
def parse_proto_file(path: Path, visited=None):
    if visited is None:
        visited = set()
    path = path.resolve()
    if path in visited:
        return OrderedDict(), OrderedDict(), []
    visited.add(path)
    text = path.read_text(encoding="utf-8")
    text = COMMENT_RE.sub("", text)

    imports = IMPORT_RE.findall(text)
    messages = OrderedDict()
    enums = OrderedDict()

    package_match = PACKAGE_RE.search(text)
    pkg = package_match.group(1) if package_match else None

    stack = []
    current_oneof = None
    lines = text.splitlines()
    for raw_line in lines:
        line = raw_line.strip()
        if not line:
            continue

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

        menum = ENUM_START_RE.search(line)
        if menum:
            name = menum.group(1)
            en = Enum(name)
            if stack and isinstance(stack[-1], Message):
                stack[-1].nested.append(en)
            else:
                enums[name] = en
            stack.append(en)
            continue

        o = ONEOF_START_RE.search(line)
        if o and stack and isinstance(stack[-1], Message):
            oneof = Oneof(o.group(1))
            stack[-1].oneofs.append(oneof)
            stack.append(oneof)
            current_oneof = oneof
            continue

        if "}" in line:
            if stack:
                stack.pop()
                if stack and isinstance(stack[-1], Oneof):
                    current_oneof = stack[-1]
                else:
                    current_oneof = None
            continue

        if stack and isinstance(stack[-1], Enum):
            ef = ENUM_FIELD_RE.search(line)
            if ef:
                stack[-1].values.append((ef.group("name"), int(ef.group("num"))))
            continue

        if stack and (isinstance(stack[-1], Message) or isinstance(stack[-1], Oneof)):
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

    # Add top-level enums to PROTO_TO_CPP so references to them are recognized
    for en_name in enums.keys():
        PROTO_TO_CPP.setdefault(en_name, en_name)

    return messages, enums, imports

# ---- Code generation -------------------------------------------------------
def emit_message(msg: Message, f, indent=0):
    sp = " " * indent
    # struct header
    f.write(f"{sp}struct {msg.name}\n{sp}{{\n")

    # nested types
    for n in msg.nested:
        if isinstance(n, Enum):
            f.write(n.cpp_enum(indent=indent+4) + "\n\n")
        elif isinstance(n, Message):
            emit_message(n, f, indent+4)
            f.write("\n")

    # fields
    for fld in msg.fields:
        f.write(f"{sp}    {fld.member_decl()}\n")

    # oneofs
    for oneof in msg.oneofs:
        vt = oneof.variant_type_list()
        if vt:
            joined = ", ".join(vt)
            f.write(f"{sp}    std::variant<std::monostate, {joined}> {oneof.name}{{ std::monostate{{}} }};\n")

    # methods
    f.write(f"""
{sp}    size_t GetByteSize() const {{ return ProtobufLight::Reflection::SerializedStructSize(*this); }}
{sp}    bool ParseFromArray(const uint8_t* buffer, size_t size) {{ return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }}
{sp}    std::string SerializeAsString() const {{ std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }}
{sp}}};
""")

def emit_traits(msg: Message, f, prefix=""):
    full_name = f"{prefix}::{msg.name}" if prefix else msg.name

    f.write("template<>\n")
    f.write(f"struct ProtobufLight::Reflection::ProtobufTrait<{full_name}>\n")
    f.write("{\n")
    f.write("    template<typename Obj, typename Callback>\n")
    f.write("    static void ForEachField(Obj& obj, Callback&& cb) {\n")

    for fld in msg.fields:
        if fld.number is None:
            continue
        f.write(f'        cb(obj.{fld.name}, FieldMeta<{fld.number}>{{"{fld.name}"}});\n')

    for oneof in msg.oneofs:
        if not oneof.fields:
            continue
        nums = ",".join(str(f.number) for f in oneof.fields)
        f.write(f'        cb(obj.{oneof.name}, FieldMeta<{nums}>{{"{oneof.name}"}});\n')

    f.write("    }\n};\n\n")

    # recurse into nested messages
    for n in msg.nested:
        if isinstance(n, Message):
            emit_traits(n, f, prefix=full_name)

def generate_header(messages: dict, enums: dict, imports, out_path: Path):
    with out_path.open("w", encoding="utf-8") as f:
        f.write("// Auto-generated from .proto\n")
        f.write("#pragma once\n\n")
        f.write("#include <ProtobufLight/ProtobufLightReflection.hpp>\n\n")
        if len(imports) > 0:
            for import_ in imports:
                f.write(f'#include <{import_.replace(".proto",".pb.h")}>\n')
            f.write('\n')

        # top-level enums
        for en in enums.values():
            f.write(en.cpp_enum() + "\n\n")

        # messages
        for msg in messages.values():
            emit_message(msg, f)
            f.write("\n")

        for msg in messages.values():
            emit_traits(msg, f)

def main():
    if len(sys.argv) != 3:
        print("Usage: protobuflight_protoc.py <input.proto> <output.hpp>")
        sys.exit(1)
    proto_file = Path(sys.argv[1])
    out_file = Path(sys.argv[2])
    if not proto_file.exists():
        print(f"Input file not found: {proto_file}")
        sys.exit(1)

    messages, enums, imports = parse_proto_file(proto_file)
    generate_header(messages, enums, imports, out_file)
    print(f"Generated {out_file} with messages: {', '.join(messages.keys())} and enums: {', '.join(enums.keys())}")

if __name__ == "__main__":
    main()
