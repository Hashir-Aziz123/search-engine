# Search Engine Project

A modular search engine that crawls, indexes, and retrieves web data efficiently. Developed in **C++** for high-performance backend processing and paired with a **JavaScript frontend** for a smooth user experience.

---

## Project Structure
search-engine/

├── crawler/                 # Crawls target domains and extracts page content
├── indexer/                 # Tokenizes content, builds inverted index
├── search/                  # Processes queries and returns ranked results
├── frontend/                # User interface for searching and displaying results
├── includes/structures/     # Shared C++ structures (Document, InvertedIndex, etc.)

## Key Features

- BFS crawling with URL normalization
- Tokenization, stop-word filtering, and stemming 
- Inverted index for fast lookup
- CLI and JS-based frontend search
- Modular design: crawler, indexer, search, and UI are decoupled
- Built for speed using C++ memory and data structures

## How It Works

1. **Crawler**
   - Performs BFS crawling on a website 
   - Extracts meaningful text from HTML (headings, paragraphs, etc.)
   - 
2. **Indexer**
   - Tokenizes and normalizes content
   - Builds an inverted index mapping words to document references
   - Saves index data for future queries
   - 
3. **Search Engine**
   - Loads index data
   - Accepts user queries via CLI or frontend
   - Retrieves and ranks documents using term frequency scoring

4. **Frontend**
   - Simple JS/HTML interface to input queries
   - Sends queries to backend or local index
   - Displays ranked results with snippets and links
     
