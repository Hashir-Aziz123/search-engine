import { Outlet } from "react-router-dom";
import Footer from "./Footer";
import Header from "../src/Header";

function Applayout() {
  return (
    <div className="grid h-screen grid-rows-[auto_1fr_auto]">
      {/* Header */}
      <Header />

      {/* Main Content */}
      <main className="overflow-auto">
        <Outlet />
      </main>
    </div>
  );
}

export default Applayout;
