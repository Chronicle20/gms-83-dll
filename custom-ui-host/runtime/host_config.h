#pragma once

namespace custom_ui_host {

struct HostConfig {
    bool verbose = false;
    unsigned short inbound_op_min = 0x2000;
    unsigned short inbound_op_max = 0x20FF;
    unsigned short outbound_op_min = 0x0F00;
    unsigned short outbound_op_max = 0x0FFF;
};

extern HostConfig g_config;

// Loads `custom-ui-host.ini` if present next to the host DLL. On parse
// failure or missing file, leaves g_config at defaults and logs.
void LoadHostConfig();

} // namespace custom_ui_host
