#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/uri.h>
#include <time.h>
#include "lib/linkedList.h"
#include "lib/curlStuff.h"
#include "lib/hashHelper.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/multi.h>


/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

Queue* URL_frontier;
Stack* PNG_stack;
Stack* HTML_stack;
struct hsearch_data* queued_URLS;
char* urls_to_free[1000]; // urls in hashmap that must be freed at the end
int urls_count = 0;
int threads_done; 
int PNGs_found;
int done = 0;

CURLM *cm=NULL;
#define MAX_WAIT_MSECS 30*1000 /* Wait max. 30 seconds */

int c = 0;
int max_connections = 1; // create 1 connection by default
int PNGs_to_find = 50; // find 50 PNGs by default
int create_log = 0; // don't create a logfile by default
char logfile_name[256] = ""; // txt file to save to all visited links to
char start_url[256] = "";

int parse_args(int argc, char** argv){
    char* str = "option requires an argument";

    while ((c = getopt (argc, argv, "t:m:v:")) != -1) {
        switch (c) {
            case 't':
                max_connections = strtoul(optarg, NULL, 10);
                //num threads should be greater than 0
                if (max_connections <= 0) {
                    fprintf(stderr, "%s: %s > 0 -- 't'\n", argv[0], str);
                    return -1;
                }
                break;
            case 'm':
                PNGs_to_find = strtoul(optarg, NULL, 10);
                // PNGs left should be greater than 0
                if (PNGs_to_find <= 0) {
                    fprintf(stderr, "%s: %s > 0 -- 'n'\n", argv[0], str);
                    return -1;
                }
                break;
            case 'v':
                strcpy(logfile_name, optarg);
                create_log = 1;
                break;
            default:
                return -1;
        }
    }
    if (optind >= argc || argc == 1) {
        strcpy(start_url, SEED_URL); 
    } else {
        strcpy(start_url, argv[optind]); // seed url is always last argument
    }
    // printf("concurrent connections to create: %d\nPNGs to find: %d\ncreate logfile: %s\nlogfile name: %s\nstart url: %s\n", 
    //     max_connections, PNGs_to_find, create_log ? "yes" : "no", create_log ? logfile_name : "NULL", start_url);
    return 0;
}


int find_http(char *buf, int size, int follow_relative_links, const char *base_url)
{

    int i;
    htmlDocPtr doc;
    xmlChar *xpath = (xmlChar*) "//a/@href";
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    xmlChar *href;
		
    if (buf == NULL) {
        return 1;
    }

    doc = mem_getdoc(buf, size, base_url);
    result = getnodeset (doc, xpath);
    if (result) {
        nodeset = result->nodesetval;
        for (i=0; i < nodeset->nodeNr; i++) {
            href = xmlNodeListGetString(doc, nodeset->nodeTab[i]->xmlChildrenNode, 1);
            if ( follow_relative_links ) {
                xmlChar *old = href;
                href = xmlBuildURI(href, (xmlChar *) base_url);
                xmlFree(old);
            }
            if ( href != NULL && !strncmp((const char *)href, "http", 4) ) {
   
                if (hash_table_get((char*)href, queued_URLS) == 0){
                    /* if URL is not in hashmap */

                    /* add to hashmap */
                    hash_table_add((char*)href, (void*)1, queued_URLS, urls_to_free, urls_count);
                    /* add to URL frontier */
                    enQueue(URL_frontier, (char*)href);
                }
            }
            xmlFree(href);
        }
        xmlXPathFreeObject (result);
    }
    xmlFreeDoc(doc);
    return 0;
}

int process_html(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    int follow_relative_link = 1;
    char *url = NULL; 

    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
    pushStack(HTML_stack, url);         // add URL to HTML stack

    if (url != NULL){
        // find more URLs on page 
        find_http(p_recv_buf->buf, p_recv_buf->size, follow_relative_link, url); 
    }
    return 0;
}

int process_png(CURL *curl_handle, RECV_BUF *p_recv_buf)
{
    char *eurl = NULL;          /* effective URL */
    curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &eurl);

    pushStack(HTML_stack, eurl);         // add URL to HTML stack

    if ( eurl != NULL) {
        // valid URL

        if (is_png(p_recv_buf->buf)){
            if (PNGs_found == PNGs_to_find){
                // found all PNGs requested, so don't add
                // instead empty URL frontier

                while (!isQEmpty(URL_frontier)){
                    // deQueue frees memory of node
                    // this frees memory of node url
                    char* temp = (deQueue(URL_frontier));
                    free(temp);
                }
            } else {
                int ret = pushStack(PNG_stack, eurl);

                if (ret){
                    PNGs_found += 1;             // increase num PNGs found
                }
            }
            
        }
    }
    return 0;
}

int process_data(CURL *curl_handle, RECV_BUF *p_recv_buf)
{   
    CURLcode res;
    long response_code;

    res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if ( res == CURLE_OK ) {
	    // printf("Response code: %ld\n", response_code);
    }

    if ( response_code >= 400 ) { 
    	// fprintf(stderr, "Error.\n");
        return 1;
    }

    char *ct = NULL;
    res = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &ct);
    if ( res == CURLE_OK && ct != NULL ) {
    	// printf("Content-Type: %s, len=%ld\n", ct, strlen(ct));
    } else {
        // fprintf(stderr, "Failed obtain Content-Type\n");
        return 2;
    }

    if ( strstr(ct, CT_HTML) ) {
        // printf("process html\n");
        return process_html(curl_handle, p_recv_buf);
    } else if ( strstr(ct, CT_PNG) ) {
        // printf("process png\n");
        return process_png(curl_handle, p_recv_buf);
    } 
    return 0;
}

