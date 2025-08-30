// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

enum class TestEnumLight : int32_t
{
    ENUM_ZERO = 0,
    ENUM_ONE = 1,
    ENUM_TWO = 2,
};

struct ScalarsLight
{
    int32_t f_int32{};
    int64_t f_int64{};
    uint32_t f_uint32{};
    uint64_t f_uint64{};
    int32_t f_sint32{};
    int64_t f_sint64{};
    uint32_t f_fixed32{};
    uint64_t f_fixed64{};
    int32_t f_sfixed32{};
    int64_t f_sfixed64{};
    bool f_bool{};
    float f_float{};
    double f_double{};
    std::string f_string{};
    std::string f_bytes{};
    TestEnumLight f_enum{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<ScalarsLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.f_int32, FieldMeta<1>{"f_int32"});
        cb(obj.f_int64, FieldMeta<2>{"f_int64"});
        cb(obj.f_uint32, FieldMeta<3>{"f_uint32"});
        cb(obj.f_uint64, FieldMeta<4>{"f_uint64"});
        cb(obj.f_sint32, FieldMeta<5>{"f_sint32"});
        cb(obj.f_sint64, FieldMeta<6>{"f_sint64"});
        cb(obj.f_fixed32, FieldMeta<7>{"f_fixed32"});
        cb(obj.f_fixed64, FieldMeta<8>{"f_fixed64"});
        cb(obj.f_sfixed32, FieldMeta<9>{"f_sfixed32"});
        cb(obj.f_sfixed64, FieldMeta<10>{"f_sfixed64"});
        cb(obj.f_bool, FieldMeta<11>{"f_bool"});
        cb(obj.f_float, FieldMeta<12>{"f_float"});
        cb(obj.f_double, FieldMeta<13>{"f_double"});
        cb(obj.f_string, FieldMeta<14>{"f_string"});
        cb(obj.f_bytes, FieldMeta<15>{"f_bytes"});
        cb(obj.f_enum, FieldMeta<16>{"f_enum"});
    }
};

