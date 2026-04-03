#pragma once

#include "../../baseparser.h"
#include "yamlvalue.h"
#include "yamllexer.h"

// -------------------------------------------------------------------------
// YAMLParser — recursive-descent YAML parser built on BaseParser
//
// Supports:
//   • Block mappings   (key: value, indentation-based)
//   • Block sequences  (- item, indentation-based)
//   • Flow mappings    { k: v, ... }
//   • Flow sequences   [ v, v, ... ]
//   • Scalars: quoted strings, bare words, integers, floats,
//              booleans (true/false/yes/no/on/off), and null (~)
// -------------------------------------------------------------------------
class YAMLParser : public BaseParser
{
public:
    YAMLParser();
    virtual ~YAMLParser() = default;

    int yyparse() override;

    // Grammar rule methods — each fills the supplied YAMLValue node
    void DoDocument (YAMLValue &node);
    void DoBlockMapping (YAMLValue &node);
    void DoBlockSequence(YAMLValue &node);
    void DoBlockValue  (YAMLValue &node);   // inline scalar or nested block
    void DoFlowMapping (YAMLValue &node);
    void DoFlowSequence(YAMLValue &node);
    void DoFlowValue   (YAMLValue &node);   // any inline scalar or flow collection
};
