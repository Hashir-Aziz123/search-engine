#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <algorithm>
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

// CALL BACK FUNCTION TO STORE HTML
size_t storeHTML(void *packetContent, size_t size, size_t nmemb, void* node); // callback function

// FUNCTIONS TO CRAWL WEBPAGES AND PARSE HTML 
int crawlWeb ( const char* baseURL ); 
char* resolveURL(const char* baseURL, const char* relativeURL);
Node makeHTTPRequest(CURL* curl, const char* baseURL);
void parseHTML(CURL* curl, char* HTML, const char* baseURL);
void trv_DOM_push_URL(CURL* curl, xmlNode* node, const char* baseURL );

// FUNTIONS TO CHECK ROBOT.TXT COMPLIANCE
Node fetchRobotsTxt ( CURL* curl , const char* baseURL );
unordered_set<string> parseRobotsTxt ( char* robotsTxtContent);
bool isURLAllowed( const string& urlPath , const unordered_set<string>& disallowedPath );







int main() 
{
    // Initialize curl and base baseURL
    const char* baseURL = "https://en.wikipedia.org/wiki/Main_Page";
    crawlWeb (baseURL);
  
    return EXIT_SUCCESS;
}

// FUNCTIONS TO CRAWL WEBPAGES AND PARSE HTML 


int crawlWeb ( const char* baseURL )
{
    CURL* curl;
    curl = curl_easy_init();
    if (curl == nullptr) 
    {
        cout << "Failed to initialize curl" << endl;
        return EXIT_FAILURE;
    }

    // Handling robots.txt for base domain
    unordered_set<string> disallowedPaths;
    Node robotTxtNode = fetchRobotsTxt( curl , baseURL );
    if ( robotTxtNode.packetSize != -1 )
    {
        disallowedPaths = parseRobotsTxt( robotTxtNode.packetHTML );
        free ( robotTxtNode.packetHTML) ;
    }

    // Enqueue the base URL and mark it as visited
    urlQueue.push(baseURL);
    visitedURLs.insert(baseURL); 

    // Making request
    while ( !urlQueue.empty() )
    {
        const string currentURL = urlQueue.front();
        urlQueue.pop();

         // Extract path starting after "https://"
        string path;
        size_t pos = currentURL.find("/", 8);  // Find position of "/" after "https://"

        // Check if the position is valid
        if (pos != string::npos) 
            path = currentURL.substr(pos);
        else 
            path = "/";  // Handle the case where no "/" is found after the domain

        if ( !isURLAllowed ( path , disallowedPaths) )
        {
            cout<< "URL dissallowed by robots.txt" << currentURL << endl;
            continue;
        }

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



// CALLBACK FUNCTION
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


// FUNTIONS TO CHECK ROBOT.TXT COMPLIANCE

Node fetchRobotsTxt ( CURL* curl , const char* baseURL )
{
    string robotTxtURL = string(baseURL) + "/robots.txt";
    Node node = makeHTTPRequest ( curl , robotTxtURL.c_str() );
    return node;
}

unordered_set<string> parseRobotsTxt ( char* robotsTxtContent )
{

    // treat sting::npos like null
    unordered_set<string> disallowedPaths;
    string userAgent = "User-agent: *"; // Checks for global rules rather than rule for specific crawler

    char* line = strtok( robotsTxtContent , "\n" );
    bool isGlobalAgent = false;

    while ( line != NULL )
    {
        string lineStr  = string ( line );

        // checking for global agent
        if ( lineStr.find("User-agent: *")  != string::npos) // if we encouter a global user agent
            isGlobalAgent = true;
       else if ( isGlobalAgent && lineStr.find("User-agent:") != string::npos) // if we encounter some other user agent
            isGlobalAgent = false;

        // extracting disallowed paths
        if ( isGlobalAgent && ( lineStr.find("Disallow") != string::npos ) ) 
        {
            size_t colonPos = lineStr.find(":");
            if ( colonPos != string::npos && ( colonPos + 1 < lineStr.size() ) )
            {   
                try
                {
                    string disallowedPath = lineStr.substr( colonPos + 1);
                    disallowedPath.erase( remove ( disallowedPath.begin() , disallowedPath.end() , ' ') , disallowedPath.end() ) ;
                    disallowedPaths.insert( disallowedPath );
                }
                catch ( exception& e)
                {
                    cout << "Caught exception: " << e.what() << endl;
                }
            }
        }

        line = strtok( NULL , "\n" );
    }

    return disallowedPaths;
}

bool isURLAllowed(const string& urlPath, const unordered_set<string>& disallowedPaths)
{
    for (const string& disallowed : disallowedPaths) 
    {
        if (urlPath.find(disallowed) == 0) // Check if URL path starts with a disallowed path
        {
            return false;
        }
    }
    return true;
}