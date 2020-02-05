#include "9cc.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


// lexer

void error(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char* loc, char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    return false;
  }
  token = token->next;
  return true;
}
void expect(char* op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    error_at(token->str, "'%c'ではありません", op);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません : %s", token->kind);
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char* p, char* q) {
  return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {

    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if(startswith(p, "==") || startswith(p, "!=" ) || startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p-q;
      continue;
    }

    error_at(token->str, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// parser

Node* new_node_num(int val) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node* new_node(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

// primary = "(" expr ")" | num | ident
Node* primary() {
  if(consume("(")) {
    Node* node = expr();
    expect(")");
    return node;
  }
  else if(token->kind == TK_NUM) {
    return new_node_num(expect_number());
  }
  else if(token->kind == TK_IDENT) {
    // TODO: return
  }

}
// unary = ("+" | "-")? primary | primary
Node* unary() {
  if(consume("+")) {
    return primary();
  }
  if(consume("-")){
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}
// mul = unary ("*" unary | "/" unary)*
Node* mul() {
  Node* node = unary();

  while(1) {
    if(consume("*")) {
      node = new_node(ND_MUL, node, unary());
    }
    else if(consume("/")) {
      node = new_node(ND_DIV, node, unary());
    }
    else  {
      return node;
    }
  }
}
// add = mul ("+" mul | "-" mul)*
Node* add() {
  Node* node = mul();

  while(1) {
    if(consume("+")) {
      node = new_node(ND_ADD, node, mul());
    }
    else if(consume("-")) {
      node = new_node(ND_SUB, node, mul());
    }
    else {
      return node;
    }
  }
}
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational() {
  Node* node = add();

  while(1) {
    if(consume("<")) {
      node = new_node(ND_LT, node, add());
    }
    else if(consume("<=")) {
      node = new_node(ND_LE, node, add());
    }
    else if(consume(">")) {
      node = new_node(ND_LT, add(), node);
    }
    else if(consume(">=")) {
      node = new_node(ND_LE, add(), node);
    }
    else {
      return node;
    }
  }
}

// equality = relational ("==" relational | "!=" relational)*
Node* equality() {
  Node* node = relational();

  while(1) {
    if(consume("==")) {
      node = new_node(ND_EQ, node, relational());
    }
    else if(consume("!=")) {
      node = new_node(ND_NE, node, relational());
    }
    else {
      return node;
    }
  }
}

// assign = equality ( "=" assign )
Node* assign() {
  Node* node = equality();

  if(consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
}

// expr = assign
Node* expr() {
  return assign();
}

// stmt = expr ";"
Node* stmt() {
  if(consume(";"))
  return expr();
}

// program = stmt

