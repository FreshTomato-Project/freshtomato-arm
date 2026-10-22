#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cap-ng.h>
#include <pthread.h>

//#define DEBUG 1

pthread_t thread1, thread2;

static void check_thread_call(int rc, const char *operation)
{
	if (rc) {
		fprintf(stderr, "%s: %s\n", operation, strerror(rc));
		exit(1);
	}
}

void *thread1_main(void *arg)
{
	capng_fill(CAPNG_SELECT_BOTH);
#ifdef DEBUG
	printf("thread1 filled capabilities\n");
#endif
	sleep(2);
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) < CAPNG_FULL) {
		printf("Capabilities missing when there should be some\n");
		exit(1);
	}
#ifdef DEBUG
		printf("SUCCESS: Full capabilities reported\n");
#endif
	return NULL;
}

void *thread2_main(void *arg)
{
	sleep(1);
#ifdef DEBUG
	printf("thread2 getting capabilities\n");
#endif
	if (capng_get_caps_process()) {
		printf("Unable to get process capabilities");
		exit(1);
	}
	if (capng_have_capabilities(CAPNG_SELECT_CAPS) != CAPNG_NONE) {
		printf("Detected capabilities when there should not be any\n");
		exit(1);
	}
	capng_clear(CAPNG_SELECT_BOTH);
#ifdef DEBUG
	printf("SUCCESS: No capabilities reported\n");
#endif
	return NULL;
}

int main(void)
{
	// This test must be run as root which naturally has all capabilities
	// set. So, we need to clear the capabilities so that we can see if
	// the test works.
	capng_clear(CAPNG_SELECT_CAPS);
	if (capng_apply(CAPNG_SELECT_CAPS)) {
		printf("Clearing capabilities failed");
		return 1;
	}

	printf("Testing thread separation of capabilities\n");
	check_thread_call(pthread_create(&thread1, NULL, thread1_main, NULL),
			  "Creating thread1");
	check_thread_call(pthread_create(&thread2, NULL, thread2_main, NULL),
			  "Creating thread2");
	/* A timed sleep can return before the workers finish their checks. */
	check_thread_call(pthread_join(thread1, NULL), "Joining thread1");
	check_thread_call(pthread_join(thread2, NULL), "Joining thread2");
	return 0;
}

