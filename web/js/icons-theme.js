// icon updater but for the contact page
// i prob should make a universal icon updater but nah... to lazy
(function () {
  function getTheme() {
    return document.documentElement.getAttribute("data-theme") || "light";
  }

  // default is light btw
  function themedIcon(name) {
    const theme = getTheme();
    return `./images/icons/${theme}/${name}.svg`;
  }

  // updates and fks the website
  function updateAllThemeIcons() {
    // .theme-icon-dynamic used only in contact thing at the side
    const icons = document.querySelectorAll(".theme-icon-dynamic");
    icons.forEach((icon) => {
      const iconName = icon.getAttribute("data-icon");
      if (iconName) {
        icon.src = themedIcon(iconName);
      }
    });
  }

  // Immediately update icons on script load (before DOMContentLoaded)
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", updateAllThemeIcons);
  } else {
    // DOM is already ready
    updateAllThemeIcons();
    //
  }

  // prob prevent a crash
  setTimeout(updateAllThemeIcons, 100);

  // update
  document.addEventListener("themeChanged", function () {
    updateAllThemeIcons();
  });
  window.addEventListener("storage", function (e) {
    if (e.key === "theme") {
      updateAllThemeIcons();
    }
  });
})();
