#include <mysql/mysql.h>
#include <string.h>
#include <stdint.h>

#define FNV1A_64_INIT 0xcbf29ce484222325ULL
#define HASH_NULL_DEFAULT 0x0a0b0c0d
#define FNV_64_PRIME 0x100000001b3ULL

extern "C" {
    bool fnv1a_64_init(UDF_INIT* initid, UDF_ARGS* args, char* message);
    void fnv1a_64_deinit(UDF_INIT* initid);
    my_ulonglong fnv1a_64(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error);
}

static my_ulonglong hash64a(const void *buf, size_t len, my_ulonglong hval) {
    const unsigned char *bp = (const unsigned char*)buf;
    const unsigned char *be = bp + len;

    for (; bp != be; ++bp) {
        hval ^= (my_ulonglong)*bp;
        hval *= FNV_64_PRIME;
    }

    return hval;
}

bool fnv1a_64_init(UDF_INIT* initid, UDF_ARGS* args, char* message) {
    if (args->arg_count == 0) {
        strcpy(message, "FNV1A_64 requires at least one argument");
        return true;
    }
    initid->maybe_null = false;
    return false;
}

void fnv1a_64_deinit(UDF_INIT* initid) {}

my_ulonglong fnv1a_64(UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error) {
    uint32_t null_default = HASH_NULL_DEFAULT;
    my_ulonglong result = FNV1A_64_INIT;

    for (unsigned int i = 0; i < args->arg_count; ++i) {
        if (args->args[i] != NULL) {
            switch (args->arg_type[i]) {
                case STRING_RESULT:
                case DECIMAL_RESULT:
                    result = hash64a(args->args[i], args->lengths[i], result);
                    break;
                case REAL_RESULT: {
                    double real_val = *((double*) args->args[i]);
                    result = hash64a(&real_val, sizeof(real_val), result);
                    break;
                }
                case INT_RESULT: {
                    long long int_val = *((long long*) args->args[i]);
                    result = hash64a(&int_val, sizeof(int_val), result);
                    break;
                }
                default:
                    break;
            }
        } else {
            result = hash64a(&null_default, sizeof(null_default), result);
        }
    }
    return result;
}
