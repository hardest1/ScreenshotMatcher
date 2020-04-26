class Timer {
  constructor(name = 'n/a'){
    this.name = name
    this.startTime = 0
    this.time = 0
    this.interval = 0
  }
  start(){
    this.startTime = Date.now()
    this.interval = setInterval(() => {
      this.time = Date.now() - this.startTime
    }, 20);
  }
  stop(){
    clearInterval(this.interval)
  }

}

class TestSuite {

  constructor(){
      
    this.btnStartTest = $('#btnStartTest')
    this.ulTimes = $('#ulTimes')
    this.logEl = $('#log')

    this.btnStartTest.on('click', () => this.startTest())

  }

  writeLog(message){
    let newMsg = $('<p/>').html((new Date().toLocaleTimeString()) + ': <b>' + message + '</b>')
    this.logEl.append(newMsg)
  }

  startTest(){
    this.writeLog('Starting test')

    let timer = new Timer('Main Timer')
    timer.start()

    setTimeout(() => {
      timer.stop()
      this.writeLog(timer.time)
    }, 2000);
  }
}
