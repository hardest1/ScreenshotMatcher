function sleep(milliseconds) {
  return new Promise(resolve => setTimeout(resolve, milliseconds));
}

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
    return this
  }
  stop(){
    clearInterval(this.interval)
  }

}

class TestSuite {

  constructor(){
      
    this.btnStartTest = $('#btnStartTest')
    this.btnClearLog = $('#btnClearLog')
    this.ulTimes = $('#ulTimes')
    this.logEl = $('#log')

    this.btnStartTest.on('click', () => this.startTest())
    this.btnClearLog.on('click', () => this.clearLog())

  }

  clearLog(){
    this.logEl.html("")
  }

  writeLog(...message){
    let newMsg = $('<p/>').html((new Date().toLocaleTimeString()) + ': <b>' + message.join(' ') + '</b>').addClass("uk-margin-remove")
    this.logEl.append(newMsg)
  }

  async startTest(){

    const timer = new Timer('Main Timer').start()

    this.writeLog('Starting tests')

    await this.doTest(1, 'Test 1', 'SURF-Algorithm with Object-heavy image')
    await this.doTest(2, 'Test 2', 'SURF-Algorithm with Text-heavy image')
    await this.doTest(3, 'Test 3', 'SURF-Algorithm with Highscore image')

    await this.doTest(4, 'Test 4', 'ORB with Text image')
    await sleep(1000)
    await this.doTest(5, 'Test 5', 'ORB with Text image')
    await sleep(1000)
    await this.doTest(6, 'Test 6', 'ORB with Text image')

    timer.stop()

    this.writeLog(timer.name, timer.time, "ms")

  }

  async doTest(id, name, desc = 'Test'){
    const timer = new Timer(name).start()

    this.writeLog(desc)

    const testResult = await fetch('/test?id=' + id)

    if(testResult.ok){
      const body = await testResult.text()
      this.writeLog("Result:", "<a target='_blank' href='" + body + "'>" + body + "</a>")
    }
    else{
      this.writeLog("Error")
    }

    timer.stop()

    this.writeLog(timer.time, "ms")
  }
}
