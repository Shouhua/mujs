#include <stdio.h>
#include <curl/curl.h>

#define NTHELE(a) (sizeof(a)/sizeof(a[0]))

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *data)
{
  (void)ptr;
  (void)data;
  
  return size * nmemb;
}

char * urls[] = 
{
	"https://www.baidu.com"
	// "https://www.sohu.com",
	// "https://www.qq.com",
	// "https://www.163.com"
};
static void init_url(CURLM *m)
{
	CURL *eh;
	for(int i = 0; i < NTHELE(urls); i++)	
	{
		eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_data);
		curl_multi_add_handle(m, eh);
	}
}

int main (int argc, char *argv[]) {
	curl_global_init(CURL_GLOBAL_ALL);
	CURLM * multi = curl_multi_init();

	// CURL *handle1 = curl_easy_init();
	// CURL *handle2 = curl_easy_init();
	// CURL *handle3 = curl_easy_init();

	// curl_easy_setopt(handle1, CURLOPT_URL, "https://www.baidu.com");
	// curl_easy_setopt(handle1, CURLOPT_WRITEFUNCTION, write_data);
	// curl_easy_setopt(handle2, CURLOPT_URL, "https://www.sohu.com");
	// curl_easy_setopt(handle2, CURLOPT_WRITEFUNCTION, write_data);
	// curl_easy_setopt(handle3, CURLOPT_URL, "https://www.microsoft.com/zh-cn");
	// curl_easy_setopt(handle3, CURLOPT_WRITEFUNCTION, write_data);
	// curl_easy_setopt(handle2, CURLOPT_VERBOSE, 1L);

	// curl_multi_add_handle(multi, handle1);
	// curl_multi_add_handle(multi, handle2);
	// curl_multi_add_handle(multi, handle3);
	init_url(multi);

	int running_handles = 0;
	curl_multi_perform(multi, &running_handles);
	// while(running_handles)
	// {
	// 	curl_multi_perform(multi, &running_handles);
	// }
	int numfds = 0;
	int repeats = 0;
	while(running_handles)
	{
		curl_multi_perform(multi, &running_handles);
		// curl_multi_poll(multi, NULL, 0, 100, &numfds);
		curl_multi_wait(multi, NULL, 0, 100, &numfds);
		// printf("running_handles: %d, numfds: %d\n", running_handles, numfds);
		if(numfds == 0)
		{
			repeats++;
		}
	}

	printf("repeats: %d, numfds: %d\n", repeats, numfds);
	int msgs_in_queue = 0;
	do
	{
		CURLMsg *msg = curl_multi_info_read(multi, &msgs_in_queue);
		if(msg->msg == CURLMSG_DONE)
		{
			int return_code = msg->data.result;
			if(return_code != CURLE_OK)
			{
				fprintf(stderr, "CURL error code: %d\n", msg->data.result);
				curl_multi_remove_handle(multi, msg->easy_handle);
				continue;
			}
			// printf("multi msg->data.result: %d\n", msg->data.result);
			int code = 0;
			char *effective_url;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &effective_url);
			curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
			printf("effective url: %s, status code: %d\n", effective_url, code);
			curl_multi_remove_handle(multi, msg->easy_handle);
			curl_easy_cleanup(msg->easy_handle);
		}
	} while(msgs_in_queue);

	// curl_easy_cleanup(handle1);
	// curl_easy_cleanup(handle2);
	curl_multi_cleanup(multi);
	curl_global_cleanup();
	return 0;
}