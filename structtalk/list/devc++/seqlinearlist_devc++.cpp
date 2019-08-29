#include <stdio.h>
#include <stdlib.h>


#define OK 1
#define ERROR 0
#define TRUE 1
#define FASLE 0

#define MAXSIZE 20

typedef int Status;
typedef int ElemType; 

Status visit(ElemType c)
{
	printf("%d",c);
	return OK;
}
typedef struct
{
	ElemType data[MAXSIZE];
	int length;
}SqList;
//初始化操作，建立一个空的线性表L
Status InitList(SqList *L)
{
	L->length=0;
	return OK;
}
//判断线性表是否为空，空返回人true，非空返回fasle
Status ListEmpty(SqList L)
{
	if(L.length==0)
		return TRUE;
	else
		return FASLE;
}
//将线性表清空
Status ClearList(SqList *L)
{
	L->length=0;
	return OK;
}
//返回线性表元素个数
int ListLength(SqList L)
{
	return L.length;
}
//将线性表第i个是元素返回给e,从1开始数
Status GetElem(SqList L,int i,ElemType *e)
{
	if(L.length==0||i<1||i>L.length)
		return ERROR;
	*e=L.data[i-1];
	return OK;
}
//在线性表L中查找与给定值e相等的元素，如果查找成功，返回
//该元素在表中序号表示成功；否则返回0表示失败
int LocateElem(SqList L,ElemType e)
{
	int i;
	if(L.length==0)
		return 0;
	for(i=0;i<L.length;i++)
	{
		if(L.data[i]==e)
			break;
	}
	if(i>=L.length)
		return 0;
	return i+1;
}
/*
初始条件：顺序线性表已存在，1<=i<=ListLength(L)
操作结果：在L中第i个位置之前插入新的数据元素e,L的长度
加1
*/
Status ListInsert(SqList *L,int i,ElemType e)
{
	int k;
	if(L->length==MAXSIZE)//顺序线性表已经满
		return ERROR;
	//当i比第一位置小或者比最后一位置后一位置还大时
	if(i<1||i>L->length+1)
		return ERROR;
	//如果插入数据位置不在表尾，把第i个元素之后的所有数据元素向后移动一位
	if(i<=L->length)
	{//数组中第i-1个元素就是用户层的第i个元素
		for(k=L->length-1;k>=i-1;k--)
				L->data[k+1]=L->data[k];
	}
	L->data[i-1]=e;
	L->length++;
	return OK;
}
/*
初始条件：顺序线性表L已存在，1<=i<=ListLength(L)
操作结果：删除L的第i个数据元素，并用e返回其值，L的
长度减1
*/
Status ListDelete(SqList *L,int i,ElemType *e)
{
	int k;
	if(L->length==0)
		return ERROR;
	if(i<1||i>L->length)
		return ERROR;
	*e=L->data[i-1];
	if(i<L->length)
	{
		for(k=1;k<L->length;k++)
			L->data[k-1]=L->data[k];
	}
	L->length--;
	return OK;
}

/*
初始条件：顺序线性表L已存在
操作结果:依次输出L中的所有元素
*/
Status ListTraverse(SqList L)
{
	int i;
	for(i=0;i<L.length;i++)
		visit(L.data[i]);
	printf("\n");
	return OK;
}
/*
	将属于Lb，不属于La的元素挂接到La尾部
*/


void unionL(SqList *La,SqList Lb)
{
	int La_len,Lb_len,i;
	ElemType e;
	La_len=ListLength(*La);
	Lb_len=ListLength(Lb);
	for(i=1;i<=Lb_len;i++)
	{
		GetElem(Lb,i,&e);
		if(!LocateElem(*La,e))
			ListInsert(La,++La_len,e);
	}
}
int main()
{
	SqList L;
	ElemType e;
	Status i;
	int j,k;
	i=InitList(&L);//初始化一个线性表
	printf("初始化L后：L.length=%d\n",L.length);
	for(j=1;j<=5;j++)
		i=ListInsert(&L,1,j);
	printf("在L的表头依次插入1~5后，L.data=");
	ListTraverse(L);

	printf("L.length=%d\n",L.length);
	i=ListEmpty(L);//表是否空
	printf("L是否空：i=%d(1:是0：否)\n",i);
	
	i=ClearList(&L);
	printf("清空L后:L.length=%d\n",L.length);
	i=ListEmpty(L);
	printf("L是否空：i=%d(1:是 0:否)\n",i);

	for(j=1;j<=10;j++)
		ListInsert(&L,j,j);
	printf("在 L的表尾依次插入1-10后：L.data=");
	ListTraverse(L);

	printf("L.length=%d\n",L.length);

	ListInsert(&L,1,0);
	printf("在L的表头插入0后：L.data=");
	ListTraverse(L);
	printf("L.length=%d\n",L.length);

	GetElem(L,5,&e);
	printf("第5个元素的值为：%d\n",e);
	for(j=3;j<=4;j++)
	{
		k=LocateElem(L,j);
		if(k)
			printf("第%d个元素的值为%d\n",k,j);
		else 
			printf("没有值为%d的元素\n",j);
	}

	k=ListLength(L);/*k为表长*/
	for(j=k+1;j>=k;j--)
	{
		i=ListDelete(&L,j,&e);
		if(i==ERROR)
			printf("删除第%d个数据失败\n",j);
		else
			printf("删除第%d个的元素值为：%d\n",j,e);
	}
	printf("依次输出L的元素：");
	ListTraverse(L);

	j=5;
	ListDelete(&L,j,&e);
	printf("删除第%d个的元素值为：%d\n",j,e);

	printf("依次输出L的元素：");
	ListTraverse(L);

	//构造一个有10个数的Lb
	SqList Lb;
	i=InitList(&Lb);
	for(j=6;j<15;j++)
		i=ListInsert(&Lb,1,j);
	unionL(&L,Lb);

	printf("依次输出合并了Lb的L的元素：");
		ListTraverse(L);

		return 0;
		
}







