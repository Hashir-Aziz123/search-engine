#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <omp.h>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Function Prototypes
void fileReadkeyWords_Urls(unordered_map<string, vector<pair<string, double>>>& keyWords_Urls);
void fileReadUrl_OutgoingLinks(unordered_map<string, vector<string>>& url_OutgoingLinks);
void TF_IDFcalculation(unordered_map<string, vector<pair<string, double>>>& keyWords_Urls);
unordered_map<string, vector<string>> creatingInboundLinksMapping( unordered_map<string, vector<string>> url_OutgoingLinks);

unordered_map<string, pair<double, vector<string>>> initializePageRank(const unordered_map<string, vector<string>> url_OutgoingLinks);
double getPageRankContributionFromPages( unordered_map<string, pair<double, vector<string>>> outboundLinksWithPageRank,  unordered_map<string, vector<string>> inboundLinks_URL, string url);
void calculateFinalPageRanks(unordered_map<string, pair<double, vector<string>>>& outboundLinksWithPageRank, unordered_map<string, vector<string>> inboundLinks_URL);

void writePageRankToFile(const unordered_map<string, pair<double, vector<string>>>& outboundLinksWithPageRank);
void writeTFIDFToFile(const unordered_map<string, vector<pair<string, double>>>& keyWords_Urls);

// Main Function
int main() {
    // Initializing data structures
    cout<< "running" << endl;
    unordered_map<string, vector<pair<string, double>>> keyWords_Urls;
    unordered_map<string, vector<string>> url_OutgoingLinks;

    // Reading the data set
    fileReadkeyWords_Urls(keyWords_Urls);
    fileReadUrl_OutgoingLinks(url_OutgoingLinks);

    // TF-IDF Calculation
    TF_IDFcalculation(keyWords_Urls);
    cout << "TF-IDF WROTE TO FILE" << endl;

    // Initialize inbound and outbound links
    unordered_map<string, vector<string>> inboundLinks_URL = creatingInboundLinksMapping(url_OutgoingLinks);
    unordered_map<string, pair<double, vector<string>>> outboundLinksWithPageRank = initializePageRank(url_OutgoingLinks);

    // Calculate final PageRanks
    calculateFinalPageRanks(outboundLinksWithPageRank, inboundLinks_URL);
    cout << "PAGE RANK WROTE TO FILE" << endl;

    writeTFIDFToFile(keyWords_Urls);
    writePageRankToFile(outboundLinksWithPageRank);

    return 0;
}

