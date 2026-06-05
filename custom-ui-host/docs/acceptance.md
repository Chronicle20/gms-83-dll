# Manual acceptance log patterns

Tail the client's `OutputDebugString` capture (DbgView or similar) and
grep for these patterns to verify each PRD §10.3–10.5 criterion.

| Criterion | Pattern |
|---|---|
| 10.3.a host+demo logged "ready" | `custom-ui-host: ready` AND `custom-ui-demo: window built, ready` |
| 10.3.b F8 toggles window | (visual) window appears/disappears on rising F8 edge |
| 10.3.c "Ping" send | `custom-ui-demo: OnPing -> SendPacket(0x0F00)` |
| 10.3.d label updates | `custom-ui-demo: OnPong opcode=0x2000 seq=N` and visible "Server says: pong N" |
| 10.4.a host absent | `custom-ui-demo: host DLL not loaded -- demo inert` |
| 10.4.b double-load | `custom-ui-host: another host instance is already running` |
| 10.4.c reserved vk rejected | `custom-ui-host: BindHotkey rejected -- vk=0xXX already mapped` |
| 10.4.d out-of-range register | `custom-ui-host: RegisterPacketHandler called …` + return 0 |
| 10.4.e out-of-range send | `custom-ui-host: SendPacket rejected -- opcode 0xXXXX outside outbound range` |
| 10.4.f AV in callback | `custom-ui-host: AV in consumer callback at site=[CustomUI button click]` |
| 10.5.a channel change | host log shows hide+show across stage transition (visual) |
| 10.5.c disconnect+reconnect | host re-installs cleanly (no double-hook crash log) |
