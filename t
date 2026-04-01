after sync_home_to_current runs; the home_cx/cy is never updated to the new window position means in the next frame it will try to copy again from the same old home coords to the new current position.


so now home_cx/cy is already BG-filled (from the clear step), so it copies BG over the content
ig i can fix it now