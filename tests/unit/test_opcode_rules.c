#include "test_framework.h"
#include "opcode_rules.h"
#include "instr_operands_encoder.h"

static void init_ictx(InstrCtx *ictx, char *instr_name)
{
    memset(ictx, 0, sizeof(*ictx));
    ictx->instr = instr_name;
    ictx->ic_count = 1;
}

static void test_two_operand_mov_add_sub(void)
{
    char *two_op_group[] = {"mov", "add", "sub"};

    for (int i = 0; i < 3; ++i)
    {
        InstrCtx ictx;
        init_ictx(&ictx, two_op_group[i]);

        TEST_ASSERT_TRUE(classify_opcode(&ictx));
        TEST_ASSERT_TRUE(ictx.src_exists);
        TEST_ASSERT_TRUE(ictx.dest_exists);

        /* src: all modes (IM, DT, ST, RG) */
        TEST_ASSERT_TRUE(ictx.src_mask[IM]);
        TEST_ASSERT_TRUE(ictx.src_mask[DT]);
        TEST_ASSERT_TRUE(ictx.src_mask[ST]);
        TEST_ASSERT_TRUE(ictx.src_mask[RG]);

        /* dest: standard modes (DT, ST, RG) - no IM */
        TEST_ASSERT_FALSE(ictx.dest_mask[IM]);
        TEST_ASSERT_TRUE(ictx.dest_mask[DT]);
        TEST_ASSERT_TRUE(ictx.dest_mask[ST]);
        TEST_ASSERT_TRUE(ictx.dest_mask[RG]);
    }
}

static void test_cmp_rules(void)
{
    InstrCtx ictx;
    init_ictx(&ictx, "cmp");

    TEST_ASSERT_TRUE(classify_opcode(&ictx));
    TEST_ASSERT_TRUE(ictx.src_exists);
    TEST_ASSERT_TRUE(ictx.dest_exists);

    /* src: all modes */
    TEST_ASSERT_TRUE(ictx.src_mask[IM]);
    TEST_ASSERT_TRUE(ictx.src_mask[DT]);
    TEST_ASSERT_TRUE(ictx.src_mask[ST]);
    TEST_ASSERT_TRUE(ictx.src_mask[RG]);

    /* dest: all modes */
    TEST_ASSERT_TRUE(ictx.dest_mask[IM]);
    TEST_ASSERT_TRUE(ictx.dest_mask[DT]);
    TEST_ASSERT_TRUE(ictx.dest_mask[ST]);
    TEST_ASSERT_TRUE(ictx.dest_mask[RG]);
}

static void test_lea_rules(void)
{
    InstrCtx ictx;
    init_ictx(&ictx, "lea");

    TEST_ASSERT_TRUE(classify_opcode(&ictx));
    TEST_ASSERT_TRUE(ictx.src_exists);
    TEST_ASSERT_TRUE(ictx.dest_exists);

    /* src: DT, ST only */
    TEST_ASSERT_FALSE(ictx.src_mask[IM]);
    TEST_ASSERT_TRUE(ictx.src_mask[DT]);
    TEST_ASSERT_TRUE(ictx.src_mask[ST]);
    TEST_ASSERT_FALSE(ictx.src_mask[RG]);

    /* dest: standard modes (DT, ST, RG) */
    TEST_ASSERT_FALSE(ictx.dest_mask[IM]);
    TEST_ASSERT_TRUE(ictx.dest_mask[DT]);
    TEST_ASSERT_TRUE(ictx.dest_mask[ST]);
    TEST_ASSERT_TRUE(ictx.dest_mask[RG]);
}

