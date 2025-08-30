// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

enum class ReEnumLight : int32_t
{
    R_ZERO = 0,
    R_ONE = 1,
    R_TWO = 2,
};

struct RepeatedScalarsLight
{
    std::vector<int32_t> r_int32_default_packed{};
    std::vector<int32_t> r_sint32_unpacked{};
    std::vector<uint32_t> r_fixed32_packed{};
    std::vector<double> r_double_unpacked{};
    std::vector<ReEnumLight> r_enums_default_packed{};
    std::vector<std::string> r_strings{};
    std::vector<std::string> r_bytes{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<RepeatedScalarsLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.r_int32_default_packed, FieldMeta<1>{"r_int32_default_packed"});
        cb(obj.r_sint32_unpacked, FieldMeta<2>{"r_sint32_unpacked"});
        cb(obj.r_fixed32_packed, FieldMeta<3>{"r_fixed32_packed"});
        cb(obj.r_double_unpacked, FieldMeta<4>{"r_double_unpacked"});
        cb(obj.r_enums_default_packed, FieldMeta<5>{"r_enums_default_packed"});
        cb(obj.r_strings, FieldMeta<6>{"r_strings"});
        cb(obj.r_bytes, FieldMeta<7>{"r_bytes"});
    }
};

