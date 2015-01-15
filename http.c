#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/queue.h>
#include<event.h>
#include<evhttp.h>
#include "memcached.h"

#define MC_EXPIRE 600

void http_handle(struct evhttp_request *req, void *arg);

int main()
{
        struct evhttp *httpd;

        //连接memcached
        mc_connect();
        
        event_init();
        httpd = evhttp_start("10.96.141.77", 9000);

        if(httpd == NULL) {
                perror("evhttp_start:");
                return -1;
        }

        evhttp_set_timeout(httpd, 30);
        evhttp_set_gencb(httpd, http_handle, NULL);
        event_dispatch();
        evhttp_free(httpd);


        //断开memcached
        mc_free();

        return 0;
}

/*
void http_handle(struct evhttp_request *req, void *arg)
{       
        struct evbuffer *buf;  

        buf = evbuffer_new(); 
        evhttp_send_reply(req, HTTP_OK, "OK", buf);     
        evbuffer_free(buf);
        fprintf(stderr, "Send!\n");
}
*/

void http_handle(struct evhttp_request *req, void *arg)
{
        struct evbuffer *buf;
        struct evkeyvalq http_query;

        buf = evbuffer_new();
        char *decode_uri = strdup((char *)evhttp_request_uri(req));
        evhttp_parse_query(decode_uri, &http_query);
        free(decode_uri);

        const char *http_input_opt = evhttp_find_header(&http_query, "opt");
        const char *http_input_name = evhttp_find_header(&http_query, "k");
        const char *http_input_data = evhttp_find_header(&http_query, "v");

        evhttp_add_header(req->output_headers, "Content-Type", "text/plain");
        evhttp_add_header(req->output_headers, "Connection", "keep-alive");
        evhttp_add_header(req->output_headers, "Cache-Control", "no-cache"); 
        evhttp_add_header(req->output_headers, "author", "Huffman<imooc>");

        if(http_input_opt != NULL && http_input_name != NULL && strlen(http_input_name) < 30) {
                if(strcmp(http_input_opt, "put") == 0) {
                        int buffer_data_len = EVBUFFER_LENGTH(req->input_buffer);
                        if(buffer_data_len > 0) {
                                char *input_val_data = EVBUFFER_DATA(req->input_buffer);        
                                fprintf(stderr, "debug:%s\n", input_val_data);
                                
                        }else if(http_input_data != NULL) {
                                //写
                                mc_set(http_input_name, http_input_data, MC_EXPIRE);
                                evbuffer_add_printf(buf, "ok");
                        }
                }else if(strcmp(http_input_opt, "get") == 0){
                        //读
                        evbuffer_add_printf(buf, mc_get(http_input_name));
                }
        }
                
        evhttp_send_reply(req, HTTP_OK, "OK", buf);     
        evbuffer_free(buf);
        fprintf(stderr, "Send!\n");
}
