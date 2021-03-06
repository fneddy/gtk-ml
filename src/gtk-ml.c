#include <stdlib.h>
#include <gtk/gtk.h>
#include <math.h>
#include "gtk-ml.h"

typedef GtkMl_S *(*GtkMl_ReaderFn)(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);

typedef struct GtkMl_Reader {
    GtkMl_TokenKind token;
    GtkMl_ReaderFn fn;
} GtkMl_Reader;

typedef struct GtkMl_Parser {
    GtkMl_Reader *readers;
    size_t len_reader;
    size_t cap_reader;
} GtkMl_Parser;

struct GtkMl_Context {
    gboolean gc_enabled;
    size_t n_values;
    size_t m_values;
    GtkMl_S *first;

    GtkMl_S *bindings;
    GtkMl_S **top_scope;

    GtkMl_Vm *vm;
    GtkMl_Parser parser;
};

struct GtkMl_Vm {
    GtkMl_Register reg[GTKML_REGISTER_COUNT];
    GtkMl_Register *stack;
    size_t stack_len;
    GtkMl_Program program;
    GtkMl_S *(**std)(GtkMl_Context *, const char **, GtkMl_S *);

    GtkMl_Context *ctx;
};


GTKML_PRIVATE GtkMl_S *parse(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);
GTKML_PRIVATE GtkMl_S *parse_vararg(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);
GTKML_PRIVATE GtkMl_S *parse_quote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);
GTKML_PRIVATE GtkMl_S *parse_quasiquote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);
GTKML_PRIVATE GtkMl_S *parse_unquote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);
GTKML_PRIVATE GtkMl_S *parse_alternative(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc);

