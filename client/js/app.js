import config from '../../config.json' assert { type: 'json' };

var ws
let plr = {}
let messages = []
let sentMessageCounter = 0
let pickedNumber = 0

plr.local = {}
plr.remote = {}


function ws_connect() {

  ws = new WebSocket('ws://' + config.client.ws_addr + ':' + config.ws.port);

  ws.onopen = () => {
    document.getElementById('status').textContent = 'yes';
    console.log('Connected');
  };
  
  ws.onmessage = (event) => {
    let data = event.data.split('-')
    if(data.includes('exit') || data.includes('left')){
      ws.close()
    } else if(data.includes('full')){
      console.log('Server is full!')
      ws.close()
    }
    if(data.includes('pickedNumber')){
      pickedNumber = data[1]
      document.getElementById('guess').textContent = '???';
    }
    if(messages.length == 0) {
      if (data[0] == "0" || data[0] == "1") {
        plr.local.id = data[0]
        plr.remote.id = plr.local.id == "0" ? "1" : "0"
        document.getElementById('id').textContent = plr.local.id
      } else {
        ws_reconnect()
      }
    }
    if (data.includes('ready')){
      document.getElementById('guess').textContent = plr.local.id == '1' ? 'pick a number' : 'opponent is picking a number...'
      document.getElementById('ready').textContent = 'yes';
    }
    messages.push(event.data)
    console.log('Server:', event.data);

    if (plr.local.id == 0){
      document.getElementById('role').textContent = 'guess'
    } else {
      document.getElementById('role').textContent = 'pick a number'
    }
  };
  
  ws.onclose = () => {
    document.getElementById('id').textContent = 'unknown';
    document.getElementById('status').textContent = 'no';
    document.getElementById('ready').textContent = 'no';
    console.log('Disconnected');
  };

}
ws_connect()

function ws_reconnect() {
  ws.send('exit')
  ws.close()
  console.log(messages)
  console.log(plr)
  messages = []
  ws_connect()
}

function ws_exit() {
  ws.send('exit')
  ws.close()
  console.log(messages)
  console.log(plr)
  messages = []
}

document.querySelector('form').addEventListener('submit', (event) => {
  event.preventDefault();
  let message
  if (plr.local.id == "1") {
    message = 'pickedNumber-' + document.querySelector('input').value
    pickedNumber = document.querySelector('input').value
    document.getElementById('guess').textContent = pickedNumber
  } else {
    message = 'guessedNumber-' + document.querySelector('input').value
  }
  ws.send(message)
});

document.querySelector('#reconnect').addEventListener('click', (event) => {
  console.log('Reconnecting...')
  ws_reconnect()
})

document.querySelector('#exit').addEventListener('click', (event) => {
  console.log('Disconnecting...')
  ws_exit()
})