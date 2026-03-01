const params = new URLSearchParams(window.location.search);
const tag = params.get("tag");
const titleEl = document.getElementById("releaseTitle");
const metaEl = document.getElementById("releaseMeta");
const contentEl = document.getElementById("releaseContent");

const REPO = "emexos/emexOS1";
const RELEASES_API = `https://api.github.com/repos/${REPO}/releases`;

if (!tag) {
  titleEl.innerText = "No release selected";
} else {
  fetch(RELEASES_API)
    .then((r) => r.json())
    .then((releases) => {
      const release = releases.find((r) => r.tag_name === tag);

      if (!release) {
        titleEl.innerText = "Release not found";
        return;
      }

      titleEl.innerText = release.tag_name;
      metaEl.innerText = release.name || "";

      // if exist then use
      if (release.body && release.body.trim().length > 0) {
        renderMarkdown(release.body, release.target_commitish);
      }
      /// kiad readme
      else {
        loadReadme(release.target_commitish);
      }
    })
    .catch((err) => {
      console.error(err);
      titleEl.innerText = "Error loading release";
    });
}

function loadReadme(branch) {
  const readmeUrl = `https://raw.githubusercontent.com/${REPO}/${branch}/README.md`;

  fetch(readmeUrl)
    .then((r) => {
      if (!r.ok) throw new Error("nothing found :( ");
      return r.text();
    })
    .then((md) => renderMarkdown(md, branch))
    .catch((err) => {
      console.error(err);
      contentEl.innerText = "couldnt find a README";
    });
}

function renderMarkdown(md, branch) {
  console.log("Original markdown:", md.substring(0, 500));

  /// relative images
  let fixedMd = md;

  // Handle images with paths starting with ./ or just filename
  fixedMd = fixedMd.replace(
    /!\[([^\]]*)\]\((?!https?:\/\/)([^)]+)\)/g,
    (match, alt, path) => {
      console.log("Found image:", path); // Debug log

      // remove leading ./ or /
      const clean = path.replace(/^\.?\//, "").trim();
      const fullUrl = `https://raw.githubusercontent.com/${REPO}/${branch}/${clean}`;

      console.log("converted to:", fullUrl);

      return `![${alt}](${fullUrl})`;
    },
  );

  // also handle HTML img tags if present
  fixedMd = fixedMd.replace(
    /<img\s+([^>]*?)src=["'](?!https?:\/\/)([^"']+)["']([^>]*?)>/gi,
    (match, before, path, after) => {
      const clean = path.replace(/^\.?\//, "").trim();
      const fullUrl = `https://raw.githubusercontent.com/${REPO}/${branch}/${clean}`;
      return `<img ${before}src="${fullUrl}"${after}>`;
    },
  );

  // convert blob URL's to raw URL's
  fixedMd = fixedMd.replace(
    /<img\s+([^>]*?)src=["'](https:\/\/github\.com\/[^\/]+\/[^\/]+\/blob\/[^\/]+\/)([^"']+)["']([^>]*?)>/gi,
    (match, before, blobUrl, path, after) => {
      const rawUrl = blobUrl
        .replace("https://github.com/", "https://raw.githubusercontent.com/")
        .replace("/blob/", "/");
      const fullUrl = rawUrl + path;
      console.log("Converted GitHub blob URL to:", fullUrl);
      return `<img ${before}src="${fullUrl}"${after}>`;
    },
  );

  // markdown images
  fixedMd = fixedMd.replace(
    /!\[([^\]]*)\]\((https:\/\/github\.com\/[^\/]+\/[^\/]+\/blob\/[^\/]+\/)([^)]+)\)/g,
    (match, alt, blobUrl, path) => {
      const rawUrl = blobUrl
        .replace("https://github.com/", "https://raw.githubusercontent.com/")
        .replace("/blob/", "/");
      const fullUrl = rawUrl + path;
      console.log("Converted GitHub blob URL to:", fullUrl);
      return `![${alt}](${fullUrl})`;
    },
  );

  console.log("Fixed markdown:", fixedMd.substring(0, 500));

  const html = marked.parse(fixedMd);
  const safe = DOMPurify.sanitize(html);

  contentEl.innerHTML = safe;

  /// opens in a new tab
  contentEl.querySelectorAll("a").forEach((a) => {
    a.target = "_blank";
    a.rel = "noopener";
  });

  // add error handling for images
  contentEl.querySelectorAll("img").forEach((img) => {
    img.addEventListener("error", function () {
      console.error("Failed to load image:", this.src);
      this.alt = `[Image failed to load: ${this.alt || "Screenshot"}]`;
      this.style.border = "2px dashed #ccc";
      this.style.padding = "20px";
      this.style.background = "#f5f5f5";
    });

    img.addEventListener("load", function () {
      console.log("Successfully loaded image:", this.src);
    });
  });

  // copy button for codeblocks
  contentEl.querySelectorAll("pre").forEach((pre) => {
    const button = document.createElement("button");
    button.className = "code-copy-button";
    button.innerHTML = `
      <img class="theme-icon-dynamic" data-icon="copy" src="./images/icons/${getTheme()}/copy.svg" alt="Copy code" />
    `;

    button.addEventListener("click", async function () {
      const code = pre.querySelector("code");
      const text = code ? code.textContent : pre.textContent;

      try {
        await navigator.clipboard.writeText(text);
        button.classList.add("copied");

        setTimeout(() => {
          button.classList.remove("copied");
        }, 1000);
      } catch (err) {
        console.error("failed to copy:", err);
      }
    });

    pre.style.position = "relative";
    pre.appendChild(button);
  });
}

function getTheme() {
  return document.documentElement.getAttribute("data-theme") || "light";
}

// changes the icon when theme is changed
document.addEventListener("themeChanged", function () {
  const copyIcons = document.querySelectorAll(
    ".code-copy-button .theme-icon-dynamic",
  );
  copyIcons.forEach((icon) => {
    icon.src = `./images/icons/${getTheme()}/copy.svg`;
  });
});
