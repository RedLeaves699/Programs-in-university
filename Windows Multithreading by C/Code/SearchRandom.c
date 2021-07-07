/*
	某个应用程序的随机数产生机制是这样的：
	1、首先产生一个0~0x7FFFFFFF之间的数，称为“密码”X
	2、用(X,0x29A)调用随机数更新函数GenerateRandomNumber，产生新的(Y,Z)
	3、用(Y,Z)调用随机数更新函数GenerateRandomNumber，产生新的(A,B)
	4、把B传给用户
	
	本程序（你的）目的，就是在知道这套机制、知道B的前提下，计算出X是多少？
	
	非常简单的思路：
	1、循环，从i=0~0x7FFFFFFF
	2、GenerateRandomNumber(i,0x29A)
	3、GenerateRandomNumber(A,B)
	4、判断B是否等于想要找的值
	
*/

#include <windows.h>
#include <stdio.h>
#include <time.h>

#define MAXN 0x7fffffff
typedef unsigned long UL;
typedef unsigned long long ULL;

/* 更新随机数 */
void GenerateRandomNumber(UL *rand1_h, UL *rand1_l)
{
	/*请忽略下列代码，知道是产生更新随机数即可，原理不必弄清楚*/
	ULL x = *rand1_h;
	x *= 0x6AC690C5;
	x += *rand1_l;

	*rand1_h = (UL)x;
	*rand1_l = (UL)(x>>32);
}

void SearchRandomNumber(UL begin_h, UL begin_l, UL search_val) {
	// 在begin_l~begin_h之间，计算B的值，看看是否和search_val相等
	UL i, h, l;
	for (i=begin_l;i<=begin_h;i++) {
		h = i;
		l = 0x29A;
		GenerateRandomNumber(&h, &l);	// 第一次调用
		GenerateRandomNumber(&h, &l);	// 第二次调用
		if (l == search_val)			// 判断B是否和search_val相等
		{
			printf("找到啦~! 密码是%u\n",i);	// 相等，找到了~~
			return;
		}
	}
}

struct pArg {
	UL begin_h, begin_l, search_val;
};

// 线程函数，注意：不能修改函数声明
DWORD WINAPI threadFunc(LPVOID pArg)
{
	/* 线程函数体，注意参数传递的方式 */
	struct pArg *p = (struct pArg *)pArg;
	printf("threadFunc...begin_h=%u begin_l=%u search_val=%u...\n", p->begin_h,p->begin_l,p->search_val);
	SearchRandomNumber(p -> begin_h, p ->begin_l, p -> search_val);
	return 0;
}

void multithreaded(UL begin_h, UL begin_l, UL search_val, int numThreads) {

	HANDLE hThread[numThreads];			// 保存线程句柄
	struct pArg theArg[numThreads];	// 保存传递给线程的参数
	time_t begin_time,end_time;			// 记录运行的开始、结束时刻
	
	printf("这是一个%d线程的搜索程序！\n", numThreads);

	UL st = 0;
	for(int i=0; i<numThreads; ++i, st += (MAXN / numThreads)) {
		theArg[i].begin_l = st;				// 给线程i的参数begin_l
		theArg[i].begin_h = st + ((UL)MAXN / numThreads);			// 给线程i的参数begin_h
		theArg[i].search_val = search_val;		// 给线程i的参数search_val
		if (i == numThreads - 1) theArg[i].begin_h = MAXN;
		hThread[i] = CreateThread(NULL, 0, threadFunc, &theArg[i], 0, NULL);
	}
	/*WaitForMultipleObjects函数资料，请参考资料*/
	/*
		等待numThreads个线程执行结束。
		如果第三个参数为TRUE，则只有当所有numThreads个线程全部执行结束之后，才向下继续执行；
		如果第三个参数为FALSE，则当numThreads个线程中任意一个执行结束之后，就向下继续执行；
	*/
	WaitForMultipleObjects(numThreads, hThread, FALSE, INFINITE);
}

int main() {
	time_t begin_time,end_time;
	
	begin_time = time(NULL);							// 记录开始时刻
	multithreaded(MAXN, 0, 1121, 10);	// 在0~0x7FFFFFFF之间进行搜索
	end_time = time(NULL);								// 记录结束时刻

	printf("耗时%d秒~~\n",end_time - begin_time);	// 结束-开始=耗时
  	return 0;
}