GTKML_PRIVATE GtkMl_S *new_value(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_SKind kind);
GTKML_PRIVATE GtkMl_S *new_nil(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_true(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_false(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_int(GtkMl_Context *ctx, GtkMl_Span *span, int64_t value);
GTKML_PRIVATE GtkMl_S *new_float(GtkMl_Context *ctx, GtkMl_Span *span, float value);
GTKML_PRIVATE GtkMl_S *new_string(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len);
GTKML_PRIVATE GtkMl_S *new_symbol(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len);
GTKML_PRIVATE GtkMl_S *new_keyword(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len);
GTKML_PRIVATE GtkMl_S *new_list(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *car, GtkMl_S *cdr);
GTKML_PRIVATE GtkMl_S *new_map(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_set(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_array(GtkMl_Context *ctx, GtkMl_Span *span);
GTKML_PRIVATE GtkMl_S *new_vararg(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr);
GTKML_PRIVATE GtkMl_S *new_quote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr);
GTKML_PRIVATE GtkMl_S *new_quasiquote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr);
GTKML_PRIVATE GtkMl_S *new_unquote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr);
GTKML_PRIVATE GtkMl_S *new_lambda(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture);
GTKML_PRIVATE GtkMl_S *new_program(GtkMl_Context *ctx, GtkMl_Span *span, const char *linkage_name, uint64_t addr, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture);
GTKML_PRIVATE GtkMl_S *new_macro(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture);
GTKML_PRIVATE GtkMl_S *new_lightdata(GtkMl_Context *ctx, GtkMl_Span *span, void *data);
GTKML_PRIVATE GtkMl_S *new_userdata(GtkMl_Context *ctx, GtkMl_Span *span, void *data, void (*del)(GtkMl_Context *ctx, void *));
GTKML_PRIVATE void delete(GtkMl_Context *ctx, GtkMl_S *s);
GTKML_PRIVATE void del(GtkMl_Context *ctx, GtkMl_S *s);
GTKML_PRIVATE void gtk_ml_object_unref(GtkMl_Context *ctx, void *obj);

GTKML_PRIVATE gboolean builder_lambda(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_vararg(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_quote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_quasiquote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_unquote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_macro(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_define(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_application(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean builder_new_window(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);

GTKML_PRIVATE GtkMl_S *vm_std_application(GtkMl_Context *ctx, const char **err, GtkMl_S *expr);
GTKML_PRIVATE GtkMl_S *vm_std_new_window(GtkMl_Context *ctx, const char **err, GtkMl_S *expr);

GTKML_PRIVATE gboolean gtk_ml_vm_run(GtkMl_Vm *vm, const char **err);

GTKML_PRIVATE gboolean gtk_ml_ia(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);
GTKML_PRIVATE gboolean gtk_ml_ii(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);
GTKML_PRIVATE gboolean gtk_ml_ibr(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);
GTKML_PRIVATE gboolean gtk_ml_eii(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);
GTKML_PRIVATE gboolean gtk_ml_eibr(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);
GTKML_PRIVATE gboolean gtk_ml_enop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr);

GTKML_PRIVATE gboolean gtk_ml_ia_nop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_halt(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_add(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_subtract(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_signed_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_unsigned_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_signed_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_unsigned_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_signed_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_integer_unsigned_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_float_add(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_float_subtract(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_float_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_float_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_float_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_and(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_or(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_xor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_nand(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_nor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bit_xnor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_branch_absolute(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_branch_relative(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_bind(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ia_define(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);

GTKML_PRIVATE gboolean gtk_ml_ii_push_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_pop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_get_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_list_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_map_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_set_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ii_array_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);

GTKML_PRIVATE gboolean gtk_ml_ibr_call(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ibr_ret(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);
GTKML_PRIVATE gboolean gtk_ml_ibr_call_std(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr);

GTKML_PRIVATE gboolean gtk_ml_eii_push_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_pop_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_get_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_list_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_map_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_set_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eii_array_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);

GTKML_PRIVATE gboolean gtk_ml_eibr_call_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eibr_ret_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);
GTKML_PRIVATE gboolean gtk_ml_eibr_call_ext_std(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64);

GTKML_PRIVATE const char *S_I_ARITH[] = {
    [GTKML_IA_NOP] = GTKML_SIA_NOP,
    [GTKML_IA_HALT] = GTKML_SIA_HALT,
    [GTKML_IA_INTEGER_ADD] = GTKML_SIA_INTEGER_ADD,
    [GTKML_IA_INTEGER_SUBTRACT] = GTKML_SIA_INTEGER_SUBTRACT,
    [GTKML_IA_INTEGER_SIGNED_MULTIPLY] = GTKML_SIA_INTEGER_SIGNED_MULTIPLY,
    [GTKML_IA_INTEGER_UNSIGNED_MULTIPLY] = GTKML_SIA_INTEGER_UNSIGNED_MULTIPLY,
    [GTKML_IA_INTEGER_SIGNED_DIVIDE] = GTKML_SIA_INTEGER_SIGNED_DIVIDE,
    [GTKML_IA_INTEGER_UNSIGNED_DIVIDE] = GTKML_SIA_INTEGER_UNSIGNED_DIVIDE,
    [GTKML_IA_INTEGER_SIGNED_MODULO] = GTKML_SIA_INTEGER_SIGNED_MODULO,
    [GTKML_IA_INTEGER_UNSIGNED_MODULO] = GTKML_SIA_INTEGER_UNSIGNED_MODULO,
    [GTKML_IA_FLOAT_ADD] = GTKML_SIA_FLOAT_ADD,
    [GTKML_IA_FLOAT_SUBTRACT] = GTKML_SIA_FLOAT_SUBTRACT,
    [GTKML_IA_FLOAT_MULTIPLY] = GTKML_SIA_FLOAT_MULTIPLY,
    [GTKML_IA_FLOAT_DIVIDE] = GTKML_SIA_FLOAT_DIVIDE,
    [GTKML_IA_FLOAT_MODULO] = GTKML_SIA_FLOAT_MODULO,
    [GTKML_IA_BIT_AND] = GTKML_SIA_BIT_AND,
    [GTKML_IA_BIT_OR] = GTKML_SIA_BIT_OR,
    [GTKML_IA_BIT_XOR] = GTKML_SIA_BIT_XOR,
    [GTKML_IA_BIT_NAND] = GTKML_SIA_BIT_NAND,
    [GTKML_IA_BIT_NOR] = GTKML_SIA_BIT_NOR,
    [GTKML_IA_BIT_XNOR] = GTKML_SIA_BIT_XNOR,
    [GTKML_IA_BRANCH_ABSOLUTE] = GTKML_SIA_BRANCH_ABSOLUTE,
    [GTKML_IA_BRANCH_RELATIVE] = GTKML_SIA_BRANCH_RELATIVE,
    [GTKML_IA_BIND] = GTKML_SIA_BIND,
    [GTKML_IA_DEFINE] = GTKML_SIA_DEFINE,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_I_IMM[] = {
    [GTKML_II_PUSH_IMM] = GTKML_SII_PUSH_IMM,
    [GTKML_II_POP] = GTKML_SII_POP,
    [GTKML_II_GET_IMM] = GTKML_SII_GET_IMM,
    [GTKML_II_LIST_IMM] = GTKML_SII_LIST_IMM,
    [GTKML_II_MAP_IMM] = GTKML_SII_MAP_IMM,
    [GTKML_II_SET_IMM] = GTKML_SII_SET_IMM,
    [GTKML_II_ARRAY_IMM] = GTKML_SII_ARRAY_IMM,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_I_IMM_EXTERN[] = {
    [GTKML_II_PUSH_IMM] = GTKML_SII_PUSH_IMM_EXTERN,
    [GTKML_II_POP] = GTKML_SII_POP,
    [GTKML_II_GET_IMM] = GTKML_SII_GET_IMM,
    [GTKML_II_LIST_IMM] = GTKML_SII_LIST_IMM,
    [GTKML_II_MAP_IMM] = GTKML_SII_MAP_IMM,
    [GTKML_II_SET_IMM] = GTKML_SII_SET_IMM,
    [GTKML_II_ARRAY_IMM] = GTKML_SII_ARRAY_IMM,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_I_BR[] = {
    [GTKML_IBR_CALL] = GTKML_SIBR_CALL,
    [GTKML_IBR_RET] = GTKML_SIBR_RET,
    [GTKML_IBR_CALL_STD] = GTKML_SIBR_CALL_STD,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_EI_IMM[] = {
    [GTKML_EII_PUSH_EXT_IMM] = GTKML_SEII_PUSH_EXT_IMM,
    [GTKML_EII_POP_EXT] = GTKML_SEII_POP_EXT,
    [GTKML_EII_GET_EXT_IMM] = GTKML_SEII_GET_EXT_IMM,
    [GTKML_EII_LIST_EXT_IMM] = GTKML_SEII_LIST_EXT_IMM,
    [GTKML_EII_MAP_EXT_IMM] = GTKML_SEII_MAP_EXT_IMM,
    [GTKML_EII_SET_EXT_IMM] = GTKML_SEII_SET_EXT_IMM,
    [GTKML_EII_ARRAY_EXT_IMM] = GTKML_SEII_ARRAY_EXT_IMM,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_EI_IMM_EXTERN[] = {
    [GTKML_EII_PUSH_EXT_IMM] = GTKML_SEII_PUSH_EXT_IMM_EXTERN,
    [GTKML_EII_POP_EXT] = GTKML_SEII_POP_EXT,
    [GTKML_EII_GET_EXT_IMM] = GTKML_SEII_GET_EXT_IMM,
    [GTKML_EII_LIST_EXT_IMM] = GTKML_SEII_LIST_EXT_IMM,
    [GTKML_EII_MAP_EXT_IMM] = GTKML_SEII_MAP_EXT_IMM,
    [GTKML_EII_SET_EXT_IMM] = GTKML_SEII_SET_EXT_IMM,
    [GTKML_EII_ARRAY_EXT_IMM] = GTKML_SEII_ARRAY_EXT_IMM,
    [255] = NULL,
};

GTKML_PRIVATE const char *S_EI_BR[] = {
    [GTKML_EIBR_CALL_EXT] = GTKML_SEIBR_CALL_EXT,
    [GTKML_EIBR_RET_EXT] = GTKML_SEIBR_RET_EXT,
    [GTKML_EIBR_CALL_EXT_STD] = GTKML_SEIBR_CALL_EXT_STD,
    [255] = NULL,
};

GTKML_PRIVATE const char **S_CATEGORY[] = {
    [0] = NULL,
    [GTKML_I_ARITH] = S_I_ARITH,
    [GTKML_I_IMM] = S_I_IMM,
    [GTKML_I_BR] = S_I_BR,
    [4] = NULL,
    [5] = NULL,
    [GTKML_I_IMM | GTKML_I_IMM_EXTERN] = S_I_IMM_EXTERN,
    [7] = NULL,
    [8] = NULL,
    [9] = NULL,
    [GTKML_EI_IMM] = S_EI_IMM,
    [GTKML_EI_BR] = S_EI_BR,
    [12] = NULL,
    [13] = NULL,
    [GTKML_EI_IMM | GTKML_EI_IMM_EXTERN] = S_EI_IMM_EXTERN,
    [15] = NULL,
};

GTKML_PRIVATE gboolean (*CATEGORY[])(GtkMl_Vm *, const char **, GtkMl_Instruction *) = {
    [0] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [GTKML_I_ARITH] = gtk_ml_ia,
    [GTKML_I_IMM] = gtk_ml_ii,
    [GTKML_I_BR] = gtk_ml_ibr,
    [4] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [5] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [6] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [7] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [8] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [9] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [GTKML_EI_IMM] = gtk_ml_eii,
    [GTKML_EI_BR] = gtk_ml_eibr,
    [GTKML_EI_EXPORT] = gtk_ml_enop,
    [13] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [14] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
    [15] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction *)) NULL,
};

GTKML_PRIVATE gboolean (*I_ARITH[])(GtkMl_Vm *, const char **, GtkMl_Instruction) = {
    [GTKML_IA_NOP] = gtk_ml_ia_nop,
    [GTKML_IA_HALT] = gtk_ml_ia_halt,
    [GTKML_IA_INTEGER_ADD] = gtk_ml_ia_integer_add,
    [GTKML_IA_INTEGER_SUBTRACT] = gtk_ml_ia_integer_subtract,
    [GTKML_IA_INTEGER_SIGNED_MULTIPLY] = gtk_ml_ia_integer_signed_multiply,
    [GTKML_IA_INTEGER_UNSIGNED_MULTIPLY] = gtk_ml_ia_integer_unsigned_multiply,
    [GTKML_IA_INTEGER_SIGNED_DIVIDE] = gtk_ml_ia_integer_signed_divide,
    [GTKML_IA_INTEGER_UNSIGNED_DIVIDE] = gtk_ml_ia_integer_unsigned_divide,
    [GTKML_IA_INTEGER_SIGNED_MODULO] = gtk_ml_ia_integer_signed_modulo,
    [GTKML_IA_INTEGER_UNSIGNED_MODULO] = gtk_ml_ia_integer_unsigned_modulo,
    [GTKML_IA_FLOAT_ADD] = gtk_ml_ia_float_add,
    [GTKML_IA_FLOAT_SUBTRACT] = gtk_ml_ia_float_subtract,
    [GTKML_IA_FLOAT_MULTIPLY] = gtk_ml_ia_float_multiply,
    [GTKML_IA_FLOAT_DIVIDE] = gtk_ml_ia_float_divide,
    [GTKML_IA_FLOAT_MODULO] = gtk_ml_ia_float_modulo,
    [GTKML_IA_BIT_AND] = gtk_ml_ia_bit_and,
    [GTKML_IA_BIT_OR] = gtk_ml_ia_bit_or,
    [GTKML_IA_BIT_XOR] = gtk_ml_ia_bit_xor,
    [GTKML_IA_BIT_NAND] = gtk_ml_ia_bit_nand,
    [GTKML_IA_BIT_NOR] = gtk_ml_ia_bit_nor,
    [GTKML_IA_BIT_XNOR] = gtk_ml_ia_bit_xnor,
    [GTKML_IA_BRANCH_ABSOLUTE] = gtk_ml_ia_branch_absolute,
    [GTKML_IA_BRANCH_RELATIVE] = gtk_ml_ia_branch_relative,
    [GTKML_IA_BIND] = gtk_ml_ia_bind,
    [GTKML_IA_DEFINE] = gtk_ml_ia_define,
    [255] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction)) NULL,
};

GTKML_PRIVATE gboolean (*I_IMM[])(GtkMl_Vm *, const char **, GtkMl_Instruction) = {
    [GTKML_II_PUSH_IMM] = gtk_ml_ii_push_imm,
    [GTKML_II_POP] = gtk_ml_ii_pop,
    [GTKML_II_GET_IMM] = gtk_ml_ii_get_imm,
    [GTKML_II_LIST_IMM] = gtk_ml_ii_list_imm,
    [GTKML_II_MAP_IMM] = gtk_ml_ii_map_imm,
    [GTKML_II_SET_IMM] = gtk_ml_ii_set_imm,
    [GTKML_II_ARRAY_IMM] = gtk_ml_ii_array_imm,
    [255] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction)) NULL,
};

GTKML_PRIVATE gboolean (*I_BR[])(GtkMl_Vm *, const char **, GtkMl_Instruction) = {
    [GTKML_IBR_CALL] = gtk_ml_ibr_call,
    [GTKML_IBR_RET] = gtk_ml_ibr_ret,
    [GTKML_IBR_CALL_STD] = gtk_ml_ibr_call_std,
    [255] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction)) NULL,
};

GTKML_PRIVATE gboolean (*EI_IMM[])(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *) = {
    [GTKML_EII_PUSH_EXT_IMM] = gtk_ml_eii_push_ext_imm,
    [GTKML_EII_POP_EXT] = gtk_ml_eii_pop_ext,
    [GTKML_EII_GET_EXT_IMM] = gtk_ml_eii_get_ext_imm,
    [GTKML_EII_LIST_EXT_IMM] = gtk_ml_eii_list_ext_imm,
    [GTKML_EII_MAP_EXT_IMM] = gtk_ml_eii_map_ext_imm,
    [GTKML_EII_SET_EXT_IMM] = gtk_ml_eii_set_ext_imm,
    [GTKML_EII_ARRAY_EXT_IMM] = gtk_ml_eii_array_ext_imm,
    [255] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *)) NULL,
};

GTKML_PRIVATE gboolean (*EI_BR[])(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *) = {
    [GTKML_EIBR_CALL_EXT] = gtk_ml_eibr_call_ext,
    [GTKML_EIBR_RET_EXT] = gtk_ml_eibr_ret_ext,
    [GTKML_EIBR_CALL_EXT_STD] = gtk_ml_eibr_call_ext_std,
    [255] = (gboolean (*)(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *)) NULL,
};

GTKML_PRIVATE GtkMl_S *(*STD[])(GtkMl_Context *, const char **, GtkMl_S *) = {
    [GTKML_STD_APPLICATION] = vm_std_application,
    [GTKML_STD_NEW_WINDOW] = vm_std_new_window,
};

GtkMl_Context *gtk_ml_new_context() {
    GtkMl_Context *ctx = malloc(sizeof(GtkMl_Context));
    ctx->gc_enabled = 1;
    ctx->n_values = 0;
    ctx->m_values = GTKML_GC_COUNT_THRESHOLD;
    ctx->first = NULL;
    ctx->vm = gtk_ml_new_vm(ctx);

    // ({'flags-none G_APPLICATION_FLAGS_NONE})
    ctx->bindings = new_list(ctx, NULL, new_map(ctx, NULL), new_nil(ctx, NULL));
    ctx->top_scope = &gtk_ml_car(ctx->bindings);

    gtk_ml_define(ctx, new_symbol(ctx, NULL, "flags-none", 10), new_int(ctx, NULL, G_APPLICATION_FLAGS_NONE));

    ctx->parser.readers = malloc(sizeof(GtkMl_Reader) * 64);
    ctx->parser.len_reader = 0;
    ctx->parser.cap_reader = 64;

    ctx->parser.readers[ctx->parser.len_reader].token = GTKML_TOK_ELLIPSIS;
    ctx->parser.readers[ctx->parser.len_reader].fn = parse_vararg;
    ++ctx->parser.len_reader;

    ctx->parser.readers[ctx->parser.len_reader].token = GTKML_TOK_TICK;
    ctx->parser.readers[ctx->parser.len_reader].fn = parse_quote;
    ++ctx->parser.len_reader;

    ctx->parser.readers[ctx->parser.len_reader].token = GTKML_TOK_BACKTICK;
    ctx->parser.readers[ctx->parser.len_reader].fn = parse_quasiquote;
    ++ctx->parser.len_reader;

    ctx->parser.readers[ctx->parser.len_reader].token = GTKML_TOK_COMMA;
    ctx->parser.readers[ctx->parser.len_reader].fn = parse_unquote;
    ++ctx->parser.len_reader;

    ctx->parser.readers[ctx->parser.len_reader].token = GTKML_TOK_POUND;
    ctx->parser.readers[ctx->parser.len_reader].fn = parse_alternative;
    ++ctx->parser.len_reader;

    return ctx;
}

void gtk_ml_del_context(GtkMl_Context *ctx) {
    gtk_ml_del_vm(ctx->vm);

    GtkMl_S *value = ctx->first;
    while (value) {
        GtkMl_S *next = value->next;
        del(ctx, value);
        value = next;
    }

    free(ctx);
}

GTKML_PRIVATE GtkMl_S *local_scope(GtkMl_Context *ctx) {
    GtkMl_S *local = new_map(ctx, NULL);

    GtkMl_S *bindings = ctx->bindings;
    while (bindings->kind != GTKML_S_NIL && gtk_ml_cdr(bindings)->kind != GTKML_S_NIL) {
        GtkMl_S *scope = gtk_ml_car(bindings);
        GtkMl_HashTrie new;
        gtk_ml_hash_trie_concat(&new, &scope->value.s_map.map, &local->value.s_map.map);
        gtk_ml_del_hash_trie(&local->value.s_map.map);
        local->value.s_map.map = new;
        bindings = gtk_ml_cdr(bindings);
    }

    return local;
}

void gtk_ml_load_program(GtkMl_Context *ctx, GtkMl_Program* program) {
    char *start = NULL;
    if (program->start) {
        start = malloc(strlen(program->start) + 1);
        strcpy(start, program->start);
    }
    ctx->vm->program.start = start;

    ctx->vm->program.exec = malloc(sizeof(GtkMl_Instruction) * program->n_exec);
    ctx->vm->program.n_exec = program->n_exec;
    memcpy(ctx->vm->program.exec, program->exec, sizeof(GtkMl_Instruction) * program->n_exec);

    ctx->vm->program.statics = malloc(sizeof(GtkMl_S *) * program->n_static);
    ctx->vm->program.n_static = program->n_static;
    memcpy(ctx->vm->program.statics, program->statics, sizeof(GtkMl_S *) * program->n_static);
}

gboolean gtk_ml_run_program(GtkMl_Context *ctx, const char **err, GtkMl_S *program, GtkMl_S *args) {
    gtk_ml_enter(ctx);

    GtkMl_S *params = program->value.s_program.args;

    while (params->kind != GTKML_S_NIL && args && args->kind != GTKML_S_NIL) {
        GtkMl_S *arg = gtk_ml_car(args);
        if (!arg) {
            break;
        }
        gtk_ml_bind(ctx, gtk_ml_car(params), arg);
        params = gtk_ml_cdr(params);
        args = gtk_ml_cdr(args);
    }

    if (params->kind == GTKML_S_LIST) {
        GtkMl_S *param = gtk_ml_car(params);
        if (param->kind == GTKML_S_VARARG) {
            gtk_ml_bind(ctx, param->value.s_var.expr, new_nil(ctx, NULL));
        } else {
            *err = GTKML_ERR_ARITY_ERROR;
            return 0;
        }
    }

    ctx->vm->reg[GTKML_R_PC].pc = program->value.s_program.addr;
    gboolean result = gtk_ml_vm_run(ctx->vm, err);

    gtk_ml_leave(ctx);

    return result;
}

GtkMl_S *gtk_ml_get_export(GtkMl_Context *ctx, const char **err, const char *linkage_name) {
    for (size_t i = 0; i < ctx->vm->program.n_exec;) {
        GtkMl_Instruction instr = ctx->vm->program.exec[i];
        if (instr.gen.category == GTKML_EI_EXPORT) {
            GtkMl_S *program = ctx->vm->program.statics[ctx->vm->program.exec[i + 1].imm64];
            const char *export = program->value.s_program.linkage_name;
            if (strcmp(linkage_name, export) == 0) {
                return program;
            }
        }
        if (instr.gen.category & GTKML_I_EXTENDED) {
            i += 2;
        } else {
            ++i;
        }
    }

    *err = GTKML_ERR_BINDING_ERROR;
    return NULL;
}

GtkMl_Builder *gtk_ml_new_builder() {
    GtkMl_Builder *b = malloc(sizeof(GtkMl_Builder));

    b->basic_blocks = malloc(sizeof(GtkMl_BasicBlock) * 64);
    b->len_bb = 0;
    b->cap_bb = 64;

    b->statics = malloc(sizeof(GtkMl_S *) * 64);
    b->statics[0] = NULL;
    b->len_static = 1;
    b->cap_static = 64;

    b->counter = 0;

    b->macro_ctx = gtk_ml_new_context();
    b->macro_vm = b->macro_ctx->vm;

    b->builders = malloc(sizeof(GtkMl_BuilderMacro) * 64);
    b->len_builder = 0;
    b->cap_builder = 64;

    char *name_lambda = malloc(strlen("lambda") + 1);
    strcpy(name_lambda, "lambda");
    b->builders[b->len_builder].name = name_lambda;
    b->builders[b->len_builder].fn = builder_lambda;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_vararg = malloc(strlen("vararg") + 1);
    strcpy(name_vararg, "vararg");
    b->builders[b->len_builder].name = name_vararg;
    b->builders[b->len_builder].fn = builder_vararg;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_quote = malloc(strlen("quote") + 1);
    strcpy(name_quote, "quote");
    b->builders[b->len_builder].name = name_quote;
    b->builders[b->len_builder].fn = builder_quote;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_quasiquote = malloc(strlen("quasiquote") + 1);
    strcpy(name_quasiquote, "quasiquote");
    b->builders[b->len_builder].name = name_quasiquote;
    b->builders[b->len_builder].fn = builder_quasiquote;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_unquote = malloc(strlen("unquote") + 1);
    strcpy(name_unquote, "unquote");
    b->builders[b->len_builder].name = name_unquote;
    b->builders[b->len_builder].fn = builder_unquote;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_macro = malloc(strlen("macro") + 1);
    strcpy(name_macro, "macro");
    b->builders[b->len_builder].name = name_macro;
    b->builders[b->len_builder].fn = builder_macro;
    b->builders[b->len_builder].require_macro = 1;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_define = malloc(strlen("define") + 1);
    strcpy(name_define, "define");
    b->builders[b->len_builder].name = name_define;
    b->builders[b->len_builder].fn = builder_define;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 0;
    ++b->len_builder;

    char *name_application = malloc(strlen("Application") + 1);
    strcpy(name_application, "Application");
    b->builders[b->len_builder].name = name_application;
    b->builders[b->len_builder].fn = builder_application;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 1;
    ++b->len_builder;

    char *name_new_window = malloc(strlen("new-window") + 1);
    strcpy(name_new_window, "new-window");
    b->builders[b->len_builder].name = name_new_window;
    b->builders[b->len_builder].fn = builder_new_window;
    b->builders[b->len_builder].require_macro = 0;
    b->builders[b->len_builder].require_runtime = 1;
    ++b->len_builder;

    return b;
}

gboolean build(GtkMl_Context *ctx, GtkMl_Program *out, const char **err, GtkMl_Builder *b, gboolean macro) {
    size_t n = 0;
    size_t n_static = b->len_static;
    for (size_t i = 0; i < b->len_bb; i++) {
        n += b->basic_blocks[i].len_exec;
    }

    GtkMl_Instruction *result = malloc(sizeof(GtkMl_Instruction) * n);
    GtkMl_S **statics = malloc(sizeof(GtkMl_S *) * n_static);
    memcpy(statics, b->statics, sizeof(GtkMl_S *) * n_static);
    uint64_t pc = 0;

    for (size_t i = 0; i < b->len_bb; i++) {
        size_t n = b->basic_blocks[i].len_exec;
        memcpy(result + pc, b->basic_blocks[i].exec, sizeof(GtkMl_Instruction) * n);
        pc += n;
    }

    for (size_t i = 0; i < n;) {
        GtkMl_Instruction instr = result[i];
        if (instr.gen.category == GTKML_EI_EXPORT) {
            GtkMl_S *program = statics[result[i + 1].imm64];
            program->value.s_program.addr = 8 * i;
        }
        if (instr.gen.category & GTKML_I_EXTENDED) {
            i += 2;
        } else {
            ++i;
        }
    }

    for (size_t i = 0; i < n;) {
        GtkMl_Instruction instr = result[i];
        if (instr.gen.category != GTKML_EI_EXPORT && (instr.gen.category & GTKML_EI_IMM_EXTERN)) {
            GtkMl_S *ext;
            if (instr.gen.category & GTKML_I_EXTENDED) {
                ext = statics[result[i + 1].imm64];
            } else if ((instr.gen.category & ~GTKML_EI_IMM_EXTERN) == GTKML_I_IMM) {
                ext = statics[instr.imm.imm];
            } else if ((instr.gen.category & ~GTKML_EI_IMM_EXTERN) == GTKML_I_BR) {
                ext = statics[instr.br.imm];
            } else {
                *err = GTKML_ERR_CATEGORY_ERROR;
                return 0;
            }
            const char *bb1 = ext->value.s_string.ptr;
            gboolean found = 0;

            for (size_t l = 0; l < n;) {
                GtkMl_Instruction instr = result[l];
                if (instr.gen.category == GTKML_EI_EXPORT) {
                    GtkMl_S *program = statics[result[l + 1].imm64];
                    const char *bb2 = program->value.s_program.linkage_name;
                    if (strcmp(bb1, bb2) == 0) {
                        result[i].gen.category &= ~GTKML_EI_IMM_EXTERN;
                        if (result[i].gen.category & GTKML_I_EXTENDED) {
                            result[i + 1].imm64 = result[l + 1].imm64;
                        } else if (result[i].gen.category == GTKML_I_IMM) {
                            result[i].imm.imm = result[l + 1].imm64;
                        } else if (result[i].gen.category == GTKML_I_BR) {
                            result[i].br.imm = result[l + 1].imm64;
                        } else {
                            *err = GTKML_ERR_CATEGORY_ERROR;
                            return 0;
                        }
                        found = 1;
                        break;
                    }
                }
                if (instr.gen.category & GTKML_I_EXTENDED) {
                    l += 2;
                } else {
                    ++l;
                }
            }

            if (!found) {
                *err = GTKML_ERR_LINKAGE_ERROR;
                return 0;
            }
        }
        if (instr.gen.category & GTKML_I_EXTENDED) {
            i += 2;
        } else {
            ++i;
        }
    }

    if (macro) {
        for (size_t i = 0; i < b->len_bb; i++) {
            free(b->basic_blocks[i].exec);
        }
        b->len_bb = 0;
        b->len_static = 1;

        out->start = NULL;
        out->exec = result;
        out->n_exec = n;
        out->statics = statics;
        out->n_static = n_static;
    } else {
        // first we transfer the values from the macro context into the real context
        GtkMl_S *macro_values = b->macro_ctx->first;
        b->macro_ctx->first = NULL;
        GtkMl_S **prev = &ctx->first;
        while (*prev) {
            prev = &(*prev)->next;
        }
        *prev = macro_values;
        // then it's safe to delete the macro context and vm
        gtk_ml_del_context(b->macro_ctx);

        for (size_t i = 0; i < b->len_bb; i++) {
            free(b->basic_blocks[i].exec);
        }
        for (size_t i = 0; i < b->len_builder; i++) {
            free((void *) b->builders[i].name);
        }
        free(b->builders);
        free(b->statics);
        free(b->basic_blocks);
        free(b);

        char *start = malloc(strlen("_start") + 1);
        strcpy(start, "_start");
        out->start = start;
        out->exec = result;
        out->n_exec = n;
        out->statics = statics;
        out->n_static = n_static;
    }

    return 1;
}

gboolean gtk_ml_build_macros(GtkMl_Program *out, const char **err, GtkMl_Builder *b) {
    return build(NULL, out, err, b, 1);
}

gboolean gtk_ml_build(GtkMl_Context *ctx, GtkMl_Program *out, const char **err, GtkMl_Builder *b) {
    return build(ctx, out, err, b, 0);
}

GTKML_PUBLIC void gtk_ml_del_program(GtkMl_Program* program) {
    free((void *) program->start);
    free(program->exec);
    free(program->statics);
}

GtkMl_BasicBlock *gtk_ml_append_basic_block(GtkMl_Builder *b, const char *name) {
    if (b->len_bb == b->cap_bb) {
        b->cap_bb *= 2;
        b->basic_blocks = realloc(b->basic_blocks, sizeof(GtkMl_BasicBlock) * b->cap_bb);
    }

    GtkMl_BasicBlock *basic_block = b->basic_blocks + b->len_bb;
    ++b->len_bb;

    basic_block->name = name;
    basic_block->exec = malloc(sizeof(GtkMl_Instruction) * 256);
    basic_block->len_exec = 0;
    basic_block->cap_exec = 256;

    return basic_block;
}

GtkMl_Static gtk_ml_append_static(GtkMl_Builder *b, GtkMl_S *value) {
    for (GtkMl_Static i = 0; i < b->len_static; i++) {
        if (b->statics[i] == value) {
            return i;
        }
    }

    if (b->len_static == b->cap_static) {
        b->cap_static *= 2;
        b->statics = realloc(b->statics, sizeof(GtkMl_S *) * b->cap_static);
    }

    GtkMl_Static handle = b->len_static;
    b->statics[handle] = value;
    ++b->len_static;

    return handle;
}

GTKML_PRIVATE gboolean IDENT_BEGIN[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

GTKML_PRIVATE gboolean IDENT_CONT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

GTKML_PRIVATE gboolean is_ident_begin(unsigned char c) {
    return IDENT_BEGIN[c];
}

GTKML_PRIVATE gboolean is_ident_cont(unsigned char c) {
    return IDENT_CONT[c];
}

GTKML_PRIVATE gboolean lex(GtkMl_Token **tokenv, size_t *tokenc, const char **err, const char *src) {
    *tokenv = malloc(sizeof(GtkMl_Token) * 64);
    *tokenc = 0;
    size_t cap = 64;

    int line = 1;
    int col = 1;

    while (*src) {
        if (*tokenc == cap) {
            cap *= 2;
            *tokenv = realloc(*tokenv, cap);
        }

        switch (*src) {
        case '\n':
            col = 1;
            ++line;
            ++src;
            continue;
        case '\t':
            ++col;
            ++src;
            continue;
        case ' ':
            ++col;
            ++src;
            continue;
        case ';':
            while (*src && *src != '\n') {
                ++col;
                ++src;
            }
            continue;
        case '#':
            (*tokenv)[*tokenc].kind = GTKML_TOK_POUND;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '.':
            if (*(src + 1) == '.' && *(src + 2) == '.') {
                (*tokenv)[*tokenc].kind = GTKML_TOK_ELLIPSIS;
                (*tokenv)[*tokenc].span.ptr = src;
                (*tokenv)[*tokenc].span.len = 3;
                (*tokenv)[*tokenc].span.line = line;
                (*tokenv)[*tokenc].span.col = col;
                ++*tokenc;
                col += 3;
                src += 3;
                continue;
            }
            (*tokenv)[*tokenc].kind = GTKML_TOK_DOT;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '\'':
            (*tokenv)[*tokenc].kind = GTKML_TOK_TICK;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '`':
            (*tokenv)[*tokenc].kind = GTKML_TOK_BACKTICK;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case ',':
            (*tokenv)[*tokenc].kind = GTKML_TOK_COMMA;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '(':
            (*tokenv)[*tokenc].kind = GTKML_TOK_PARENL;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case ')':
            (*tokenv)[*tokenc].kind = GTKML_TOK_PARENR;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '[':
            (*tokenv)[*tokenc].kind = GTKML_TOK_SQUAREL;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case ']':
            (*tokenv)[*tokenc].kind = GTKML_TOK_SQUARER;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '{':
            (*tokenv)[*tokenc].kind = GTKML_TOK_CURLYL;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '}':
            (*tokenv)[*tokenc].kind = GTKML_TOK_CURLYR;
            (*tokenv)[*tokenc].span.ptr = src;
            (*tokenv)[*tokenc].span.len = 1;
            (*tokenv)[*tokenc].span.line = line;
            (*tokenv)[*tokenc].span.col = col;
            ++*tokenc;
            break;
        case '"': {
            ++col;
            ++src;
            // TODO(walterpi): escape codes
            const char *str_ptr = src - 1;
            int str_line = line;
            int str_col = col - 1;
            size_t str_len = 1;
            do {
                ++str_len;
                ++col;
                ++src;
            } while (*src != '"');
            (*tokenv)[*tokenc].kind = GTKML_TOK_STRING;
            (*tokenv)[*tokenc].span.ptr = str_ptr;
            (*tokenv)[*tokenc].span.len = str_len + 1;
            (*tokenv)[*tokenc].span.line = str_line;
            (*tokenv)[*tokenc].span.col = str_col;
            ++*tokenc;
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            char *endptr;
            int64_t intval = strtoll(src, &endptr, 10);
            if (*endptr == ' ' || *endptr == '\n' || *endptr == '\t'
                    || *endptr == '(' || *endptr == ')'
                    || *endptr == '{' || *endptr == '}') {
                (*tokenv)[*tokenc].kind = GTKML_TOK_INT;
                (*tokenv)[*tokenc].value.intval = intval;
                (*tokenv)[*tokenc].span.ptr = src;
                (*tokenv)[*tokenc].span.len = 1;
                (*tokenv)[*tokenc].span.line = line;
                (*tokenv)[*tokenc].span.col = col;
                ++*tokenc;
                src = endptr;
                continue;
            } else if (*endptr == '.') {
                double floatval = strtod(src, &endptr);
                if (*endptr == ' ' || *endptr == '\n' || *endptr == '\t'
                        || *endptr == '(' || *endptr == ')'
                        || *endptr == '{' || *endptr == '}') {
                    (*tokenv)[*tokenc].kind = GTKML_TOK_FLOAT;
                    (*tokenv)[*tokenc].value.floatval = floatval;
                    (*tokenv)[*tokenc].span.ptr = src;
                    (*tokenv)[*tokenc].span.len = 1;
                    (*tokenv)[*tokenc].span.line = line;
                    (*tokenv)[*tokenc].span.col = col;
                    ++*tokenc;
                    src = endptr;
                    continue;
                } else {
                    *err = GTKML_ERR_CHARACTER_ERROR;
                    return 0;
                }
            } else {
                *err = GTKML_ERR_CHARACTER_ERROR;
                return 0;
            }
        } break;
        case ':':
            ++col;
            ++src;
            if (is_ident_begin(*src)) {
                const char *kw_ptr = src - 1;
                int kw_line = line;
                int kw_col = col - 1;
                size_t kw_len = 1;
                while (is_ident_cont(*src)) {
                    ++kw_len;
                    ++col;
                    ++src;
                }
                (*tokenv)[*tokenc].kind = GTKML_TOK_KEYWORD;
                (*tokenv)[*tokenc].span.ptr = kw_ptr;
                (*tokenv)[*tokenc].span.len = kw_len;
                (*tokenv)[*tokenc].span.line = kw_line;
                (*tokenv)[*tokenc].span.col = kw_col;
                ++*tokenc;
                continue;
            } else {
                *err = GTKML_ERR_CHARACTER_ERROR;
                return 0;
            }
            break;
        default:
            if (is_ident_begin(*src)) {
                const char *ident_ptr = src;
                int ident_line = line;
                int ident_col = col;
                size_t ident_len = 0;
                while (is_ident_cont(*src)) {
                    ++ident_len;
                    ++col;
                    ++src;
                }
                (*tokenv)[*tokenc].kind = GTKML_TOK_IDENT;
                (*tokenv)[*tokenc].span.ptr = ident_ptr;
                (*tokenv)[*tokenc].span.len = ident_len;
                (*tokenv)[*tokenc].span.line = ident_line;
                (*tokenv)[*tokenc].span.col = ident_col;
                ++*tokenc;
                continue;
            } else {
                *err = GTKML_ERR_CHARACTER_ERROR;
                return 0;
            }
            break;
        }

        ++col;
        ++src;
    }

    return 1;
}

GTKML_PRIVATE GtkMl_S *new_value(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_SKind kind) {
    ++ctx->n_values;

    GtkMl_S *s = malloc(sizeof(GtkMl_S));
    s->next = ctx->first;
    ctx->first = s;

    s->flags = GTKML_FLAG_NONE;
    s->kind = kind;
    if (span) {
        s->span = *span;
    } else {
        s->span = (GtkMl_Span) {0};
    }
    s->hashed = 0;

    return s;
}

GTKML_PRIVATE GtkMl_S *new_nil(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_NIL);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_true(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_TRUE);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_false(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_FALSE);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_int(GtkMl_Context *ctx, GtkMl_Span *span, int64_t value) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_INT);
    s->value.s_int.value = value;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_float(GtkMl_Context *ctx, GtkMl_Span *span, float value) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_FLOAT);
    s->value.s_float.value = value;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_string(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_STRING);
    s->value.s_string.ptr = ptr;
    s->value.s_string.len = len;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_symbol(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_SYMBOL);
    s->value.s_symbol.ptr = ptr;
    s->value.s_symbol.len = len;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_keyword(GtkMl_Context *ctx, GtkMl_Span *span, const char *ptr, size_t len) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_KEYWORD);
    s->value.s_keyword.ptr = ptr;
    s->value.s_keyword.len = len;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_list(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *car, GtkMl_S *cdr) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_LIST);
    s->value.s_list.car = car;
    s->value.s_list.cdr = cdr;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_map(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_MAP);
    gtk_ml_new_hash_trie(&s->value.s_map.map);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_set(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_SET);
    gtk_ml_new_hash_set(&s->value.s_set.set);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_array(GtkMl_Context *ctx, GtkMl_Span *span) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_ARRAY);
    gtk_ml_new_array(&s->value.s_array.array);
    return s;
}

GTKML_PRIVATE GtkMl_S *new_vararg(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_VARARG);
    s->value.s_var.expr = expr;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_quote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_QUOTE);
    s->value.s_var.expr = expr;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_quasiquote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_QUASIQUOTE);
    s->value.s_var.expr = expr;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_unquote(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *expr) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_UNQUOTE);
    s->value.s_var.expr = expr;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_lambda(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_LAMBDA);
    s->value.s_lambda.args = args;
    s->value.s_lambda.body = body;
    s->value.s_lambda.capture = capture;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_program(GtkMl_Context *ctx, GtkMl_Span *span, const char *linkage_name, uint64_t addr, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_PROGRAM);
    s->value.s_program.linkage_name = linkage_name;
    s->value.s_program.addr = addr;
    s->value.s_program.args = args;
    s->value.s_program.body = body;
    s->value.s_program.capture = capture;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_macro(GtkMl_Context *ctx, GtkMl_Span *span, GtkMl_S *args, GtkMl_S *body, GtkMl_S *capture) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_MACRO);
    s->value.s_macro.args = args;
    s->value.s_macro.body = body;
    s->value.s_macro.capture = capture;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_lightdata(GtkMl_Context *ctx, GtkMl_Span *span, void *data) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_LIGHTDATA);
    s->value.s_lightdata.userdata = data;
    return s;
}

