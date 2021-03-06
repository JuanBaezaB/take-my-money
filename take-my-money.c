#include <curl/curl.h>
#include <stdio.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

#define CHUNK_SIZE 2048

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb; 
    get_request *req = (get_request *) userdata;

    printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + CHUNK_SIZE);
        req->buflen += CHUNK_SIZE;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    return realsize;
}


int main(int argc, char **argv) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    struct json_object *parsed_json;
    struct json_object *result;
    

    get_request req = {.buffer = NULL, .len = 0, .buflen = 0};

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.apilayer.com/exchangerates_data/convert?to=CLP&from=ARS&amount=879");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "apikey: 9COfh6HcQz1tPC6M67W17Eqz1wv6r0vq");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        req.buffer = malloc(CHUNK_SIZE);
        req.buflen = CHUNK_SIZE;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);

        res = curl_easy_perform(curl);
        printf("Result = %u\n",res);

        printf("Total received bytes: %zu\n", req.len);
        printf("Received data:\n %s\n", req.buffer);

        parsed_json = json_tokener_parse(req.buffer);
        json_object_object_get_ex(parsed_json, "result", &result);
        char * x = (char *)json_object_get_string(result);
        printf("result: %s\n", x);

        free(req.buffer);      
        
          
    }

    curl_easy_cleanup(curl);
}
