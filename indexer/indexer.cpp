#include <iostream>
//#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <omp.h>
#include <nlohmann/json.hpp>
#include "structure/hashmap.hpp"

using namespace std;
using json = nlohmann::json;

// Function Prototypes
void fileReadUrl_OutgoingLinks(HashMap<string, vector<string>>& url_OutgoingLinks);
HashMap<string, vector<string>> creatingInboundLinksMapping(const HashMap<string, vector<string>>& url_OutgoingLinks);
HashMap<string, pair<double, vector<string>>> initializePageRank(const HashMap<string, vector<string>>& url_OutgoingLinks);
double getPageRankContributionFromPages(const HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank, const HashMap<string, vector<string>>& inboundLinks_URL, const string& url);
void calculateFinalPageRanks(HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank, const HashMap<string, vector<string>>& inboundLinks_URL);
void writePageRankToFile(const HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank);

void fileReadkeyWords_Urls(HashMap<string, vector<pair<string, double>>>& keyWords_Urls);
void TF_IDFcalculation(HashMap<string, vector<pair<string, double>>>& keyWords_Urls);
void writeTFIDFToFile(const HashMap<string, vector<pair<string, double>>>& keyWords_Urls);

// Main Function
int main() {
    // Initializing data structures
    cout << "running" << endl;
    HashMap<string, vector<pair<string, double>>> keyWords_Urls;
    HashMap<string, vector<string>> url_OutgoingLinks;

    // Reading the data set
    fileReadkeyWords_Urls(keyWords_Urls);
    fileReadUrl_OutgoingLinks(url_OutgoingLinks);

    // TF-IDF Calculation
    TF_IDFcalculation(keyWords_Urls);
    cout << "TF-IDF WROTE TO FILE" << endl;

    // Initialize inbound and outbound links
    HashMap<string, vector<string>> inboundLinks_URL = creatingInboundLinksMapping(url_OutgoingLinks);
    HashMap<string, pair<double, vector<string>>> outboundLinksWithPageRank = initializePageRank(url_OutgoingLinks);

    // Calculate final PageRanks
    calculateFinalPageRanks(outboundLinksWithPageRank, inboundLinks_URL);
    cout << "PAGE RANK WROTE TO FILE" << endl;

    writeTFIDFToFile(keyWords_Urls);
    writePageRankToFile(outboundLinksWithPageRank);

    return 0;
}

void fileReadUrl_OutgoingLinks(HashMap<string, vector<string>>& url_OutgoingLinks) {
    json tempJson;
    ifstream inputFile("../jsonFiles/outgoingLinks.json");
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open the file: " << "outgoingLinks.json" << endl;
        return;
    }

    inputFile >> tempJson;

    for (const auto& [key, value] : tempJson.items()) {
        if (value.is_array()) {
            for (const auto& outgoingUrl : value) {
                url_OutgoingLinks[key].emplace_back(outgoingUrl.get<string>());
                // Ensure all URLs are included
                if (url_OutgoingLinks.find(outgoingUrl) == url_OutgoingLinks.end()) {
                    url_OutgoingLinks[outgoingUrl] = {};
                }
            }
        }
        // Ensure all URLs are included
        if (url_OutgoingLinks.find(key) == url_OutgoingLinks.end()) {
            url_OutgoingLinks[key] = {};
        }
    }
}

HashMap<string, vector<string>> creatingInboundLinksMapping(const HashMap<string, vector<string>>& url_OutgoingLinks) {
    HashMap<string, vector<string>> inboundLinks_URL;

    #pragma omp parallel for
    for (const auto& [sourceUrl, outgoingUrls] : url_OutgoingLinks) {
        for (const auto& targetUrl : outgoingUrls) {
            // Add the source URL as an inbound link for the target URL
            inboundLinks_URL[targetUrl].push_back(sourceUrl);
        }
    }

    return inboundLinks_URL;
}

HashMap<string, pair<double, vector<string>>> initializePageRank(const HashMap<string, vector<string>>& url_OutgoingLinks) {
    HashMap<string, pair<double, vector<string>>> pageRankMap;
    size_t numberOfDocs = url_OutgoingLinks.size();
    double initialPageRank = 1.0 / numberOfDocs;

    for (const auto& entry : url_OutgoingLinks) {
        const string& url = entry.first;
        pageRankMap[url] = {initialPageRank, entry.second};  // Initialize PageRank and outbound links
    }

    return pageRankMap;
}

