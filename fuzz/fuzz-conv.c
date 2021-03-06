#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <gc.h>
#include "wc.h"

char *get_null_terminated(const uint8_t *data, size_t size) {
    char *new_str = (char *)malloc(size+1);
    if (new_str == NULL){
            return NULL;
    }
    memcpy(new_str, data, size);
    new_str[size] = '\0';
    return new_str;
}

static void *die_oom(size_t bytes) {
    fprintf(stderr, "Out of memory: %lu bytes unavailable!\n", (unsigned long)bytes);
    exit(1);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    static int init_done = 0;

    if (!init_done) {
	GC_INIT();
#if (GC_VERSION_MAJOR>7) || ((GC_VERSION_MAJOR==7) && (GC_VERSION_MINOR>=2))
	GC_set_oom_fn(die_oom);
#else
	GC_oom_fn = die_oom;
#endif
	init_done = 1;
    }

    if (size < 30) {
        return 0;
    }

    char *new_str1 = get_null_terminated(data, 20);
    data += 20; size -= 20;
    char *new_str2 = get_null_terminated(data, size);

    wc_ces old, from, to;
    from = wc_guess_charset_short(new_str1,0);
    to = wc_guess_charset_short(new_str2, 0);

    char filename[256];
    sprintf(filename, "/tmp/libfuzzer.%d", getpid());

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
            return 0;
    }
    fwrite(data, size, 1, fp);
    fclose(fp);

    FILE *f = fopen(filename, "r");
    Str s = Strfgetall(f);
    wc_Str_conv_with_detect(s, &from, from, to);
    if (s != NULL) {
            Strfree(s);
    }

    unlink(filename);

    free(new_str1);
    free(new_str2);
    return 0;
}
