//目的 : 如何编写多线程程序

#include <windows.h>
#include <stdio.h>
#include <time.h>

// 创建的线程数目
const int numThreads = 4;

struct pArg
{
	unsigned long begin_h;
	unsigned long begin_l;
	unsigned long search_val;
};


// 线程函数，注意：不能修改函数声明
DWORD WINAPI threadFunc(LPVOID pArg)
{
	/* 线程函数体，注意参数传递的方式 */
	struct pArg *p = (struct pArg *)pArg;
	printf("threadFunc...begin_h=%u begin_l=%u search_val=%u...\n", p->begin_h,p->begin_l,p->search_val);
	return 0;
}

int main()
{
	HANDLE hThread[numThreads];			// 保存线程句柄
	struct pArg theArg[numThreads];	// 保存传递给线程的参数
	time_t begin_time,end_time;			// 记录运行的开始、结束时刻
	
	printf("这是一个%d线程的搜索程序！\n",numThreads);

	begin_time = time(NULL);				// 记录开始时刻
	for(int i=0; i<numThreads; i++)
	{
		theArg[i].begin_l = i;				// 给线程i的参数begin_l
		theArg[i].begin_h = i+1;			// 给线程i的参数begin_h
		theArg[i].search_val = 11;		// 给线程i的参数search_val
		hThread[i] = CreateThread(NULL, 0, threadFunc, &theArg[i], 0, NULL);
	}
	
	/*WaitForMultipleObjects函数资料，请参考资料*/
	/*
		等待numThreads个线程执行结束。
		如果第三个参数为TRUE，则只有当所有numThreads个线程全部执行结束之后，才向下继续执行；
		如果第三个参数为FALSE，则当numThreads个线程中任意一个执行结束之后，就向下继续执行；
	*/
	WaitForMultipleObjects(numThreads, hThread, FALSE, INFINITE);
	end_time = time(NULL);					// 记录结束时刻

	printf("耗时%d秒~\n",end_time-begin_time);	// 结束-开始=耗时
	return 0;
}
