import React, { useEffect } from "react";

function WeatherWidget() {
  useEffect(() => {
    // Dynamically add the weather widget script
    const script = document.createElement("script");
    script.src = "https://weatherwidget.io/js/widget.min.js";
    script.id = "weatherwidget-io-js";
    script.async = true;
    document.body.appendChild(script);

    // Cleanup the script when the component unmounts
    return () => {
      document.body.removeChild(script);
    };
  }, []);

  return (
    <div>
      <a
        className="weatherwidget-io custom-widget-style"
        href="https://forecast7.com/en/33d6273d09/federal-capital-territory/"
        data-label_1="ISLAMABAD"
        data-icons="Climacons Animated"
        data-mode="Current"
        data-days="3"
        data-theme="pure"
      >
        ISLAMABAD
      </a>
    </div>
  );
}

export default WeatherWidget;
