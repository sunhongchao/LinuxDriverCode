#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>

 size_t read_data(void *buffer, size_t size, size_t nmemb, void *user_p)
 { 	
 	return fread(buffer, size, nmemb, (FILE *)user_p); 
 } 
 int main(int argc, char **argv) 
 { 	
 	// 初始化libcurl 	
 	CURLcode code = 0;
	int file_size = 0;
	code = curl_global_init(CURL_GLOBAL_SSL); 	
	if (code != CURLE_OK) 	
	{ 		
		printf("init libcurl failed.\n"); 	
		return -1; 	
	} 
 	FILE *fp = fopen("a.html", "r"); 
	if (NULL == fp) 
	{
		printf("can't open file.\n"); 	
		curl_global_cleanup(); 		
		return -1; 	
	}
 	// 获取文件大小 	
 	fseek(fp, 0, SEEK_END);//成功返回0
 	file_size = ftell(fp); //返回给定流 stream 的当前文件位置
	rewind(fp); //设置文件位置为给定流 stream 的文件的开头
 	// 获取easy handle 
	CURL *easy_handle = NULL; 	
	easy_handle = curl_easy_init(); 	
	if (NULL == easy_handle) 	{ 	
		printf("get a easy handle failed.\n"); 	
		fclose(fp); 
		curl_easy_cleanup(easy_handle);
		curl_global_cleanup(); 		
		return -1;
	}

 	// 设置eash handle属性 	
 	curl_easy_setopt(easy_handle, CURLOPT_URL, "ftp://127.0.0.1/upload.html");
 	curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L); 	
 	curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, &read_data); 
 	curl_easy_setopt(easy_handle, CURLOPT_READDATA, fp); 
 	curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE, file_size); 
 	 //执行上传操作 	
	 	code = curl_easy_perform(easy_handle);
 	 if (code == CURLE_OK) 
	 { 		
	 	printf("upload successfully.\n"); 	
	 }

 	// 释放资源 	
 	fclose(fp);
	curl_easy_cleanup(easy_handle);
	curl_global_cleanup();
 	return 0; 
 }

