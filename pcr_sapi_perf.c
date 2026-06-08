#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tss2/tss2_sys.h>
#include <tss2/tss2_tctildr.h>

#define LOOP_COUNT 100

int main() {
	TSS2_RC rc;
	size_t size;
	TSS2_TCTI_CONTEXT *tcti = NULL;
	TSS2_SYS_CONTEXT *s_ctx = NULL;

	rc = Tss2_TctiLdr_Initialize(NULL, &tcti);
	if (rc != TSS2_RC_SUCCESS) {
		printf("tctildr initialize failed\n");
		return 1;
	}

	size = Tss2_Sys_GetContextSize(0);
	s_ctx = (TSS2_SYS_CONTEXT*)calloc(1, size);

	rc = Tss2_Sys_Initialize(s_ctx, size, tcti, NULL);
	if (rc != TSS2_RC_SUCCESS) {
		printf("sys initialize failed:0x%x\n", rc);
		free(tcti);
		free(s_ctx);
		return 1;
	}

	printf("SAPI initialized successfully. Using default TCTI.\n");
	printf("Starting %d iterations of Tss2_Sys_PCR_Read...\n", LOOP_COUNT);

	TPML_PCR_SELECTION pcr_selection_in;
	memset(&pcr_selection_in, 0, sizeof(TPML_PCR_SELECTION));
	pcr_selection_in.count = 1;
	pcr_selection_in.pcrSelections[0].hash = TPM2_ALG_SHA256;
	pcr_selection_in.pcrSelections[0].sizeofSelect = 3;
	pcr_selection_in.pcrSelections[0].pcrSelect[0] = 1;
	
	UINT32 pcr_counter = 0;
        TPML_PCR_SELECTION pcr_selection_out;
	memset(&pcr_selection_out, 0, sizeof(TPML_PCR_SELECTION));

	TPML_DIGEST pcr_values;
	memset(&pcr_values, 0, sizeof(TPML_DIGEST));

	struct timespec start, end;

	clock_gettime(CLOCK_MONOTONIC, &start);

	for(int i = 0; i < LOOP_COUNT; i++) {
		pcr_values.count = 1;

		rc = Tss2_Sys_PCR_Read_Prepare(s_ctx, &pcr_selection_in);
		if (rc != TSS2_RC_SUCCESS) {
			printf("Prepare failed at %d: 0x%x\n", i, rc);
			break;
		}

		rc = Tss2_Sys_Execute(s_ctx);
		if (rc != TSS2_RC_SUCCESS) {
			printf("Execute failed at %d: 0x%x\n", i, rc);
			break;
		}

		rc = Tss2_Sys_PCR_Read_Complete(s_ctx, &pcr_counter, &pcr_selection_out, &pcr_values);
		if (rc != TSS2_RC_SUCCESS) {
			printf("Complete failed at %d: 0x%x\n", i, rc);
			break;
		}
	}
		
	clock_gettime(CLOCK_MONOTONIC, &end);
	if (rc == TSS2_RC_SUCCESS && pcr_values.count > 0) {
		printf("Successfully read PCR 0 via SAPI! First 4 bytes:");
		for(int j = 0; j < 4; j++) { 
			printf("%02x", pcr_values.digests[0].buffer[j]);
		}
		printf("\n");
									        }

	long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
	double total_ms = (double)elapsed_ns / 1000000.0;
	double avg_ms = total_ms / (double)LOOP_COUNT;

	printf("\n--- Performance Evaluation Results (SAPI) ---\n");
	printf("Total time for %d reads : %.3f ms\n", LOOP_COUNT, total_ms);
	printf("Average time per read : %.3f ms\n", avg_ms);

	Tss2_Sys_Finalize(s_ctx);
	free(s_ctx);
	Tss2_TctiLdr_Finalize(&tcti);
	return 0;
}



