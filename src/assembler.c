#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "list.h"
#include "map.h"
#include "util.h"

#define AMOUNT_META_VARS 4
#define AMOUNT_INSTRUCTIONS 17

#define DEFAULT_MEMORY 128
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100
#define DEFAULT_TICKRATE 60

#define INITAL_LABEL_CAPACITY 32UL


typedef enum {
    META,
    SUBROUTINES
} AssemblerState;


typedef struct {
    uint8_t opcode;
    Token arguments[4];
} Instruction;


typedef enum {
    LITERAL_ARG,
    ADDRESS_ARG
} ArgumentType;


typedef struct {
    ArgumentType type;
    int32_t value;
} Argument;


const char *META_VARIABLES[AMOUNT_META_VARS] = {
    "memory",
    "width",
    "height",
    "tickrate"
};


const char *INSTRUCTIONS[AMOUNT_INSTRUCTIONS] = {
    "mov", "movp",
    "add", "sub", "mul", "div", "mod",
    "less", "equal", "not",
    "jmp", 
    "color", "point", "line", "rect",
    "log", "getp"
};

const uint8_t ARGUMENT_COUNTS[AMOUNT_INSTRUCTIONS] = {2, 2, 3, 3, 3, 3, 3, 3, 3, 2, 2, 3, 2, 4, 4, 1, 3};


void error(uint64_t line, uint64_t column, const char *source_file, const char *message) {
    printf("\x1b[31mERROR (%s:%ld:%ld): %s\n", source_file, line, column, message);
}


void token_error(const Token *token, const char *source_file, const char *message) {
    error(token->source_line+1, token->source_column+1, source_file, message);
}


int get_meta_var_index(const char *s) {
    for (int i = 0; i < AMOUNT_META_VARS; i++) {
        if (strcmp(s, META_VARIABLES[i]) == 0) {
            return i;
        }
    }
    return -1;
}


int get_instruction_opcode(const char *s) {
    for (int i = 0; i < AMOUNT_INSTRUCTIONS; i++) {
        if (strcmp(s, INSTRUCTIONS[i]) == 0) {
            return i;
        }
    }
    return -1;
}


int get_instruction_args(Token *arg_dest, uint8_t arg_count, Lexer *lexer, const char *source_file) {
    for (uint8_t i = 0; i < arg_count; i++) {
        Token token;
        lexer_next(lexer, &token);
        if (token.type != INTEGER && token.type != ADDRESS && token.type != NAME) {
            token_error(&token, source_file, "Expected integer, address, or name for instruction argument.");
            return -1;
        }
        arg_dest[i] = token;
    }
    return 0;
}


int parse_argument_token(Argument *arg_dest, const Token *token, const Map *labels, const char *source_file) {
    char *token_value = get_token_value(token);
    int error_code = 0;
    switch (token->type) {
        case INTEGER:
            arg_dest->type = LITERAL_ARG;
            arg_dest->value = atoi(token_value);
            break;
        case NAME:
            arg_dest->type = LITERAL_ARG;
            void *index;
            int label_result = get_map_value(&index, labels, token_value);
            if (label_result == -1) {
                token_error(token, source_file, "Tried to reference undefined label.");
                error_code = -1;
            }
            else {
                arg_dest->value = *(int32_t*) index;
            }
            break;
        case ADDRESS:
            arg_dest->type = ADDRESS_ARG;
            arg_dest->value = atoi(token_value+1);  // Add 1 to cut off '$'
            break;
        default:
            token_error(token, source_file, "Invalid argument type.");
            error_code = -2;
    }

    free(token_value);
    return error_code;
}


int write_output_file(
        const char *source_file, const char *output_file, 
        int32_t meta_vars[AMOUNT_META_VARS], const List *instructions, const Map *labels,
        int32_t start_label, int32_t tick_label
    ) {
    // Clear existing file
    FILE *clearfile = fopen(output_file, "w");
    if (clearfile != NULL) {
        fclose(clearfile);
    }

    FILE *outfile = fopen(output_file, "a");
    if (outfile == NULL) {
        return -1;
    }

    // Write signature
    fprintf(outfile, "g1");

    // Write meta vars
    write_i32_big(outfile, meta_vars[0]);
    write_i16_big(outfile, (uint16_t) meta_vars[1]);
    write_i16_big(outfile, (uint16_t) meta_vars[2]);
    write_i16_big(outfile, (uint16_t) meta_vars[3]);

    // Write start and tick labels
    write_i32_big(outfile, tick_label);
    write_i32_big(outfile, start_label);

    // Write instruction count
    write_i32_big(outfile, (uint32_t) instructions->size);

    // Write instructions
    bool got_error = false;
    for (size_t i = 0; i < instructions->size; i++) {
        Instruction *ins = get_list_value(instructions, i);
        
        // Write opcode
        fwrite(&ins->opcode, sizeof(uint8_t), 1, outfile);
        
        // Write arguments
        uint8_t arg_count = ARGUMENT_COUNTS[ins->opcode];
        for (uint8_t j = 0; j < arg_count; j++) {
            Argument arg;
            int parse_result = parse_argument_token(&arg, &ins->arguments[j], labels, source_file);
            if (parse_result != 0) {
                got_error = true;
                break;
            }
            uint8_t arg_type = (uint8_t) arg.type;
            fwrite(&arg_type, sizeof(uint8_t), 1, outfile);
            write_i32_big(outfile, arg.value);
        }
        if (got_error) {
            break;
        }
    }

    // TODO: data entries
    write_i32_big(outfile, 0);
    
    fclose(outfile);

    if (got_error) {
        return -1;
    }
    return 0;
}


