/// sorry for the cursed js syntax its my code editor which randomly uses the STUPIDS SYNTAX IN THE WORLD....................

const API_URL = "https://api.github.com/repos/emexos/emexOS1/releases";
const tbody = document.getElementById("releaseTable");

function showError(msg) {
  tbody.innerHTML = `
<tr>
  <td colspan="3" style="padding:10px">
    ${msg}
  </td>
</tr>
    `;
}

// detect current theme
function getTheme() {
  return document.documentElement.getAttribute("data-theme") || "light";
}

// helper to get themed icon path
function themedIcon(name) {
  const theme = getTheme();
  return `./images/icons/${theme}/${name}.svg`;
}

fetch(API_URL)
  .then((resp) => {
    if (!resp.ok) throw new Error("api gives no response");
    return resp.json();
  })
  .then((releases) => {
    /// newest are at the top
    releases.sort(
      (a, b) => new Date(b.published_at) - new Date(a.published_at),
    );

    if (!Array.isArray(releases) || releases.length === 0) {
      showError("nothing found");
      return;
    }

    tbody.innerHTML = "";

    releases.forEach((release) => {
      /// search for .iso
      const iso = (release.assets || []).find(
        (a) => a.name && a.name.toLowerCase().endsWith(".iso"),
      );

      const tr = document.createElement("tr");

      /// version == tag_name
      /// name == release.name
      tr.innerHTML = `
<td style="padding: 10px; border-bottom: 1px solid #ddd">
  ${release.tag_name}
  </td>
<td style="padding: 10px; border-bottom: 1px solid #ddd">
  <!--${escapeHtml(release.name || "")}-->
  </td>
<td style="padding: 10px; border-bottom: 1px solid #ddd">
  ${
    iso
      ? `<a class="button downloadButton" href="${iso.browser_download_url}">
          <img class="theme-icon-dynamic" data-icon="download" src="${themedIcon("download")}" alt="" style="width: 20px; height: 20px; display: inline-block; vertical-align: middle; margin-right: 6px;">
          Download ISO
        </a>`
      : `<span class="placeholder">
          No ISO
        </span>`
  }
  <a class="button downloadButton" style="margin-left:8px" href="release.html?tag=${encodeURIComponent(release.tag_name)}">
    <img class="theme-icon-dynamic" data-icon="arrow-right" src="${themedIcon("arrow-right")}" alt="" style="width: 20px; height: 20px; display: inline-block; vertical-align: middle; margin-right: 6px;">
    Read more
  </a>
</td>
            `;

      tbody.appendChild(tr);
    });
  })
  .catch((err) => {
    console.error(err);
    showError("Failed to load releases");
  });

// theme changes
window.addEventListener("storage", function (e) {
  if (e.key === "theme") {
    updateAllThemeIcons();
  }
});

// manual theme changes on the same page
document.addEventListener("themeChanged", function () {
  updateAllThemeIcons();
});

function updateAllThemeIcons() {
  const icons = document.querySelectorAll(".theme-icon-dynamic");
  icons.forEach((icon) => {
    const iconName = icon.getAttribute("data-icon");
    if (iconName) {
      icon.src = themedIcon(iconName);
    }
  });
}

/// saftey against unwanted html in release.name
function escapeHtml(str) {
  if (!str) return "";
  return str.replace(/[&<>"']/g, function (m) {
    return {
      "&": "&amp;",
      "<": "&lt;",
      ">": "&gt;",
      '"': "&quot;",
      "'": "&#39;",
    }[m];
  });
}