double getPageRankContributionFromPages(const HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank, const HashMap<string, vector<string>>& inboundLinks_URL, const string& url) {
    double contribution = 0.0;

    // Check if the URL exists in the inboundLinks_URL map
    if (inboundLinks_URL.find(url) != inboundLinks_URL.end()) {
        vector<string> inboundUrls = inboundLinks_URL.at(url);

        for (const string& incomingUrl : inboundUrls) {
            const auto& [pageRank, outboundLinks] = outboundLinksWithPageRank.at(incomingUrl);
            if (!outboundLinks.empty()) {
                contribution += pageRank / outboundLinks.size();  // Contribution from each inbound link
            }
        }
    }

    return contribution;
}

void calculateFinalPageRanks(HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank, const HashMap<string, vector<string>>& inboundLinks_URL) {
    const double errorMargin = 0.0001;
    const double dampingFactor = 0.85;
    double noOfPages = outboundLinksWithPageRank.size();
    double teleportationProb = (1 - dampingFactor) / noOfPages;
    double error;

    do {
        error = 0.0;  // Reset error for this iteration
        HashMap<string, double> newPageRanks;
        double sinkPageRank = 0.0;

        #pragma omp parallel for reduction(+:sinkPageRank)
        for (const auto& [url, data] : outboundLinksWithPageRank) {
            const auto& [currentPageRank, outboundLinks] = data;
            if (outboundLinks.empty()) {
                sinkPageRank += currentPageRank;
            }
        }

        #pragma omp parallel for reduction(+:error)
        for (auto& [url, data] : outboundLinksWithPageRank) {
            auto& [currentPageRank, outboundLinks] = data;

            double newPageRank = teleportationProb + dampingFactor * (sinkPageRank / noOfPages + getPageRankContributionFromPages(outboundLinksWithPageRank, inboundLinks_URL, url));

            error += abs(newPageRank - currentPageRank);
            newPageRanks[url] = newPageRank;
        }

        // Update the PageRanks
        for (auto& [url, data] : outboundLinksWithPageRank) {
            data.first = newPageRanks[url];
        }

    } while (error > errorMargin);
}

void writePageRankToFile(const HashMap<string, pair<double, vector<string>>>& outboundLinksWithPageRank) {
    json pageRankJson;

    for (const auto& entry : outboundLinksWithPageRank) {
        string url = entry.first;
        double pageRank = entry.second.first;

        pageRankJson[url] = pageRank;
    }

    // Write to file
    ofstream outputFile("../jsonFiles/pagerank_output.json");
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open the file to write PageRank data." << endl;
        return;
    }

    outputFile << pageRankJson.dump(4);  // Dump the JSON with indentation of 4 spaces
    outputFile.close();
}

void fileReadkeyWords_Urls(HashMap<string, vector<pair<string,double>>> &keyWords_Urls)
{
    json tempJson;
    ifstream inputFile("../jsonFiles/keywords_domains.json");
    if( !inputFile.is_open())
    {
        cerr << "Error: Could not open the file keywords_domains.json";
        return;
    }

    inputFile >> tempJson;

    for ( const auto& [ key, value] : tempJson.items() )
        if ( value.is_array())
            for ( const auto& obj:value)
                if ( obj.is_object() ) 
                    for (const auto& [url, count] : obj.items()) 
                        if (count.is_number()) 
                            keyWords_Urls[key].emplace_back(url, count.get<double>());

}

void TF_IDFcalculation(  HashMap<string , vector< pair<string,double> > > &keyWords_Urls )
{
    size_t NumberOfDocs = keyWords_Urls.size();


    #pragma omp parallel for
    for ( const auto& entry : keyWords_Urls)
    {
        string key = entry.first;
        vector< pair<string,double> > vec = entry.second;
        double numberOfDocsContainingTerm = vec.size();

        // Calculate idf
        double inverseDocumentFrequency = log ( static_cast<double> (NumberOfDocs) / ( 1 + numberOfDocsContainingTerm) );
        
        //Calculate tf-idf and store it
        // here the int will be replaced with relative fequency so each doc has actual tdidf
        for ( auto& entryInVector : vec )
        {
            double relativeFrequency =  entryInVector.second;
            entryInVector.second = relativeFrequency * inverseDocumentFrequency ;
        }
    }
}

void writeTFIDFToFile(const HashMap<string, vector<pair<string, double>>>& keyWords_Urls)
{
    json tfidfJson;

    for (const auto& entry : keyWords_Urls) {
        string key = entry.first;
        json keywordsArray;

        for (const auto& keyword : entry.second) {
            json keywordObject;
            keywordObject["url"] = keyword.first;
            keywordObject["tfidf"] = keyword.second;
            keywordsArray.push_back(keywordObject);
        }

        tfidfJson[key] = keywordsArray;
    }

    // Write to file
    ofstream outputFile("../jsonFiles/tfidf_output.json");
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open the file to write TF-IDF data." << endl;
        return;
    }

    outputFile << tfidfJson.dump(4);  // Dump the JSON with indentation of 4 spaces
    outputFile.close();
}