GTKML_PRIVATE GtkMl_S *new_userdata(GtkMl_Context *ctx, GtkMl_Span *span, void *data, void (*del)(GtkMl_Context *ctx, void *)) {
    GtkMl_S *s = new_value(ctx, span, GTKML_S_USERDATA);
    s->value.s_userdata.userdata = data;
    s->value.s_userdata.del = del;
    s->value.s_userdata.keep = new_nil(ctx, span);
    return s;
}

GTKML_PRIVATE void span_add(GtkMl_Span *dest, GtkMl_Span *lhs, GtkMl_Span *rhs) {
    size_t len = rhs->ptr - lhs->ptr + rhs->len;
    const char *ptr = lhs->ptr;
    int line = lhs->line;
    int col = lhs->col;
    dest->ptr = ptr;
    dest->len = len;
    dest->line = line;
    dest->col = col;
}

GtkMl_S *parse_vararg(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_ELLIPSIS) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    GtkMl_S *expr = parse(ctx, err, tokenv, tokenc);
    if (!expr) {
        return NULL;
    }

    span_add(&span, &span, &expr->span);

    return new_vararg(ctx, &span, expr);
}

GtkMl_S *parse_quote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_TICK) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    GtkMl_S *expr = parse(ctx, err, tokenv, tokenc);
    if (!expr) {
        return NULL;
    }

    span_add(&span, &span, &expr->span);

    return new_quote(ctx, &span, expr);
}

GtkMl_S *parse_quasiquote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_BACKTICK) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    GtkMl_S *expr = parse(ctx, err, tokenv, tokenc);
    if (!expr) {
        return NULL;
    }

    span_add(&span, &span, &expr->span);

    return new_quasiquote(ctx, &span, expr);
}

GtkMl_S *parse_unquote(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_COMMA) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    GtkMl_S *expr = parse(ctx, err, tokenv, tokenc);
    if (!expr) {
        return NULL;
    }

    span_add(&span, &span, &expr->span);

    return new_unquote(ctx, &span, expr);
}

GTKML_PRIVATE GtkMl_S *parse_set(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_CURLYL) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    if (*tokenc == 0) {
        *err = GTKML_ERR_EOF_ERROR;
        return NULL;
    }

    GtkMl_S *result = new_set(ctx, &span);

    while ((*tokenv)[0].kind != GTKML_TOK_CURLYR) {
        GtkMl_S *key = parse(ctx, err, tokenv, tokenc);
        if (!key) {
            return NULL;
        }

        if (*tokenc == 0) {
            *err = GTKML_ERR_EOF_ERROR;
            return NULL;
        }

        span_add(&span, &span, &(*tokenv)[0].span);

        GtkMl_S *set = result;
        result = new_set(ctx, &span);
        gtk_ml_hash_set_insert(&result->value.s_set.set, &set->value.s_set.set, key);
    }

    result->span = span;

    ++*tokenv;
    --*tokenc;

    return result;
}

GTKML_PRIVATE GtkMl_S *parse_constant(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_IDENT) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    GtkMl_S *result = NULL;
    if (strncmp((*tokenv)[0].span.ptr, "nil", (*tokenv)[0].span.len) == 0) {
        result = new_nil(ctx, &(*tokenv)[0].span);
    } else if (strncmp((*tokenv)[0].span.ptr, "t", (*tokenv)[0].span.len) == 0) {
        result = new_true(ctx, &(*tokenv)[0].span);
    } else if (strncmp((*tokenv)[0].span.ptr, "f", (*tokenv)[0].span.len) == 0) {
        result = new_false(ctx, &(*tokenv)[0].span);
    } else {
        *err = GTKML_ERR_CONSTANT_ERROR;
    }
    ++*tokenv;
    --*tokenc;
    return result;
}

GTKML_PRIVATE GtkMl_S *parse_alternative(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_POUND) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    GtkMl_S *expr;
    switch ((*tokenv)[0].kind) {
    case GTKML_TOK_CURLYL:
        expr = parse_set(ctx, err, tokenv, tokenc);
        break;
    case GTKML_TOK_IDENT:
        expr = parse_constant(ctx, err, tokenv, tokenc);
        break;
    default:
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    span_add(&span, &span, &expr->span);
    if (expr) {
        expr->span = span;
    }

    return expr;
}

GTKML_PRIVATE GtkMl_S *parse_list_rest(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if (*tokenc == 0) {
        *err = GTKML_ERR_EOF_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    if ((*tokenv)[0].kind == GTKML_TOK_PARENR) {
        span_add(&span, &span, &(*tokenv)[0].span);

        ++*tokenv;
        --*tokenc;

        return new_nil(ctx, &span);
    }

    GtkMl_S *car = parse(ctx, err, tokenv, tokenc);
    GtkMl_S *cdr = parse_list_rest(ctx, err, tokenv, tokenc);
    if (!cdr) {
        return NULL;
    }

    span_add(&span, &span, &cdr->span);

    return new_list(ctx, &span, car, cdr);
}

GTKML_PRIVATE GtkMl_S *parse_list(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_PARENL) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    ++*tokenv;
    --*tokenc;

    return parse_list_rest(ctx, err, tokenv, tokenc);
}

GTKML_PRIVATE GtkMl_S *parse_array(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_SQUAREL) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    if (*tokenc == 0) {
        *err = GTKML_ERR_EOF_ERROR;
        return NULL;
    }

    GtkMl_S *result = new_array(ctx, &span);

    while ((*tokenv)[0].kind != GTKML_TOK_SQUARER) {
        GtkMl_S *elem = parse(ctx, err, tokenv, tokenc);
        if (!elem) {
            return NULL;
        }

        if (*tokenc == 0) {
            *err = GTKML_ERR_EOF_ERROR;
            return NULL;
        }

        span_add(&span, &span, &(*tokenv)[0].span);

        GtkMl_S *array = result;
        result = new_array(ctx, &span);
        gtk_ml_array_push(&result->value.s_array.array, &array->value.s_array.array, elem);
    }

    result->span = span;

    ++*tokenv;
    --*tokenc;

    return result;
}

GTKML_PRIVATE GtkMl_S *parse_map(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_CURLYL) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }

    GtkMl_Span span = (*tokenv)[0].span;

    ++*tokenv;
    --*tokenc;

    if (*tokenc == 0) {
        *err = GTKML_ERR_EOF_ERROR;
        return NULL;
    }

    GtkMl_S *result = new_map(ctx, &span);

    while ((*tokenv)[0].kind != GTKML_TOK_CURLYR) {
        GtkMl_S *key = parse(ctx, err, tokenv, tokenc);
        if (!key) {
            return NULL;
        }

        if (*tokenc == 0) {
            *err = GTKML_ERR_EOF_ERROR;
            return NULL;
        }

        GtkMl_S *value = parse(ctx, err, tokenv, tokenc);
        if (!value) {
            return NULL;
        }

        if (*tokenc == 0) {
            *err = GTKML_ERR_EOF_ERROR;
            return NULL;
        }

        span_add(&span, &span, &(*tokenv)[0].span);

        GtkMl_S *map = result;
        result = new_map(ctx, &span);
        gtk_ml_hash_trie_insert(&result->value.s_map.map, &map->value.s_map.map, key, value);
    }

    result->span = span;

    ++*tokenv;
    --*tokenc;

    return result;
}

GTKML_PRIVATE GtkMl_S *parse_int(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_INT) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    GtkMl_S *result = new_int(ctx, &(*tokenv)[0].span, (*tokenv)[0].value.intval);
    ++*tokenv;
    --*tokenc;
    return result;
}

GTKML_PRIVATE GtkMl_S *parse_float(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_FLOAT) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    GtkMl_S *result = new_float(ctx, &(*tokenv)[0].span, (*tokenv)[0].value.floatval);
    ++*tokenv;
    --*tokenc;
    return result;
}

GTKML_PRIVATE GtkMl_S *parse_string(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_STRING) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    size_t len = (*tokenv)[0].span.len - 2;
    char *ptr = malloc(len + 1);
    memcpy(ptr, (*tokenv)[0].span.ptr + 1, len);
    ptr[len] = 0;
    GtkMl_S *result = new_string(ctx, &(*tokenv)[0].span, ptr, len);
    ++*tokenv;
    --*tokenc;
    return result;
}

GTKML_PRIVATE GtkMl_S *parse_symbol(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_IDENT) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    GtkMl_S *result = new_symbol(ctx, &(*tokenv)[0].span, (*tokenv)[0].span.ptr, (*tokenv)[0].span.len);
    ++*tokenv;
    --*tokenc;
    return result;
}

