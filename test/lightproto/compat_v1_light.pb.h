// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct CompatV1Light
{
    int32_t a{};
    std::string b{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<CompatV1Light>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.a, FieldMeta<1>{"a"});
        cb(obj.b, FieldMeta<2>{"b"});
    }
};

