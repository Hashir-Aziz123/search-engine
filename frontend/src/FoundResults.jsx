import React, { useEffect, useState } from "react";
import Loader from "./Loader";
import { Link } from "react-router-dom";

const data = [
  {
    title: "Title 1",
    URL: "https://example.com/title-1",
    description:
      "Pakistan, located in South Asia, is a country rich in cultural diversity, history, and natural beauty. It shares borders with India to the east, Afghanistan and Iran to the west, China to the north, and the Arabian Sea to the south. Established in 1947 as a homeland for Muslims of the Indian subcontinent, Pakistan has a complex history shaped by ancient civilizations like the Indus Valley, colonial influences, and its struggle for independence. The country is known for its breathtaking landscapes, ranging from the towering peaks of the Karakoram and Himalayas to the serene deserts of Sindh and Balochistan. Pakistan's vibrant culture is a blend of traditions, languages, and cuisines, reflecting its diverse population. Despite challenges, including economic development and political stability, Pakistan remains a resilient nation with immense potential, known for its warm hospitality and rich contributions to arts, sports, and science",
  },
  {
    title: "Title 2",
    URL: "https://example.com/title-2",
    description: "Description for title 2",
  },
  {
    title: "Title 3",
    URL: "https://example.com/title-3",
    description: "Description for title 3",
  },
];

function FoundResults({ query }) {
  const [results, setResults] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const fetchResults = async () => {
      setLoading(true);
      try {
        const response = await fetch(
          `https://api.example.com/search?q=${query}`,
        );
        const data = await response.json();
        setResults(data.results);
      } catch (error) {
        console.error("Error fetching data:", error);
      } finally {
        setLoading(false);
      }
    };

    if (query) {
      fetchResults();
    }
  }, [query]);

  return (
    <div>
      {loading ? (
        <Loader />
      ) : (
        <div>
          <p className="text-xl font-semibold">
            Found <span className="font-extrabold">{results.length}</span>{" "}
            results for{" "}
            <span className="font-extrabold">&quot;{query}&quot;</span>
          </p>
          <ul>
            {data.map((data, index) => (
              <div
                key={index}
                className="mx-52 my-4 rounded-lg bg-gray-100 py-2"
              >
                <a href={data.URL} target="_blank" rel="noreferrer">
                  <h1 className="px-6 !text-left text-4xl underline">
                    {data.title}
                  </h1>
                  <p className="line-clamp-3 !px-6 py-2 text-left text-lg">
                    {data.description}
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
