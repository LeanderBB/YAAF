/*
 * YAAF Test Compression
 * Copyright (c) 2014 Leander Beernaert
 *
 * YAAFCL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YAAFCL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YAAFCL. If not, see <http://www.gnu.org/licenses/>.
 *
 * You can contact the author at :
 * - YAAF source repository : http://www.github.com/LeanderBB/YAAF
 */


#include "YAAF.h"
#include "YAAF_HashMap.h"

#define DATA_COUNT 11
#define DUPLICATE_KEY_IDX 4
#define DUPLICATE_KEY_IDX_DUPLICATE 10

static const int g_data[DATA_COUNT] = {20, 30, 40, 50, 60, 70, 80, 90, 10, 100, 200};
static const char* g_keys[DATA_COUNT] =
{
    "Nathalie",
    "Lashunda",
    "Teressa",
    "Herb",
    "Wade",
    "Mirna",
    "Lon",
    "Jesusa",
    "Loreen",
    "Karine",
    "Wade"
};

static int
test_put_remove_hm(YAAF_HashMap* hm)
{
    int i, res = YAAF_SUCCESS;
    for (i = 0; i < DATA_COUNT && res == YAAF_SUCCESS; ++i)
    {
        res = YAAF_HashMapPut(hm, g_keys[i], &g_data[i]);
    }

    for (i = 0; i < DATA_COUNT && res == YAAF_SUCCESS; ++i)
    {
        int idx = DATA_COUNT - (i + 1);
        res = YAAF_HashMapRemove(hm, g_keys[idx]);

        /* duplicate key  must fail since it has been removed already*/
        if (idx == DUPLICATE_KEY_IDX)
        {
            res = (res == YAAF_FAIL) ? YAAF_SUCCESS : YAAF_FAIL;
        }
    }
    return res;
}

static int
test_put_remove()
{
    YAAF_HashMap hm;
    int res = YAAF_FAIL;
    YAAF_HashMapInit(&hm, 1);
    res = test_put_remove_hm(&hm);
    YAAF_HashMapDestroy(&hm);
    return res;
}


static int
test_put_remove_put_remove()
{
    YAAF_HashMap hm;
    int res = YAAF_FAIL;
    YAAF_HashMapInit(&hm, 1);
    res = test_put_remove_hm(&hm);
    if (res == YAAF_SUCCESS)
    {
        res = test_put_remove_hm(&hm);
    }
    YAAF_HashMapDestroy(&hm);
    return res;
}

static int
test_get()
{
    YAAF_HashMap hm;
    int i, res = YAAF_SUCCESS;
    YAAF_HashMapInit(&hm, 1);

    for (i = 0; i < DATA_COUNT && res == YAAF_SUCCESS; ++i)
    {
        res = YAAF_HashMapPut(&hm, g_keys[i], &g_data[i]);
    }

    if (res != YAAF_FAIL)
    {
        for (i = 0; i < DATA_COUNT && res == YAAF_SUCCESS; ++i)
        {
            const int* ptr = (const int*) YAAF_HashMapGet(&hm, g_keys[i]);
            if (i != DUPLICATE_KEY_IDX)
            {
                res = (ptr && *ptr == g_data[i]) ? YAAF_SUCCESS : YAAF_FAIL;
            }
            else
            {
                /* check duplicate value instead*/
                res = (ptr && *ptr == g_data[DUPLICATE_KEY_IDX_DUPLICATE]) ? YAAF_SUCCESS : YAAF_FAIL;
            }
        }
    }
    YAAF_HashMapDestroy(&hm);
    return res;
}

YAAF_INLINE static int
is_in_data(const int v)
{
    int i;
    for(i = 0; i < DATA_COUNT; ++i)
    {
        if (g_data[i] == v)
        {
            return 1;
        }
    }
    return 0;
}

static int
test_iter()
{
    YAAF_HashMap hm;
    int i, res = YAAF_SUCCESS;
    YAAF_HashMapInit(&hm, 1);

    for (i = 0; i < DATA_COUNT && res == YAAF_SUCCESS; ++i)
    {
        res = YAAF_HashMapPut(&hm, g_keys[i], &g_data[i]);
    }

    if (res != YAAF_FAIL)
    {
        const YAAF_HashMapEntry* it, *it_end;
        it_end = YAAF_HashMapItEnd(&hm);
        for (it = YAAF_HashMapItBegin(&hm);
             it != it_end && res == YAAF_SUCCESS;
             YAAF_HashMapItNext(&hm, &it))
        {
            const int* ptr = (const int*) YAAF_HashMapItGet(it);

            res = (is_in_data(*ptr)) ? YAAF_SUCCESS : YAAF_FAIL;
        }
    }
    YAAF_HashMapDestroy(&hm);
    return res;
}

int main()
{
    int exit_status = EXIT_FAILURE;
    YAAF_Init(NULL);


    if (test_put_remove() != YAAF_SUCCESS)
    {
        fprintf(stderr, "test_put_remove() failed\n");
        goto exit;
    }

    if (test_put_remove_put_remove() != YAAF_SUCCESS)
    {
        fprintf(stderr, "test_put_remove_put_remove() failed\n");
        goto exit;
    }

    if (test_get() != YAAF_SUCCESS)
    {
        fprintf(stderr, "test_get() failed\n");
        goto exit;
    }

    if (test_iter() != YAAF_SUCCESS)
    {
        fprintf(stderr, "test_get() failed\n");
        goto exit;
    }

    exit_status = YAAF_SUCCESS;
exit:
    YAAF_Shutdown();
    return exit_status;
}
