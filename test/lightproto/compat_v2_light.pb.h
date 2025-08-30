// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct CompatV2Light
{
    int32_t a{};
    std::string b{};
    int64_t c{};
    std::string d{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<CompatV2Light>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.a, FieldMeta<1>{"a"});
        cb(obj.b, FieldMeta<2>{"b"});
        cb(obj.c, FieldMeta<1001>{"c"});
        cb(obj.d, FieldMeta<1002>{"d"});
    }
};

