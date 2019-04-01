#include <stdio.h>
#include<string.h>
#include<stdlib.h>

#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0

#define MAXSIZE 20//�洢�ռ��ʼ��������
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
//��ʼ��˳�����Ա�
Status InitList(LinkList *L)
{
	*L=(LinkList)malloc(sizeof(Node));//����ͷ��㣬��ʹLָ���ͷ���
	if(!(*L))//�洢����ʧ��
		return ERROR;
	(*L)->next=NULL;
	return OK;
}
/*
	��ʼ������˳�����Ա�L�Ѵ��ڡ������������LΪ�ձ��򷵻�TRUE,���򷵻�FALSE
*/
Status ListEmpty(LinkList L)
{
	if(L->next)
		return FALSE;
	else 
		return TRUE;
}
/*
��ʼ������˳�����Ա�L�Ѿ����ڡ������������L����Ϊ�ձ�
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
/*��ʼ������˳�����Ա�L�Ѵ��ڡ��������������L������Ԫ�ظ���
*/
int ListLength(LinkList L)
{
	int i=0;
	LinkList p=L->next;/*pָ���һ�����*/
	while(p)
	{
		i++;
		p=p->next;
	}
	return i;
}
/*
��ʼ��������˳�����Ա�L�Ѵ��ڣ�1<=i<=ListLength(L)
�����������e����L�е�i������Ԫ�ص�ֵ
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
		return ERROR;//��i��Ԫ�ز�����
	*e=p->data;//ȡ��1��Ԫ�ص�����
	return OK;
}
//������������Ԫ�ز����ڣ��򷵻�ֵΪ0
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
��ʼ������˳�����Ա�L�Ѵ��ڣ�1<=i<=ListLength(L)
�����������L�е�i��λ��֮ǰ�����µ�����Ԫ��e,L�ĳ��ȼ�1
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
/*��ʼ������˳�����Ա�L�Ѵ��ڣ�1<=i<=ListLength(L)
���������ɾ��L�ĵ�i������Ԫ�أ�����e������ֵ��L�ĳ��ȼ�1*/
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
	��ʼ������˳�����Ա�L�Ѿ�����
	������������ζ�L��ÿ������Ԫ�����
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
�������n��Ԫ�ص�ֵ����������ͷ���ĵ������Ա�L(ͷ�巨)
*/
void CreateListHead(LinkList *L,int n)
{
	LinkList p;
	int i;
	srand(time(0));//��ʼ�����������
	*L=(LinkList)malloc(sizeof(Node));
	(*L)->next=NULL;
	for(i=0;i<n;i++)
	{
		p=(LinkList)malloc(sizeof(Node));//�Ƚ���һ����ͷ���ĵ�����
		p->data=rand()%100+1;//�������100���ڵ�����
		p->next=(*L)->next;
		(*L)->next=p;//���뵽��ͷ
	}
}
//�������n��Ԫ�ص�ֵ����������ͷ���ĵ������Ա�L��β�巨��
void CreateListTail(LinkList *L,int n)
{
	LinkList p,r;
	int i;
	srand(time(0));
	*L=(LinkList)malloc(sizeof(Node));
	r=*L;
	for(i=0;i<n;i++)
	{
		p=(Node*)malloc(sizeof(Node));//�����½��
		p->data=rand()%100+1;//�������100���ڵ�����
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
	printf("��ʼ��L��ListLength(L)=%d\n",ListLength(L));
	for(j=1;j<=5;j++)
		i=ListInsert(&L,1,j);
	printf("��L�ı�ͷ���β���1-5��L.data=");
	ListTraverse(L);

	printf("ListLength(L)=%d\n",ListLength(L));
	i=ListEmpty(L); 
	printf("L�Ƿ�գ�i=%d(1:��0����)\n",i);

	i=ClearList(&L);
	printf("���L��ListLength(L)=%d\n",ListLength(L));
	i=ListEmpty(L);
	printf("L�Ƿ�գ�i=%d(1:��0����)\n",i);
	for(j=1;j<=10;j++)
		ListInsert(&L,j,j);
	printf("��L�ı�β���β���1-10��L.data=");
	ListTraverse(L);

	printf("ListLength(L)=%d\n",ListLength(L));
	
	ListInsert(&L,1,0);
	printf("��L�ı�ͷ����0��L.data=");
	ListTraverse(L);
	printf("ListLength(L)=%d\n",ListLength(L));
	GetElem(L,5,&e);
	printf("��5��Ԫ�ص�ֵΪ��%d\n",e);
	for(j=3;j<=4;j++)
	{
		k=LocateElem(L,j);
		if(k)
			printf("��%d��Ԫ�ص�ֵΪ%d\n",k,j);
		else 
			printf("û��ֵΪ%d��Ԫ��\n",j);
	}
	k=ListLength(L);
	for(j=k+1;j>=k;j--)
	{
		i=ListDelete(&L,j,&e);
		if(i==ERROR)
			printf("ɾ����%d������ʧ��\n",j);
		else 
			printf("ɾ����%d����Ԫ��ֵΪ��%d\n",j,e);
	}
	printf("�������L��Ԫ�أ�");
	ListTraverse(L);

	i=ClearList(&L);
	printf("\n���L��ListLength(L)=%d",ListLength(L));
	ListTraverse(L);

	i=ClearList(&L);
	printf("\nɾ��L��ListLentgth(L)=%d",ListLength(L));
	CreateListTail(&L,20);
	printf("���崴��L��Ԫ��(β�巨)��");
	ListTraverse(L);

	return 0;
}


