(function () {
  // Creates custom cursor texture
  const cursor = document.createElement("div");
  cursor.className = "custom-cursor";
  document.body.appendChild(cursor);

  document.addEventListener("mousemove", function (e) {
    cursor.style.left = e.clientX + "px";
    cursor.style.top = e.clientY + "px";
  });
  document.addEventListener("mouseleave", function () {
    cursor.classList.add("hidden");
  });
  document.addEventListener("mouseenter", function () {
    cursor.classList.remove("hidden");
  });
})();
