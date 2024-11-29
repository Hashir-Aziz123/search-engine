#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <regex>

#include <unordered_set>
#include <map>
#include <queue>
#include <set>

#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/uri.h>

#include <pybind11/embed.h>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
namespace py = pybind11;
using json = nlohmann::json;

//Global queue and unordered set for bfs web crawling
unordered_set<string> visitedURLs;
queue<string> urlQueue;

//Keyword map
json keyword_to_url_hashmap;
json url_to_outgoingLinks_hashmap;



//Stop words to contain
const unordered_set<string> stopWords = {
    "I" , "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours",
    "yourself", "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself",
    "it", "its", "itself", "they", "them", "their", "theirs", "themselves", "what", "which",
    "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be",
    "been", "being", "have", "has", "had", "having", "do", "does", "did", "doing", "a", "an",
    "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by",
    "for", "with", "about", "against", "between", "into", "through", "during", "before", "after",
    "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under",
    "again", "further", "then", "once", "here", "there", "when", "where", "why", "how", "all",
    "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not",
    "only", "own", "same", "so", "than", "too", "very", "s", "t", "can", "will", "just",
    "don", "should", "now" // maybe add more
};

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
void parseHTML(char* HTML, const char* baseURL, const string& currentURL);
void dom_traversal_and_processing(xmlNode* node, const char* baseURL , const string& currentURL,  char* HTML_Content);
string extractDomain(const string& url);


//FUNCTIONS TO PROCESS HTML CONTENT
void handleKeyWordsDetection( xmlNode* node, const string& currentURL , char* HTML_Content);
void handleURLDetection( xmlNode* node , const char* baseURL , const string& currentURL);
int getKeywordCount(string keyword , char* HTML_Content);

// FUNCTIONS TO CHECK ROBOT.TXT COMPLIANCE
Node fetchRobotsTxt ( CURL* curl , const char* baseURL );
unordered_set<string> parseRobotsTxt ( char* robotsTxtContent);
bool isURLAllowed( const string currentURL , const unordered_set<string>& disallowedPath );

//FUNCTIONS TO PROCESS KEYWORDS EXTRACTION
vector<string> processKeyWords(string text);
void handleURLDetection( xmlNode* node , const char* baseURL);

//removes duplicate URLs from output
void removeDuplicates(json& j);

py::scoped_interpreter guard{}; 

py:: module lemmatizer = py::module::import("lemmatizer");
py::object lemmatize_word = lemmatizer.attr("lemmatize_word");




int main() 
{
    // Initialize curl and base baseURL
    const char* baseURL = "https://alltop.com/";
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

    // keep current domain
    string currentDomain = extractDomain(baseURL);

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

        string newDomain  = extractDomain( currentURL ) ; 
        if ( newDomain != currentDomain) // getting robots.txt of new domain
        {
            cout << "Switching to new domain: " << newDomain << endl;
            currentDomain = newDomain;

            // Fetch and check the robots.txt for the new domain
            Node newRobotTxtNode = fetchRobotsTxt( curl , currentDomain.c_str() );
            if (newRobotTxtNode.packetSize != -1)
            {
                disallowedPaths = parseRobotsTxt( newRobotTxtNode.packetHTML );
                free( newRobotTxtNode.packetHTML );
            }
            else 
            {
                // Clear disallowed paths if no robots.txt found
                disallowedPaths.clear();
            }
        }

        if ( !isURLAllowed ( currentURL, disallowedPaths) )
        {
            cout<< "URL dissallowed by robots.txt" << currentURL << endl;
            continue;
        }

        Node node = makeHTTPRequest(curl, currentURL.c_str() );
        // Parse the HTML
        if ( node.packetSize != -1) {
            parseHTML(node.packetHTML, newDomain.c_str(), currentURL);

            // Creating the reverse indexing hashmap

            ofstream outFile("/home/lbp400/DSAProject/jsonFiles/keywords_domains2.json");
            if (!outFile) {
                cerr << "Failed to open output file" << endl;
                return 1;
            }
            removeDuplicates(keyword_to_url_hashmap);
            outFile << keyword_to_url_hashmap.dump(4); // prints to the file with indentation 4
            outFile.close();

            // Creating the url to outgoing links hashmap
            ofstream outFile2("/home/lbp400/DSAProject/jsonFiles/outgoingLinks.json");
            if (!outFile2) {
                cerr << "Failed to open output2 file" << endl;
                return 1;
            }
            outFile2 << url_to_outgoingLinks_hashmap.dump(4);
            outFile2.close();

        }
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000);



    // Perform curl action
    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) 
    {
        cout << "ERROR: " << curl_easy_strerror(result) << endl;
        node.packetSize = -1;
        return node;
    }

    // converting node HTML to lowercase before returning
    string content(node.packetHTML);
    transform(content.begin(), content.end(), content.begin(), ::tolower);
    strcpy(node.packetHTML, content.c_str());

    return node;
}


