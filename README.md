# LRE

LRE (Lexicographic REpresentation) - is a rapid serializer of numbers and strings for composite keys. It converts a set of values into special ASCII-string providing a natural order for serialized numbers at lexicographically comparing. This is very helpful for key-value databases like LMDB or Berkeley DB which stores arbitrary keys as byte arrays in lexicographical order.

Features:
* Lexicographic natural order
* Screamingly fast
* ASCII-safe
* Header-only library
* Cross platform C code with no dependencies
* Simple sequential SAX-like API
* Uniform format for floating-point and integer numbers

Data types:
* Strings (can be marked as raw or UTF8)
* Signed 64-bit integers
* Double with 15 decimal digits of fixed precision
* +INF and -INF are supported
* Big numbers are supported by external assistance

Limits:
* NaN not supported due to ambiguity
* No difference between -0.0 and +0.0
* Built-in float-point range is -281474976710655.0, 281474976710655.0
* Built-in integer range is -9223372036854775808, 9223372036854775807

#### Serialization

```C
#include <stdio.h>
#include <lre.h>

int main() {
    /* Optional; &error may be replaced by null pointer */
    lre_error_t error = 0;

    /* 512 is a size of preallocated capacity */
    lre_buffer_t *lrbuf = lre_buffer_create(512, &error);

    lre_pack_str(lrbuf, (const uint8_t *) "abc", 3, 0, &error);
    lre_pack_int(lrbuf, 0xffaa, &error);
    lre_pack_float(lrbuf, 10.9, &error);

    if (error) {
        printf("LRE Error: %s\n", lre_strerror(error));
        return 1;
    }

    /* lrbuf->data is a result string */
    /* lrbuf->size is a length of result string */
    printf("lre: %s\n", lrbuf->data);
    printf("len: %zu\n", lrbuf->size);

    lre_buffer_reset(lrbuf, &error);
}
```

Output:
```
lre: X616263H+Nffaa+M0a900000000000000+
len: 34
```

#### Deserialization
```C
#include <string.h>
#include <stdio.h>
#include <lre.h>

int handler_str(lre_loader_t *loader, lre_slice_t *slice, lre_mod_t mod) {
    uint8_t buf[64] = {0};
    lrex_read_str(&slice->src, buf, lre_slice_len(slice), 0);
    printf("String: %s\n", buf);
    return LRE_OK;
}

int handler_int(lre_loader_t *loader, int64_t value) {
    printf("Integer: %lli\n", value);
    return LRE_OK;
}

int handler_float(lre_loader_t *loader, double value) {
    printf("Float: %f\n", value);
    return LRE_OK;
}

int main() {
    char  *str = "X616263H+Nffaa+M0a900000000000000+";
    size_t len = strlen(str);

    lre_error_t error = 0;
    lre_loader_t loader;
    lre_loader_init(&loader, 0);

    /* Pointer to any inner application object */
    loader.app_private = 0;

    /* Register callbacks for each type */
    loader.handler_int   = &handler_int;
    loader.handler_float = &handler_float;
    loader.handler_str   = &handler_str;

    /* Tokenization */
    lre_tokenize(&loader, (const uint8_t *) str, len, &error);

    if (error) {
        printf("LRE Error: %s\n", lre_strerror(error));
        return 1;
    }
}
```

Output:
```
String: abc
Integer: 65450
Float: 10.900000
```
