import { useEffect, useState } from "react";
import WeatherWidget from "./WeatherWidget";

function Header() {
  const [dateTime, setDateTime] = useState({
    day: "Tuesday",
    time: "12:00 PM",
  });

  useEffect(() => {
    const updateDateTime = () => {
      const currentDate = new Date();

      // Get the day of the week
      const daysOfWeek = [
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
      ];
      const day = daysOfWeek[currentDate.getDay()];

      // Format the time
      const time = currentDate.toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
        hour12: true,
      });

      setDateTime({ day, time });
    };

    updateDateTime(); // Initial call to set date and time immediately

    // Update the date and time every minute
    const interval = setInterval(updateDateTime, 60000);

    // Cleanup the interval on component unmount
    return () => clearInterval(interval);
  }, []);

  return (
    <header className="flex justify-between">
      <WeatherWidget />
      <p className="py-4 pr-10 text-center align-middle font-mono text-2xl font-semibold">
        {dateTime.day.toUpperCase()} {dateTime.time}
      </p>
    </header>
  );
}

export default Header;
