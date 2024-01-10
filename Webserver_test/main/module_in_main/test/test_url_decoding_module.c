#include "limits.h"
#include "unity.h"
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
