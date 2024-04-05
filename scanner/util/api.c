#include "../inc/api.h"
#include "../inc/log.h"

#include <curl/curl.h>
#include <stdbool.h>
#include <string.h>

bool api_check() {
  CURL *curl = curl_easy_init();
  FILE *null = fopen("/dev/null", "wb");
  bool ret = true;

  char *password = get_str("password");
  char *url = get_str("url");

  char path[strlen(password) + 30];
  sprintf(path, "scanner/check?pass=%s", password);

  char newurl[strlen(url) + strlen(path) + 5];
  urljoin(newurl, url, path);

  curl_easy_setopt(curl, CURLOPT_URL, newurl);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/massacr");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, null);
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    error("API connection failed");
    ret = false;
    goto DONE;
  }

  long code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

  if (code == 403) {
    error("Bad password for API");
    ret = false;
    goto DONE;
  }

  if (code != 200) {
    error("Bad status code from API (%d)", code);
    ret = false;
    goto DONE;
  }

DONE:
  curl_easy_cleanup(curl);
  fclose(null);
  return ret;
}

bool api_add(void *ap) {
  api_args_t *args = (api_args_t *)ap;
  if (NULL == args)
    return false;

  CURL *curl = curl_easy_init();
  FILE *null = fopen("/dev/null", "wb");
  bool ret = true;

  char *password = get_str("password");
  char *url = get_str("url");

  char path[strlen(password) + strlen(args->ip) + 55];
  sprintf(path, "scanner/add?pass=%s&ip=%s&port=%d", password, args->ip,
          args->port);

  char newurl[strlen(url) + strlen(path) + 10];
  urljoin(newurl, url, path);

  curl_easy_setopt(curl, CURLOPT_URL, newurl);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/massacr");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, null);
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    error("API connection failed");
    ret = false;
    goto DONE;
  }

  long code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

  if (code == 403) {
    error("Bad password for API");
    ret = false;
    goto DONE;
  }

  if (code != 200) {
    error("Bad status code from API (%d)", code);
    ret = false;
    goto DONE;
  }

DONE:
  free(ap);
  curl_easy_cleanup(curl);
  fclose(null);
  return ret;
}