int assemble_file(const char *input_file, const char *output_file) {
    Lexer lexer;
    char *file_content;
    size_t file_length;
    int read_result = read_file_bytes(&file_content, &file_length, input_file);
    if (read_result != 0) {
        printf("Failed to read input file.\n");
        return -1;
    }

    int lexer_result = create_lexer(&lexer, file_content, file_length);
    if (lexer_result != 0) {
        printf("Failed to initialize lexer.\n");
        return -2;
    }

    AssemblerState state = META;

    List instructions;
    create_list(&instructions, sizeof(Instruction), 32);

    Map labels;
    create_map(&labels, INITAL_LABEL_CAPACITY);

    int32_t meta_vars[AMOUNT_META_VARS] = {DEFAULT_MEMORY, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_TICKRATE};
    int32_t instruction_index = 0;

    bool got_error = false;

    while (!lexer.is_done && !got_error) {
        Token token;
        int next_response = lexer_next(&lexer, &token);
        if (next_response < 0) {
            error(lexer.source_line+1, lexer.source_column+1, input_file, "Unrecognized token.");
            break;
        }
        if (next_response == 1) {
            break;
        }
        switch (token.type) {
            case META_VARIABLE:
                if (state != META) {
                    token_error(&token, input_file, "Found meta variable outside file header.");
                    got_error = true;
                    break;
                }

                char meta_var[16];
                copy_token_value(meta_var, &token);
                int index = get_meta_var_index(meta_var+1);  // Cut off '#'
                if (index == -1) {
                    token_error(&token, input_file, "Unrecognized meta variable.");
                    got_error = true;
                    break;
                }

                Token value_token;
                lexer_next(&lexer, &value_token);
                char var_value[16];
                copy_token_value(var_value, &value_token);
                meta_vars[index] = atoi(var_value);
                break;
            
            case LABEL_NAME:
                if (state != SUBROUTINES) {
                    state = SUBROUTINES;
                }
                
                char *label_name = get_token_value(&token);
                label_name[token.length-1] = '\0';  // Cut off ':'

                // Check if the label was already declared
                if (get_map_value(NULL, &labels, label_name) == 0) {
                    token_error(&token, input_file, "Label declared more than once.");
                    free(label_name);
                    got_error = true;
                    break;
                }
                else {
                    void *ins_index_ptr = malloc(sizeof(int32_t));
                    *(int32_t*) ins_index_ptr = instruction_index;
                    add_map_value(&labels, label_name, ins_index_ptr);
                }
                break;
            
            case NAME:
                char instruction[16];
                copy_token_value(instruction, &token);
                int opcode = get_instruction_opcode(instruction);
                if (opcode == -1) {
                    token_error(&token, input_file, "Unrecognized instruction.");
                    got_error = true;
                    break;
                }

                uint8_t arg_count = ARGUMENT_COUNTS[opcode];
                Instruction ins;
                ins.opcode = opcode;
                int args_result = get_instruction_args(ins.arguments, arg_count, &lexer, input_file);
                if (args_result == -1) {
                    got_error = true;
                    break;
                }
                append_list_value(&instructions, &ins);

                instruction_index++;
                break;
            
            case INTEGER:
            case ADDRESS:
                token_error(&token, input_file, "Got value outside of instruction.");
                got_error = true;
                break;
            
            case COMMENT:
            case NEWLINE:
                continue;
        }
    }

    if (!got_error) {
        void *start_label_ptr, *tick_label_ptr;
        int32_t start_label = -1, tick_label = -1;
        if (get_map_value(&start_label_ptr, &labels, "start") == 0) {
            start_label = *(int32_t*) start_label_ptr;
        }
        if (get_map_value(&tick_label_ptr, &labels, "tick") == 0) {
            tick_label = *(int32_t*) tick_label_ptr;
        }

        write_output_file(
            input_file, output_file,
            meta_vars, &instructions, &labels,
            start_label, tick_label
        );
    }

    // Free memory
    free_list(&instructions);

    for (size_t i = 0; i < labels.size; i++) {
        MapNode *node = &labels.data[i];
        if (node->key != NULL) {
            free(node->key);
            free(node->value);
        }
    }
    free_map(&labels);

    free(file_content);

    return 0;
}