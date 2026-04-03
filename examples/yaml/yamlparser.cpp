//
// YAMLParser — hand-written recursive-descent parser using BaseParser.
// Mirrors the structure of the json/ example.
//
#include "yamlparser.h"

// Token table: keywords are matched case-insensitively inside YAMLLexer,
// so the table here is only a placeholder to satisfy the LexicalAnalyzer
// constructor signature.
static TokenTable _tokenTable[] =
{
    { nullptr, TV_DONE }
};

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
YAMLParser::YAMLParser() : BaseParser(std::make_unique<SymbolTable>())
{
    m_lexer = std::make_unique<YAMLLexer>(_tokenTable, this, &yylval);
}

// -------------------------------------------------------------------------
// yyparse — entry point; primes the lookahead then starts the document rule
// -------------------------------------------------------------------------
int YAMLParser::yyparse()
{
    BaseParser::yyparse(); // primes lookahead

    YAMLValue root;
    DoDocument(root);

    if (yydebug)
        root.dump();

    return 0;
}

// -------------------------------------------------------------------------
// DoDocument
//
// A YAML document is either:
//   • A block mapping or sequence  (starts with KEY or DASH)
//   • A single inline value        (anything else — a bare scalar or flow)
// -------------------------------------------------------------------------
void YAMLParser::DoDocument(YAMLValue &node)
{
    yylog("DoDocument");

    if (lookahead == TV_KEY)
    {
        node.type = YAMLType::Mapping;
        DoBlockMapping(node);
    }
    else if (lookahead == TV_DASH)
    {
        node.type = YAMLType::Sequence;
        DoBlockSequence(node);
    }
    else
    {
        DoFlowValue(node);
        if (lookahead == TV_NEWLINE)
            match(TV_NEWLINE);
    }
}

// -------------------------------------------------------------------------
// DoBlockMapping
//
// Consumes one or more KEY block_value pairs at the current indent level.
// Stops when the next token is not KEY (i.e. DEDENT or end of input).
// -------------------------------------------------------------------------
void YAMLParser::DoBlockMapping(YAMLValue &node)
{
    yylog("DoBlockMapping");

    while (lookahead == TV_KEY)
    {
        std::string key = yylval.sym->lexeme; // save before match advances lookahead
        yylog("  key: %s", key.c_str());
        match(TV_KEY);

        auto val = std::make_unique<YAMLValue>();
        DoBlockValue(*val);
        node.mapping.emplace_back(key, std::move(val));
    }
}

// -------------------------------------------------------------------------
// DoBlockSequence
//
// Consumes one or more DASH items at the current indent level.
// -------------------------------------------------------------------------
void YAMLParser::DoBlockSequence(YAMLValue &node)
{
    yylog("DoBlockSequence");

    while (lookahead == TV_DASH)
    {
        match(TV_DASH);

        auto val = std::make_unique<YAMLValue>();
        DoBlockValue(*val);
        node.sequence.push_back(std::move(val));
    }
}

// -------------------------------------------------------------------------
// DoBlockValue
//
// Reads the value half of a block mapping pair or block sequence item.
// Two forms:
//   scalar NEWLINE               — inline value on same line
//   NEWLINE INDENT block DEDENT  — nested structure on following lines
// -------------------------------------------------------------------------
void YAMLParser::DoBlockValue(YAMLValue &node)
{
    yylog("DoBlockValue");

    if (lookahead == TV_NEWLINE)
    {
        // Nested block structure
        match(TV_NEWLINE);
        match(TV_INDENT);

        if (lookahead == TV_KEY)
        {
            node.type = YAMLType::Mapping;
            DoBlockMapping(node);
        }
        else if (lookahead == TV_DASH)
        {
            node.type = YAMLType::Sequence;
            DoBlockSequence(node);
        }
        else
        {
            yyerror("expected block mapping or sequence after indent");
        }

        match(TV_DEDENT);
    }
    else
    {
        // Inline value
        DoFlowValue(node);
        match(TV_NEWLINE);
    }
}

// -------------------------------------------------------------------------
// DoFlowMapping  { key: value, ... }
// -------------------------------------------------------------------------
void YAMLParser::DoFlowMapping(YAMLValue &node)
{
    yylog("DoFlowMapping");

    match('{');

    while (lookahead != '}' && lookahead != TV_DONE)
    {
        // Flow mapping keys are SCALAR or STRING (the lexer does not emit
        // KEY inside flow context)
        std::string key;
        if (lookahead == TV_SCALAR || lookahead == TV_STRING)
        {
            key = yylval.sym->lexeme;
            match(lookahead);
        }
        else
        {
            yyerror("expected string key in flow mapping");
            match(lookahead);
        }

        match(':');

        auto val = std::make_unique<YAMLValue>();
        DoFlowValue(*val);
        node.mapping.emplace_back(key, std::move(val));

        if (lookahead == ',')
            match(',');
    }

    match('}');
}

// -------------------------------------------------------------------------
// DoFlowSequence  [ value, ... ]
// -------------------------------------------------------------------------
void YAMLParser::DoFlowSequence(YAMLValue &node)
{
    yylog("DoFlowSequence");

    match('[');

    while (lookahead != ']' && lookahead != TV_DONE)
    {
        auto val = std::make_unique<YAMLValue>();
        DoFlowValue(*val);
        node.sequence.push_back(std::move(val));

        if (lookahead == ',')
            match(',');
    }

    match(']');
}

// -------------------------------------------------------------------------
// DoFlowValue — any inline scalar or flow collection
// -------------------------------------------------------------------------
void YAMLParser::DoFlowValue(YAMLValue &node)
{
    yylog("DoFlowValue (lookahead=%d)", lookahead);

    switch (lookahead)
    {
    case TV_STRING:
    case TV_SCALAR:
        node = YAMLValue::makeString(yylval.sym->lexeme);
        match(lookahead);
        break;

    case TV_YAML_INT:
        node = YAMLValue::makeInt(yylval.ival);
        match(TV_YAML_INT);
        break;

    case TV_YAML_FLOAT:
        node = YAMLValue::makeFloat(yylval.fval);
        match(TV_YAML_FLOAT);
        break;

    case TV_YAML_TRUE:
        node = YAMLValue::makeBool(true);
        match(TV_YAML_TRUE);
        break;

    case TV_YAML_FALSE:
        node = YAMLValue::makeBool(false);
        match(TV_YAML_FALSE);
        break;

    case TV_YAML_NULL:
        node = YAMLValue::makeNull();
        match(TV_YAML_NULL);
        break;

    case '{':
        node.type = YAMLType::Mapping;
        DoFlowMapping(node);
        break;

    case '[':
        node.type = YAMLType::Sequence;
        DoFlowSequence(node);
        break;

    default:
        yyerror("unexpected token in value");
        match(lookahead);
        break;
    }
}