void fileReadkeyWords_Urls(  unordered_map<string , vector< pair<string,double> > > &keyWords_Urls )
{
    json tempJson;
    ifstream inputFile("/home/lbp400/DSAProject/jsonFiles/keywords_domains.json");
    if( !inputFile.is_open())
    {
        cerr << "Error: Could not open the file: " << "keywords_domains2" ;
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

void fileReadUrl_OutgoingLinks(  unordered_map<string , vector<string> > &url_OutgoingLinks )
{
    json tempJson;
    ifstream inputFile("/home/lbp400/DSAProject/jsonFiles/outgoingLinks.json");
    if( !inputFile.is_open())
    {
        cerr << "Error: Could not open the file: " << "outgoingLinks2" ;
        return;
    }

    inputFile >> tempJson;

    for( const auto& [key,value] : tempJson.items() )
        if (value.is_array())
            for ( const auto& outgoingUrl : value )  
                url_OutgoingLinks[key].emplace_back(outgoingUrl.get<string>());


}

void TF_IDFcalculation(  unordered_map<string , vector< pair<string,double> > > &keyWords_Urls )
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


unordered_map<string , vector<string> > creatingInboundLinksMapping ( unordered_map<string , vector<string> > url_OutgoingLinks  )
{
    unordered_map<string , vector<string> > inboundLinks_URL;

     // Traverse the outgoing links unordered_map
     // might switch to this later
    #pragma omp parallel for 
    for (const auto& [sourceUrl, outgoingUrls] : url_OutgoingLinks) {
        for (const auto& targetUrl : outgoingUrls) {
            // Add the source URL as an inbound link for the target URL
            inboundLinks_URL[targetUrl].push_back(sourceUrl);
        }
    }

    return inboundLinks_URL;
}    

unordered_map<string, pair<double, vector<string>>> initializePageRank( unordered_map<string, vector<string>> url_OutgoingLinks) 
{
    unordered_map<string, pair<double, vector<string>>> pageRankMap;
    size_t numberOfDocs = url_OutgoingLinks.size();
    double initialPageRank = 1.0 / numberOfDocs;

    for (const auto& entry : url_OutgoingLinks) {
        const string& url = entry.first;
        pageRankMap[url] = {initialPageRank, entry.second};  // Initialize PageRank and outbound links
    }

    return pageRankMap;
}

double getPageRankContributionFromPages( unordered_map<string, pair<double, vector<string>>> outboundLinksWithPageRank, unordered_map<string, vector<string>> inboundLinks_URL, string url) 
{
    double contribution = 0.0;
    vector<string> inboundUrls = inboundLinks_URL[url];

    for (const string& incomingUrl : inboundUrls) 
    {
        const auto& [pageRank, outboundLinks] = outboundLinksWithPageRank[incomingUrl];
        if (!outboundLinks.empty()) 
            contribution += pageRank / outboundLinks.size();  // Contribution from each inbound link
    }

    return contribution;
}

void calculateFinalPageRanks(unordered_map<string, pair<double, vector<string>>>& outboundLinksWithPageRank, unordered_map<string, vector<string>> inboundLinks_URL) 
{
    double error = 1.0;
    const double errorMargin = 0.0001;
    const double dampingFactor = 0.85;
    double noOfPages = outboundLinksWithPageRank.size();
    bool converged = false;

    while (!converged) 
    {
        converged = true;
        for (auto& [url, data] : outboundLinksWithPageRank) {
            auto& [currentPageRank, outboundLinks] = data;

            double newPageRank = (1 - dampingFactor) / noOfPages +
                                 dampingFactor * getPageRankContributionFromPages(outboundLinksWithPageRank, inboundLinks_URL, url);

            if (abs(newPageRank - currentPageRank) < errorMargin) {
                converged = true;
            }

            currentPageRank = newPageRank;  // Update the PageRank
        }
    }

}


void writeTFIDFToFile(const unordered_map<string, vector<pair<string, double>>>& keyWords_Urls) 
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
    ofstream outputFile("/home/lbp400/DSAProject/jsonFiles/tfidf_output.json");
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open the file to write TF-IDF data." << endl;
        return;
    }

    outputFile << tfidfJson.dump(4);  // Dump the JSON with indentation of 4 spaces
    outputFile.close();
}

void writePageRankToFile(const unordered_map<string, pair<double, vector<string>>>& outboundLinksWithPageRank) 
{
    json pageRankJson;

    for (const auto& entry : outboundLinksWithPageRank) {
        string url = entry.first;
        double pageRank = entry.second.first;

        pageRankJson[url] = pageRank;
    }

    // Write to file
    ofstream outputFile("/home/lbp400/DSAProject/jsonFiles/pagerank_output.json");
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open the file to write PageRank data." << endl;
        return;
    }

    outputFile << pageRankJson.dump(4);  // Dump the JSON with indentation of 4 spaces
    outputFile.close();
}

// void calculateFinalPageRanks(unordered_map<string, pair<double, vector<string>>>& outboundLinksWithPageRank, 
//                              unordered_map<string, vector<string>> inboundLinks_URL) 
// {
//     const double errorMargin = 0.0001;
//     const double dampingFactor = 0.85;
//     double noOfPages = outboundLinksWithPageRank.size();
//     unordered_map<string, double> previousPageRankMap;

//     // Initialize previousPageRankMap with current PageRank values
//     for (const auto& [url, data] : outboundLinksWithPageRank) {
//         previousPageRankMap[url] = data.first;
//     }

//     bool converged = false;

//     while (!converged) 
//     {
//         double totalChange = 0.0;

//         for (auto& [url, data] : outboundLinksWithPageRank) {
//             auto& [currentPageRank, outboundLinks] = data;

//             // Calculate the new PageRank
//             double newPageRank = (1 - dampingFactor) / noOfPages +
//                                  dampingFactor * getPageRankContributionFromPages(outboundLinksWithPageRank, inboundLinks_URL, url);

//             // Add the absolute change to totalChange
//             totalChange += abs(newPageRank - previousPageRankMap[url]);

//             // Update previousPageRankMap for the next iteration
//             previousPageRankMap[url] = currentPageRank;

//             // Update the current PageRank
//             currentPageRank = newPageRank;
//         }

//         // Check convergence condition based on totalChange
//         if (totalChange < errorMargin) {
//             converged = true;
//         }
//     }
// }
