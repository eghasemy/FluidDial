# FluidDial Transport Configuration Examples

## WebSocket Configuration (transport.json)
```json
{
  "type": "websocket",
  "host": "192.168.1.100",
  "port": 81
}
```

## Telnet Configuration (transport.json)
```json
{
  "type": "telnet",
  "host": "192.168.1.100", 
  "port": 23
}
```

## Configuration File Location
Place the `transport.json` file in the root directory of the LittleFS filesystem.

## Default Values
If no configuration file is present, the system will use:
- Transport Type: WebSocket
- Host: 192.168.1.100
- Port: 81 (WebSocket) or 23 (Telnet)

## Transport Features
- **Auto-reconnect**: Both transports will automatically reconnect with exponential backoff (1.5s → 5s max)
- **Non-blocking**: All operations are non-blocking and suitable for real-time use
- **Protocol compliance**: sendLine() appends single newline, sendRT() sends single byte
- **Jog-cancel support**: Real-time command 0x85 is reliably transmitted