void parseHTML(char* HTML, const char* baseURL, const string& currentURL) {
    htmlDocPtr doc = htmlReadMemory(HTML, strlen(HTML), baseURL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL) {
        cout << "Parsing failed. Exiting function" << endl;
        return;
    }

    xmlNode* rootNode = xmlDocGetRootElement(doc);
    dom_traversal_and_processing(rootNode, baseURL, currentURL , HTML);

    xmlFreeDoc(doc);
}

void dom_traversal_and_processing(xmlNode* node, const char* baseURL, const string& currentURL , char* HTML_CONTENT) {
    for (; node; node = node->next) {
        if (xmlStrcasecmp(node->name, BAD_CAST "a") == 0)
            handleURLDetection(node, baseURL , currentURL);

        if (xmlStrcasecmp(node->name, BAD_CAST "title") == 0 || 
            xmlStrcasecmp(node->name, BAD_CAST "h1") == 0 ||
            xmlStrcasecmp(node->name, BAD_CAST "h2") == 0 || 
            xmlStrcasecmp(node->name, BAD_CAST "h3") == 0 ||
            xmlStrcasecmp(node->name, BAD_CAST "h4") == 0 ||
            xmlStrcasecmp(node->name, BAD_CAST "h5") == 0 ||
            xmlStrcasecmp(node->name, BAD_CAST "h6") == 0) {
            handleKeyWordsDetection(node, currentURL , HTML_CONTENT);
        }

        dom_traversal_and_processing(node->children, baseURL, currentURL , HTML_CONTENT);
    }
}



//FUNCTIONS TO PROCESS HTML CONTENT




void handleURLDetection( xmlNode* node , const char* baseURL , const string& currentURL )
{
    xmlChar* href = xmlGetProp(node, BAD_CAST "href");

    if (href == nullptr) 
        return;
    
    // Check if the href starts with '#' (fragment link)
    if (strncmp((const char*)href, "#", 1) == 0) 
    {
        xmlFree(href);
        return;
    }

    char* resolvedURL = resolveURL(baseURL, (const char*)href);
    if (resolvedURL != nullptr)
    {
        // CONVERT INTO SEPERATE FUNCTION 

            // checking if url has valid scheme
        if (strncmp(resolvedURL, "http://", 7) != 0 && strncmp(resolvedURL, "https://", 8) != 0) 
        {
            free(resolvedURL);
            xmlFree(href);
            return;
        }

        // Ensuring domain consistency
        string tempUrlString(resolvedURL);  // THESE THREE ATTRIBUTES ARE USED LATER
        size_t posDot1 = tempUrlString.find("//") + 2; // Find the start of the domain
        size_t posDot2 = tempUrlString.find("/", posDot1); // Find the end of the domain part

        if (posDot2 == string::npos) 
            posDot2 = tempUrlString.length(); 
        

        // Convert the domain part to lowercase
        for (int i = posDot1; i < posDot2; i++) {
            tempUrlString[i] = tolower(tempUrlString[i]);
        }

        // Standardizing all URLs to "www" prefix
        if (strncmp(tempUrlString.c_str(), "http://www.", 11) != 0 && strncmp(tempUrlString.c_str(), "https://www.", 12) != 0) 
        {
            tempUrlString = tempUrlString.substr(0, posDot1) + "www." + tempUrlString.substr(posDot1);
        }

        // removing traling slashes
        if (tempUrlString[tempUrlString.length() - 1] == '/') 
            tempUrlString = tempUrlString.substr(0, tempUrlString.length() - 1);  // Remove the last character


        // If you need to convert back to char* (if other parts of your code require it):
        free(resolvedURL);  // Free the old resolvedURL memory if it was previously allocated
        resolvedURL = strdup(tempUrlString.c_str());  // Allocate new memory for char* with the updated URL



        // Check if this URL has already been visited
        string urlString(resolvedURL);
        if  (visitedURLs.find(urlString) == visitedURLs.end()) 
        {
            cout << "Found URL: " << urlString << endl;
            visitedURLs.insert(urlString);  // Add to visited set
            urlQueue.push( urlString);
            
            url_to_outgoingLinks_hashmap[currentURL].push_back(resolvedURL);
        }

        free(resolvedURL);
    }
    xmlFree(href);
}

