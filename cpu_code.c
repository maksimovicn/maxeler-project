/**
 * Document: MaxCompiler Tutorial (maxcompiler-tutorial)
 * Chapter: 8      Example: 1      Name: Simple offset
 * MaxFile name: SimpleOffset
 * Summary:
 *   Streams floats to the DFE and confirms the stream that comes back has each element added
 *     added to its neighbouring elements.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <MaxSLiCInterface.h>
#include "DFE.h"
#include "nrutil.h"

#define SWAP(a,b) itemp=(a);(a)=(b);(b)=itemp;
#define M 7
#define NSTACK 50

/* Indexes an array arr[1..n], i.e., outputs the array indx[1..n] such that arr[indx[j]] is
in ascending order for j = 1, 2,...,N. The input quantities n and arr are not changed. */

void indexx(unsigned long n, float arr[], unsigned long indx[]) {
	unsigned long i,indxt,ir=n,itemp,j,k,l=1;
	int jstack=0,*istack;
	float a;
	istack=ivector(1,NSTACK);
	for (j=1;j<=n;j++) indx[j]=j;
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				indxt=indx[j];
				a=arr[indxt];
				for (i=j-1;i>=l;i--) {
					if (arr[indx[i]] <= a) break;
					indx[i+1]=indx[i];
				}
				indx[i+1]=indxt;
			}
			if (jstack == 0) break;
			ir=istack[jstack--];
			l=istack[jstack--];
		} else {
			k=(l+ir) >> 1;
			SWAP(indx[k],indx[l+1]);
			if (arr[indx[l]] > arr[indx[ir]]) {
				SWAP(indx[l],indx[ir])
			}
			if (arr[indx[l+1]] > arr[indx[ir]]) {
				SWAP(indx[l+1],indx[ir])
			}
			if (arr[indx[l]] > arr[indx[l+1]]) {
				SWAP(indx[l],indx[l+1])
			}
			i=l+1;
			j=ir;
			indxt=indx[l+1];
			a=arr[indxt];
			for (;;) {
				do i++; while (arr[indx[i]] < a);
				do j--; while (arr[indx[j]] > a);
				if (j < i) break;
				SWAP(indx[i],indx[j])
			}
			indx[l+1]=indx[j];
			indx[j]=indxt;
			jstack += 2;
			if (jstack > NSTACK) nrerror("NSTACK too small in indexx.");
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
	
	free_ivector(istack,1,NSTACK);
	
}

/* Sorts an array ra[1..n] into ascending numerical order while making the corresponding rearrangements
of the arrays rb[1..n] and rc[1..n]. An index table is constructed via the
routine indexx. */
void sort3(unsigned long n, float ra[], float rb[], float rc[]) {
	//void indexx(unsigned long n, float arr[], unsigned long indx[]);
	unsigned long j,*iwksp;
	float *wksp;
	iwksp=lvector(1,n);
	wksp=vector(1,n);
	indexx(n,ra,iwksp); //Make the index table.
	for (j=1;j<=n;j++) wksp[j]=ra[j]; //Save the array ra.
	for (j=1;j<=n;j++) ra[j]=wksp[iwksp[j]]; //Copy it back in rearranged order.
	for (j=1;j<=n;j++) wksp[j]=rb[j]; //Ditto rb.
	for (j=1;j<=n;j++) rb[j]=wksp[iwksp[j]];
	for (j=1;j<=n;j++) wksp[j]=rc[j]; //Ditto rc.
	for (j=1;j<=n;j++) rc[j]=wksp[iwksp[j]];
	free_vector(wksp,1,n);
	free_lvector(iwksp,1,n);
}

/* Given indx[1..n] as output from the routine indexx, returns an array irank[1..n], the
corresponding table of ranks. */
void rankCPU(unsigned long n, unsigned long indx[], unsigned long irank[]) {
	
	unsigned long j;
	
	for (j=1;j<=n;j++) irank[indx[j]]=j;
	
}

int check(int n, float *dataOut, float *expected)
{
	int status = 0;
	for (int i = 1; i < n - 1; i++) // First and last data point are undefined!
	{
		if (dataOut[i] != expected[i]) {
			fprintf(
				stderr, "Output data @ %d = %f (expected %f)\n",
				i, dataOut[i], expected[i]);
			fprintf(stderr, "This is incorrect!\n");
			status = 1;
		}
	}
	return status;
}

void SimpleOffsetCPU(int n, float *dataIn, float *dataOut)
{
	for (int i = 1; i < n - 1; i++) // First and last data point are undefined!
	{
		dataOut[i] = dataIn[i-1] + dataIn[i] + dataIn[i+1];
	}
}

int main()
{
	const int N = 1024;
	size_t sizeBytes = N * sizeof(float);
	float *dataIn;
	float *dataToSort = malloc(sizeBytes);
	float *dataOut = malloc(sizeBytes);
	float *expected;
	time_t time;
	unsigned long j;
	float *arrayIndexSorted;
	float *rankArray;
	
	srand((unsigned) time(&t));

 	for (int i = 0; i < N; i++) {
		dataToSort[i] = rand();
		dataOut[i] = 0;
	}


	dataIn=lvector(1,n);
	arrayIndexSorted=vector(1,n);
	
	indexx(n,dataToSort,dataIn); //Make the index table.
	
	for (j=1;j<=n;j++) arrayIndexSorted[j]=dataToSort[j]; //Save the array ra.
	for (j=1;j<=n;j++) dataToSort[j]=arrayIndexSorted[dataIn[j]]; //Copy it back in rearranged order.
	
	rankCPU(N, dataIn, expected);

	printf("Running DFE.\n");
	DFE(N, dataIn, dataOut);

	int status = check(N, dataOut, expected);
	if (status)
		printf("Test failed.\n");
	else
		printf("Test passed OK!\n");

	return status;
}