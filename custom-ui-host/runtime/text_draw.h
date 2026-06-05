#pragma once

namespace custom_ui_host {

// Creates the shared label font (Arial, size 12, ARGB 0xFFFFFFFF white) once
// via PcCreateObject::IWzFont + IWzFont::Create and caches the raw IWzFont* in
// a file-scope global. Returns false on failure (e.g. font factory unavailable);
// a false return leaves label rendering disabled but is non-fatal for the host.
bool InitLabelFont();

// Draws a single UTF-8 label inside a CUIWnd Draw override. `cuiwnd_self` is the
// raw game-side CUIWnd `this`. (x,y) are window-relative coordinates. No-op when
// the label font failed to initialise. Acquires the window canvas (owned ref),
// draws via IWzCanvas::DrawTextA, then releases the canvas. Must only be called
// from within the slot-11 Draw override (the canvas is valid there).
void DrawLabel(void *cuiwnd_self, int x, int y, const char *utf8);

}  // namespace custom_ui_host