void handleKeyWordsDetection(xmlNode* node, const string& currentURL , char* HTML_CONTENT) {
    map<string, int> keywordsCount;
    vector<string> keyWordsList;

    xmlChar* rawText = xmlNodeGetContent(node);
    if (rawText) {
        string rawTextString((char*)rawText);
        keyWordsList = processKeyWords(rawTextString);
        for (const auto& keyword : keyWordsList) {
            keywordsCount.insert({keyword, getKeywordCount(keyword , HTML_CONTENT)});
        }

        for (const auto& [keyword, count] : keywordsCount) {
            if (count > 0) {
                keyword_to_url_hashmap[keyword].push_back({{currentURL, count}});
            }
        }

        xmlFree(rawText);
    }
}

void removeDuplicates(json& j) {
    for (auto& [key, value] : j.items()) {
        if (value.is_array()) {
            std::set<json> unique_values(value.begin(), value.end());
            value = json::array(); // Clear the current array
            for (const auto& unique_val : unique_values) {
                value.push_back(unique_val); // Add back unique values
            }
        }
    }
}

int getKeywordCount(string keyword , char* HTML_Content) {
    
    int count = 0;
    const string content(HTML_Content);
    size_t pos = content.find(keyword);

    while (pos != string::npos) {
        ++count;
        pos = content.find(keyword, pos + 1); // Move to the next occurrence
    }

    return count;
}


// UTILITY FUNCTIONS 

// funtion to get absoloute URL
char* resolveURL(const char* baseURL, const char* relativeURL) 
{
    // If the relative URL is actually an absolute URL, return it directly
    if (strncmp(relativeURL, "http://", 7) == 0 || strncmp(relativeURL, "https://", 8) == 0) {
        return strdup(relativeURL);  // Return the absolute URL directly
    }

    //In case or relative url
    xmlChar* fullURL = xmlBuildURI(BAD_CAST relativeURL, BAD_CAST baseURL);
    if (fullURL == NULL)
        return NULL;

    char* result = strdup((const char*) fullURL);
    xmlFree(fullURL);

    return result;
}

string extractDomain(const string& url)  // "https://en.wikipedia.org/wiki/Main_Page"
{
    size_t protocolPos = url.find("://");
    size_t domainStart = (protocolPos == string::npos) ? 0 : protocolPos + 3;

    // Find the position of the first '/' after the domain
    size_t domainEnd = url.find("/", domainStart);
    if (domainEnd == string::npos) {
        domainEnd = url.length();
    }

    // Extract the domain part
    string domain = url.substr(domainStart, domainEnd - domainStart);

    // Split the domain by '.' to identify parts
    size_t lastDot = domain.rfind('.');
    size_t secondLastDot = domain.rfind('.', lastDot - 1);

    // If there are two or more parts, extract SLD + TLD
    if (secondLastDot != string::npos) {
        return domain.substr(secondLastDot + 1);
    }

    return domain;
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
                    disallowedPath.erase( remove ( disallowedPath.begin() , disallowedPath.end() , ' ') , disallowedPath.end() ) ; // removing spaces from disallowed path
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

bool isURLAllowed(const string currentURL , const unordered_set<string>& disallowedPaths)
{
    // Extract path starting after "https://"
    string urlPath;
    size_t pos = currentURL.find("/", 8);  // Find position of "/" after "https://"

    // Check if the position is valid
    if (pos != string::npos) 
        urlPath = currentURL.substr(pos);
    else 
        urlPath = "/";  // Handle the case where no "/" is found after the domain

    for (const string& disallowed : disallowedPaths) 
    {
        if (urlPath.find(disallowed) == 0) // Check if URL path starts with a disallowed path
        {
            return false;
        }
    }
    return true;
}




//FUNCTIONS TO PROCESS KEYWORDS EXTRACTION


vector<string> processKeyWords( string text)
{
    cout<< "Processing keyword from word stream";
    vector<string> keywords;
    py::gil_scoped_acquire acquire; 

    stringstream ss(text);
    string word;
    while ( ss >> word )
    {
        word = regex_replace(word, regex("[^a-zA-Z]"), "");
        
        if (word.empty()) continue;

        try 
        {
            py::object result = lemmatize_word(word);
            if ( stopWords.find(result.cast<string>()) == stopWords.end() )
            {
                string keyword = result.cast<string>();
                for (int i = 0; i < keyword.length(); i++)
                    keyword[i] = tolower(keyword[i]);
                keywords.push_back(keyword);
            }
        }
        catch( const py:: error_already_set & e )
        {
            cout << "Python error :" << e.what() << endl;
        }   
    }

    return keywords;
}