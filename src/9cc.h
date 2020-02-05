#ifndef _9CC_F_
#define _9CC_H_

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
  TK_IDENT
} TokenKind;

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
};
Token *token;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,
  ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
};

char* user_input;

// funcs

void gen(Node* node);
void error(char* fmt, ...);
Token* tokenize(char* p);
Node* expr();

#endif // __9CC_H_s