#include <stdbool.h>
#include <curl/curl.h>
#include "../inc/db.h"
#include "../inc/log.h"

char* gurl;
char* gpass;

void urljoin(char* newurl, char* url, char* path) {
  if(url[strlen(url)-1]=='/')
    sprintf(newurl, "%s%s", url, path);
  else
    sprintf(newurl, "%s/%s", url, path);
}

bool check_db(char* url, char* pass) {
  CURL* curl = curl_easy_init();
  FILE* null = fopen("/dev/null", "wb");
  bool ret = true;

  char newurl[strlen(url)+strlen(pass)+20];
  char path[strlen(pass)+30];
  sprintf(path, "check?pass=%s", pass);
  urljoin(newurl, url, path);

  curl_easy_setopt(curl, CURLOPT_URL, newurl);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/massacr");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, null);
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    error("Database connection failed");
    ret = false;
    goto DONE;
  }

  long code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

  if(code == 403) {
    error("Bad password for database");
    ret = false;
    goto DONE;
  }
    
  if(code != 200) {
    error("Bad status code from database (%d)", code);
    ret = false;
    goto DONE;
  }

  gurl = url;
  gpass = pass;

DONE:
  curl_easy_cleanup(curl);
  fclose(null);
  return ret;
}

bool add_entry(void* args) {
  struct Entry* en = (struct Entry*)args;

  CURL* curl = curl_easy_init();
  FILE* null = fopen("/dev/null", "wb");
  bool ret = true;

  char newurl[strlen(gurl)+strlen(gpass)+strlen(en->ip)+50];
  char path[strlen(gpass)+strlen(en->ip)+50];
  sprintf(path, "add?pass=%s&ip=%s&port=%d", gpass, en->ip, en->port);
  urljoin(newurl, gurl, path);

  curl_easy_setopt(curl, CURLOPT_URL, newurl);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/massacr");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, null);
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    error("Database connection failed");
    ret = false;
    goto DONE;
  }

  long code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

  if(code == 403) {
    error("Bad password for database");
    ret = false;
    goto DONE;
  }
    
  if(code != 200) {
    error("Bad status code from database");
    ret = false;
    goto DONE;
  }

DONE:
  free(args);
  curl_easy_cleanup(curl);
  fclose(null);
  return ret;
}
