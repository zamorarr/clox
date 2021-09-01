#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

// initialize scanner
void initScanner(Scanner* scanner, const char* source) {
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

static bool isAtEnd(Scanner* scanner) {
  return *scanner->current == '\0';
}

static bool isAlpha(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

// consume next character
static char advance(Scanner* scanner) {
  // increase current
  scanner->current++;
  // return previous character
  return scanner->current[-1];
}

// look at next character, but do not consume
static char peek(Scanner* scanner) {
  return *scanner->current;
}

// look two characters ahead, but do not consume either
static char peekNext(Scanner* scanner) {
  if (isAtEnd(scanner)) return '\0';
  return scanner->current[1];
}

// check if current value matches expected
static bool match(Scanner* scanner, char expected) {
  // if at end of file, no match
  if (isAtEnd(scanner)) return false;

  // if current val not equal to expected, no match
  if (*scanner->current != expected) return false;

  // move pointer up and return true
  scanner->current++;
  return true;
}

// create new token
static Token makeToken(Scanner* scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int) (scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

static Token errorToken(Scanner* scanner, const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int) strlen(message);
  token.line = scanner->line;
  return token;
}

static void skipWhiteSpace(Scanner* scanner) {
  for (;;) {
    // get next character
    char c = peek(scanner);

    switch (c) {
      // consume whitespace
      case ' ':
      case '\r':
      case '\t':
        advance(scanner);
        break;
      // consume new line
      case '\n':
        scanner->line++;
        advance(scanner);
        break;
      // consume comments
      case '/':
        if (peekNext(scanner) == '/') {
          // comment goes until end of line
          while(peek(scanner) != '\n' && !isAtEnd(scanner)) advance(scanner);
        } else {
          // not a comment, probably division
          return;
        }
        break;
      default:
        return;
    }
  }
}

static TokenType checkKeyword(Scanner* scanner, int start, int length, const char* rest, TokenType type) {
  bool correct_length = scanner->current - scanner->start == start + length;
  // memcmp comparison tests that the rest of the char* in scanner matches rest
  if (correct_length && memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Scanner* scanner) {
  // determine if identifer token is a reserved keyword
  switch (scanner->start[0]) {
    case 'a': return checkKeyword(scanner, 1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(scanner, 1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(scanner, 1, 3, "lse", TOKEN_ELSE);
    case 'f':
      // token length > 1
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a': return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(scanner, 2, 1, "r",TOKEN_FOR);
          case 'u': return checkKeyword(scanner, 2, 1, "n", TOKEN_FUN);
        }
      }
      break;
    case 'i': return checkKeyword(scanner, 1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(scanner, 1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(scanner, 1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(scanner, 1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(scanner, 1, 4, "uper", TOKEN_SUPER);
    case 't':
      // token length > 1
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'h': return checkKeyword(scanner, 2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(scanner, 2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;
    case 'v': return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
  }

  // it wasn't a reserved keyword - just return identifier
  return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner* scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) advance(scanner);
  return makeToken(scanner, identifierType(scanner));
}

static Token number(Scanner* scanner) {
  // advance scanner while current char is a digit
  while (isDigit(peek(scanner))) advance(scanner);

  // look for a fractional part
  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    // consume the dot
    advance(scanner);

    // consume the fraction
    while (isDigit(peek(scanner))) advance(scanner);
  }

  // make token
  return makeToken(scanner, TOKEN_NUMBER);
}

static Token string(Scanner* scanner) {
  // loop until we find terminating quotes
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') scanner->line++;
    advance(scanner);
  }

  // found end of file before terminating quotes
  if (isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");

  // found terminating quotes
  advance(scanner);
  return makeToken(scanner, TOKEN_STRING);
}

// scan next token
Token scanToken(Scanner* scanner) {
  // skip whitespace
  skipWhiteSpace(scanner);

  // set scanner start to current location
  scanner->start = scanner->current;

  // if we are at end of file, make an EOF token
  if (isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);

  // get next character
  char c = advance(scanner);

  // handle identifiers
  if (isAlpha(c)) return identifier(scanner);

  // handle numbers
  if (isDigit(c)) return number(scanner);

  // make token
  switch (c) {
    case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
    case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
    case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
    case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
    case ';': return makeToken(scanner, TOKEN_SEMICOLON);
    case ',': return makeToken(scanner, TOKEN_COMMA);
    case '.': return makeToken(scanner, TOKEN_DOT);
    case '-': return makeToken(scanner, TOKEN_MINUS);
    case '+': return makeToken(scanner, TOKEN_PLUS);
    case '/': return makeToken(scanner, TOKEN_SLASH);
    case '*': return makeToken(scanner, TOKEN_STAR);
    case '!':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL: TOKEN_BANG);
    case '=':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL: TOKEN_EQUAL);
    case '<':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string(scanner);
  }

  return errorToken(scanner, "Unexpected character.");
}