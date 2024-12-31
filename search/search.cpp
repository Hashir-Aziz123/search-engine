#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cmath>
#include <regex>
#include <curl/curl.h>
#include <pybind11/embed.h>
#include <nlohmann/json.hpp>
#include "structures/hashmap.hpp"
#include "crow.h"
#include "crow/middlewares/cors.h"

using json = nlohmann::json;

pybind11::scoped_interpreter guard{};
pybind11::module lemmatizer = pybind11::module::import("lemmatizer");
pybind11::object lemmatize_word = lemmatizer.attr("lemmatize_word");

void read_pagerank(HashMap<std::string, double> &pagerankmap) {
    json temp_json;

    std::ifstream input_file("../jsonFiles/pagerank_output.json");
    if (!input_file) {
        std::cerr << "Error: Could not open file pagerank_output.json\n";
    }
    input_file >> temp_json;

    for (const auto &[url, rank] : temp_json.items()) {
        pagerankmap[url] = rank;
    }
}

void read_tfidf(HashMap<std::string, std::vector<std::pair<std::string, double>>> &tfidfmap) {
    json temp_json;
    
    std::ifstream input_file("../jsonFiles/tfidf_output.json");
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open TF-IDF file\n";
        return;
    }

    input_file >> temp_json;

    for (const auto &[key, value] : temp_json.items()) {
        std::vector<std::pair<std::string, double>> vec;
        for (auto &item : value) {
            vec.emplace_back(item["url"], item["tfidf"]);
        }
        tfidfmap[key] = vec;
    }
}

std::vector<std::string> split_query(std::string &query) {
    std::vector<std::string> words;
    std::istringstream ss(query);
    std::string word;

    pybind11::gil_scoped_acquire acquire;
    while (ss >> word) {
        words.push_back(lemmatize_word(word).cast<std::string>());
    }
    query.clear();
    
    for (const auto &keyword : words) {
        query += " " + keyword;
    }

    return words;
}

HashMap<std::string, double> cosine_similarity(std::string &query, \
    HashMap<std::string, std::vector<std::pair<std::string, double>>> &tfidfmap) {
        pybind11::gil_scoped_acquire acquire;
        std::vector<std::string> query_terms = split_query(query);

        std::unordered_map<std::string, double> query_vector;
        for (const auto &term : query_terms) {
            query_vector[term] += 1.0;
        }

        double query_magnitude = 0;
        for (const auto &[term, frequency] : query_vector) {
            query_magnitude += frequency * frequency;
        }
        query_magnitude = std::sqrt(query_magnitude);

        for (auto &[term, frequency] : query_vector) {
            frequency /= query_magnitude;
        }

        HashMap<std::string, double> doc_vector_magnitudes;
        HashMap<std::string, double> dot_products;

        for (const auto &[term, query_weight] : query_vector) {
            if (tfidfmap.count(term) > 0) {
                for (const auto &[doc_id, tfidf_value] : tfidfmap.at(term)) {
                    dot_products[doc_id] += query_weight * tfidf_value;
                    doc_vector_magnitudes[doc_id] += tfidf_value * tfidf_value;
                }
            }
        }

        HashMap<std::string, double> cosine_similarities;
        for (const auto &[doc_id, dot_product] : dot_products) {
            double doc_magnitude = std::sqrt(doc_vector_magnitudes[doc_id]);
            if (doc_magnitude > 0)
                cosine_similarities[doc_id] = dot_product / doc_magnitude;
        }

        return cosine_similarities;
}

std::vector<std::pair<std::string, double>> get_results(HashMap<std::string, double> sim, HashMap<std::string, double> &pagerankmap) {
    std::vector<std::pair<std::string, double>> results;
    for (const auto &[url, cs] : sim) {
        results.push_back({url, cs});
    }

    //{{url1, cs1}, {url2, cs2}, ...}
    for (auto &[url, cs] : results) {
        double pageranking = *pagerankmap.find(url);
        if (!pageranking) continue;
        cs = 0.7 * cs + 0.3 * (pageranking);
    }

    return results;
}

std::vector<std::string> order_results(std::vector<std::pair<std::string, double>> unordered_results) {
    // Sort the vector in descending order based on the double value in the pair
    std::sort(unordered_results.begin(), unordered_results.end(), 
              [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                  return a.second > b.second; // Compare the second element of the pairs
              });
    
    // Create a vector to store the ordered strings
    std::vector<std::string> ordered_strings;
    ordered_strings.reserve(unordered_results.size()); // Reserve space for efficiency

    // Extract the strings from the sorted pairs
    for (const auto& pair : unordered_results) {
        ordered_strings.push_back(pair.first);
    }

    return ordered_strings;
}

// Callback function for writing data received by libcurl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to extract the title from the HTML
std::string extractTitle(const std::string& html) {
    std::regex titleRegex("<title>(.*?)</title>", std::regex::icase);
    std::smatch match;
    if (std::regex_search(html, match, titleRegex)) {
        return match[1].str();
    }
    return "";
}

// Function to extract the meta description from the HTML
std::string extractMetaDescription(const std::string& html) {
    std::regex metaRegex("<meta[^>]*name=\"description\"[^>]*content=\"(.*?)\"[^>]*>", std::regex::icase);
    std::smatch match;
    if (std::regex_search(html, match, metaRegex)) {
        return match[1].str();
    }
    return "";
}

HashMap<std::string, std::pair<std::string, std::string>> get_title_and_desc(std::vector<std::string> &result) {
    HashMap<std::string, std::pair<std::string, std::string>> res;

    CURL *curl = curl_easy_init();
    CURLcode resp;

    for (const auto &url : result) {
        std::string html_content, title, description;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html_content);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

        resp = curl_easy_perform(curl);

        title = extractTitle(html_content);
        description = extractMetaDescription(html_content);

        res[url] = {title, description};
    }

    curl_easy_cleanup(curl);
    return res;
}

int main() {
    crow::App<crow::CORSHandler> app;
    
    HashMap<std::string, std::vector<std::pair<std::string, double>>> tfidfmap;
    read_tfidf(tfidfmap);

    HashMap<std::string, double> pagerankmap;
    read_pagerank(pagerankmap);

    CROW_ROUTE(app, "/search").methods("POST"_method)([&](const crow::request &req) {
        auto body = json::parse(req.body);
        std::string query = body["query"];
        std::transform(query.begin(), query.end(), query.begin(), [](char c) {
            return tolower(c);
        });

        HashMap<std::string, double> sim = cosine_similarity(query, tfidfmap);
        auto results = get_results(sim, pagerankmap);

        std::vector<std::string> final_result = order_results(results);
        HashMap<std::string, std::pair<std::string, std::string>> full_result = get_title_and_desc(final_result);

        json response;
        for (const auto &url : final_result) {
            response.push_back({{"title", full_result.find(url)->first}, {"URL", url}, {"description", full_result.find(url)->second}});
        }

        return crow::response(response.dump());
    });

    pybind11::gil_scoped_release release;
    app.port(1337).multithreaded().run();
    pybind11::gil_scoped_acquire acquire;

    return 0;
}
