
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "rs_decoder.h"
#include "rs_encoder.h"
#include "rs_gf.h"

#define CHECK_ERR_COUNT 0

static unsigned int
test_one(unsigned int num_errs)
{
    unsigned int i;
    int inbuf[233 * 8], encbuf[255 * 8];
    int outbuf[223 * 8], codebuf[255 * 8];
    bool errpos[255] = { false };
    unsigned int err = 0;
#if CHECK_ERR_COUNT
    unsigned int errcount = 0;
#endif

    for (i = 0; i < 223 * 8; i++)
	inbuf[i] = rand() & 1;

    rs_encode(inbuf, encbuf);

    /* Inject some errors. */
    for (i = 0; i < num_errs; ) {
	unsigned int pos = rand() % (255 * 8);

	encbuf[pos] ^= 1;
	if (errpos[pos / 8])
	    /* In the same symbol, it won't be a new error. */
	    continue;

	errpos[pos / 8] = true;
	i++;
    }
#if CHECK_ERR_COUNT
    errcount = rs_decode(encbuf, codebuf, outbuf);
    if (errcount != num_errs)
	printf("Error count mismatch: %u %u\n", num_errs, errcount);
#else
    rs_decode(encbuf, codebuf, outbuf);
#endif

    for (i = 0; i < 223; i++) {
	if (outbuf[i] != inbuf[i]) {
	    err = 1;
	    break;
	}
    }

    return err;
}

static unsigned int
test_loop(unsigned int num_loops,
	  unsigned int num_errs)
{
    unsigned int i, errcount = 0;

    for (i = 0; i < num_loops; i++)
	errcount += test_one(num_errs);
    printf("Testing with %u errors: %u output errors\n", num_errs, errcount);
    return errcount;
}

int
main(int argc, char *argv[])
{
    unsigned int i;
    int err = 0;

    srand(time(NULL));

    rs_gf_init(8, 255, 223, 255 - 223);

    for (i = 0; ; i++) {
	unsigned int errs = test_loop(100, i);

	if (errs == 100)
	    break;
    }

    return err;
}
