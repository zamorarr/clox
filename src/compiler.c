#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

typedef enum {
  PREC_NONE, // lowest precedence (evaluted last)
  PREC_ASSIGNMENT, // =
  PREC_OR, // or
  PREC_AND, // and
  PREC_EQUALITY, // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM, // + -
  PREC_FACTOR, // * /
  PREC_UNARY, // ! -
  PREC_CALL, // . ( )
  PREC_PRIMARY // highest precedence (evaluated first)
} Precedence;

// function pointer type
typedef void (*ParseFn)();

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;


// initialize parser
void initParser(Parser* parser) {
  parser->hadError = false;
  parser->panicMode = false;
}

static void errorAt(Parser* parser, Token* token, const char* message) {
  // if already in panic mode, just skip
  if (parser->panicMode) return;
  
  // set parser to panic mode
  parser->panicMode = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    //nothing
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser->hadError = true;
}

// throw error at current token
static void errorAtCurrent(Parser* parser, const char* message) {
  errorAt(parser, &(parser->current), message);
}

// throw error at previous token
static void error(Parser* parser, const char* message) {
  errorAt(parser, &(parser->previous), message);
}


static void advance(Scanner* scanner, Parser* parser) {
  // save current token into previous
  parser->previous = parser->current;

  for (;;) {
    // put next token into current
    parser->current = scanToken(scanner);

    // check if error
    if (parser->current.type != TOKEN_ERROR) break;

    // throw error
    errorAtCurrent(parser, parser->current.start);
  }
}

static void consume(Scanner* scanner, Parser* parser, TokenType type, const char* message) {
  // if current token is type, consume and goto next token
  if (parser->current.type == type) {
    advance(scanner, parser);
    return;
  }

  // else throw error
  errorAtCurrent(parser, message);
}

// @param chunk pointer to chunk we are writing to [to]
// @param parser pointer to parser [from]
// @param byte byte to write to chunk [what]
static void emitByte(Chunk* chunk, Parser* parser, uint8_t byte) {
  writeChunk(chunk, byte, parser->previous.line);
}

//static void emitBytes(Chunk* chunk, Parser* parser, uint8_t byte1, uint8_t byte2) {
//  emitByte(chunk, parser, byte1);
//  emitByte(chunk, parser, byte2);
//}

static void emitReturn(Chunk* chunk, Parser* parser) {
  emitByte(chunk, parser, OP_RETURN);
}

static void emitConstant(Chunk* chunk, Parser* parser, Value value) {
  writeConstant(chunk, value, parser->previous.line);
}

static void endCompiler(Chunk* chunk, Parser* parser) {
  emitReturn(chunk, parser);
}

static void expression(Chunk* chunk, Parser* parser, Scanner* scanner);
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Chunk* chunk, Parser* parser, Scanner* scanner, Precedence precedence);

static void binary(Chunk* chunk, Parser* parser, Scanner* scanner) {
  TokenType operatorType = parser->previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence(chunk, parser, scanner, (Precedence) (rule->precedence + 1));

  switch (operatorType) {
    case TOKEN_PLUS:  emitByte(chunk, parser, OP_ADD); break;
    case TOKEN_MINUS: emitByte(chunk, parser, OP_SUBTRACT); break;
    case TOKEN_STAR:  emitByte(chunk, parser, OP_MULTIPLY); break;
    case TOKEN_SLASH: emitByte(chunk, parser, OP_DIVIDE); break;
    default: return;
  }
}

static void grouping(Chunk* chunk, Parser* parser, Scanner* scanner) {
  expression(chunk, parser, scanner);
  consume(scanner, parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(Chunk* chunk, Parser* parser, Scanner* scanner) {
  // convert previous token to decimal value
  double value = strtod(parser->previous.start, NULL);

  // emit op code
  emitConstant(chunk, parser, value);
}

static void unary(Chunk* chunk, Parser* parser, Scanner* scanner) {
  // get operator type
  TokenType operatorType = parser->previous.type;

  // compile the operand
  parsePrecedence(chunk, parser, scanner, PREC_UNARY);

  // emit the operator instruction
  switch (operatorType) {
    case TOKEN_MINUS: emitByte(chunk, parser, OP_NEGATE); break;
    default: return;
  }
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static void parsePrecedence(Chunk* chunk, Parser* parser, Scanner* scanner, Precedence precedence) {
  // read next token
  advance(scanner, parser);

  // get prefix rule for previous token
  // this determines how to parse the token when it is treated as a prefix operator
  ParseFn prefixRule = getRule(parser->previous.type)->prefix;
  if (prefixRule == NULL) {
    error(parser, "Expect expression.");
    return;
  }

  // call prefix rule
  prefixRule(chunk, parser, scanner);

  while (precedence <= getRule(parser->current.type)->precedence) {
    // load next token into parser
    advance(scanner, parser);
    // get infix rule for previous token
    // this determines how to parse the token when it is treated as an infix operator
    ParseFn infixRule =  getRule(parser->previous.type)->infix;
    
    // call infix rule
    infixRule(chunk, parser, scanner);
  }
}

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

static void expression(Chunk* chunk, Parser* parser, Scanner* scanner) {
  parsePrecedence(chunk, parser, scanner, PREC_ASSIGNMENT);
}

// returns true if no error, false is error
bool compile(VM* vm, const char* source, Chunk* chunk) {
  Scanner scanner;
  Parser parser;
  Chunk* compilingChunk;

  initScanner(&scanner, source);
  initParser(&parser);
  compilingChunk = chunk;

  // load next token into parser
  advance(&scanner, &parser);

  // parse expression
  expression(compilingChunk, &parser, &scanner);

  // consume EOF token
  consume(&scanner, &parser, TOKEN_EOF, "Expect end of expression");

  // end
  endCompiler(compilingChunk, &parser);

  return !parser.hadError;
}