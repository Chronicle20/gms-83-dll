#pragma once

namespace custom_ui_host {

using UIThreadFn = void(__cdecl*)(void* user);

// Queue a callback to run exactly once on the game UI thread. Thread-safe;
// the queue is drained by the s_Update hook (which runs on the UI thread
// every frame once the game is rendering). Use this for any work that
// constructs/touches game UI or graphics objects (windows, controls, fonts):
// those are only safe on the UI thread with the engine initialised.
void EnqueueUIThreadTask(UIThreadFn fn, void* user);

// Run and clear all queued tasks. Call ONLY from the game UI thread
// (the s_Update hook does this). Each task runs under SEH.
void DrainUIThreadTasks();

} // namespace custom_ui_host
