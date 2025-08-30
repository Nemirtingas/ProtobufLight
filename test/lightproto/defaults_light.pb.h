// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct DefaultsLight
{
    int32_t i32{};
    bool b{};
    std::string s{};
    std::string by{};
    double d{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<DefaultsLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.i32, FieldMeta<1>{"i32"});
        cb(obj.b, FieldMeta<2>{"b"});
        cb(obj.s, FieldMeta<3>{"s"});
        cb(obj.by, FieldMeta<4>{"by"});
        cb(obj.d, FieldMeta<5>{"d"});
    }
};

