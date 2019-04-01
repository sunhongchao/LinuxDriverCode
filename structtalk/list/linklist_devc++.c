#include <stdio.h>
#include<string.h>
#include<stdlib.h>

#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0

#define MAXSIZE 20//存储空间初始化分配量
typedef int Status;
typedef int ElemType;
Status visit(ElemType c)
{
	printf("%d",c);
	return OK;
}
typedef struct Node
{
	ElemType data;
	struct Node *next;
}Node;
typedef struct Node * LinkList;
//初始化顺序线性表
Status InitList(LinkList *L)
{
	*L=(LinkList)malloc(sizeof(Node));//产生头结点，并使L指向此头结点
	if(!(*L))//存储分配失败
		return ERROR;
	(*L)->next=NULL;
	return OK;
}
/*
	初始条件：顺序线性表L已存在。操作结果：若L为空表，则返回TRUE,否则返回FALSE
*/
Status ListEmpty(LinkList L)
{
	if(L->next)
		return FALSE;
	else 
		return TRUE;
}
/*
初始条件，顺序线性表L已经存在。操作结果：将L重置为空表
*/
Status ClearList(LinkList *L)
{
	LinkList p,q;
	p=(*L)->next;
	while(p)
	{
		q=p->next;
		free(p);
		p=q;
	}
	(*L)->next=NULL;
	return OK;
}
/*初始条件：顺序线性表L已存在。操作结果：返回L中数据元素个数
*/
int ListLength(LinkList L)
{
	int i=0;
	LinkList p=L->next;/*p指向第一个结点*/
	while(p)
	{
		i++;
		p=p->next;
	}
	return i;
}
/*
初始化条件：顺序线性表L已存在，1<=i<=ListLength(L)
操作结果：用e返回L中第i个数据元素的值
*/
Status GetElem(LinkList L,int i,ElemType *e)
{
	int j;
	LinkList p;
	p=L->next;
	j=1;
	while(p&&j<i)
	{
		p=p->next;
		++j;
	}
	if(!p||j<i)
		return ERROR;//第i个元素不存在
	*e=p->data;//取第1个元素的数据
	return OK;
}
//若这样的数据元素不存在，则返回值为0
int LocateElem(LinkList L,ElemType e)
{
	int i=0;
	LinkList p=L->next;
	while(p)
	{
		i++;
		if(p->data==e)
			return i;
		p=p->next;
	}
	return 0;
}
/*
初始条件：顺序线性表L已存在，1<=i<=ListLength(L)
操作结果：在L中第i个位置之前插入新的数据元素e,L的长度加1
*/
Status ListInsert(LinkList *L,int i,ElemType e)
{
	int j;
	LinkList p,s;
	p=*L;
	j=1;
	while(p&&j<1)
	{
		p=p->next;
		++j;
	}
	if(!p||j>i)
		return ERROR;
	s=(LinkList)malloc(sizeof(Node));//
	s->data=e;
	s->next=p->next;
	p->next=s;
	return OK;
}
/*初始条件：顺序线性表L已存在，1<=i<=ListLength(L)
操作结果：删除L的第i个数据元素，并用e返回其值，L的长度减1*/
Status ListDelete(LinkList *L,int i,ElemType *e)
{
	int j;
	LinkList p,q;
	p=*L;
	j=1;
	while(p->next&&j<i)
	{
		p=p->next;
		++j;
	}
	if(!(p->next)||j>i)
		return ERROR;
	q=p->next;
	p->next=q->next;
	*e=q->data;
	free(q);
	return OK;
}
/*
	初始条件：顺序线性表L已经存在
	操作结果：依次对L的每个数据元素输出
*/
Status ListTraverse(LinkList L)
{
	LinkList p=L->next;
	while(p)
	{
		visit(p->data);
		p=p->next;
	}
	printf("\n");
	return OK;
}
/*
随机产生n个元素的值，建立带表头结点的单链线性表L(头插法)
*/
void CreateListHead(LinkList *L,int n)
{
	LinkList p;
	int i;
	srand(time(0));//初始化随机数种子
	*L=(LinkList)malloc(sizeof(Node));
	(*L)->next=NULL;
	for(i=0;i<n;i++)
	{
		p=(LinkList)malloc(sizeof(Node));//先建立一个带头结点的单链表
		p->data=rand()%100+1;//随机生成100以内的数字
		p->next=(*L)->next;
		(*L)->next=p;//插入到表头
	}
}
//随机产生n个元素的值，建立带表头结点的单链线性表L（尾插法）
void CreateListTail(LinkList *L,int n)
{
	LinkList p,r;
	int i;
	srand(time(0));
	*L=(LinkList)malloc(sizeof(Node));
	r=*L;
	for(i=0;i<n;i++)
	{
		p=(Node*)malloc(sizeof(Node));//生成新结点
		p->data=rand()%100+1;//随机生成100以内的数字
		r->next=p;
		r=p;
	}
	r->next=NULL;
}
int main()
{
	LinkList L;
	ElemType e;
	Status i;
	int j,k;
	i=InitList(&L);
	printf("初始化L后：ListLength(L)=%d\n",ListLength(L));
	for(j=1;j<=5;j++)
		i=ListInsert(&L,1,j);
	printf("在L的表头依次插入1-5后：L.data=");
	ListTraverse(L);

	printf("ListLength(L)=%d\n",ListLength(L));
	i=ListEmpty(L); 
	printf("L是否空：i=%d(1:是0：否)\n",i);

	i=ClearList(&L);
	printf("清空L后：ListLength(L)=%d\n",ListLength(L));
	i=ListEmpty(L);
	printf("L是否空：i=%d(1:是0：否)\n",i);
	for(j=1;j<=10;j++)
		ListInsert(&L,j,j);
	printf("在L的表尾依次插入1-10后：L.data=");
	ListTraverse(L);

	printf("ListLength(L)=%d\n",ListLength(L));
	
	ListInsert(&L,1,0);
	printf("在L的表头插入0后：L.data=");
	ListTraverse(L);
	printf("ListLength(L)=%d\n",ListLength(L));
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
	k=ListLength(L);
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

	i=ClearList(&L);
	printf("\n清空L后：ListLength(L)=%d",ListLength(L));
	ListTraverse(L);

	i=ClearList(&L);
	printf("\n删除L后：ListLentgth(L)=%d",ListLength(L));
	CreateListTail(&L,20);
	printf("整体创建L的元素(尾插法)：");
	ListTraverse(L);

	return 0;
}


