#include <cstdio>
#include "yamlvalue.h"

// -------------------------------------------------------------------------
// Indented debug dump of a YAML value tree
// -------------------------------------------------------------------------
void YAMLValue::dump(int indent) const
{
    auto pad = [&]() { for (int k = 0; k < indent * 2; ++k) fputc(' ', stdout); };

    switch (type)
    {
    case YAMLType::Null:
        pad(); printf("null\n"); break;

    case YAMLType::Bool:
        pad(); printf("%s\n", b ? "true" : "false"); break;

    case YAMLType::Int:
        pad(); printf("%d\n", i); break;

    case YAMLType::Float:
        pad(); printf("%g\n", f); break;

    case YAMLType::String:
        pad(); printf("\"%s\"\n", s.c_str()); break;

    case YAMLType::Sequence:
        for (const auto &item : sequence)
        {
            pad(); printf("- \n");
            item->dump(indent + 1);
        }
        break;

    case YAMLType::Mapping:
        for (const auto &kv : mapping)
        {
            pad(); printf("%s:\n", kv.first.c_str());
            kv.second->dump(indent + 1);
        }
        break;
    }
}
