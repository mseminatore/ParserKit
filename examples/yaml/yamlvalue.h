#pragma once

#include <string>
#include <vector>
#include <memory>

// -------------------------------------------------------------------------
// YAMLType — discriminator for the value variant
// -------------------------------------------------------------------------
enum class YAMLType
{
    Null,
    Bool,
    Int,
    Float,
    String,
    Sequence,
    Mapping
};

// -------------------------------------------------------------------------
// YAMLValue — a single YAML node
//
// Use the factory helpers or set 'type' and the matching field directly.
// Sequence and Mapping store their children as owned unique_ptr<YAMLValue>.
// Mapping preserves insertion order (std::vector of pairs).
// -------------------------------------------------------------------------
struct YAMLValue
{
    YAMLType type = YAMLType::Null;

    // Scalar payloads (only one is meaningful at a time, per 'type')
    bool        b = false;
    int         i = 0;
    float       f = 0.0f;
    std::string s;

    // Collection payloads
    std::vector<std::unique_ptr<YAMLValue>>                          sequence;
    std::vector<std::pair<std::string, std::unique_ptr<YAMLValue>>>  mapping;

    // Non-copyable (owns children)
    YAMLValue() = default;
    YAMLValue(YAMLValue &&) = default;
    YAMLValue &operator=(YAMLValue &&) = default;
    YAMLValue(const YAMLValue &) = delete;
    YAMLValue &operator=(const YAMLValue &) = delete;

    // Convenience factories
    static YAMLValue makeNull()               { YAMLValue v; v.type = YAMLType::Null;     return v; }
    static YAMLValue makeBool(bool b)         { YAMLValue v; v.type = YAMLType::Bool;     v.b = b; return v; }
    static YAMLValue makeInt(int i)           { YAMLValue v; v.type = YAMLType::Int;      v.i = i; return v; }
    static YAMLValue makeFloat(float f)       { YAMLValue v; v.type = YAMLType::Float;    v.f = f; return v; }
    static YAMLValue makeString(std::string s){ YAMLValue v; v.type = YAMLType::String;   v.s = std::move(s); return v; }

    // Debug output
    void dump(int indent = 0) const;
};
