import { createBrowserRouter, RouterProvider } from "react-router-dom";
import { useState } from "react";
import { Search } from "./Search";
import Results from "./Results";
import Applayout from "../ui/Applayout";
import "./index.css";
import WeatherWidget from "./WeatherWidget";

function App() {
  const [query, setQuery] = useState("");
  const router = createBrowserRouter([
    {
      element: <Applayout />,
      children: [
        {
          path: "/",
          element: <Search query={query} setQuery={setQuery} />,
        },
        {
          path: "/results",
          element: <Results query={query} setQuery={setQuery} />,
        },
        {
          path: "/weather",
          element: <WeatherWidget />,
        },
      ],
    },
  ]);

  return <RouterProvider router={router} />;
}

export default App;
