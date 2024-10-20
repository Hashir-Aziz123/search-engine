#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/uri.h>
#include <unordered_set>
#include <queue> 

using namespace std;

//Global queue and unordered set for bfs web crawling
unordered_set<string> visitedURLs;
queue<string> urlQueue;

// Data Structure for memory usage
struct Node {
    char* packetHTML;
    int packetSize;
};

// Function declarations
size_t storeHTML(void *packetContent, size_t size, size_t nmemb, void* node); // callback function

int crawlWeb ( const char* baseURL ); 
void parseHTML(CURL* curl, char* HTML, const char* baseURL);
void trv_DOM_push_URL(CURL* curl, xmlNode* node, const char* baseURL );
Node makeHTTPRequest(CURL* curl, const char* baseURL);
char* resolveURL(const char* baseURL, const char* relativeURL);





int main() 
{
    // Initialize curl and base baseURL
    const char* baseURL = "https://en.wikipedia.org/wiki/Main_Page";
    crawlWeb (baseURL);
  
    return EXIT_SUCCESS;
}




int crawlWeb ( const char* baseURL )
{
    CURL* curl;
    curl = curl_easy_init();
    if (curl == nullptr) 
    {
        cout << "Failed to initialize curl" << endl;
        return EXIT_FAILURE;
    }
    // Enqueue the base URL and mark it as visited
    urlQueue.push(baseURL);
    visitedURLs.insert(baseURL); 

    // Making request
    while ( !urlQueue.empty() )
    {
        const string currentURL = urlQueue.front();
        urlQueue.pop();

        Node node = makeHTTPRequest(curl, currentURL.c_str() );
        // Parse the HTML
        if ( node.packetSize != -1)
            parseHTML(curl, node.packetHTML, baseURL);

        free(node.packetHTML); // Free the memory after parsing

    }

    // Perform curl cleanup
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;
}

Node makeHTTPRequest(CURL* curl, const char* baseURL)
{
    // Node declaration
    Node node;
    node.packetHTML = (char*) malloc(1);
    node.packetSize = 0;

    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, baseURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, storeHTML);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&node);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);


    // Perform curl action
    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) 
    {
        cout << "ERROR: " << curl_easy_strerror(result) << endl;
        node.packetSize = -1;
        return node;
    }

    return node;
}



void parseHTML(CURL* curl, char* HTML, const char* baseURL) 
{
    htmlDocPtr doc = htmlReadMemory(HTML, strlen(HTML), baseURL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

    if (doc == NULL) {
        cout << "Parsing failed. Exiting function" << endl;
        return;
    }

    // Get the root node and start traversing
    xmlNode* rootNode = xmlDocGetRootElement(doc);
    trv_DOM_push_URL(curl, rootNode, baseURL); // Traverse the entire DOM tree

    xmlFreeDoc(doc); // Free the document after use
}



// Recursive function to traverse nodes
void trv_DOM_push_URL(CURL* curl, xmlNode* node, const char* baseURL) 
{
    for (; node; node = node->next) 
    {
        // Check if the node is an <a> tag
        if (xmlStrcasecmp(node->name, BAD_CAST "a") == 0) 
        {
            xmlChar* href = xmlGetProp(node, BAD_CAST "href");

            if (href == nullptr) 
                continue;
            // Check if the href starts with '#' (fragment link)
            if (strncmp((const char*)href, "#", 1) == 0) {
                xmlFree(href);
                continue;
            }

            char* resolvedURL = resolveURL(baseURL, (const char*)href);
            if (resolvedURL != nullptr)
            {
                // Check if this URL has already been visited
                string urlString(resolvedURL);
                if  (visitedURLs.find(urlString) == visitedURLs.end() ) {
                    cout << "Found URL: " << urlString << endl;
                    visitedURLs.insert(urlString);  // Add to visited set
                    urlQueue.push( urlString);
                }

                free(resolvedURL);
            }
            xmlFree(href);
            
        }
        // Recursively traverse child nodes
        trv_DOM_push_URL(curl, node->children, baseURL);
    }
}



// funtion to get absoloute URL
char* resolveURL(const char* baseURL, const char* relativeURL) 
{
    xmlChar* fullURL = xmlBuildURI(BAD_CAST relativeURL, BAD_CAST baseURL);
    if (fullURL == NULL)
        return NULL;

    char* result = strdup((const char*) fullURL);
    xmlFree(fullURL);

    return result;
}



// Callback functions
size_t storeHTML(void* packetContent, size_t size, size_t nmemb, void* node) 
{
    Node* castedNode = (Node*) node;

    char* tempPtr = (char*) realloc(castedNode->packetHTML, castedNode->packetSize + (size * nmemb) + 1);
    if (tempPtr == NULL)
        return 0;

    castedNode->packetHTML = tempPtr;
    memcpy(&(castedNode->packetHTML[castedNode->packetSize]), packetContent, size * nmemb);
    castedNode->packetSize += size * nmemb;
    castedNode->packetHTML[castedNode->packetSize] = 0;

    return size * nmemb;
}
