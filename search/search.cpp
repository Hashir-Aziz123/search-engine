/*
g++ search.cpp -o search -L/usr/lib/x86_64-linux-gnu -I/usr/include/python3.10 -I../includes -lpython3.10 -g
*/

#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cmath>
#include <pybind11/embed.h>
#include <nlohmann/json.hpp>
#include "structures/hashmap.hpp"

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

int main() {
    HashMap<std::string, std::vector<std::pair<std::string, double>>> tfidfmap;
    read_tfidf(tfidfmap);

    std::string query;
    
    std::cout << "Enter query: ";
    std::getline(std::cin, query);

    std::transform(query.begin(), query.end(), query.begin(), [](char c) {
        return tolower(c);
    });

    // pybind11::gil_scoped_acquire acquire;
    // pybind11::object lemmatized_query = lemmatize_word(query);
    // query = lemmatized_query.cast<std::string>();

    HashMap<std::string, double> sim = cosine_similarity(query, tfidfmap);

    std::cout << "Query:" << query << '\n';

    std::cout << "COSINE SIMILARITY:\n";
    for (const auto &[doc, cs] : sim) {
        std::cout << doc << ": " << cs << '\n';
    }

    HashMap<std::string, double> pagerankmap;
    read_pagerank(pagerankmap);

    std::cout << "\nPAGE RANK:\n";
    for (const auto &[url, rank] : pagerankmap) {
        std::cout << url << ": " << rank << '\n';
    }

    std::cout << "\nRESULTS:\n";
    std::vector<std::pair<std::string, double>> results = get_results(sim, pagerankmap);
    for (const auto &[url, rank] : results) {
        std::cout << url << ": " << rank << '\n';
    }

    return 0;
}