import config from '../../config.json' assert { type: 'json' };

var ws
let plr = {}
let messages = []
let MessageCounter = 0
let pickedNumber = -1
let guessedNumber = -2

plr.local = {}
plr.remote = {}

function log(text){
  document.getElementById("textarea").value = text + document.getElementById("textarea").value
}


function ws_connect() {

  ws = new WebSocket('ws://' + config.client.ws_addr + ':' + config.ws.port);

  ws.onopen = () => {
    document.getElementById('status').textContent = 'Tak';
    console.log('Connected');
    log('Połączyłeś się\n\n')
  };
  
  ws.onmessage = (event) => {
    let data = event.data.split('-')
    if(data.includes('exit') || data.includes('left')){
      ws.close()
      log('Przeciwnik wychodzi z gry\n\n')
    } else if(data.includes('full')){
      console.log('Server is full!')
      ws.close()
    }
    if(data.includes('pickedNumber')){
      pickedNumber = data[1]
      document.getElementById('guess').textContent = '???';
      document.getElementById('input').disabled = false
      document.getElementById('button').disabled = false
      log('Przeciwnik wybrał liczbę\n\n')
    }
    if(data.includes('guessedNumber')){
      guessedNumber = data[1]
      MessageCounter += 1
      document.getElementById('trials').textContent = MessageCounter
      log('Przeciwnik próbuję zgadnąc liczbę ' + guessedNumber + '\n\n')
    }
    if(data.includes('win')){
      document.getElementById('win').textContent = 'Tak'
      document.getElementById('guess').textContent = pickedNumber
      document.getElementById('input').disabled = true
      document.getElementById('button').disabled = true
      log('Zgadujesz liczbę\n\n')
      log('Koniec gry\n\n')
    }
    if(pickedNumber == guessedNumber) {
      log('Przeciwnik zgaduje liczbę\n\n')
      log('Koniec gry\n\n')
      ws.send('win')
      document.getElementById('win').textContent = 'Tak'
    }
    if(messages.length == 0) {
      if (data[0] == "0" || data[0] == "1") {
        plr.local.id = data[0]
        plr.remote.id = plr.local.id == "0" ? "1" : "0"
        document.getElementById('id').textContent = plr.local.id
        log('Przydzielono ci identyfikator i rolę\n\n')
      } else {
        ws_reconnect()
      }
    }
    if (data.includes('ready')){
      log('Wszyscy są na serwerze, zmieniono status gry na gotową\n\n')
      document.getElementById('guess').textContent = plr.local.id == '1' ? 'Wybierz liczbę' : 'Przeciwnik wybiera liczbę'
      document.getElementById('ready').textContent = 'Tak';
      if (plr.local.id == 0) {
        document.getElementById('input').disabled = true
        document.getElementById('button').disabled = true
        log('Oczekiwanie na oponenta na wybranie liczby\n\n')
      } else {
        log('Wybierz liczbę\n\n')
        document.getElementById('input').disabled = false
        document.getElementById('button').disabled = false
      }
    }
    messages.push(event.data)
    console.log('Server:', event.data);

    if (plr.local.id == 0){
      document.getElementById('role').textContent = 'Zgadnij liczbę'
    } else {
      document.getElementById('role').textContent = 'Wybierz liczbę'
    }
  };
  
  ws.onclose = () => {
    log('Rozłączono\n\n')
    document.getElementById('input').disabled = true
    document.getElementById('button').disabled = true
    pickedNumber = -1
    guessedNumber = -2
    document.getElementById('id').textContent = 'Nieznany';
    document.getElementById('status').textContent = 'Nie';
    document.getElementById('ready').textContent = 'Nie';
    document.getElementById('role').textContent = 'Nieznana'
    console.log('Disconnected');
  };

}
ws_connect()

function ws_reconnect() {
  document.getElementById("textarea").value = ''
  log('Łączenie ponownie...\n\n')
  ws.send('exit')
  ws.close()
  console.log(messages)
  console.log(plr)
  messages = []
  ws_connect()
}

function ws_exit() {
  document.getElementById("textarea").value = ''
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
    log('Wybierasz liczbę ' + pickedNumber + '\n\n')
    document.getElementById('input').disabled = true
    document.getElementById('button').disabled = true
  } else {
    MessageCounter += 1
    document.getElementById('trials').textContent = MessageCounter
    log('Próbujesz zgadnąć liczbę ' + document.querySelector('input').value + '\n\n')
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