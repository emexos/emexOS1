(function () {
  const currentTheme = localStorage.getItem("theme") || "light";

  // prefenvt flash
  document.documentElement.setAttribute("data-theme", currentTheme);

  document.addEventListener("DOMContentLoaded", function () {
    const toggleButton = document.getElementById("themeToggle");
    if (!toggleButton) return;

    const iconContainer = toggleButton.querySelector(".theme-icon img");
    const logo = document.getElementById("logo");

    // iniitals
    updateIcon(currentTheme);
    updateLogo(currentTheme);

    toggleButton.addEventListener("click", function () {
      const currentTheme = document.documentElement.getAttribute("data-theme");
      const newTheme = currentTheme === "dark" ? "light" : "dark";

      document.documentElement.setAttribute("data-theme", newTheme);
      localStorage.setItem("theme", newTheme);

      updateIcon(newTheme);
      updateLogo(newTheme);

      // dispatch custom event for other scripts to listen to
      document.dispatchEvent(
        new CustomEvent("themeChanged", { detail: { theme: newTheme } }),
      );
    });

    function updateIcon(theme) {
      if (!iconContainer) return;
      if (theme === "dark") {
        iconContainer.src = "./images/icons/dark/sun.svg";
        iconContainer.alt = "Light Theme";
      } else {
        iconContainer.src = "./images/icons/light/moon.svg";
        iconContainer.alt = "Dark Theme";
      }
    }

    function updateLogo(theme) {
      if (!logo) return;
      if (theme === "light") {
        logo.src = "images/logo.png";
        logo.alt = "emexOS Logo Black";
      } else {
        logo.src = "images/logo.png";
        logo.alt = "emexOS Logo White";
      }
    }
  });
})();
