import React, { useEffect, useState } from "react";
import { Link, useLocation, useNavigate } from "react-router-dom";
import FoundResults from "./FoundResults";

function Results({ query, setQuery }) {
  const location = useLocation();
  const navigate = useNavigate();
  const searchQuery = new URLSearchParams(location.search).get("query");
  const [resultsQuery, setResultsQuery] = useState("");

  useEffect(() => {
    if (searchQuery) {
      setQuery(searchQuery);
      setResultsQuery(searchQuery);
    }
  }, [searchQuery, setQuery]);

  const handleSearch = (event) => {
    event.preventDefault();
    setResultsQuery(query); // Trigger the search with the current input value
    navigate(`/results?query=${query}`);
  };

  return (
    <div>
      <div className="flex items-center justify-center space-x-0 p-2 pb-1">
        <Link to="/">
          <img
            src="/public/pic4.png"
            alt="Astronaut"
            className="h-32 w-32 pr-0"
          />
        </Link>
        <form className="w-full max-w-md pl-0" onSubmit={handleSearch}>
          <div className="relative">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              fill="none"
              viewBox="0 0 24 24"
              strokeWidth="1.5"
              stroke="currentColor"
              className="absolute left-3 top-4 h-5 w-5 text-gray-400"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z"
              />
            </svg>
            <input
              type="text"
              placeholder="Search Here"
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              className="w-full rounded-lg border border-gray-300 py-3 pl-10 pr-20 focus:outline-none"
            />
            <button
              type="submit"
              className="absolute right-2 top-2 rounded-md bg-blue-800 px-3 py-1 align-middle text-white hover:bg-blue-600 focus:outline-none"
            >
              Search
            </button>
          </div>
        </form>
      </div>
      <div className="text-center">
        <FoundResults query={resultsQuery} />
      </div>
    </div>
  );
}

export default Results;
