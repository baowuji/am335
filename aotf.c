#include"apue.h"
typedef struct 
{
	int freq;
	int power;
}AotfData;
typedef AotfData* pAotfData;
AotfData aotf1={1,1};
pAotfData paotf1=&aotf1;
void* aotf(void *arg)
{
	printids("aotf\n");
//	while(1)    	
	{
		printf("aotf Freq is %d\n",aotf1.freq);
	}
	return (void*)0;
}
int main(void)
{
	
	;

}
