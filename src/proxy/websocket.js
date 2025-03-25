import config from '../../config.json' assert { type: 'json' };
import WebSocket, { WebSocketServer } from 'ws';
import net from 'net';

const socket = new WebSocketServer({ port: config.ws.port });

socket.on('connection', (ws) => {
  console.log('Someone just connected to websocket!');

  const client = new net.Socket();

  client.connect(config.ws.connection.port, config.ws.connection.ip, () => {
    console.log('Websocket connected to socket!');
  });

  client.on('data', (data) => {
    ws.send(data.toString());
    console.log(data.toString())
  });

  client.on('close', () => {
    ws.send('exit')
    console.log('Telnet connection closed');
  });

  ws.on('message', (message) => {
    client.write(message);
  });

  ws.on('close', () => {
    client.write('exit')
    client.end();
  });
});

console.log('Proxy listening on ' + config.ws.port);