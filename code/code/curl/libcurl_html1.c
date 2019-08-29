#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
int main(int argc,char *argv[])
{
	CURL *curl;//定义CURL类别的指针
	CURLcode res;//定义CURLcode类型的变量，保存返回状态码
	if(argc != 2)
	{
		printf("Usage:file <url>;\n");
		exit(1);
	}
	curl = curl_easy_init();//初始化一个CURL类型的指针
	if(curl != NULL)
	{
		//设置curl选项，其中CURLOPT_URL是让用户指定url.
		curl_easy_setopt(curl,CURLOPT_URL,argv[1]);
		//执行设置，只在屏幕上显示出来
		res = curl_easy_perform(curl);
		//清除curl操作
		curl_easy_cleanup(curl);
		
	}
	return 0;
}

















