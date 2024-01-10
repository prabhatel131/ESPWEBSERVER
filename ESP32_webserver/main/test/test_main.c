#include "unity.h"
#include "url_decoding_module.h"

void test_url_decode(void)
{
    TEST_ASSERT_EQUAL_STRING("Hello World", url_decode("Hello%20World"));
    
}





int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_url_decode);
    

    UNITY_END();
    return 0;
}
