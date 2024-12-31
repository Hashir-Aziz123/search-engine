import React, { useEffect, useState } from "react";
import Loader from "./Loader";

const BaseUrl = "http://localhost:1337/";

function FoundResults({ query }) {
  const [results, setResults] = useState([]);
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    const fetchResults = async () => {
      if (!query) return; // Avoid fetching if query is empty
      setLoading(true);
      try {
        const response = await fetch(`${BaseUrl}search`, {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({ query }),
        });

        if (!response.ok) {
          throw new Error(`Error: ${response.status}`);
        }

        const data = await response.json();
        setResults(data || []);
      } catch (error) {
        console.error("Error fetching data:", error);
        setResults([]);
      } finally {
        setLoading(false);
      }
    };

    fetchResults();
  }, [query]); // Fetch only when query changes

  return (
    <div>
      {loading ? (
        <Loader />
      ) : !query ? (
        <p className="text-xl font-semibold">Search for results</p>
      ) : (
        <div>
          <p className="text-xl font-semibold">
            Found <span className="font-extrabold">{results.length}</span> results for
            <span className="font-extrabold"> &quot;{query}&quot;</span>
          </p>
          <ul>
            {results.map((result, index) => (
              <div
                key={index}
                className="mx-52 my-4 rounded-lg bg-gray-100 py-2"
              >
                <a href={result.URL} target="_blank" rel="noreferrer">
                  <h1 className="px-6 !text-left text-4xl underline">
                    {result.title}
                  </h1>
                  <p className="line-clamp-3 !px-6 py-2 text-left text-lg">
                    {result.description}
                  </p>
                </a>
              </div>
            ))}
          </ul>
        </div>
      )}
    </div>
  );
}

export default FoundResults;
