#include "limits.h"
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include "url_decoding_module.h"


TEST_CASE("url_decoding_module check", "[url_decoding_module]")
{
    
    TEST_ASSERT_EQUAL_STRING("Hello World", url_decode("Hello%20World"));
}

/*
void test_url_decode(void)
{
    TEST_ASSERT_EQUAL_STRING("Hello World", url_decode("Hello%20World"));
    
}
*/


void setUp()
{

}

void tearDown()
{

}


static void print_banner(const char *text);

void app_main(void)
{
    print_banner("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

static void print_banner(const char *text)
{
    printf("\n#### %s #####\n\n", text);
}