GTKML_PRIVATE GtkMl_S *parse_keyword(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    if ((*tokenv)[0].kind != GTKML_TOK_KEYWORD) {
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
    GtkMl_S *result = new_keyword(ctx, &(*tokenv)[0].span, (*tokenv)[0].span.ptr + 1, (*tokenv)[0].span.len - 1);
    ++*tokenv;
    --*tokenc;
    return result;
}

GtkMl_S *parse(GtkMl_Context *ctx, const char **err, GtkMl_Token **tokenv, size_t *tokenc) {
    switch ((*tokenv)[0].kind) {
    case GTKML_TOK_PARENL:
        return parse_list(ctx, err, tokenv, tokenc);
    case GTKML_TOK_SQUAREL:
        return parse_array(ctx, err, tokenv, tokenc);
    case GTKML_TOK_CURLYL:
        return parse_map(ctx, err, tokenv, tokenc);
    case GTKML_TOK_INT:
        return parse_int(ctx, err, tokenv, tokenc);
    case GTKML_TOK_FLOAT:
        return parse_float(ctx, err, tokenv, tokenc);
    case GTKML_TOK_STRING:
        return parse_string(ctx, err, tokenv, tokenc);
    case GTKML_TOK_IDENT:
        return parse_symbol(ctx, err, tokenv, tokenc);
    case GTKML_TOK_KEYWORD:
        return parse_keyword(ctx, err, tokenv, tokenc);
    default:
        for (size_t i = 0; i < ctx->parser.len_reader; i++) {
            if (ctx->parser.readers[i].token == (*tokenv)[0].kind) {
                return ctx->parser.readers[i].fn(ctx, err, tokenv, tokenc);
            }
        }
        *err = GTKML_ERR_TOKEN_ERROR;
        return NULL;
    }
}

GtkMl_S *gtk_ml_load(GtkMl_Context *ctx, char **src, const char **err, const char *file) {
    FILE *stream = fopen(file, "r");
    if (!stream) {
        *err = GTKML_ERR_IO_ERROR;
        return NULL;
    }
    GtkMl_S *result = gtk_ml_loadf(ctx, src, err, stream);
    fclose(stream);
    return result;
}

GtkMl_S *gtk_ml_loadf(GtkMl_Context *ctx, char **src, const char **err, FILE *stream) {
    fseek(stream, 0l, SEEK_END);
    size_t size = ftell(stream);
    fseek(stream, 0l, SEEK_SET);
    *src = malloc(size + 1);
    size_t read = fread(*src, 1, size, stream);
    if (read != size) {
        *err = GTKML_ERR_IO_ERROR;
        return NULL;
    }
    (*src)[size] = 0;
    return gtk_ml_loads(ctx, err, *src);
}

GtkMl_S *gtk_ml_loads(GtkMl_Context *ctx, const char **err, const char *src) {
    GtkMl_Token *tokenv;
    size_t tokenc;
    if (!lex(&tokenv, &tokenc, err, src)) {
        return NULL;
    }
    GtkMl_Token *_tokenv = tokenv;

    GtkMl_S *body = new_nil(ctx, NULL);
    GtkMl_S **last = &body;

    while (tokenc) {
        GtkMl_S *line = parse(ctx, err, &_tokenv, &tokenc);
        if (!line) {
            return NULL;
        }
        GtkMl_S *new = new_list(ctx, NULL, line, *last);
        *last = new;
        last = &gtk_ml_cdr(new);
    }

    GtkMl_S *result = new_lambda(ctx, NULL, new_nil(ctx, NULL), body, new_nil(ctx, NULL));

    free(tokenv);
    return result;
}

GTKML_PRIVATE gboolean compile_std_call(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, uint64_t function, GtkMl_S *args, gboolean compile_first);
GTKML_PRIVATE gboolean compile_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt);
GTKML_PRIVATE gboolean compile_program(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, const char *linkage_name, GtkMl_S *stmt, gboolean ret);

GTKML_PRIVATE gboolean compile_macro_quasi_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S *stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean compile_macro_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean compile_macro_body(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S *stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);
GTKML_PRIVATE gboolean compile_macro_program(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, const char *linkage_name, GtkMl_S *stmt, gboolean ret, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion);

gboolean builder_application(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    (void) allow_macro;
    (void) allow_runtime;
    (void) allow_macro_expansion;

    GtkMl_S *args = gtk_ml_cdr(*stmt);
    if (args->kind == GTKML_S_NIL
            || gtk_ml_cdr(args)->kind == GTKML_S_NIL
            || gtk_ml_cddr(args)->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    return compile_std_call(ctx, b, basic_block, err, GTKML_STD_APPLICATION, *stmt, 0);
}

gboolean builder_new_window(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    (void) allow_macro;
    (void) allow_runtime;
    (void) allow_macro_expansion;

    GtkMl_S *args = gtk_ml_cdr(*stmt);
    if (args->kind == GTKML_S_NIL
            || gtk_ml_cdr(args)->kind == GTKML_S_NIL
            || gtk_ml_cddr(args)->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    return compile_std_call(ctx, b, basic_block, err, GTKML_STD_NEW_WINDOW, *stmt, 0);
}

gboolean builder_define(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    (void) ctx;
    (void) b;
    (void) basic_block;
    (void) stmt;
    (void) allow_macro;
    (void) allow_runtime;
    (void) allow_macro_expansion;

    *err = GTKML_ERR_UNIMPLEMENTED;
    return 0;
}

gboolean builder_lambda(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL
            || gtk_ml_cdr(args)->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *lambda_args = gtk_ml_car(args);
    GtkMl_S *lambda_body = gtk_ml_cdr(args);

    GtkMl_S *lambda = new_lambda(ctx, &(*stmt)->span, lambda_args, lambda_body, local_scope(ctx));
    if (!lambda) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &lambda, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean builder_macro(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL
            || gtk_ml_cdr(args)->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *macro_args = gtk_ml_car(args);
    GtkMl_S *macro_body = gtk_ml_cdr(args);

    GtkMl_S *macro = new_macro(ctx, &(*stmt)->span, macro_args, macro_body, local_scope(ctx));
    if (!macro) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &macro, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean builder_vararg(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *vararg = new_vararg(ctx, &(*stmt)->span, gtk_ml_car(args));
    if (!vararg) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &vararg, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean builder_quote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *quote = new_quote(ctx, &(*stmt)->span, gtk_ml_car(args));
    if (!quote) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &quote, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean builder_quasiquote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *quasiquote = new_quasiquote(ctx, &(*stmt)->span, gtk_ml_car(args));
    if (!quasiquote) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &quasiquote, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean builder_unquote(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    GtkMl_S *args = gtk_ml_cdr(*stmt);

    if (args->kind == GTKML_S_NIL) {
        *err = GTKML_ERR_ARITY_ERROR;
        return 0;
    }

    GtkMl_S *unquote = new_unquote(ctx, &(*stmt)->span, gtk_ml_car(args));
    if (!unquote) {
        return 0;
    }

    return compile_macro_expression(ctx, b, basic_block, err, &unquote, allow_macro, allow_runtime, allow_macro_expansion);
}

gboolean compile_std_call(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, uint64_t function, GtkMl_S *args, gboolean compile_first) {
    int64_t n = 0;
    if (!compile_first) {
        if (!gtk_ml_build_push_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, gtk_ml_car(args)))) {
            return 0;
        }
        args = gtk_ml_cdr(args);
        ++n;
    }
    while (args->kind != GTKML_S_NIL) {
        if (!compile_expression(ctx, b, basic_block, err, &gtk_ml_car(args))) {
            return 0;
        }
        args = gtk_ml_cdr(args);
        ++n;
    }

    if (!gtk_ml_build_push_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)))) {
        return 0;
    }
    if (!gtk_ml_build_call_extended_std(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, function)))) {
        return 0;
    }

    return 1;
}

struct CompileData {
    GtkMl_Context *ctx;
    GtkMl_Builder *b;
    GtkMl_BasicBlock *basic_block;
    const char **err;
    GtkMl_S *stmt;
    gboolean allow_macro;
    gboolean allow_runtime;
    gboolean allow_macro_expansion;
    gboolean result;
};

