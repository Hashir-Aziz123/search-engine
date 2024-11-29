#include <iostream>
#include<map>
#include<vector>
#include<string>
#include <fstream>

#include <nlohmann/json.hpp>


using namespace std;
using json= nlohmann::json;

void fileReadkeyWords_Urls(  map<string , vector< pair<string,int> > > &keyWords_Urls );
void fileReadUrl_OutgoingLinks(  map<string , vector<string> > &url_OutgoingLinks );
void TF_IDFcalculation();

int main()
{
    // intitializing data structs
    map<string , vector< pair<string,int> > > keyWords_Urls;
    map<string , vector<string> > url_OutgoingLinks;

    // reading the data set
    fileReadUrl_OutgoingLinks( url_OutgoingLinks);
    // for (const auto& [key,value] : url_OutgoingLinks )
    //     {
    //         cout<< key << "-> " << endl;
    //         for ( const auto& urls : value)
    //             cout << "  " << urls << endl;
    //     }

    fileReadkeyWords_Urls(keyWords_Urls);
    // for (const auto& [key, values] : keyWords_Urls) {
    //     cout << key << " ->" << endl;
    //     for (const auto& [url, count] : values) {
    //         cout << "  " << url << " : " << count << endl;
    //     }
    // }

    // tdfidf calculation:

    // pagerank calculation

    // wrting the final output to a file


    return 0;
}



void fileReadkeyWords_Urls(  map<string , vector< pair<string,int> > > &keyWords_Urls )
{
    json tempJson;
    ifstream inputFile("/home/lbp400/DSAProject/jsonFiles/keywords_domains2.json");
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
                        if (count.is_number_integer()) 
                            keyWords_Urls[key].emplace_back(url, count.get<int>());

}

void fileReadUrl_OutgoingLinks(  map<string , vector<string> > &url_OutgoingLinks )
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
