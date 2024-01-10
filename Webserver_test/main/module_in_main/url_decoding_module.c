#include "include/url_decoding_module.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *url_decode(const char *input)
{
    size_t len = strlen(input);
    char *output = (char *)malloc(len + 1); // +1 for null terminator
    if (!output)
    {
        // Handle memory allocation failure
        return NULL;
    }

    size_t i, j = 0;
    for (i = 0; i < len; ++i)
    {
        if (input[i] == '%' && i + 2 < len)
        {
            // Valid percent encoding
            //int hex1, hex2;
            int hex1;
            if (sscanf(input + i + 1, "%2x", &hex1) == 1)
            {
                output[j++] = hex1;
                i += 2;
            }
        }
        else if (input[i] == '+')
        {
            // '+' is commonly used to represent space in URL encoding
            output[j++] = ' ';
        }
        else
        {
            // Copy the character as is
            output[j++] = input[i];
        }
    }

    output[j] = '\0'; // Null-terminate the string
    return output;
}