int main( int argc, char** argv ) 
{
    // timing variables
    double times[2];
    struct timeval tv;

    if(gettimeofday(&tv, NULL) != 0){
        abort();
    }
    times[0] = (tv.tv_sec) + tv.tv_usec / 1000000.;

    /* PARSE ARGS */
    parse_args(argc, argv);

    /* INIT DATA STRUCTURES */
    URL_frontier = createQueue();
    PNG_stack = createStack();
    HTML_stack = createStack();

    // create visited URLs hashmap
    hash_tables_init(&queued_URLS);

    // add start URL to hash table
    enQueue(URL_frontier, start_url);
    hash_table_add(start_url, (void*)1, queued_URLS, urls_to_free, urls_count);

    // curl multi
    CURLM *cm=NULL;
    CURL *eh=NULL;
    CURLMsg *msg=NULL;
    CURLcode return_code=0;
    int still_running=0, msgs_left=0;


    curl_global_init(CURL_GLOBAL_ALL);
    /* create a multi handle */
    cm = curl_multi_init();

    while(!(isQEmpty(URL_frontier) && still_running == 0 && msgs_left == 0)){

        /* add upto num connections */
        /* add urls from URL frontier to multi handle */
        int curr_connections = 0;
        while(!isQEmpty(URL_frontier)){
            char* curr_url = deQueue(URL_frontier); // get url from URL frontier
            easy_handle_init(curr_url, cm);         // initialize easy handle, curr url gets strdup'd inside
            free(curr_url);                         // free current url
            curr_connections += 1;
            // printf("curr connections: %d\n", curr_connections);
            if (curr_connections == max_connections)break;  
        }

        /* wait for curl to finish */
        curl_multi_perform(cm, &still_running);

        do {
            // printf("still running: %d\n", still_running);

            int numfds=0;
            int res = curl_multi_wait(cm, NULL, 0, 20*1000, &numfds);
            if(res != CURLM_OK) {
                fprintf(stderr, "error: curl_multi_wait() returned %d\n", res);
                return EXIT_FAILURE;
            }
            curl_multi_perform(cm, &still_running);
        } while(still_running);
        // printf("still running: %d\n", still_running);



        /* check curls */
        while ((msg = curl_multi_info_read(cm, &msgs_left))) {
            if (msg->msg == CURLMSG_DONE) {
                eh = msg->easy_handle;

                return_code = msg->data.result;
                if(return_code!=CURLE_OK) {
                    curl_multi_remove_handle(cm, eh);
                    cleanup(eh);
                    // fprintf(stderr, "CURL error code: %d\n", msg->data.result);
                } else {
                    /* get recv buf from easy handle curl */
                    RECV_BUF *curl_recv_buf;
                    curl_easy_getinfo(eh, CURLINFO_PRIVATE, &curl_recv_buf);

                    // printf("memory after easy get %p\n", curl_recv_buf);


                    // printf("%lu bytes received in memory %p, seq=%d.\n", 
                    //         curl_recv_buf->size, curl_recv_buf->buf, curl_recv_buf->seq);
                            
                    /* process data */
                    process_data(eh, curl_recv_buf);

                    /* remove easy handle from multi */
                    curl_multi_remove_handle(cm, eh);
                    cleanup(eh);
                    recv_buf_cleanup(curl_recv_buf);

                }
            }
            else {
                fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
            }
        }
    }
    curl_multi_cleanup(cm);

    // write found pngs to png_urls.txt
    FILE *png_urls;
    png_urls = fopen("png_urls.txt", "w");
    while(!isSEmpty(PNG_stack)){
        char* curr_line = popStack(PNG_stack);
        fwrite(curr_line, sizeof(char), strlen(curr_line), png_urls);
        free(curr_line);
        fwrite("\n", sizeof(char), 1, png_urls);
    }
    fclose(png_urls);

    // write visited urls to <log>.txt if -v provided
    if (create_log){
        FILE *log;
        log = fopen(logfile_name, "w");
        while(!isSEmpty(HTML_stack)){
            char* curr_line = popStack(HTML_stack);
            fwrite(curr_line, sizeof(char), strlen(curr_line), log);
            free(curr_line);
            fwrite("\n", sizeof(char), 1, log);
        }
        fclose(log);
    }


    /* cleaning up */
    hdestroy_r(queued_URLS);
    free(queued_URLS);
    freeQueue(URL_frontier);
    freeStack(PNG_stack);
    freeStack(HTML_stack);

    for (int i = 0; i < urls_count; i++){
        free(urls_to_free[i]);
    }
    


    if(gettimeofday(&tv, NULL) != 0){
        abort();
    }
    times[1] = (tv.tv_sec) + tv.tv_usec / 1000000.;
    printf("findpng2 execution time: %.6lf seconds\n", times[1] - times[0]);

    return 0;
}
