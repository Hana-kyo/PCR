#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tss2/tss2_esys.h>

#define LOOP_COUNT 100

int main() {
	TSS2_RC rc;
	ESYS_CONTEXT *esys_context = NULL;

	rc = Esys_Initialize(&esys_context, NULL, NULL);
	if(rc != TSS2_RC_SUCCESS) {
		fprintf(stderr,"Error: Failed to intialize ESAPI (0x%x)\n", rc);
		return 1;
	}

	printf("ESAPI initialized successfully. Ready for GetRandom performance test.\n");
	printf("Starting %d iterations of Esys_GetRandom...\n", LOOP_COUNT);

	UINT16 bytes_requested = 20;
	TPM2B_DIGEST *random_bytes = NULL;

	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < LOOP_COUNT; i++) {
		rc = Esys_GetRandom(
				esys_context,
				ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
				bytes_requested,
				&random_bytes
				);

		if (rc != TSS2_RC_SUCCESS) {
			fprintf(stderr, "Error at iteration %d: 0x%x\n", i, rc);
			if (random_bytes) free(random_bytes);
			break;
		}

		if (random_bytes) {
			free(random_bytes);
			random_bytes = NULL;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
	double total_ms = (double)elapsed_ns / 1000000.0;
	double avg_ms = total_ms / (double)LOOP_COUNT;

	printf("\n--- Performance Evaluation Results (ESAPI GetRandom) ---\n");
	printf("Total time for %d random requests : %.3f ms\n", LOOP_COUNT, total_ms);
	printf("Average time per request : %.3f ms\n", avg_ms);

	Esys_Finalize(&esys_context);
	return 0;
}

