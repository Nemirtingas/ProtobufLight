// Auto-generated from .proto
#pragma once

#include <ProtobufLight/ProtobufLightReflection.hpp>

struct MapsScalarsLight
{
    std::map<std::string, int32_t> m_str_i32{};
    std::map<int32_t, std::string> m_i32_str{};
    std::map<int64_t, uint64_t> m_i64_u64{};
    std::map<uint32_t, int32_t> m_u32_s32{};

    size_t GetByteSize() const { return ProtobufLight::Reflection::SerializedStructSize(*this); }
    bool ParseFromArray(const uint8_t* buffer, size_t size) { return ProtobufLight::Reflection::ParseStruct(*this, buffer, size); }
    std::string SerializeAsString() const { std::string out; ProtobufLight::Reflection::SerializeStruct(*this, out); return out; }
};

template<>
struct ProtobufLight::Reflection::ProtobufTrait<MapsScalarsLight>
{
    template<typename Obj, typename Callback>
    static void ForEachField(Obj& obj, Callback&& cb) {
        cb(obj.m_str_i32, FieldMeta<1>{"m_str_i32"});
        cb(obj.m_i32_str, FieldMeta<2>{"m_i32_str"});
        cb(obj.m_i64_u64, FieldMeta<3>{"m_i64_u64"});
        cb(obj.m_u32_s32, FieldMeta<4>{"m_u32_s32"});
    }
};

