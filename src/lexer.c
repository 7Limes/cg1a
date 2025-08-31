#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <regex.h>
#include "util.h"
#include "lexer.h"


const char* TOKEN_EXPRESSIONS[AMOUNT_TOKEN_TYPES] = {
    "^#[A-z]+",
    "^-?[0-9]+",
    "^\\$[0-9]+",
    "^[A-z0-9_]+:",
    "^[A-z_][A-z0-9_]*",
    "^;[^\r\n]*",
    "^\r?\n"
};

const char* IGNORED_EXPRESSIONS[AMOUNT_IGNORED_EXPRESSIONS] = {
    "^[ \t]+"
};


int compile_expressions(regex_t *expressions_dest, const char **expressions, size_t amount_expressions) {
    for (size_t i = 0; i < amount_expressions; i++) {
        int compile_result = regcomp(&expressions_dest[i], expressions[i], REG_EXTENDED);
        if (compile_result != 0) {
            return -1;
        }
    }
    return 0;
}


void lexer_error(uint64_t source_line, uint64_t source_column) {
    printf("\x1b[31mERROR: Unrecognized token at line %ld, column %ld.\x1b[0m\n", source_line, source_column);
}


int create_lexer(Lexer *lexer_dest, char *source, size_t source_length) {
    lexer_dest->source = source;
    lexer_dest->current_char_pointer = source;
    lexer_dest->end_char_pointer = source + source_length;
    lexer_dest->source_index = 0;
    lexer_dest->source_line = 0;
    lexer_dest->source_column = 0;
    lexer_dest->is_done = false;

    int comp_result_1 = compile_expressions(lexer_dest->token_expressions, TOKEN_EXPRESSIONS, AMOUNT_TOKEN_TYPES);
    int comp_result_2 = compile_expressions(lexer_dest->ignored_expressions, IGNORED_EXPRESSIONS, AMOUNT_IGNORED_EXPRESSIONS);
    if (comp_result_1 || comp_result_2) {
        return -1;
    }

    return 0;
}


int lexer_next(Lexer *lexer, Token *token_dest) {
    // Check if we've reached the end of the string
    if (lexer->is_done || lexer->current_char_pointer >= lexer->end_char_pointer) {
        lexer->is_done = true;
        return 1;
    }
    
    // Iterate until we find a token
    while (true) {
        regmatch_t match;

        // Check for tokens
        for (int i = 0; i < AMOUNT_TOKEN_TYPES; i++) {
            regex_t current_expression = lexer->token_expressions[i];
            if (regexec(&current_expression, lexer->current_char_pointer, 1, &match, 0) == 0) {
                // // Allocate space for the token value
                // char *token_value = malloc(sizeof (char) * (match.rm_eo + 1));
                // strncpy(token_value, lexer->current_char_pointer, match.rm_eo);
                // token_value[match.rm_eo] = '\0';
                
                // Create the token
                token_dest->source = lexer->source;
                token_dest->type = (TokenType) i;
                token_dest->source_index = lexer->source_index;
                token_dest->length = match.rm_eo;
                token_dest->source_line = lexer->source_line;
                token_dest->source_column = lexer->source_column;

                lexer->current_char_pointer += match.rm_eo;
                lexer->source_index += match.rm_eo;
                lexer->source_column += match.rm_eo;
                if (i == NEWLINE) {
                    lexer->source_line += 1;
                    lexer->source_column = 0;
                }
                
                return 0;
            }
        }

        // Check for ignored expressions
        bool is_ignored = false;
        for (int i = 0; i < AMOUNT_IGNORED_EXPRESSIONS; i++) {
            regex_t current_expression = lexer->ignored_expressions[i];
            if (regexec(&current_expression, lexer->current_char_pointer, 1, &match, 0) == 0) {
                lexer->current_char_pointer += match.rm_eo;
                lexer->source_index += match.rm_eo;
                lexer->source_column += match.rm_eo;
                is_ignored = true;
                break;
            }
        }

        // Unrecognized token
        if (!is_ignored) {
            lexer_error(lexer->source_line, lexer->source_column);
            lexer->is_done = true;
            return -1;
        }
    }

    return 0;
}


void copy_token_value(char *dest, const Token *token) {
    strncpy(dest, token->source+token->source_index, token->length);
    dest[token->length] = '\0';
}


char* get_token_value(const Token *token) {
    char *value = malloc(token->length + 1);
    copy_token_value(value, token);
    return value;
}