GTKML_PRIVATE GtkMl_VisitResult compile_quasi_map(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *_data) {
    (void) ht;

    struct CompileData *data = _data;
    if (!compile_macro_quasi_expression(data->ctx, data->b, data->basic_block, data->err, key, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    if (!compile_macro_quasi_expression(data->ctx, data->b, data->basic_block, data->err, value, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult compile_quasi_set(GtkMl_HashSet *hs, GtkMl_S *key, void *_data) {
    (void) hs;

    struct CompileData *data = _data;
    if (!compile_macro_quasi_expression(data->ctx, data->b, data->basic_block, data->err, key, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult compile_quasi_array(GtkMl_Array *array, size_t idx, GtkMl_S *value, void *_data) {
    (void) array;
    (void) idx;

    struct CompileData *data = _data;
    if (!compile_macro_quasi_expression(data->ctx, data->b, data->basic_block, data->err, value, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult compile_map(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *_data) {
    (void) ht;

    struct CompileData *data = _data;
    if (!compile_macro_expression(data->ctx, data->b, data->basic_block, data->err, &key, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    if (!compile_macro_expression(data->ctx, data->b, data->basic_block, data->err, &value, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult compile_set(GtkMl_HashSet *hs, GtkMl_S *key, void *_data) {
    (void) hs;

    struct CompileData *data = _data;
    if (!compile_macro_expression(data->ctx, data->b, data->basic_block, data->err, &key, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult compile_array(GtkMl_Array *array, size_t idx, GtkMl_S *value, void *_data) {
    (void) array;
    (void) idx;

    struct CompileData *data = _data;
    if (!compile_macro_expression(data->ctx, data->b, data->basic_block, data->err, &value, data->allow_macro, data->allow_runtime, data->allow_macro_expansion)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    return GTKML_VISIT_RECURSE;
}

gboolean compile_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt) {
    return compile_macro_expression(ctx, b, basic_block, err, stmt, 0, 1, 1);
}

gboolean compile_program(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, const char *linkage_name, GtkMl_S *lambda, gboolean ret) {
    return compile_macro_program(ctx, b, basic_block, err, linkage_name, lambda, ret, 0, 1, 1);
}

gboolean compile_macro_quasi_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S *stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    switch (stmt->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
    case GTKML_S_INT:
    case GTKML_S_FLOAT:
    case GTKML_S_STRING:
    case GTKML_S_KEYWORD:
    case GTKML_S_PROGRAM:
    case GTKML_S_ADDRESS:
    case GTKML_S_LIGHTDATA:
    case GTKML_S_USERDATA:
    case GTKML_S_SYMBOL:
    case GTKML_S_LAMBDA:
    case GTKML_S_MACRO:
    case GTKML_S_VARARG:
    case GTKML_S_QUOTE:
    case GTKML_S_QUASIQUOTE:
        return gtk_ml_build_push_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, stmt));
    case GTKML_S_MAP: {
        int64_t n = gtk_ml_hash_trie_len(&stmt->value.s_map.map);
        struct CompileData data = { ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_hash_trie_foreach(&stmt->value.s_map.map, compile_quasi_map, &data);
        return gtk_ml_build_map_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_SET: {
        int64_t n = gtk_ml_hash_set_len(&stmt->value.s_set.set);
        struct CompileData data = { ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_hash_set_foreach(&stmt->value.s_set.set, compile_quasi_set, &data);
        return gtk_ml_build_set_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_ARRAY: {
        int64_t n = gtk_ml_array_len(&stmt->value.s_array.array);
        struct CompileData data = { ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_array_foreach(&stmt->value.s_array.array, compile_quasi_array, &data);
        return gtk_ml_build_array_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_UNQUOTE:
        return compile_macro_expression(ctx, b, basic_block, err, &stmt->value.s_unquote.expr, allow_macro, allow_runtime, allow_macro_expansion);
    case GTKML_S_LIST: {
        GtkMl_S *function = gtk_ml_car(stmt);

        GtkMl_S *args = gtk_ml_cdr(stmt);

        if (function->kind == GTKML_S_SYMBOL) {
            const char *symbol_unquote = "unquote";

            if (function->value.s_symbol.len == strlen(symbol_unquote)
                    && memcmp(function->value.s_symbol.ptr, symbol_unquote, function->value.s_symbol.len) == 0) {
                if (args->kind == GTKML_S_NIL) {
                    *err = GTKML_ERR_ARITY_ERROR;
                    return 0;
                }

                return compile_macro_expression(ctx, b, basic_block, err, &gtk_ml_car(args), allow_macro, allow_runtime, allow_macro_expansion);
            }
        }

        int64_t n = 0;
        while (stmt->kind != GTKML_S_NIL) {
            if (!compile_macro_quasi_expression(ctx, b, basic_block, err, gtk_ml_car(stmt), allow_macro, allow_runtime, allow_macro_expansion)) {
                return 0;
            }
            stmt = gtk_ml_cdr(stmt);
            ++n;
        }
        return gtk_ml_build_list_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    }
}

gboolean compile_macro_expression(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S **stmt, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    switch ((*stmt)->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
    case GTKML_S_INT:
    case GTKML_S_FLOAT:
    case GTKML_S_STRING:
    case GTKML_S_KEYWORD:
    case GTKML_S_PROGRAM:
    case GTKML_S_ADDRESS:
    case GTKML_S_LIGHTDATA:
    case GTKML_S_USERDATA:
        return gtk_ml_build_push_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, *stmt));
    case GTKML_S_SYMBOL:
        return gtk_ml_build_get_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, *stmt));
    case GTKML_S_LAMBDA: {
        char *linkage_name = malloc(strlen("lambda$") + 16);
        snprintf(linkage_name, strlen("lambda$") + 16, "lambda$%u", b->counter++);
        GtkMl_BasicBlock *bb = gtk_ml_append_basic_block(b, linkage_name);
        if (!compile_macro_program(ctx, b, bb, err, linkage_name, *stmt, 1, allow_macro, allow_runtime, allow_macro_expansion)) {
            return 0;
        }
        return gtk_ml_build_push_extended_addr(ctx, b, basic_block, err, gtk_ml_append_static(b, new_string(ctx, NULL, linkage_name, strlen(linkage_name))));
    }
    case GTKML_S_MACRO: if (allow_macro) {
        char *linkage_name = malloc(strlen("macro$") + 16);
        snprintf(linkage_name, strlen("macro$") + 16, "macro$%u", b->counter++);
        GtkMl_BasicBlock *bb = gtk_ml_append_basic_block(b, linkage_name);
        if (!compile_macro_program(ctx, b, bb, err, linkage_name, *stmt, 1, allow_macro, allow_runtime, allow_macro_expansion)) {
            return 0;
        }
        return gtk_ml_build_push_extended_addr(ctx, b, basic_block, err, gtk_ml_append_static(b, new_string(ctx, NULL, linkage_name, strlen(linkage_name))));
    }
    case GTKML_S_MAP: {
        int64_t n = gtk_ml_hash_trie_len(&(*stmt)->value.s_map.map);
        struct CompileData data = { ctx, b, basic_block, err, (*stmt), allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_hash_trie_foreach(&(*stmt)->value.s_map.map, compile_map, &data);
        return gtk_ml_build_map_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_SET: {
        int64_t n = gtk_ml_hash_set_len(&(*stmt)->value.s_set.set);
        struct CompileData data = { ctx, b, basic_block, err, (*stmt), allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_hash_set_foreach(&(*stmt)->value.s_set.set, compile_set, &data);
        return gtk_ml_build_set_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_ARRAY: {
        int64_t n = gtk_ml_array_len(&(*stmt)->value.s_array.array);
        struct CompileData data = { ctx, b, basic_block, err, (*stmt), allow_macro, allow_runtime, allow_macro_expansion, 1 }; 
        gtk_ml_array_foreach(&(*stmt)->value.s_array.array, compile_array, &data);
        return gtk_ml_build_array_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, new_int(ctx, NULL, n)));
    }
    case GTKML_S_VARARG:
        *err = GTKML_ERR_UNIMPLEMENTED;
        return 0;
    case GTKML_S_QUOTE:
        return gtk_ml_build_push_extended_imm(ctx, b, basic_block, err, gtk_ml_append_static(b, (*stmt)->value.s_quote.expr));
    case GTKML_S_QUASIQUOTE:
        return compile_macro_quasi_expression(ctx, b, basic_block, err, (*stmt)->value.s_quasiquote.expr, allow_macro, allow_runtime, allow_macro_expansion);
    case GTKML_S_UNQUOTE:
        *err = GTKML_ERR_UNQUOTE_ERROR;
        return 0;
    case GTKML_S_LIST: {
        GtkMl_S *function = gtk_ml_car(*stmt);
        GtkMl_S *args = gtk_ml_cdr(*stmt);

        if (function->kind == GTKML_S_SYMBOL) {
            size_t len = function->value.s_symbol.len;
            const char *ptr = function->value.s_symbol.ptr;

            for (size_t i = 0; i < b->len_builder; i++) {
                GtkMl_BuilderMacro *bm = b->builders + i;
                if (strlen(bm->name) == len && strncmp(bm->name, ptr, len) == 0) {
                    if (bm->require_macro) {
                        if (allow_macro) {
                            return bm->fn(ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion);
                        }
                    } else if (bm->require_runtime) {
                        if (allow_runtime) {
                            return bm->fn(ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion);
                        }
                    } else {
                        return bm->fn(ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion);
                    }
                }
            }
        }

        if (allow_macro_expansion && function->kind == GTKML_S_SYMBOL) {
            size_t len = function->value.s_symbol.len;
            char *linkage_name = malloc(len + 1);
            memcpy(linkage_name, function->value.s_symbol.ptr, len);
            linkage_name[len] = 0;

            const char *_err = NULL;
            GtkMl_S *program = gtk_ml_get_export(b->macro_ctx, &_err, linkage_name);
            if (program) {
                if (!gtk_ml_run_program(b->macro_ctx, err, program, args)) {
                    return 0;
                }

                GtkMl_S *result = gtk_ml_peek(b->macro_ctx);

                if (!result) {
                    return 0;
                }

                *stmt = result;

                return compile_macro_expression(ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion);
            }
        }

        *err = GTKML_ERR_UNIMPLEMENTED;
        return 0;
    }
    }
}

gboolean compile_macro_body(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_S *body, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    while (body->kind != GTKML_S_NIL) {
        GtkMl_S **stmt = &gtk_ml_car(body);
        if (!compile_macro_expression(ctx, b, basic_block, err, stmt, allow_macro, allow_runtime, allow_macro_expansion)) {
            return 0;
        }
        body = gtk_ml_cdr(body);
    }

    return 1;
}

gboolean compile_macro_program(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, const char *linkage_name, GtkMl_S *lambda, gboolean ret, gboolean allow_macro, gboolean allow_runtime, gboolean allow_macro_expansion) {
    if ((!allow_macro || lambda->kind != GTKML_S_MACRO) && lambda->kind != GTKML_S_LAMBDA) {
        *err = GTKML_ERR_PROGRAM_ERROR;
        return 0;
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_EXPORT;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    char *name = malloc(strlen(linkage_name) + 1);
    strcpy(name, linkage_name);
    GtkMl_S *program = new_program(ctx, &lambda->span, name, 0, lambda->value.s_lambda.args, lambda->value.s_lambda.body, lambda->value.s_lambda.capture);
    basic_block->exec[basic_block->len_exec].imm64 = gtk_ml_append_static(b, program);
    ++basic_block->len_exec;

    if (!compile_macro_body(ctx, b, basic_block, err, lambda->value.s_lambda.body, allow_macro, allow_runtime, allow_macro_expansion)) {
        return 0;
    }

    if (ret) {
        if (!gtk_ml_build_ret(ctx, b, basic_block, err)) {
            return 0;
        }
    }

    return 1;
}

gboolean gtk_ml_build_halt(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].arith.cond = 0;
    basic_block->exec[basic_block->len_exec].arith.category = GTKML_I_ARITH;
    basic_block->exec[basic_block->len_exec].arith.opcode = GTKML_IA_HALT;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_push_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_push_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_PUSH_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_push_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_PUSH_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_push_extended_addr(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static linkage_name) {
    if (linkage_name < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_push_addr(ctx, b, basic_block, err, linkage_name);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM | GTKML_EI_IMM_EXTERN;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_PUSH_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = linkage_name;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_push_addr(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static linkage_name) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM | GTKML_I_IMM_EXTERN;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_PUSH_IMM;

    basic_block->exec[basic_block->len_exec].imm.imm = linkage_name;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_get_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_get_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_GET_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_get_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_GET_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_list_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_list_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_LIST_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_list_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_LIST_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_map_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_map_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_MAP_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_map_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_MAP_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_set_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_set_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_SET_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_set_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_SET_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_array_extended_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 32)) {
        return gtk_ml_build_array_imm(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_EI_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_EII_ARRAY_EXT_IMM;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_array_imm(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].imm.cond = 0;
    basic_block->exec[basic_block->len_exec].imm.category = GTKML_I_IMM;
    basic_block->exec[basic_block->len_exec].imm.opcode = GTKML_II_ARRAY_IMM;
    basic_block->exec[basic_block->len_exec].imm.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_call_extended_std(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    if (imm64 < ((GtkMl_Static) 1 << 48)) {
        return gtk_ml_build_call_std(ctx, b, basic_block, err, imm64);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].br.cond = 0;
    basic_block->exec[basic_block->len_exec].br.category = GTKML_EI_BR;
    basic_block->exec[basic_block->len_exec].br.opcode = GTKML_EIBR_CALL_EXT_STD;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_call_std(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static imm64) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].br.cond = 0;
    basic_block->exec[basic_block->len_exec].br.category = GTKML_I_BR;
    basic_block->exec[basic_block->len_exec].br.opcode = GTKML_IBR_CALL_STD;
    basic_block->exec[basic_block->len_exec].br.imm = imm64;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_call_extended(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static program) {
    if (program < ((GtkMl_Static) 1 << 48)) {
        return gtk_ml_build_call(ctx, b, basic_block, err, program);
    }

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = 0;
    basic_block->exec[basic_block->len_exec].br.cond = 0;
    basic_block->exec[basic_block->len_exec].br.category = GTKML_EI_BR;
    basic_block->exec[basic_block->len_exec].br.opcode = GTKML_EIBR_CALL_EXT;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = program;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_call(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err, GtkMl_Static program) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].imm64 = 0;
    basic_block->exec[basic_block->len_exec].br.cond = 0;
    basic_block->exec[basic_block->len_exec].br.category = GTKML_I_BR;
    basic_block->exec[basic_block->len_exec].br.opcode = GTKML_IBR_CALL;
    basic_block->exec[basic_block->len_exec].br.imm = program;
    ++basic_block->len_exec;

    return 1;
}

gboolean gtk_ml_build_ret(GtkMl_Context *ctx, GtkMl_Builder *b, GtkMl_BasicBlock *basic_block, const char **err) {
    (void) ctx;
    (void) b;
    (void) err;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    basic_block->exec[basic_block->len_exec].instr = 0;
    basic_block->exec[basic_block->len_exec].br.cond = 0;
    basic_block->exec[basic_block->len_exec].br.category = GTKML_I_BR;
    basic_block->exec[basic_block->len_exec].br.opcode = GTKML_IBR_RET;
    ++basic_block->len_exec;

    if (basic_block->len_exec == basic_block->cap_exec) {
        basic_block->cap_exec *= 2;
        basic_block->exec = realloc(basic_block->exec, sizeof(GtkMl_Instruction) * basic_block->cap_exec);
    }

    return 1;
}

gboolean gtk_ml_compile_macros(GtkMl_Builder *b, const char **err, GtkMl_S *lambda) {
    GtkMl_S **prev = &lambda->value.s_lambda.body;
    GtkMl_S *body = *prev;
    while (body->kind != GTKML_S_NIL) {
        GtkMl_S *stmt = gtk_ml_car(body);

        GtkMl_S *function = gtk_ml_car(stmt);

        GtkMl_S *args = gtk_ml_cdr(stmt);

        if (function->kind == GTKML_S_SYMBOL) {
            const char *symbol_define_macro = "define-macro";

            if (function->value.s_symbol.len == strlen(symbol_define_macro)
                    && memcmp(function->value.s_symbol.ptr, symbol_define_macro, function->value.s_symbol.len) == 0) {
                *prev = gtk_ml_cdr(body);
                body = *prev;

                GtkMl_S *macro_definition = gtk_ml_car(args);
                GtkMl_S *macro_name = gtk_ml_car(macro_definition);
                GtkMl_S *macro_args = gtk_ml_cdr(macro_definition);
                GtkMl_S *macro_body = gtk_ml_cdr(args);

                GtkMl_S *macro = new_macro(b->macro_ctx, NULL, macro_args, macro_body, local_scope(b->macro_ctx));
                if (!macro) {
                    return 0;
                }

                size_t len = macro_name->value.s_symbol.len;
                char *linkage_name = malloc(len + 1);
                memcpy(linkage_name, macro_name->value.s_symbol.ptr, len);
                linkage_name[len] = 0;

                GtkMl_BasicBlock *basic_block = gtk_ml_append_basic_block(b, linkage_name);

                if (!compile_macro_program(b->macro_ctx, b, basic_block, err, linkage_name, macro, 1, 1, 0, 0)) {
                    free(linkage_name);
                    return 0;
                }

                free(linkage_name);

                continue;
            }
        }
        body = gtk_ml_cdr(body);
    }
    
    return 1;
}

gboolean gtk_ml_compile(GtkMl_Context *ctx, GtkMl_Builder *b, const char **err, GtkMl_S *lambda) {
    const char *linkage_name = "_start";
    GtkMl_BasicBlock *basic_block = gtk_ml_append_basic_block(b, linkage_name);

    if (!compile_program(ctx, b, basic_block, err, linkage_name, lambda, 0)) {
        return 0;
    }

    if (!gtk_ml_build_halt(ctx, b, basic_block, err)) {
        return 0;
    }
    
    return 1;
}

gboolean gtk_ml_compile_program(GtkMl_Context *ctx, GtkMl_Builder *b, const char **err, GtkMl_S *lambda) {
    if (!gtk_ml_compile_macros(b, err, lambda)) {
        return 0;
    }
    GtkMl_Program macros;
    if (!gtk_ml_build_macros(&macros, err, b)) {
        return 0;
    }
    gtk_ml_load_program(b->macro_ctx, &macros);
    return gtk_ml_compile(ctx, b, err, lambda);
}

void gtk_ml_push(GtkMl_Context *ctx, GtkMl_S *value) {
    ctx->vm->stack[ctx->vm->reg[GTKML_R_SP].sp].s_expr = value;
    ctx->vm->reg[GTKML_R_SP].sp++;
}

GtkMl_S *gtk_ml_pop(GtkMl_Context *ctx) {
    --ctx->vm->reg[GTKML_R_SP].sp;
    return ctx->vm->stack[ctx->vm->reg[GTKML_R_SP].sp].s_expr;
}

GtkMl_S *gtk_ml_peek(GtkMl_Context *ctx) {
    return ctx->vm->stack[ctx->vm->reg[GTKML_R_SP].sp - 1].s_expr;
}

void gtk_ml_enter(GtkMl_Context *ctx) {
    ctx->bindings = new_list(ctx, NULL, new_map(ctx, NULL), ctx->bindings);
}

void gtk_ml_leave(GtkMl_Context *ctx) {
    ctx->bindings = gtk_ml_cdr(ctx->bindings);
}

void gtk_ml_define(GtkMl_Context *ctx, GtkMl_S *key, GtkMl_S *value) {
    GtkMl_S *new_context = new_map(ctx, NULL);
    gtk_ml_hash_trie_insert(&new_context->value.s_map.map, &(*ctx->top_scope)->value.s_map.map, key, value);
    *ctx->top_scope = new_context;
}

void gtk_ml_bind(GtkMl_Context *ctx, GtkMl_S *key, GtkMl_S *value) {
    GtkMl_S *new_context = new_map(ctx, NULL);
    gtk_ml_hash_trie_insert(&new_context->value.s_map.map, &gtk_ml_car(ctx->bindings)->value.s_map.map, key, value);
    gtk_ml_car(ctx->bindings) = new_context;
}

GTKML_PRIVATE GtkMl_S *map_find(GtkMl_S *map, GtkMl_S *key) {
    return gtk_ml_hash_trie_get(&map->value.s_map.map, key);
}

GTKML_PRIVATE GtkMl_S *get_inner(GtkMl_S *bindings, GtkMl_S *key) {
    if (bindings->kind == GTKML_S_NIL) {
        return NULL;
    }

    GtkMl_S *value = map_find(gtk_ml_car(bindings), key);
    if (value) {
        return value;
    }

    return get_inner(gtk_ml_cdr(bindings), key);
}

GtkMl_S *gtk_ml_get(GtkMl_Context *ctx, GtkMl_S *key) {
    return get_inner(ctx->bindings, key);
}

GTKML_PRIVATE void mark_value(GtkMl_S *s);

GTKML_PRIVATE GtkMl_VisitResult mark_hash_trie(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *data) {
    (void) ht;
    (void) data;

    mark_value(key);
    mark_value(value);

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE void mark_value(GtkMl_S *s) {
    if (s->flags & GTKML_FLAG_REACHABLE) {
        return;
    }

    s->flags |= GTKML_FLAG_REACHABLE;

    switch (s->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
    case GTKML_S_INT:
    case GTKML_S_FLOAT:
    case GTKML_S_STRING:
    case GTKML_S_KEYWORD:
    case GTKML_S_SYMBOL:
    case GTKML_S_ADDRESS:
    case GTKML_S_LIGHTDATA:
        break;
    case GTKML_S_USERDATA:
        mark_value(s->value.s_userdata.keep);
        break;
    case GTKML_S_LIST:
        mark_value(gtk_ml_car(s));
        mark_value(gtk_ml_cdr(s));
        break;
    case GTKML_S_MAP:
        gtk_ml_hash_trie_foreach(&s->value.s_map.map, mark_hash_trie, NULL);
        break;
    case GTKML_S_SET:
        // TODO
        break;
    case GTKML_S_ARRAY:
        // TODO
        break;
    case GTKML_S_VARARG:
        mark_value(s->value.s_var.expr);
        break;
    case GTKML_S_QUOTE:
        mark_value(s->value.s_quote.expr);
        break;
    case GTKML_S_QUASIQUOTE:
        mark_value(s->value.s_quasiquote.expr);
        break;
    case GTKML_S_UNQUOTE:
        mark_value(s->value.s_unquote.expr);
        break;
    case GTKML_S_PROGRAM:
        mark_value(s->value.s_program.args);
        mark_value(s->value.s_program.capture);
        break;
    case GTKML_S_LAMBDA:
        mark_value(s->value.s_lambda.args);
        mark_value(s->value.s_lambda.body);
        mark_value(s->value.s_lambda.capture);
        break;
    case GTKML_S_MACRO:
        mark_value(s->value.s_macro.args);
        mark_value(s->value.s_macro.body);
        mark_value(s->value.s_macro.capture);
        break;
    }
}

GTKML_PRIVATE void mark_program(GtkMl_Program *program) {
    for (GtkMl_Static i = 0; i < program->n_static;) {
        mark_value(program->statics[i]);
    }
}

GTKML_PRIVATE void mark(GtkMl_Context *ctx) {
    for (size_t sp = 0; sp < ctx->vm->reg[GTKML_R_SP].sp; sp++) {
        mark_value(ctx->vm->stack[sp].s_expr);
    }
    mark_value(ctx->bindings);
    mark_program(&ctx->vm->program);
}

GTKML_PRIVATE void delete(GtkMl_Context *ctx, GtkMl_S *s) {
    if ((s->flags & GTKML_FLAG_REACHABLE) || (s->flags & GTKML_FLAG_DELETE)) {
        return;
    }

    s->flags |= GTKML_FLAG_DELETE;

    switch (s->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
    case GTKML_S_INT:
    case GTKML_S_FLOAT:
    case GTKML_S_KEYWORD:
    case GTKML_S_SYMBOL:
    case GTKML_S_ADDRESS:
    case GTKML_S_LIGHTDATA:
        break;
    case GTKML_S_STRING:
        // const cast required
        free((void *) s->value.s_string.ptr);
        break;
    case GTKML_S_USERDATA:
        s->value.s_userdata.del(ctx, s->value.s_userdata.userdata);
        break;
    case GTKML_S_LIST:
        delete(ctx, gtk_ml_cdr(s));
        delete(ctx, gtk_ml_car(s));
        break;
    case GTKML_S_MAP:
        gtk_ml_del_hash_trie(&s->value.s_map.map);
        break;
    case GTKML_S_SET:
        gtk_ml_del_hash_set(&s->value.s_set.set);
        break;
    case GTKML_S_ARRAY:
        gtk_ml_del_array(&s->value.s_array.array);
        break;
    case GTKML_S_VARARG:
        delete(ctx, s->value.s_var.expr);
        break;
    case GTKML_S_QUOTE:
        delete(ctx, s->value.s_quote.expr);
        break;
    case GTKML_S_QUASIQUOTE:
        delete(ctx, s->value.s_quasiquote.expr);
        break;
    case GTKML_S_UNQUOTE:
        delete(ctx, s->value.s_unquote.expr);
        break;
    case GTKML_S_PROGRAM:
        free((void *) s->value.s_program.linkage_name);
        delete(ctx, s->value.s_program.args);
        delete(ctx, s->value.s_program.capture);
        break;
    case GTKML_S_LAMBDA:
        delete(ctx, s->value.s_lambda.args);
        delete(ctx, s->value.s_lambda.body);
        delete(ctx, s->value.s_lambda.capture);
        break;
    case GTKML_S_MACRO:
        delete(ctx, s->value.s_macro.args);
        delete(ctx, s->value.s_macro.body);
        delete(ctx, s->value.s_macro.capture);
        break;
    }
    free(s);
    --ctx->n_values;
}

GTKML_PRIVATE void del(GtkMl_Context *ctx, GtkMl_S *s) {
    switch (s->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
    case GTKML_S_INT:
    case GTKML_S_FLOAT:
    case GTKML_S_KEYWORD:
    case GTKML_S_SYMBOL:
    case GTKML_S_LIGHTDATA:
    case GTKML_S_LIST:
    case GTKML_S_VARARG:
    case GTKML_S_QUOTE:
    case GTKML_S_QUASIQUOTE:
    case GTKML_S_UNQUOTE:
    case GTKML_S_LAMBDA:
    case GTKML_S_ADDRESS:
    case GTKML_S_MACRO:
        break;
    case GTKML_S_MAP:
        gtk_ml_del_hash_trie(&s->value.s_map.map);
        break;
    case GTKML_S_SET:
        gtk_ml_del_hash_set(&s->value.s_set.set);
        break;
    case GTKML_S_ARRAY:
        gtk_ml_del_array(&s->value.s_array.array);
        break;
    case GTKML_S_STRING:
        // const cast required
        free((void *) s->value.s_string.ptr);
        break;
    case GTKML_S_PROGRAM:
        free((void *) s->value.s_program.linkage_name);
        break;
    case GTKML_S_USERDATA:
        s->value.s_userdata.del(ctx, s->value.s_userdata.userdata);
        break;
    }
    free(s);
    --ctx->n_values;
}

GTKML_PRIVATE void sweep(GtkMl_Context *ctx) {
    GtkMl_S **value = &ctx->first;
    while (*value) {
        if ((*value)->flags & GTKML_FLAG_REACHABLE) {
            (*value)->flags &= ~GTKML_FLAG_REACHABLE;
            value = &(*value)->next;
        } else {
            GtkMl_S *unreachable = *value;
            *value = (*value)->next;
            delete(ctx, unreachable);
        }
    }
}

// simple mark & sweep gc
gboolean gtk_ml_collect(GtkMl_Context *ctx) {
    if (!ctx->gc_enabled) {
        return 0;
    }

    if (ctx->n_values < ctx->m_values) {
        return 0;
    }

    size_t n_values = ctx->n_values;
    mark(ctx);
    sweep(ctx);
    ctx->m_values = 2 * n_values;

    return 1;
}

gboolean gtk_ml_disable_gc(GtkMl_Context *ctx) {
    gboolean enabled = ctx->gc_enabled;
    ctx->gc_enabled = 0;
    return enabled;
}

void gtk_ml_enable_gc(GtkMl_Context *ctx, gboolean enabled) {
    ctx->gc_enabled = enabled;
}

struct DumpfData {
    FILE *stream;
    const char **err;
    size_t n;
};

GTKML_PRIVATE GtkMl_VisitResult dumpf_hash_trie(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *_data) {
    struct DumpfData *data = _data;

    gtk_ml_dumpf(data->stream, data->err, key);
    fprintf(data->stream, " ");
    gtk_ml_dumpf(data->stream, data->err, value);
    ++data->n;
    if (data->n < gtk_ml_hash_trie_len(ht)) {
        fprintf(data->stream, " ");
    }

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult dumpf_hash_set(GtkMl_HashSet *hs, GtkMl_S *key, void *_data) {
    struct DumpfData *data = _data;

    gtk_ml_dumpf(data->stream, data->err, key);
    ++data->n;
    if (data->n < gtk_ml_hash_set_len(hs)) {
        fprintf(data->stream, " ");
    }

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult dumpf_array(GtkMl_Array *array, size_t idx, GtkMl_S *key, void *_data) {
    (void) idx;

    struct DumpfData *data = _data;

    gtk_ml_dumpf(data->stream, data->err, key);
    ++data->n;
    if (data->n < gtk_ml_array_len(array)) {
        fprintf(data->stream, " ");
    }

    return GTKML_VISIT_RECURSE;
}

gboolean gtk_ml_dumpf(FILE *stream, const char **err, GtkMl_S *expr) {
    switch (expr->kind) {
    case GTKML_S_NIL:
        fprintf(stream, "#nil");
        return 1;
    case GTKML_S_TRUE:
        fprintf(stream, "#t");
        return 1;
    case GTKML_S_FALSE:
        fprintf(stream, "#f");
        return 1;
    case GTKML_S_INT:
        fprintf(stream, "%ld", expr->value.s_int.value);
        return 1;
    case GTKML_S_FLOAT:
        fprintf(stream, "%f", expr->value.s_float.value);
        return 1;
    case GTKML_S_STRING:
        fprintf(stream, "\"%.*s\"", (int) expr->value.s_string.len, expr->value.s_string.ptr);
        return 1;
    case GTKML_S_KEYWORD:
        fprintf(stream, ":%.*s", (int) expr->value.s_keyword.len, expr->value.s_keyword.ptr);
        return 1;
    case GTKML_S_SYMBOL:
        fprintf(stream, "'%.*s", (int) expr->value.s_symbol.len, expr->value.s_symbol.ptr);
        return 1;
    case GTKML_S_LIST:
        fprintf(stream, "(");
        while (expr->kind != GTKML_S_NIL) {
            gtk_ml_dumpf(stream, err, gtk_ml_car(expr));
            expr = gtk_ml_cdr(expr);
            if (expr->kind != GTKML_S_NIL) {
                fprintf(stream, " ");
            }
        }
        fprintf(stream, ")");
        return 1;
    case GTKML_S_MAP: {
        fprintf(stream, "{");
        struct DumpfData data = { stream, err, 0 }; 
        gtk_ml_hash_trie_foreach(&expr->value.s_map.map, dumpf_hash_trie, &data);
        fprintf(stream, "}");
        return 1;
    }
    case GTKML_S_SET: {
        fprintf(stream, "#{");
        struct DumpfData data = { stream, err, 0 }; 
        gtk_ml_hash_set_foreach(&expr->value.s_set.set, dumpf_hash_set, &data);
        fprintf(stream, "}");
        return 1;
    }
    case GTKML_S_ARRAY: {
        fprintf(stream, "[");
        struct DumpfData data = { stream, err, 0 }; 
        gtk_ml_array_foreach(&expr->value.s_array.array, dumpf_array, &data);
        fprintf(stream, "]");
        return 1;
    }
    case GTKML_S_VARARG:
        fprintf(stream, "...");
        return gtk_ml_dumpf(stream, err, expr->value.s_var.expr);
    case GTKML_S_QUOTE:
        fprintf(stream, "'");
        return gtk_ml_dumpf(stream, err, expr->value.s_var.expr);
    case GTKML_S_QUASIQUOTE:
        fprintf(stream, "`");
        return gtk_ml_dumpf(stream, err, expr->value.s_var.expr);
    case GTKML_S_UNQUOTE:
        fprintf(stream, ",");
        return gtk_ml_dumpf(stream, err, expr->value.s_var.expr);
    case GTKML_S_LAMBDA: {
        fprintf(stream, "(lambda ");
        if (!gtk_ml_dumpf(stream, err, expr->value.s_lambda.args)) {
            return 0;
        }
        fprintf(stream, " ");
        GtkMl_S *body = expr->value.s_lambda.body;
        while (body->kind != GTKML_S_NIL) {
            if (!gtk_ml_dumpf(stream, err, gtk_ml_car(body))) {
                return 0;
            }
            body = gtk_ml_cdr(body);
            if (body->kind != GTKML_S_NIL) {
                fprintf(stream, " ");
            }
        }
        fprintf(stream, ")");
        return 1;
    }
    case GTKML_S_PROGRAM:
        fprintf(stream, "(program \"%s\" 0x%lx ", expr->value.s_program.linkage_name, expr->value.s_program.addr);
        if (!gtk_ml_dumpf(stream, err, expr->value.s_program.args)) {
            return 0;
        }
        fprintf(stream, " ");
        GtkMl_S *body = expr->value.s_program.body;
        while (body->kind != GTKML_S_NIL) {
            if (!gtk_ml_dumpf(stream, err, gtk_ml_car(body))) {
                return 0;
            }
            body = gtk_ml_cdr(body);
            if (body->kind != GTKML_S_NIL) {
                fprintf(stream, " ");
            }
        }
        fprintf(stream, ")");
        return 1;
    case GTKML_S_ADDRESS:
        fprintf(stream, "(address 0x%lx)", expr->value.s_address.addr);
        return 1;
    case GTKML_S_MACRO: {
        fprintf(stream, "(macro ");
        gtk_ml_dumpf(stream, err, expr->value.s_macro.args);
        fprintf(stream, " ");
        GtkMl_S *body = expr->value.s_macro.body;
        while (body->kind != GTKML_S_NIL) {
            gtk_ml_dumpf(stream, err, gtk_ml_car(body));
            body = gtk_ml_cdr(body);
            if (body->kind != GTKML_S_NIL) {
                fprintf(stream, " ");
            }
        }
        fprintf(stream, ")");
        return 1;
    }
    case GTKML_S_LIGHTDATA:
        fprintf(stream, "%p", expr->value.s_lightdata.userdata);
        return 1;
    case GTKML_S_USERDATA:
        fprintf(stream, "%p", expr->value.s_lightdata.userdata);
        return 1;
    default:
        *err = GTKML_ERR_INVALID_SEXPR;
        return 0;
    }
}

char *gtk_ml_dumpsn(char *ptr, size_t n, const char **err, GtkMl_S *expr) {
    (void) ptr;
    (void) n;
    (void) err;
    (void) expr;
    return NULL;
}

char *gtk_ml_dumpsnr(char *ptr, size_t n, const char **err, GtkMl_S *expr) {
    (void) ptr;
    (void) n;
    (void) err;
    (void) expr;
    return NULL;
}

gboolean gtk_ml_dumpf_program(GtkMl_Context *ctx, FILE *stream, const char **err) {
    if (!ctx->vm->program.exec) {
        return 0;
    }

    fprintf(stream, "section TEXT\n\n");
    for (size_t pc = 0; pc < ctx->vm->program.n_exec;) {
        GtkMl_Instruction instr = ctx->vm->program.exec[pc];
        fprintf(stream, "%zx ", 8 * pc);
        if (S_CATEGORY[instr.gen.category]) {
            fprintf(stream, "%s ", S_CATEGORY[instr.gen.category][instr.arith.opcode]);
            switch (instr.gen.category) {
            case GTKML_I_ARITH:
                fprintf(stream, "%u, %u, %u", instr.arith.rd, instr.arith.rs, instr.arith.ra);
                break;
            case GTKML_I_IMM:
            case GTKML_I_IMM | GTKML_I_IMM_EXTERN:
                fprintf(stream, "%u, %u, %u", instr.imm.rd, instr.imm.rs, instr.imm.imm);
                break;
            case GTKML_I_BR:
                fprintf(stream, "%lu", instr.br.imm);
                break;
            case GTKML_EI_IMM:
            case GTKML_EI_IMM | GTKML_EI_IMM_EXTERN:
                fprintf(stream, "%u, %u, ", instr.imm.rd, instr.imm.rs);
                break;
            case GTKML_EI_BR:
                break;
            }
            if (instr.gen.category & GTKML_I_EXTENDED) {
                fprintf(stream, "%lu", ctx->vm->program.exec[pc + 1].imm64);
            }
            fprintf(stream, "\n");
        } else if (instr.gen.category == GTKML_EI_EXPORT) {
            fprintf(stream, "EXPORT %lu\n", ctx->vm->program.exec[pc + 1].imm64);
        } else if (instr.gen.category & GTKML_I_EXTENDED) {
            fprintf(stream, "INVALID %lx %lu\n", instr.instr, ctx->vm->program.exec[pc + 1].imm64);
        } else {
            fprintf(stream, "INVALID %lx\n", instr.instr);
        }
        
        if (instr.gen.category & GTKML_I_EXTENDED) {
            pc += 2;
        } else {
            ++pc;
        }
    }

    fprintf(stream, "\n");
    fprintf(stream, "section STATIC\n\n");
    for (size_t i = 1; i < ctx->vm->program.n_static; i++) {
        GtkMl_S *s = ctx->vm->program.statics[i];
        fprintf(stream, "%zu ", i);
        if (!gtk_ml_dumpf(stream, err, s)) {
            return 0;
        }
        fprintf(stream, "\n");
    }

    return 1;
}

char *gtk_ml_dumpsn_program(GtkMl_Context *ctx, char *ptr, size_t n, const char **err) {
    (void) ctx;
    (void) ptr;
    (void) n;
    (void) err;
    return NULL;
}

char *gtk_ml_dumpsnr_program(GtkMl_Context *ctx, char *ptr, size_t n, const char **err) {
    (void) ctx;
    (void) ptr;
    (void) n;
    (void) err;
    return NULL;
}

GTKML_PRIVATE void activate_program(GtkApplication* app, gpointer userdata) {
    (void) app;

    GtkMl_S *args = userdata;
    GtkMl_S *ctx_expr = gtk_ml_car(args);
    GtkMl_S *app_expr = gtk_ml_cdar(args);
    GtkMl_S *program_expr = gtk_ml_cddar(args);

    GtkMl_Context *ctx = ctx_expr->value.s_lightdata.userdata;

    const char *err;
    uint64_t pc = ctx->vm->reg[GTKML_R_PC].pc;
    uint64_t flags = ctx->vm->reg[GTKML_R_FLAGS].flags & GTKML_F_TOPCALL;
    if (gtk_ml_run_program(ctx, &err, program_expr, new_list(ctx, NULL, app_expr, new_nil(ctx, NULL)))) {
        GtkMl_S *result = gtk_ml_pop(ctx);
        app_expr->value.s_userdata.keep = new_list(ctx, NULL, result, app_expr->value.s_userdata.keep);
    }
    ctx->vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_TOPCALL;
    ctx->vm->reg[GTKML_R_FLAGS].flags |= flags;
    ctx->vm->reg[GTKML_R_PC].pc = pc;
}

GTKML_PRIVATE GtkMl_S *vm_std_application(GtkMl_Context *ctx, const char **err, GtkMl_S *expr) {
    (void) err;

    GtkMl_S *map_expr = gtk_ml_pop(ctx);
    GtkMl_S *flags_expr = gtk_ml_pop(ctx);
    GtkMl_S *id_expr = gtk_ml_pop(ctx);
    GtkMl_S *application = gtk_ml_pop(ctx);
    (void) application;

    GtkApplication *app = gtk_application_new(id_expr->value.s_string.ptr, flags_expr->value.s_int.value);
    GtkMl_S *app_expr = new_userdata(ctx, &expr->span, app, gtk_ml_object_unref);

    GtkMl_S *activate = map_find(map_expr, new_keyword(ctx, NULL, "activate", strlen("activate")));
    if (activate) {
        GtkMl_S *ctx_expr = new_lightdata(ctx, NULL, ctx);
        GtkMl_S *userdata = new_list(ctx, NULL, ctx_expr, new_list(ctx, NULL, app_expr, new_list(ctx, NULL, activate, new_nil(ctx, NULL))));
        app_expr->value.s_userdata.keep = new_list(ctx, &app_expr->span, userdata, app_expr->value.s_userdata.keep);
        g_signal_connect(app, "activate", G_CALLBACK(activate_program), userdata);
    }

    return app_expr;
}

GTKML_PRIVATE GtkMl_S *vm_std_new_window(GtkMl_Context *ctx, const char **err, GtkMl_S *expr) {
    (void) err;

    GtkMl_S *height_expr = gtk_ml_pop(ctx);
    GtkMl_S *width_expr = gtk_ml_pop(ctx);
    GtkMl_S *title_expr = gtk_ml_pop(ctx);
    GtkMl_S *app_expr = gtk_ml_pop(ctx);
    GtkMl_S *new_window = gtk_ml_pop(ctx);
    (void) new_window;

    GtkWidget *window = gtk_application_window_new(app_expr->value.s_userdata.userdata);
    gtk_window_set_title(GTK_WINDOW(window), title_expr->value.s_string.ptr);
    gtk_window_set_default_size(GTK_WINDOW(window), width_expr->value.s_int.value, height_expr->value.s_int.value);
    gtk_widget_show_all(window);

    return new_lightdata(ctx, &expr->span, window);
}

gboolean gtk_ml_equal(GtkMl_S *lhs, GtkMl_S *rhs) {
    if (lhs == rhs) {
        return 1;
    }

    if (lhs->kind != rhs->kind) {
        return 0;
    }

    switch (lhs->kind) {
    case GTKML_S_NIL:
        return 1;
    case GTKML_S_TRUE:
        return 1;
    case GTKML_S_FALSE:
        return 1;
    case GTKML_S_INT:
        return lhs->value.s_int.value == rhs->value.s_int.value;
    case GTKML_S_FLOAT:
        return lhs->value.s_float.value == rhs->value.s_float.value;
    case GTKML_S_STRING:
        return lhs->value.s_string.len == rhs->value.s_string.len && memcmp(lhs->value.s_string.ptr, rhs->value.s_string.ptr, lhs->value.s_string.len) == 0;
    case GTKML_S_KEYWORD:
        return lhs->value.s_keyword.len == rhs->value.s_keyword.len && memcmp(lhs->value.s_keyword.ptr, rhs->value.s_keyword.ptr, lhs->value.s_keyword.len) == 0;
    case GTKML_S_SYMBOL:
        return lhs->value.s_symbol.len == rhs->value.s_symbol.len && memcmp(lhs->value.s_symbol.ptr, rhs->value.s_symbol.ptr, lhs->value.s_symbol.len) == 0;
    case GTKML_S_LIGHTDATA:
        return lhs->value.s_lightdata.userdata == rhs->value.s_lightdata.userdata;
    case GTKML_S_USERDATA:
        return lhs->value.s_userdata.userdata == rhs->value.s_userdata.userdata;
    case GTKML_S_LAMBDA:
        if (gtk_ml_equal(lhs->value.s_lambda.args, rhs->value.s_lambda.args)) {
            return gtk_ml_equal(lhs->value.s_lambda.body, rhs->value.s_lambda.body);
        }
        break;
    case GTKML_S_PROGRAM:
        if (gtk_ml_equal(lhs->value.s_program.args, rhs->value.s_program.args)) {
            return lhs->value.s_program.addr == rhs->value.s_program.addr;
        }
        break;
    case GTKML_S_ADDRESS:
        return lhs->value.s_address.addr == rhs->value.s_address.addr;
    case GTKML_S_MACRO:
        if (gtk_ml_equal(lhs->value.s_macro.args, rhs->value.s_macro.args)) {
            return gtk_ml_equal(lhs->value.s_macro.body, rhs->value.s_macro.body);
        }
        break;
    case GTKML_S_LIST:
        if (gtk_ml_equal(gtk_ml_car(lhs), gtk_ml_car(rhs))) {
            return gtk_ml_equal(gtk_ml_cdr(lhs), gtk_ml_cdr(rhs));
        }
        break;
    case GTKML_S_MAP:
        return gtk_ml_hash_trie_equal(&lhs->value.s_map.map, &rhs->value.s_map.map);
    case GTKML_S_SET:
        // TODO
        break;
    case GTKML_S_ARRAY:
        // TODO
        break;
    case GTKML_S_VARARG:
        return gtk_ml_equal(lhs->value.s_var.expr, rhs->value.s_var.expr);
    case GTKML_S_QUOTE:
        return gtk_ml_equal(lhs->value.s_quote.expr, rhs->value.s_quote.expr);
    case GTKML_S_QUASIQUOTE:
        return gtk_ml_equal(lhs->value.s_quasiquote.expr, rhs->value.s_quasiquote.expr);
    case GTKML_S_UNQUOTE:
        return gtk_ml_equal(lhs->value.s_unquote.expr, rhs->value.s_unquote.expr);
    }

    return 0;
}

GTKML_PRIVATE void jenkins_start(GtkMl_Hash *hash) {
    *hash = 0;
}

GTKML_PRIVATE void jenkins_update(GtkMl_Hash *hash, const void *_ptr, size_t len) {
    GtkMl_Hash h = *hash;
    const char *ptr = _ptr;
    for (size_t i = 0; i < len; i++) {
        h += ptr[i];
        h += h << 10;
        h ^= h >> 6;
    }
    *hash = h;
}

GTKML_PRIVATE void jenkins_finish(GtkMl_Hash *hash) {
    GtkMl_Hash h = *hash;
    h += h << 3;
    h ^= h >> 11;
    h += h << 15;
    *hash = h;
}

struct HashData {
    GtkMl_Hash *hash;
    gboolean (*hasher)(GtkMl_Hash *hash, GtkMl_S *value);
};

GTKML_PRIVATE GtkMl_VisitResult hash_trie_update(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *_data) {
    (void) ht;

    struct HashData *data = _data;
    data->hasher(data->hash, key);
    data->hasher(data->hash, value);

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult hash_set_update(GtkMl_HashSet *hs, GtkMl_S *value, void *_data) {
    (void) hs;

    struct HashData *data = _data;
    data->hasher(data->hash, value);

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult array_update(GtkMl_Array *array, size_t index, GtkMl_S *value, void *_data) {
    (void) array;
    (void) index;

    struct HashData *data = _data;
    data->hasher(data->hash, value);

    return GTKML_VISIT_RECURSE;
}

void default_hash_start(GtkMl_Hash *hash) {
    jenkins_start(hash);
}

gboolean default_hash_update(GtkMl_Hash *hash, GtkMl_S *value) {
    jenkins_update(hash, &value->kind, sizeof(GtkMl_SKind));
    switch (value->kind) {
    case GTKML_S_NIL:
    case GTKML_S_FALSE:
    case GTKML_S_TRUE:
        break;
    case GTKML_S_INT:
        jenkins_update(hash, &value->value.s_int.value, sizeof(int64_t));
        break;
    case GTKML_S_FLOAT:
        return 0;
    case GTKML_S_STRING:
        jenkins_update(hash, value->value.s_string.ptr, value->value.s_string.len);
        break;
    case GTKML_S_SYMBOL:
        jenkins_update(hash, value->value.s_symbol.ptr, value->value.s_symbol.len);
        break;
    case GTKML_S_KEYWORD:
        jenkins_update(hash, value->value.s_keyword.ptr, value->value.s_keyword.len);
        break;
    case GTKML_S_LIST:
        do {
            default_hash_update(hash, gtk_ml_car(value));
            value = gtk_ml_cdr(value);
        } while (value->kind != GTKML_S_NIL);
        break;
    case GTKML_S_MAP: {
        struct HashData data = { hash, default_hash_update }; 
        gtk_ml_hash_trie_foreach(&value->value.s_map.map, hash_trie_update, &data);
    } break;
    case GTKML_S_SET: {
        struct HashData data = { hash, default_hash_update }; 
        gtk_ml_hash_set_foreach(&value->value.s_set.set, hash_set_update, &data);
    } break;
    case GTKML_S_ARRAY: {
        struct HashData data = { hash, default_hash_update }; 
        gtk_ml_array_foreach(&value->value.s_array.array, array_update, &data);
    } break;
    case GTKML_S_VARARG:
        default_hash_update(hash, value->value.s_var.expr);
        break;
    case GTKML_S_QUOTE:
        default_hash_update(hash, value->value.s_quote.expr);
        break;
    case GTKML_S_QUASIQUOTE:
        default_hash_update(hash, value->value.s_quasiquote.expr);
        break;
    case GTKML_S_UNQUOTE:
        default_hash_update(hash, value->value.s_unquote.expr);
        break;
    case GTKML_S_LAMBDA:
        default_hash_update(hash, value->value.s_lambda.args);
        default_hash_update(hash, value->value.s_lambda.body);
        default_hash_update(hash, value->value.s_lambda.capture);
        break;
    case GTKML_S_PROGRAM:
        jenkins_update(hash, value->value.s_program.linkage_name, strlen(value->value.s_program.linkage_name));
        jenkins_update(hash, &value->value.s_program.addr, sizeof(uint64_t));
        default_hash_update(hash, value->value.s_program.args);
        default_hash_update(hash, value->value.s_program.body);
        default_hash_update(hash, value->value.s_program.capture);
        break;
    case GTKML_S_ADDRESS:
        jenkins_update(hash, &value->value.s_address.addr, sizeof(uint64_t));
        break;
    case GTKML_S_MACRO:
        default_hash_update(hash, value->value.s_macro.args);
        default_hash_update(hash, value->value.s_macro.body);
        default_hash_update(hash, value->value.s_macro.capture);
        break;
    case GTKML_S_LIGHTDATA:
        jenkins_update(hash, &value->value.s_lightdata.userdata, sizeof(void *));
        break;
    case GTKML_S_USERDATA:
        jenkins_update(hash, &value->value.s_userdata.userdata, sizeof(void *));
        break;
    }
    return 1;
}

void default_hash_finish(GtkMl_Hash *hash) {
    jenkins_finish(hash);
}

gboolean gtk_ml_hash(GtkMl_Hash *hash, GtkMl_S *value) {
    if (value->hashed) {
        *hash = value->hash;
        return 1;
    } else {
        default_hash_start(hash);
        if (default_hash_update(hash, value)) {
            default_hash_finish(hash);
            value->hashed = 1;
            value->hash = *hash;
            return 1;
        }
        return 0;
    }
}

GtkMl_S *gtk_ml_nil(GtkMl_Context *ctx) {
    return new_nil(ctx, NULL);
}

void gtk_ml_delete_reference(GtkMl_Context *ctx, void *reference) {
    (void) ctx;
    (void) reference;
}

void gtk_ml_delete_value(GtkMl_Context *ctx, void *s) {
    delete(ctx, s);
}

void gtk_ml_object_unref(GtkMl_Context *ctx, void *obj) {
    (void) ctx;
    g_object_unref(obj);
}

GtkMl_Vm *gtk_ml_new_vm(GtkMl_Context *ctx) {
    GtkMl_Vm *vm = malloc(sizeof(GtkMl_Vm));

    vm->stack = malloc(sizeof(GtkMl_Register) * GTKML_VM_STACK);
    vm->stack_len = GTKML_VM_STACK;

    vm->program.start = NULL;
    vm->program.exec = NULL;
    vm->program.n_exec = 0;
    vm->program.statics = NULL;
    vm->program.n_static = 0;

    vm->reg[GTKML_R_ZERO].value = 0;
    vm->reg[GTKML_R_FLAGS].flags = GTKML_F_NONE;
    vm->reg[GTKML_R_SP].sp = 0;
    vm->reg[GTKML_R_BP].sp = 0;

    vm->std = STD;

    vm->ctx = ctx;

    return vm;
}

void gtk_ml_del_vm(GtkMl_Vm *vm) {
    free(vm->stack);
    gtk_ml_del_program(&vm->program);
    free(vm);
}

gboolean gtk_ml_vm_step(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    if (!instr->gen.cond || (instr->gen.cond && (vm->reg[GTKML_R_FLAGS].flags & instr->gen.cond))) {
        gboolean (*category)(GtkMl_Vm *, const char **, GtkMl_Instruction *) = CATEGORY[instr->gen.category];
        if (category) {
            return category(vm, err, instr);
        } else {
            *err = GTKML_ERR_CATEGORY_ERROR;
            return 0;
        }
    } else {
        if (instr->gen.category & GTKML_I_EXTENDED) {
            vm->reg[GTKML_R_PC].pc += 16;
        } else {
            vm->reg[GTKML_R_PC].pc += 8;
        }
        return 1;
    }
}

gboolean gtk_ml_vm_run(GtkMl_Vm *vm, const char **err) {
    vm->reg[GTKML_R_FLAGS].flags |= GTKML_F_TOPCALL;
    vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_HALT;
    size_t gc_counter = 0;
    while (!(vm->reg[GTKML_R_FLAGS].flags & GTKML_F_HALT)) {
        if (!gtk_ml_vm_step(vm, err, vm->program.exec + (vm->reg[GTKML_R_PC].pc >> 3))) {
            return 0;
        }
        if (gc_counter++ == GTKML_GC_STEP_THRESHOLD) {
            if (gtk_ml_collect(vm->ctx)) {
                gc_counter = 0;
            }
        }
        vm->reg[GTKML_R_ZERO].flags = 0;
    }
    return 1;
}

GTKML_PRIVATE gboolean gtk_ml_ia(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    gboolean (*opcode)(GtkMl_Vm *, const char **, GtkMl_Instruction) = I_ARITH[instr->arith.opcode];
    if (opcode) {
        return opcode(vm, err, *instr);
    } else {
        *err = GTKML_ERR_OPCODE_ERROR;
        return 0;
    }
}

GTKML_PRIVATE gboolean gtk_ml_ii(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    gboolean (*opcode)(GtkMl_Vm *, const char **, GtkMl_Instruction) = I_IMM[instr->imm.opcode];
    if (opcode) {
        return opcode(vm, err, *instr);
    } else {
        *err = GTKML_ERR_OPCODE_ERROR;
        return 0;
    }
}

GTKML_PRIVATE gboolean gtk_ml_ibr(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    gboolean (*opcode)(GtkMl_Vm *, const char **, GtkMl_Instruction) = I_BR[instr->br.opcode];
    if (opcode) {
        return opcode(vm, err, *instr);
    } else {
        *err = GTKML_ERR_OPCODE_ERROR;
        return 0;
    }
}

GTKML_PRIVATE gboolean gtk_ml_eii(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    gboolean (*opcode)(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *) = EI_IMM[instr->imm.opcode];
    if (opcode) {
        return opcode(vm, err, instr[0], vm->program.statics[instr[1].imm64]);
    } else {
        *err = GTKML_ERR_OPCODE_ERROR;
        return 0;
    }
}

GTKML_PRIVATE gboolean gtk_ml_eibr(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    gboolean (*opcode)(GtkMl_Vm *, const char **, GtkMl_Instruction, GtkMl_S *) = EI_BR[instr->br.opcode];
    if (opcode) {
        return opcode(vm, err, instr[0], vm->program.statics[instr[1].imm64]);
    } else {
        *err = GTKML_ERR_OPCODE_ERROR;
        return 0;
    }
}

GTKML_PRIVATE gboolean gtk_ml_enop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction *instr) {
    (void) err;
    (void) instr;

    vm->reg[GTKML_R_PC].pc += 16;

    return 1;
}

#define PC_INCREMENT vm->reg[GTKML_R_PC].pc += 8
#define PC_INCREMENT_EXTENDED vm->reg[GTKML_R_PC].pc += 16

gboolean gtk_ml_ia_nop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_halt(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    vm->reg[GTKML_R_FLAGS].flags |= GTKML_F_HALT;
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_add(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value + rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_subtract(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value - rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_signed_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value * rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_unsigned_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, (uint64_t) lhs->value.s_int.value * (uint64_t) rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_signed_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value / rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_unsigned_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, (uint64_t) lhs->value.s_int.value / (uint64_t) rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_signed_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value % rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_integer_unsigned_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, (uint64_t) lhs->value.s_int.value % (uint64_t) rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_float_add(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_float(vm->ctx, NULL, lhs->value.s_float.value + rhs->value.s_float.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_float_subtract(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_float(vm->ctx, NULL, lhs->value.s_float.value - rhs->value.s_float.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_float_multiply(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_float(vm->ctx, NULL, lhs->value.s_float.value * rhs->value.s_float.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_float_divide(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_float(vm->ctx, NULL, lhs->value.s_float.value / rhs->value.s_float.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_float_modulo(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_float(vm->ctx, NULL, fmod(lhs->value.s_float.value, rhs->value.s_float.value)));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_and(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value & rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_or(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value | rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_xor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, lhs->value.s_int.value ^ rhs->value.s_int.value));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_nand(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, ~(lhs->value.s_int.value & rhs->value.s_int.value)));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_nor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, ~(lhs->value.s_int.value | rhs->value.s_int.value)));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bit_xnor(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *lhs = gtk_ml_pop(vm->ctx);
    GtkMl_S *rhs = gtk_ml_pop(vm->ctx);
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, ~(lhs->value.s_int.value ^ rhs->value.s_int.value)));
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_branch_absolute(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *addr = gtk_ml_pop(vm->ctx);
    vm->reg[GTKML_R_PC].pc = addr->value.s_address.addr;
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_branch_relative(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    // TODO(walterpi): flags
    GtkMl_S *addr = gtk_ml_pop(vm->ctx);
    vm->reg[GTKML_R_PC].pc += 8 + addr->value.s_address.addr;
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_bind(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    GtkMl_S *key = gtk_ml_pop(vm->ctx);
    GtkMl_S *value = gtk_ml_pop(vm->ctx);
    gtk_ml_bind(vm->ctx, key, value);
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ia_define(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    GtkMl_S *key = gtk_ml_pop(vm->ctx);
    GtkMl_S *value = gtk_ml_pop(vm->ctx);
    gtk_ml_define(vm->ctx, key, value);
    return 1;
}

gboolean gtk_ml_ii_push_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    gtk_ml_push(vm->ctx, vm->program.statics[instr.imm.imm]);
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_pop(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    gtk_ml_pop(vm->ctx);
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_get_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    GtkMl_S *value = gtk_ml_get(vm->ctx, vm->program.statics[instr.imm.imm]);
    if (value) {
        gtk_ml_push(vm->ctx, value);
    } else {
        *err = GTKML_ERR_BINDING_ERROR;
        return 0;
    }
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_list_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;

    int64_t n = vm->program.statics[instr.imm.imm]->value.s_int.value;

    GtkMl_S *map = new_nil(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *expr = gtk_ml_pop(vm->ctx);
        map = new_list(vm->ctx, NULL, expr, map);
    }

    gtk_ml_push(vm->ctx, map);

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_map_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;

    int64_t n = vm->program.statics[instr.imm.imm]->value.s_int.value;

    GtkMl_S *map = new_map(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *value = gtk_ml_pop(vm->ctx);
        GtkMl_S *key = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_map(vm->ctx, NULL);
        gtk_ml_hash_trie_insert(&new->value.s_map.map, &map->value.s_map.map, key, value);
        map = new;
    }

    gtk_ml_push(vm->ctx, map);

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_set_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;

    int64_t n = vm->program.statics[instr.imm.imm]->value.s_int.value;

    GtkMl_S *set = new_set(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *key = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_set(vm->ctx, NULL);
        gtk_ml_hash_set_insert(&new->value.s_set.set, &set->value.s_set.set, key);
        set = new;
    }

    gtk_ml_push(vm->ctx, set);

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ii_array_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;

    int64_t n = vm->program.statics[instr.imm.imm]->value.s_int.value;

    GtkMl_S *array = new_array(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *value = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_array(vm->ctx, NULL);
        gtk_ml_array_push(&new->value.s_array.array, &array->value.s_array.array, value);
        array = new;
    }

    gtk_ml_push(vm->ctx, array);

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ibr_call(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;

    uint64_t flags = vm->reg[GTKML_R_FLAGS].flags & GTKML_F_TOPCALL;
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, flags));
    vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_TOPCALL;

    uint64_t pc = vm->reg[GTKML_R_PC].pc;
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, pc));
    vm->reg[GTKML_R_PC].pc = vm->program.statics[instr.br.imm]->value.s_program.addr;

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ibr_ret(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) err;
    (void) instr;
    if (vm->reg[GTKML_R_FLAGS].flags & GTKML_F_TOPCALL) {
        vm->reg[GTKML_R_FLAGS].flags |= GTKML_F_HALT;
    } else {
        GtkMl_S *pc = gtk_ml_pop(vm->ctx);
        GtkMl_S *flags = gtk_ml_pop(vm->ctx);
        vm->reg[GTKML_R_PC].pc = pc->value.s_int.value;
        vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_TOPCALL;
        vm->reg[GTKML_R_FLAGS].flags |= flags->value.s_int.value;
    }
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_ibr_call_std(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr) {
    (void) instr;
    GtkMl_S *(*function)(GtkMl_Context *, const char **, GtkMl_S *) = vm->std[vm->program.statics[instr.br.imm]->value.s_int.value];
    GtkMl_S *expr = gtk_ml_pop(vm->ctx);
    GtkMl_S *value = function(vm->ctx, err, expr);
    if (value) {
        gtk_ml_push(vm->ctx, value);
    } else {
        return 0;
    }
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_eii_push_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;
    gtk_ml_push(vm->ctx, imm64);
    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eii_pop_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;
    (void) imm64;
    gtk_ml_pop(vm->ctx);
    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eii_get_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;
    GtkMl_S *value = gtk_ml_get(vm->ctx, imm64);
    if (value) {
        gtk_ml_push(vm->ctx, value);
    } else {
        *err = GTKML_ERR_BINDING_ERROR;
        return 0;
    }
    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eii_list_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;

    int64_t n = imm64->value.s_int.value;

    GtkMl_S *map = new_nil(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *expr = gtk_ml_pop(vm->ctx);
        map = new_list(vm->ctx, NULL, expr, map);
    }

    gtk_ml_push(vm->ctx, map);

    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_eii_map_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;

    int64_t n = imm64->value.s_int.value;

    GtkMl_S *map = new_map(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *key = gtk_ml_pop(vm->ctx);
        GtkMl_S *value = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_map(vm->ctx, NULL);
        gtk_ml_hash_trie_insert(&new->value.s_map.map, &map->value.s_map.map, key, value);
        map = new;
    }

    gtk_ml_push(vm->ctx, map);

    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eii_set_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;

    int64_t n = imm64->value.s_int.value;

    GtkMl_S *set = new_set(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *key = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_set(vm->ctx, NULL);
        gtk_ml_hash_set_insert(&new->value.s_set.set, &set->value.s_set.set, key);
        set = new;
    }

    gtk_ml_push(vm->ctx, set);

    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eii_array_ext_imm(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;

    int64_t n = imm64->value.s_int.value;

    GtkMl_S *array = new_array(vm->ctx, NULL);

    while (n--) {
        GtkMl_S *value = gtk_ml_pop(vm->ctx);
        GtkMl_S *new = new_array(vm->ctx, NULL);
        gtk_ml_array_push(&new->value.s_array.array, &array->value.s_array.array, value);
        array = new;
    }

    gtk_ml_push(vm->ctx, array);

    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eibr_call_ext_std(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) instr;
    GtkMl_S *(*function)(GtkMl_Context *, const char **, GtkMl_S *) = vm->std[imm64->value.s_int.value];
    GtkMl_S *expr = gtk_ml_pop(vm->ctx);
    GtkMl_S *value = function(vm->ctx, err, expr);
    if (value) {
        gtk_ml_push(vm->ctx, value);
    } else {
        return 0;
    }
    PC_INCREMENT;
    return 1;
}

gboolean gtk_ml_eibr_call_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;

    uint64_t flags = vm->reg[GTKML_R_FLAGS].flags & GTKML_F_TOPCALL;
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, flags));
    vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_TOPCALL;

    uint64_t pc = vm->reg[GTKML_R_PC].pc;
    gtk_ml_push(vm->ctx, new_int(vm->ctx, NULL, pc));
    vm->reg[GTKML_R_PC].pc = imm64->value.s_program.addr;

    PC_INCREMENT_EXTENDED;
    return 1;
}

gboolean gtk_ml_eibr_ret_ext(GtkMl_Vm *vm, const char **err, GtkMl_Instruction instr, GtkMl_S *imm64) {
    (void) err;
    (void) instr;
    (void) imm64;
    if (vm->reg[GTKML_R_FLAGS].flags & GTKML_F_TOPCALL) {
        vm->reg[GTKML_R_FLAGS].flags |= GTKML_F_HALT;
    } else {
        GtkMl_S *pc = gtk_ml_pop(vm->ctx);
        GtkMl_S *flags = gtk_ml_pop(vm->ctx);
        vm->reg[GTKML_R_PC].pc = pc->value.s_int.value;
        vm->reg[GTKML_R_FLAGS].flags &= ~GTKML_F_TOPCALL;
        vm->reg[GTKML_R_FLAGS].flags |= flags->value.s_int.value;
    }
    PC_INCREMENT_EXTENDED;
    return 1;
}

struct SerfData {
    FILE *stream;
    const char **err;
    size_t n;
    gboolean result;
};

GTKML_PRIVATE GtkMl_VisitResult serf_hash_trie(GtkMl_HashTrie *ht, GtkMl_S *key, GtkMl_S *value, void *_data) {
    struct SerfData *data = _data;
    if (!gtk_ml_serf_value(data->stream, data->err, key)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    fprintf(data->stream, ":");
    if (!gtk_ml_serf_value(data->stream, data->err, value)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    ++data->n;
    if (data->n < gtk_ml_hash_trie_len(ht)) {
        fprintf(data->stream, ",");
    }

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult serf_hash_set(GtkMl_HashSet *hs, GtkMl_S *key, void *_data) {
    struct SerfData *data = _data;
    if (!gtk_ml_serf_value(data->stream, data->err, key)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    ++data->n;
    if (data->n < gtk_ml_hash_set_len(hs)) {
        fprintf(data->stream, ",");
    }

    return GTKML_VISIT_RECURSE;
}

GTKML_PRIVATE GtkMl_VisitResult serf_array(GtkMl_Array *array, size_t idx, GtkMl_S *value, void *_data) {
    (void) idx;
    struct SerfData *data = _data;
    if (!gtk_ml_serf_value(data->stream, data->err, value)) {
        data->result = 0;
        return GTKML_VISIT_BREAK;
    }
    ++data->n;
    if (data->n < gtk_ml_array_len(array)) {
        fprintf(data->stream, ",");
    }

    return GTKML_VISIT_RECURSE;
}

gboolean gtk_ml_serf_value(FILE *stream, const char **err, GtkMl_S *value) {
    fprintf(stream, "GTKML-V(");
    uint32_t kind = value->kind;
    fwrite(&kind, sizeof(uint32_t), 1, stream);

    gboolean result = 1;
    switch (value->kind) {
    case GTKML_S_NIL:
    case GTKML_S_TRUE:
    case GTKML_S_FALSE:
        break;
    case GTKML_S_INT:
        fwrite(&value->value.s_int.value, sizeof(int64_t), 1, stream);
        break;
    case GTKML_S_FLOAT:
        fwrite(&value->value.s_float.value, sizeof(double), 1, stream);
        break;
    case GTKML_S_STRING: {
        uint64_t len = value->value.s_string.len;
        fwrite(&len, sizeof(uint64_t), 1, stream);
        fwrite(value->value.s_string.ptr, 1, len + 1, stream);
        break;
    }
    case GTKML_S_KEYWORD: {
        uint64_t len = value->value.s_keyword.len;
        fwrite(&len, sizeof(uint64_t), 1, stream);
        fwrite(value->value.s_keyword.ptr, 1, len, stream);
        break;
    }
    case GTKML_S_SYMBOL: {
        uint64_t len = value->value.s_symbol.len;
        fwrite(&len, sizeof(uint64_t), 1, stream);
        fwrite(value->value.s_symbol.ptr, 1, len, stream);
        break;
    }
    case GTKML_S_PROGRAM: {
        uint64_t len = strlen(value->value.s_program.linkage_name);
        fwrite(&len, sizeof(uint64_t), 1, stream);
        fwrite(value->value.s_program.linkage_name, 1, len + 1, stream);
        fwrite(&value->value.s_program.addr, sizeof(uint64_t), 1, stream);
        if (!gtk_ml_serf_value(stream, err, value->value.s_program.args)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_program.body)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_program.capture)) {
            return 0;
        }
        break;
    }
    case GTKML_S_ADDRESS:
        fwrite(&value->value.s_address.addr, sizeof(uint64_t), 1, stream);
        break;
    case GTKML_S_LIGHTDATA:
    case GTKML_S_USERDATA:
        *err = GTKML_ERR_SER_ERROR;
        return 0;
    case GTKML_S_LAMBDA:
        if (!gtk_ml_serf_value(stream, err, value->value.s_lambda.args)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_lambda.body)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_lambda.capture)) {
            return 0;
        }
        break;
    case GTKML_S_MACRO:
        if (!gtk_ml_serf_value(stream, err, value->value.s_macro.args)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_macro.body)) {
            return 0;
        }
        if (!gtk_ml_serf_value(stream, err, value->value.s_macro.capture)) {
            return 0;
        }
        break;
    case GTKML_S_MAP: {
        struct SerfData data = { stream, err, 0, 1 }; 
        gtk_ml_hash_trie_foreach(&value->value.s_map.map, serf_hash_trie, &data);
        result = data.result;
    } break;
    case GTKML_S_SET: {
        struct SerfData data = { stream, err, 0, 1 }; 
        gtk_ml_hash_set_foreach(&value->value.s_set.set, serf_hash_set, &data);
        result = data.result;
    } break;
    case GTKML_S_ARRAY: {
        struct SerfData data = { stream, err, 0, 1 }; 
        gtk_ml_array_foreach(&value->value.s_array.array, serf_array, &data);
        result = data.result;
    } break;
    case GTKML_S_VARARG:
        if (!gtk_ml_serf_value(stream, err, value->value.s_var.expr)) {
            return 0;
        }
        break;
    case GTKML_S_QUOTE:
        if (!gtk_ml_serf_value(stream, err, value->value.s_quote.expr)) {
            return 0;
        }
        break;
    case GTKML_S_QUASIQUOTE:
        if (!gtk_ml_serf_value(stream, err, value->value.s_quasiquote.expr)) {
            return 0;
        }
        break;
    case GTKML_S_UNQUOTE:
        if (!gtk_ml_serf_value(stream, err, value->value.s_unquote.expr)) {
            return 0;
        }
        break;
    case GTKML_S_LIST:
        while (value->kind != GTKML_S_NIL) {
            if (!gtk_ml_serf_value(stream, err, gtk_ml_car(value))) {
                return 0;
            }
            value = gtk_ml_cdr(value);
            if (value->kind != GTKML_S_NIL) {
                fprintf(stream, ",");
            }
        }
        break;
    }
    fprintf(stream, ")");
    return result;
}

GtkMl_S *gtk_ml_deserf_value(GtkMl_Context *ctx, FILE *stream, const char **err) {
    char *gtkml_v = malloc(strlen("GTKML-V(") + 1);
    fread(gtkml_v, 1, strlen("GTKML-V("), stream);
    gtkml_v[strlen("GTKML-V(")] = 0;
    if (strcmp(gtkml_v, "GTKML-V(") != 0) {
        free(gtkml_v);
        *err = GTKML_ERR_DESER_ERROR;
        return NULL;
    }
    free(gtkml_v);

    uint32_t kind;
    fread(&kind, sizeof(uint32_t), 1, stream);

    GtkMl_S *result = new_value(ctx, NULL, kind);

    switch (result->kind) {
    case GTKML_S_NIL:
    case GTKML_S_FALSE:
    case GTKML_S_TRUE:
        break;
    case GTKML_S_INT:
        fread(&result->value.s_int.value, sizeof(int64_t), 1, stream);
        break;
    case GTKML_S_FLOAT:
        fread(&result->value.s_float.value, sizeof(double), 1, stream);
        break;
    case GTKML_S_STRING: {
        uint64_t len;
        fread(&len, sizeof(uint64_t), 1, stream);
        char *ptr = malloc(len + 1);
        fread(ptr, 1, len + 1, stream);
        ptr[len] = 0;
        result->value.s_string.ptr = ptr;
        result->value.s_string.len = len;
        break;
    }
    case GTKML_S_SYMBOL: {
        uint64_t len;
        fread(&len, sizeof(uint64_t), 1, stream);
        char *ptr = malloc(len);
        fread(ptr, 1, len, stream);
        result->value.s_symbol.ptr = ptr;
        result->value.s_symbol.len = len;
        break;
    }
    case GTKML_S_KEYWORD: {
        uint64_t len;
        fread(&len, sizeof(uint64_t), 1, stream);
        char *ptr = malloc(len);
        fread(ptr, 1, len, stream);
        result->value.s_keyword.ptr = ptr;
        result->value.s_keyword.len = len;
        break;
    }
    case GTKML_S_LIST: {
        char next = 0;
        fread(&next, 1, 1, stream);
        if (next != ')') {
            fseek(stream, -1, SEEK_CUR);
        }
        GtkMl_S **tail = &result;
        while (next != ')') {
            GtkMl_S *value = gtk_ml_deserf_value(ctx, stream, err);
            if (!value) {
                return NULL;
            }
            *tail = new_list(ctx, NULL, value, new_nil(ctx, NULL));
            tail = &gtk_ml_cdr(*tail);
            fread(&next, 1, 1, stream);
        }
        fseek(stream, -1, SEEK_CUR);
        break;
    }
    case GTKML_S_MAP: {
        char next = 0;
        fread(&next, 1, 1, stream);
        if (next != ')') {
            fseek(stream, -1, SEEK_CUR);
        }
        gtk_ml_new_hash_trie(&result->value.s_map.map);
        while (next != ')') {
            GtkMl_S *key = gtk_ml_deserf_value(ctx, stream, err);
            if (!key) {
                return NULL;
            }
            char colon;
            fread(&colon, 1, 1, stream);
            if (colon != ':') {
                *err = GTKML_ERR_DESER_ERROR;
                return NULL;
            }
            GtkMl_S *value = gtk_ml_deserf_value(ctx, stream, err);
            if (!value) {
                return NULL;
            }
            GtkMl_S *new = new_map(ctx, NULL);
            gtk_ml_hash_trie_insert(&new->value.s_map.map, &result->value.s_map.map, key, value);
            result = new;
            fread(&next, 1, 1, stream);
        }
        fseek(stream, -1, SEEK_CUR);
        break;
    }
    case GTKML_S_SET: {
        char next = 0;
        fread(&next, 1, 1, stream);
        if (next != ')') {
            fseek(stream, -1, SEEK_CUR);
        }
        gtk_ml_new_hash_set(&result->value.s_set.set);
        while (next != ')') {
            GtkMl_S *key = gtk_ml_deserf_value(ctx, stream, err);
            if (!key) {
                return NULL;
            }
            GtkMl_S *new = new_set(ctx, NULL);
            gtk_ml_hash_set_insert(&new->value.s_set.set, &result->value.s_set.set, key);
            result = new;
            fread(&next, 1, 1, stream);
        }
        fseek(stream, -1, SEEK_CUR);
        break;
    }
    case GTKML_S_ARRAY: {
        char next = 0;
        fread(&next, 1, 1, stream);
        if (next != ')') {
            fseek(stream, -1, SEEK_CUR);
        }
        gtk_ml_new_array(&result->value.s_array.array);
        while (next != ')') {
            GtkMl_S *value = gtk_ml_deserf_value(ctx, stream, err);
            if (!value) {
                return NULL;
            }
            GtkMl_S *new = new_array(ctx, NULL);
            gtk_ml_array_push(&new->value.s_array.array, &result->value.s_array.array, value);
            result = new;
            fread(&next, 1, 1, stream);
        }
        fseek(stream, -1, SEEK_CUR);
        break;
    }
    case GTKML_S_VARARG: {
        GtkMl_S *expr = gtk_ml_deserf_value(ctx, stream, err);
        if (!expr) {
            return NULL;
        }
        result->value.s_var.expr = expr;
    } break;
    case GTKML_S_QUOTE: {
        GtkMl_S *expr = gtk_ml_deserf_value(ctx, stream, err);
        if (!expr) {
            return NULL;
        }
        result->value.s_quote.expr = expr;
    } break;
    case GTKML_S_QUASIQUOTE: {
        GtkMl_S *expr = gtk_ml_deserf_value(ctx, stream, err);
        if (!expr) {
            return NULL;
        }
        result->value.s_quasiquote.expr = expr;
    } break;
    case GTKML_S_UNQUOTE: {
        GtkMl_S *expr = gtk_ml_deserf_value(ctx, stream, err);
        if (!expr) {
            return NULL;
        }
        result->value.s_unquote.expr = expr;
    } break;
    case GTKML_S_LAMBDA: {
        GtkMl_S *args = gtk_ml_deserf_value(ctx, stream, err);
        if (!args) {
            return NULL;
        }
        GtkMl_S *body = gtk_ml_deserf_value(ctx, stream, err);
        if (!body) {
            return NULL;
        }
        GtkMl_S *capture = gtk_ml_deserf_value(ctx, stream, err);
        if (!capture) {
            return NULL;
        }
        result->value.s_lambda.args = args;
        result->value.s_lambda.body = body;
        result->value.s_lambda.capture = capture;
        break;
    }
    case GTKML_S_PROGRAM: {
        uint64_t len;
        fread(&len, sizeof(uint64_t), 1, stream);
        char *ptr = malloc(len + 1);
        fread(ptr, 1, len + 1, stream);
        result->value.s_program.linkage_name = ptr;
        uint64_t addr;
        fread(&addr, sizeof(uint64_t), 1, stream);
        result->value.s_program.addr = addr;
        GtkMl_S *args = gtk_ml_deserf_value(ctx, stream, err);
        if (!args) {
            return NULL;
        }
        GtkMl_S *body = gtk_ml_deserf_value(ctx, stream, err);
        if (!body) {
            return NULL;
        }
        GtkMl_S *capture = gtk_ml_deserf_value(ctx, stream, err);
        if (!capture) {
            return NULL;
        }
        result->value.s_program.args = args;
        result->value.s_program.body = body;
        result->value.s_program.capture = capture;
        break;
    }
    case GTKML_S_ADDRESS: {
        uint64_t addr;
        fread(&addr, sizeof(uint64_t), 1, stream);
        result->value.s_address.addr = addr;
        break;
    }
    case GTKML_S_MACRO: {
        GtkMl_S *args = gtk_ml_deserf_value(ctx, stream, err);
        if (!args) {
            return NULL;
        }
        GtkMl_S *body = gtk_ml_deserf_value(ctx, stream, err);
        if (!body) {
            return NULL;
        }
        GtkMl_S *capture = gtk_ml_deserf_value(ctx, stream, err);
        if (!capture) {
            return NULL;
        }
        result->value.s_macro.args = args;
        result->value.s_macro.body = body;
        result->value.s_macro.capture = capture;
        break;
    }
    case GTKML_S_LIGHTDATA:
    case GTKML_S_USERDATA:
        *err = GTKML_ERR_DESER_ERROR;
        return 0;
    }

    char *end = malloc(2);
    fread(end, 1, 1, stream);
    end[1] = 0;
    if (strcmp(end, ")") != 0) {
        free(end);
        *err = GTKML_ERR_DESER_ERROR;
        return NULL;
    }
    free(end);

    return result;
}

gboolean gtk_ml_serf_program(FILE *stream, const char **err, const GtkMl_Program *program) {
    fprintf(stream, "GTKML-P(");

    uint64_t n_start = strlen(program->start);
    fwrite(&n_start, sizeof(uint64_t), 1, stream);
    fwrite(program->start, 1, n_start + 1, stream);

    uint64_t n_exec = program->n_exec;
    fwrite(&n_exec, sizeof(uint64_t), 1, stream);
    fwrite(program->exec, sizeof(GtkMl_Instruction), program->n_exec, stream);

    uint64_t n_static = program->n_static;
    fwrite(&n_static, sizeof(uint64_t), 1, stream);

    for (size_t i = 1; i < n_static; i++) {
        if (!gtk_ml_serf_value(stream, err, program->statics[i])) {
            return 0;
        }
    }
    fprintf(stream, ")");
    return 1;
}

gboolean gtk_ml_deserf_program(GtkMl_Context *ctx, GtkMl_Program *program, FILE *stream, const char **err) {
    char *gtkml_p = malloc(strlen("GTKML-P(") + 1);
    fread(gtkml_p, 1, strlen("GTKML-P("), stream);
    gtkml_p[strlen("GTKML-P(")] = 0;
    if (strcmp(gtkml_p, "GTKML-P(") != 0) {
        free(gtkml_p);
        *err = GTKML_ERR_DESER_ERROR;
        return 0;
    }
    free(gtkml_p);

    uint64_t n_start;
    fread(&n_start, sizeof(uint64_t), 1, stream);
    program->start = malloc(n_start + 1);
    fread((void *) program->start, 1, n_start + 1, stream);

    uint64_t n_exec;
    fread(&n_exec, sizeof(uint64_t), 1, stream);
    program->n_exec = n_exec;
    program->exec = malloc(sizeof(GtkMl_Instruction) * program->n_exec);
    fread(program->exec, sizeof(GtkMl_Instruction), program->n_exec, stream);

    uint64_t n_static;
    fread(&n_static, sizeof(uint64_t), 1, stream);
    program->n_static = n_static;
    program->statics = malloc(sizeof(GtkMl_S *) * program->n_static);

    for (size_t i = 1; i < program->n_static; i++) {
        GtkMl_S *value = gtk_ml_deserf_value(ctx, stream, err);
        if (!value) {
            return 0;
        }
        program->statics[i] = value;
    }

    char *end = malloc(2);
    fread(end, 1, 1, stream);
    end[1] = 0;
    if (strcmp(end, ")") != 0) {
        free(end);
        *err = GTKML_ERR_DESER_ERROR;
        return 0;
    }
    free(end);

    return 1;
}
