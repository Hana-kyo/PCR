#include <stdio.h>
#include <stdlib.h>
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

	printf("SAPI initialized successfully. Ready for GetRandom performance test.\n");

	TPM2B_DIGEST random_bytes = { 0 };
	TSS2L_SYS_AUTH_RESPONSE rsp_auths = {.count = 0};
	UINT16 bytes_requested = 20;

	struct timespec start, end;

	printf("Starting %d iterations of Tss2_Sys_GetRandom...\n", LOOP_COUNT);
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < LOOP_COUNT; i++) {
	       rc = Tss2_Sys_GetRandom (
		s_ctx,
		NULL,
		bytes_requested,
		&random_bytes,
		&rsp_auths
		);

	       if (rc != TSS2_RC_SUCCESS) {
		       printf("Error at iteration %d: 0x%x\n", i, rc);
		       break;
	       }
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
	double total_ms = (double)elapsed_ns / 1000000.0;
	double avg_ms = total_ms / (double)LOOP_COUNT;

	printf("\n---Performance Evaluation Results(SAPI GetRandom)---\n");
	printf("Total time for %d random requests: %.3f ms\n", LOOP_COUNT, total_ms);
	printf("Average time per request: %.3f ms\n", avg_ms);

	Tss2_Sys_Finalize(s_ctx);
	free(s_ctx);
	Tss2_TctiLdr_Finalize(&tcti);
	return 0;
}
