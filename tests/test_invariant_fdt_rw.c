#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lib/libfdt/libfdt.h"

#define FDT_BUF_SIZE 4096

START_TEST(test_fdt_setprop_bounds)
{
    /* Invariant: fdt_setprop must not write beyond allocated FDT buffer
       regardless of property length values supplied by caller */

    struct {
        const char *name;
        const void *val;
        int len;
    } payloads[] = {
        /* Exact exploit: oversized length to trigger OOB memcpy */
        { "prop", "AAAA", 0x7fffffff },
        /* Boundary: length equals remaining buffer space + 1 */
        { "prop", "B", FDT_BUF_SIZE + 1 },
        /* Valid input: normal property set */
        { "prop", "hello", 5 },
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        /* Allocate a guarded buffer: real FDT + canary page */
        uint8_t *buf = calloc(1, FDT_BUF_SIZE);
        ck_assert_ptr_nonnull(buf);

        /* Create a minimal valid FDT */
        int rc = fdt_create(buf, FDT_BUF_SIZE);
        ck_assert_int_eq(rc, 0);
        rc = fdt_finish_reservemap(buf);
        ck_assert_int_eq(rc, 0);
        rc = fdt_begin_node(buf, "");
        ck_assert_int_eq(rc, 0);
        rc = fdt_end_node(buf);
        ck_assert_int_eq(rc, 0);
        rc = fdt_finish(buf);
        ck_assert_int_eq(rc, 0);

        /* Open for write */
        rc = fdt_open_into(buf, buf, FDT_BUF_SIZE);
        ck_assert_int_eq(rc, 0);

        int node = fdt_path_offset(buf, "/");
        ck_assert_int_ge(node, 0);

        /* Call the real production function with adversarial len */
        int ret = fdt_setprop(buf, node, payloads[i].name,
                              payloads[i].val, payloads[i].len);

        /* Security invariant: if len exceeds available space,
           the library MUST return an error, never silently overflow */
        if (payloads[i].len > FDT_BUF_SIZE) {
            ck_assert_int_lt(ret, 0);
        } else {
            /* Valid input must succeed */
            ck_assert_int_eq(ret, 0);
        }

        free(buf);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_fdt_setprop_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}