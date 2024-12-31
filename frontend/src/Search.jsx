import React from "react";
import { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { useSearch } from "./useSearch";

function Search({ query, setQuery }) {
  // const [imageLoaded, setImageLoaded] = useState(false);

  // useEffect(() => {
  //   const img = new Image();
  //   img.src = "/public/astronaut.png";
  //   img.onload = () => setImageLoaded(true);
  // }, []);

  const navigate = useNavigate();

  const handleSearch = (event) => {
    event.preventDefault();
    navigate(`/results?query=${query}`);
  };
  return (
    <div className="text-center">
      <form className="mx-auto max-w-md">
        <h1 className="text-center font-mono text-5xl font-bold">
          Hello Wanderer!
        </h1>
        {/* {imageLoaded && ( */}
        <img
          src="/public/astronaut.png" // Path to the image in the public folder
          alt="Astronaut"
          className="mx-auto mt-0 h-64 w-64" // Adjust the size and margin
        />

        <div className="w-100 relative">
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
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            placeholder="Search..."
            className="w-full rounded-lg border border-gray-300 py-3 pl-10 pr-20 focus:outline-none"
          />
          <button
            type="submit"
            onClick={handleSearch}
            className="absolute right-2 top-2 rounded-md bg-blue-800 px-3 py-1 align-middle text-white hover:bg-blue-600 focus:outline-none"
          >
            Search
          </button>
        </div>
      </form>
    </div>
  );
}

export { Search, useSearch };
