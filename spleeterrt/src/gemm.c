#include "conv_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
void gemm_nn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
{
	int i, j, k;
#pragma omp parallel for
	for (i = 0; i < M; ++i) {
		for (k = 0; k < K; ++k) {
			register float A_PART = ALPHA * A[i*lda + k];
			for (j = 0; j < N; ++j) {
				C[i*ldc + j] += A_PART * B[k*ldb + j];
			}
		}
	}
}
void gemm_nt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
{
	int i, j, k;
#pragma omp parallel for
	for (i = 0; i < M; ++i) {
		for (j = 0; j < N; ++j) {
			register float sum = 0;
			for (k = 0; k < K; ++k) {
				sum += ALPHA * A[i*lda + k] * B[j*ldb + k];
			}
			C[i*ldc + j] += sum;
		}
	}
}
void gemm_tn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
{
	int i, j, k;
#pragma omp parallel for
	for (i = 0; i < M; ++i) {
		for (k = 0; k < K; ++k) {
			register float A_PART = ALPHA * A[k*lda + i];
			for (j = 0; j < N; ++j) {
				C[i*ldc + j] += A_PART * B[k*ldb + j];
			}
		}
	}
}
void gemm_tt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
{
	int i, j, k;
#pragma omp parallel for
	for (i = 0; i < M; ++i) {
		for (j = 0; j < N; ++j) {
			register float sum = 0;
			for (k = 0; k < K; ++k) {
				sum += ALPHA * A[i + k * lda] * B[k + j * ldb];
			}
			C[i*ldc + j] += sum;
		}
	}
}
// Platform-specific BLAS acceleration:
// - macOS: Apple Accelerate framework
// - Windows: OpenBLAS (if USE_OPENBLAS defined) or pure C fallback
// - Linux: OpenBLAS (if USE_OPENBLAS defined) or pure C fallback

#if defined(__APPLE__)
#define USE_ACCELERATE
#include <Accelerate/Accelerate.h>
#elif defined(USE_OPENBLAS)
// OpenBLAS header location varies by system
// MSYS2/MinGW: openblas/cblas.h, Linux: cblas.h or openblas/cblas.h
#if defined(__MINGW32__) || defined(__MINGW64__)
#include <openblas/cblas.h>
#else
#include <cblas.h>
#endif
#endif

void gemm_cpu(int TA, int TB, int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float BETA, float *C, int ldc)
{
#if defined(USE_ACCELERATE) || defined(USE_OPENBLAS)
	// Use optimized CBLAS (Accelerate on macOS, OpenBLAS on Windows/Linux)
	if (!TA && !TB)
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, ALPHA, A, lda, B, ldb, BETA, C, ldc);
	else if (TA && !TB)
		cblas_sgemm(CblasRowMajor, CblasTrans, CblasNoTrans, M, N, K, ALPHA, A, lda, B, ldb, BETA, C, ldc);
	else if (!TA && TB)
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, M, N, K, ALPHA, A, lda, B, ldb, BETA, C, ldc);
	else
		cblas_sgemm(CblasRowMajor, CblasTrans, CblasTrans, M, N, K, ALPHA, A, lda, B, ldb, BETA, C, ldc);
#else
	// Pure C fallback with OpenMP parallelization
	for (int i = 0; i < N * M; ++i)
		C[i] *= BETA;
	if (!TA && !TB)
		gemm_nn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
	else if (TA && !TB)
		gemm_tn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
	else if (!TA && TB)
		gemm_nt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
	else
		gemm_tt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
#endif
}
void gemm(int TA, int TB, int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float BETA, float *C, int ldc)
{
	gemm_cpu(TA, TB, M, N, K, ALPHA, A, lda, B, ldb, BETA, C, ldc);
}

