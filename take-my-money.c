#include <curl/curl.h>
#include <stdio.h>

int main(void){
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.apilayer.com/exchangerates_data/convert?to=CLP&from=ARS&amount=879");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "apikey: 9COfh6HcQz1tPC6M67W17Eqz1wv6r0vq");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    }
    curl_easy_cleanup(curl);
    return 0;
}