static void test_single_operand_standard_group(void)
{
    char *single_op_group[] = {
        "not", "clr", "inc", "dec", "jmp", "bne", "get", "jsr"
    };

    for (int i = 0; i < 8; ++i)
    {
        InstrCtx ictx;
        init_ictx(&ictx, single_op_group[i]);

        TEST_ASSERT_TRUE(classify_opcode(&ictx));
        TEST_ASSERT_FALSE(ictx.src_exists);
        TEST_ASSERT_TRUE(ictx.dest_exists);

        /* src: none */
        TEST_ASSERT_FALSE(ictx.src_mask[IM]);
        TEST_ASSERT_FALSE(ictx.src_mask[DT]);
        TEST_ASSERT_FALSE(ictx.src_mask[ST]);
        TEST_ASSERT_FALSE(ictx.src_mask[RG]);

        /* dest: standard modes (DT, ST, RG) */
        TEST_ASSERT_FALSE(ictx.dest_mask[IM]);
        TEST_ASSERT_TRUE(ictx.dest_mask[DT]);
        TEST_ASSERT_TRUE(ictx.dest_mask[ST]);
        TEST_ASSERT_TRUE(ictx.dest_mask[RG]);
    }
}

static void test_single_operand_prn(void)
{
    InstrCtx ictx;
    init_ictx(&ictx, "prn");

    TEST_ASSERT_TRUE(classify_opcode(&ictx));
    TEST_ASSERT_FALSE(ictx.src_exists);
    TEST_ASSERT_TRUE(ictx.dest_exists);

    /* src: none */
    TEST_ASSERT_FALSE(ictx.src_mask[IM]);
    TEST_ASSERT_FALSE(ictx.src_mask[DT]);
    TEST_ASSERT_FALSE(ictx.src_mask[ST]);
    TEST_ASSERT_FALSE(ictx.src_mask[RG]);

    /* dest: all modes (IM, DT, ST, RG) */
    TEST_ASSERT_TRUE(ictx.dest_mask[IM]);
    TEST_ASSERT_TRUE(ictx.dest_mask[DT]);
    TEST_ASSERT_TRUE(ictx.dest_mask[ST]);
    TEST_ASSERT_TRUE(ictx.dest_mask[RG]);
}

static void test_zero_operand_group(void)
{
    char *zero_op_group[] = {"rts", "hlt"};

    for (int i = 0; i < 2; ++i)
    {
        InstrCtx ictx;
        init_ictx(&ictx, zero_op_group[i]);

        TEST_ASSERT_TRUE(classify_opcode(&ictx));
        TEST_ASSERT_FALSE(ictx.src_exists);
        TEST_ASSERT_FALSE(ictx.dest_exists);

        TEST_ASSERT_FALSE(ictx.src_mask[IM]);
        TEST_ASSERT_FALSE(ictx.src_mask[DT]);
        TEST_ASSERT_FALSE(ictx.src_mask[ST]);
        TEST_ASSERT_FALSE(ictx.src_mask[RG]);

        TEST_ASSERT_FALSE(ictx.dest_mask[IM]);
        TEST_ASSERT_FALSE(ictx.dest_mask[DT]);
        TEST_ASSERT_FALSE(ictx.dest_mask[ST]);
        TEST_ASSERT_FALSE(ictx.dest_mask[RG]);
    }
}

static void test_invalid_opcode_classification(void)
{
    InstrCtx ictx;

    init_ictx(&ictx, "invalid");
    TEST_ASSERT_FALSE(classify_opcode(&ictx));

    init_ictx(&ictx, "");
    TEST_ASSERT_FALSE(classify_opcode(&ictx));

    init_ictx(&ictx, NULL);
    TEST_ASSERT_FALSE(classify_opcode(&ictx));

    TEST_ASSERT_FALSE(classify_opcode(NULL));
}

void run_opcode_rules_tests(void)
{
    TEST_SUITE_START("Opcode & Addressing Rules");
    RUN_TEST(test_two_operand_mov_add_sub);
    RUN_TEST(test_cmp_rules);
    RUN_TEST(test_lea_rules);
    RUN_TEST(test_single_operand_standard_group);
    RUN_TEST(test_single_operand_prn);
    RUN_TEST(test_zero_operand_group);
    RUN_TEST(test_invalid_opcode_classification);
}
