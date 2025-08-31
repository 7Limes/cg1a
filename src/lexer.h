#ifndef G1_LEXER_H
#define G1_LEXER_H


#include <stdint.h>
#include <stdbool.h>
#include <regex.h>


#define AMOUNT_TOKEN_TYPES 7
#define AMOUNT_IGNORED_EXPRESSIONS 1


typedef enum {
    META_VARIABLE,
    INTEGER,
    ADDRESS,
    LABEL_NAME,
    NAME,
    COMMENT,
    NEWLINE
} TokenType;


typedef struct {
    const char *source;
    char *current_char_pointer;
    const char *end_char_pointer;

    regex_t token_expressions[AMOUNT_TOKEN_TYPES];
    regex_t ignored_expressions[AMOUNT_IGNORED_EXPRESSIONS];

    uint64_t source_index, source_line, source_column;

    bool is_done;
} Lexer;


typedef struct {
    const char *source;
    TokenType type;
    uint64_t source_index, length, source_line, source_column;
} Token;


// Create a new lexer.
int create_lexer(Lexer *lexer_dest, char *source, size_t source_length);

// Get the next token from the lexer.
int lexer_next(Lexer *lexer, Token *token_dest);

// Copy the string value of `token` into `dest`.
void copy_token_value(char *dest, const Token *token);

// Returns an owned string containing the value of `token`.
char* get_token_value(const Token *token);


#endif
