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
//��ʼ������������һ���յ����Ա�L
Status InitList(SqList *L)
{
	L->length=0;
	return OK;
}
//�ж����Ա��Ƿ�Ϊ�գ��շ�����true���ǿշ���fasle
Status ListEmpty(SqList L)
{
	if(L.length==0)
		return TRUE;
	else
		return FASLE;
}
//�����Ա����
Status ClearList(SqList *L)
{
	L->length=0;
	return OK;
}
//�������Ա�Ԫ�ظ���
int ListLength(SqList L)
{
	return L.length;
}
//�����Ա��i����Ԫ�ط��ظ�e,��1��ʼ��
Status GetElem(SqList L,int i,ElemType *e)
{
	if(L.length==0||i<1||i>L.length)
		return ERROR;
	*e=L.data[i-1];
	return OK;
}
//�����Ա�L�в��������ֵe��ȵ�Ԫ�أ�������ҳɹ�������
//��Ԫ���ڱ�����ű�ʾ�ɹ������򷵻�0��ʾʧ��
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
��ʼ������˳�����Ա��Ѵ��ڣ�1<=i<=ListLength(L)
�����������L�е�i��λ��֮ǰ�����µ�����Ԫ��e,L�ĳ���
��1
*/
Status ListInsert(SqList *L,int i,ElemType e)
{
	int k;
	if(L->length==MAXSIZE)//˳�����Ա��Ѿ���
		return ERROR;
	//��i�ȵ�һλ��С���߱����һλ�ú�һλ�û���ʱ
	if(i<1||i>L->length+1)
		return ERROR;
	//�����������λ�ò��ڱ�β���ѵ�i��Ԫ��֮�����������Ԫ������ƶ�һλ
	if(i<=L->length)
	{//�����е�i-1��Ԫ�ؾ����û���ĵ�i��Ԫ��
		for(k=L->length-1;k>=i-1;k--)
				L->data[k+1]=L->data[k];
	}
	L->data[i-1]=e;
	L->length++;
	return OK;
}
/*
��ʼ������˳�����Ա�L�Ѵ��ڣ�1<=i<=ListLength(L)
���������ɾ��L�ĵ�i������Ԫ�أ�����e������ֵ��L��
���ȼ�1
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
��ʼ������˳�����Ա�L�Ѵ���
�������:�������L�е�����Ԫ��
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
	������Lb��������La��Ԫ�عҽӵ�Laβ��
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
	i=InitList(&L);//��ʼ��һ�����Ա�
	printf("��ʼ��L��L.length=%d\n",L.length);
	for(j=1;j<=5;j++)
		i=ListInsert(&L,1,j);
	printf("��L�ı�ͷ���β���1~5��L.data=");
	ListTraverse(L);

	printf("L.length=%d\n",L.length);
	i=ListEmpty(L);//���Ƿ��
	printf("L�Ƿ�գ�i=%d(1:��0����)\n",i);
	
	i=ClearList(&L);
	printf("���L��:L.length=%d\n",L.length);
	i=ListEmpty(L);
	printf("L�Ƿ�գ�i=%d(1:�� 0:��)\n",i);

	for(j=1;j<=10;j++)
		ListInsert(&L,j,j);
	printf("�� L�ı�β���β���1-10��L.data=");
	ListTraverse(L);

	printf("L.length=%d\n",L.length);

	ListInsert(&L,1,0);
	printf("��L�ı�ͷ����0��L.data=");
	ListTraverse(L);
	printf("L.length=%d\n",L.length);

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

	k=ListLength(L);/*kΪ��*/
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

	j=5;
	ListDelete(&L,j,&e);
	printf("ɾ����%d����Ԫ��ֵΪ��%d\n",j,e);

	printf("�������L��Ԫ�أ�");
	ListTraverse(L);

	//����һ����10������Lb
	SqList Lb;
	i=InitList(&Lb);
	for(j=6;j<15;j++)
		i=ListInsert(&Lb,1,j);
	unionL(&L,Lb);

	printf("��������ϲ���Lb��L��Ԫ�أ�");
		ListTraverse(L);

		return 0;
		
}







