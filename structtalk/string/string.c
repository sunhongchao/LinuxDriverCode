#include <string>
#include <stdio.h>
#include <stdlib.h>

#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0

#define MAXSIZE 40

typedef int Status;
typedef int ElemType;
typedef char String[MAXSIZE+1];//0号单元存放串的长度
//生成一个其值等于chars的串T
Status StrAssign(String T,char *charas)
{
	int i;
	if(strlen(chars)>MAXSIZE)
		return ERROR;
	else 
	{
		T[0]=strlen(chars);
		for(i=1;i<=T[0];i++)
			T[i]=*(chars+i-1);
		return OK;
	}
}
//由串S复制得串T
Status StrCopy(String T,Stirng S)
{
	int i;
	for(i=0;i<=s[0];i++)
		T[i]=S[i];
	return OK;
}
//若s为空串，则返回TRUE，否则返回FALSE
Status StrEmpty(String S)
{
	if(S[0]==0)
		return TRUE;
	else 
		return FALSE;
}

