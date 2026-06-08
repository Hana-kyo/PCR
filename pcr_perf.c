#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tss2/tss2_esys.h>

#define LOOP_COUNT 100

int main() {
	TSS2_RC rc;
	ESYS_CONTEXT *esys_context = NULL;

	rc = Esys_Initialize(&esys_context, NULL, NULL);
	if (rc != TSS2_RC_SUCCESS) {
		fprintf(stderr, "Error: Failed to initialize ESAPI (0x%x)\n",rc);
		return 1;
	}

	printf("ESAPI initialized successfully. Using /dev/tpmrm0.\n");
	printf("Starting %d iterations of Esys_PCR_Read...\n",LOOP_COUNT);

	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < LOOP_COUNT; i++) {
		TPML_PCR_SELECTION pcr_selection_in = {
			.count = 1,
			.pcrSelections = {
				{
					.hash = TPM2_ALG_SHA256,
					.sizeofSelect = 3,
					.pcrSelect = { 0x01, 0x00, 0x00}
				}
			}
		};

		UINT32 pcr_update_counter;
		TPML_PCR_SELECTION *pcr_selection_out = NULL;
		TPML_DIGEST *pcr_values = NULL;

		rc = Esys_PCR_Read(
				esys_context,
				ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
				&pcr_selection_in,
				&pcr_update_counter,
				&pcr_selection_out,
				&pcr_values
				);

		if (rc != TSS2_RC_SUCCESS) {
			fprintf(stderr, "Error at iteration %d: 0x%x\n", i, rc);
			if (pcr_selection_out) free(pcr_selection_out);
			if (pcr_values) free(pcr_values);
			break;
		}

		if (pcr_selection_out) free(pcr_selection_out);
		if (pcr_values) free(pcr_values);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
	double total_ms = (double)elapsed_ns / 1000000.0;
	double avg_ms = total_ms / (double)LOOP_COUNT;

	printf("\n--- Performance Evaluation Results ---\n");
	printf("Total time for %d reads : %.3f ms\n", LOOP_COUNT, total_ms);
	printf("Average time per read : %.3f ms\n", avg_ms);

	Esys_Finalize(&esys_context);
	return 0;
}
