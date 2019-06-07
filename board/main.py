#!/usr/bin/env python3 
#coding: utf-8

import sys, time, requests, json, ast
from threading import Thread

import serial
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QTime, QTimer, QThread
from PyQt5.QtQml import QQmlApplicationEngine

""" Platform number """
PLATFORM = 1
COMPETITION = 1

""" Ip adress """
DATABASE_IP = "giridb.herokuapp.com"

""" Serial port settings """
#SERIALPORT = '/dev/ttyS1' # usually used for rasberry/orange pi boards
SERIALPORT = '/dev/ttyUSB0'
BAUDRATE = 115200
#BAUDRATE = 9600
DELAY = 1

""" CONFIGURE RAK811 """
RAK_ENABLE = 1

""" DEBUG mode """
DEBUG = True

def log(s):
    if DEBUG:
        print (s)


class MainWindow(QObject):

    setTimer = pyqtSignal(str, arguments=['strtimer'])
    setCounter = pyqtSignal(str, arguments=['strcounter'])
    setVisible = pyqtSignal(int, arguments=['intvisible'])
    setName = pyqtSignal(str, arguments=['strname'])

    startTimer = pyqtSignal()

    def __init__(self):
        super().__init__()

        # количество подходов
        self.num_count = 0
        self.num_count_last = 0
        # статус таймера
        self.flag_timer = 0

        self.startTimer.connect(self.timer_start)

        log("[UART] Creating UART Thread")
        Thread(target = self.uart_run).start()

        log("[DB] Creating DB request Thread")
        self.req_namevisible = 1
        self.req_id_lenght = 0      # Длинна списка
        self.req_id = []            # Лист ID спортсмена
        self.req_text = []          # Лист NAME
        Thread(target = self.database_run).start()

        self.timer=QTimer(self)
        self.curr_time= QTime(0,0)

    @pyqtSlot(int)
    def counter_add(self, arg1):
        log ("[COUNTER] Added")
        if (arg1 == 1):
            self.num_count += arg1
        else: 
            self.num_count = arg1
        self.setCounter.emit(str(self.num_count))
        log (self.num_count)
        log (arg1)

    @pyqtSlot()
    def timer_reset(self):
        log ("[TIMER] Timer reseted")
        self.curr_time = QTime(0,0)
        self.num_count_last = self.num_count
        self.num_count = 0
        self.flag_timer = 0
        self.setTimer.emit(self.curr_time.toString('mm:ss'))
        self.setCounter.emit(str(self.num_count))

    @pyqtSlot()
    def timer_start(self):
        log ("[TIMER] Timer started")

        if (self.flag_timer == 0):
            self.timer.singleShot(1000,self.timer_tick)
            self.flag_timer = 1

    def timer_tick(self):
        if (self.flag_timer == 1):
            self.timer.singleShot(1000,self.timer_tick)
            self.curr_time = self.curr_time.addSecs(1)
            self.setTimer.emit(self.curr_time.toString('mm:ss'))

    @pyqtSlot()
    def timer_pause(self):
        log ("[TIMER] Timer paused")
        self.flag_timer = 0

    @pyqtSlot(int)
    def namevisible_change(self,arg1):
        log ("Name visible: {0}".format(arg1))
        self.setVisible.emit(arg1)
        #self.setVisible.emit("22")

    #@pyqtSlot(str)
    def name_change(self,string):
        log ("Name changed: {0}".format(string))
        self.setName.emit(string)

    # Serial port reading thread
    def uart_run(self):
        self.ser=serial.Serial()
        self.ser.port= SERIALPORT
        self.ser.baudrate= BAUDRATE
        self.ser.timeout=0
        self.ser.parity=serial.PARITY_NONE
        self.ser.bytesize=serial.EIGHTBITS
        self.ser.stopbits=serial.STOPBITS_ONE
        try:
            self.ser.open()
            self.ser.flushInput()
            self.ser.flushOutput()
            if (RAK_ENABLE):
                self.uart_rak_init()
            b = b''
            intcounter = 0
            while (1):
                
                #time.sleep(3)  
                b = self.uart_read()
                #log (b)
                #     at+recv=0,0,27,
                #b = "at+recv=0,0,29,494438383838383838383838383838383838434F4D31503336454E44"
                b = b.strip()
                b_lenght = len(b)
                if (b_lenght > 0):
                    log (b)
                    intcounter+=1
                    log (intcounter)
                log ("[UART] Received lenght: {0}".format(b_lenght))
                
                if (b_lenght > 3):
                    if (b[0] == 'a'):
                        # Парсим раки тут
                        # ID8888888888888888COM1P36END
                        parse_data = b[self.rak811_parse(b,',',3):]
                        log ("[UART] Received hex: {0}".format(parse_data))
                        parse_data = bytearray.fromhex(parse_data).decode()
                        log ("[UART] Received message: {0}".format(parse_data))
                        #log (parse_data[20])
                        parse_lol = self.rak811_parse(parse_data,'M',1)
                        command = parse_data[parse_lol:]
                        log ("[UART] Received command: {0}".format(command))

                        if (command[0] == "1" or command[0] == "2"):
                            log ("command 1 or 2")
                            
                            approach = ''
                            counter_data = ''
                            for approach in parse_data[23:]:
                                if (approach == 'E'):
                                    break
                                counter_data += approach
                            
                            #counter_data = parse_data[self.rak811_parse(parse_data,'E',1):]
                            log (counter_data)
                            #self.namevisible_change(self.req_namevisible)
                            self.counter_add(counter_data)
                        if (command[0] == '3'):
                            #at+txc=1,10,494438383838383838383838383838383838434f4d33503336454e44
                            self.startTimer.emit()
                        if (command[0] == '4'):
                            self.timer_reset()
                            self.database_next()
                            self.database_response()
                    """                 
                    if (b[0] == 'I'):
                        if (len(b) > 22):
                            # I807B85902000058Bffe258020200 - typical umdk board message
                            log ("[UART] Received message: {0}".format(b))
                            if (b[26] == '1'):
                                #self.namevisible_change(self.req_namevisible)
                                self.counter_add(1)
                            if (b[26] == '2'):
                                self.counter_add(-1)
                            if (b[26] == '3'):
                                self.startTimer.emit()
                            if (b[26] == '4'):
                                self.timer_reset()
                                self.database_next()
                                self.database_response()
            """
            self.ser.close()
        except Exception as e:
            log ("[UART] ERROR: {0}".format(e))
        finally:
            self.ser.close()

    def rak811_parse(self, msg, seporator, iteration):
        dot_counter = 0
        str_counter = 0
        for dot in msg:
            #log (dot)
            str_counter+=1
            #log (str_counter)
            #log (dot_counter)
            if (dot == seporator):
                dot_counter+=1
            if (dot_counter == iteration):
                break
        return str_counter

    def uart_rak_init(self):
        self.uart_write("at+mode=1")
        time.sleep(1)
        log ("[UART] Configure RAK811, set mode: {0}".format(self.uart_read()))

        self.uart_write("at+rf_config=867700000,10,0,1,8,14")
        time.sleep(0.5)
        log ("[UART] Configure RAK811, set config: {0}".format(self.uart_read()))
        
        self.uart_write("at+rxc=1")
        time.sleep(0.5)
        log ("[UART] Configure RAK811, set rx mode: {0}".format(self.uart_read()))
        
        log ("[UART] RAK811 configured")
        
    def uart_write(self, arg1):
        self.ser.write(arg1.encode())
        self.ser.write(b'P\x0D\x0A') # send end line

    def uart_read(self):
        out = b''
        time.sleep(DELAY)
        while self.ser.inWaiting() > 0:
            out += self.ser.read(1)
        return out.decode("utf-8")

    def database_run(self):
        self.req_id_current = 0
        try:
            logfile = open("database.log", "w")
            time.sleep(1) # Задержка для отрисовки интерфейса
            sportsman_name = " "
            # Запрос к базе данных
            response = requests.get('http://{0}/dashboard_get/'.format(DATABASE_IP),
                        params={"platform":"{0}".format(PLATFORM), "competition":"{0}".format(COMPETITION)})
            # Считывание ответа от сервера
            log ("[DB] Received status header: {0}".format(response.status_code))
            if (response.status_code == requests.codes.ok):
                # Парсим содержимое ответа
                response_string = ast.literal_eval(response.content.decode())
                log ("[DB] Received message: {0}".format(response_string))
                self.req_text = json.loads(json.dumps(response_string, ensure_ascii=False))
                logfile.write(str(self.req_text))
                # Формируем лист участников
                #self.req_id_lenght = len(self.req_id)
                self.req_id_lenght = 0
                for x in self.req_text:
                    self.req_id.append(x)
                    self.req_id_lenght += 1
                    log ("[DB] Parse: {0} : {1}".format(x,self.req_text[x]))
                log ("[DB] Received list lenght: {0}".format(self.req_id_lenght))
                # Выбираем участника из списка
                sportsman_name = self.req_text[self.req_id[self.req_id_current]]
                # Показываем имя участника
                self.req_namevisible = 1
                self.namevisible_change(self.req_namevisible)
                self.name_change(sportsman_name)
            else:
                log ("[DB] Connection error ")
                log ("[DB] Closing Thread ")
                self.req_namevisible = 0
                self.namevisible_change(self.req_namevisible)
            logfile.close()

        except Exception as e:
            log ("[DB] ERROR: {0}".format(e))
            log ("[DB] Connection error ")
            log ("[DB] Closing Thread ")
            self.req_namevisible = 0
            self.namevisible_change(self.req_namevisible)
        finally:
            logfile.close()

    @pyqtSlot()
    def database_next(self):
        if (self.req_id_lenght > self.req_id_current+1):
            self.req_id_current += 1
        else:
            self.req_id_current = 0
        sportsman_name = self.req_text[self.req_id[self.req_id_current]]
        self.name_change(sportsman_name)

    def database_response(self):
        # Запрещаем записывать нулевые результаты
        if (self.num_count_last == 0):
            log ("[DB] Counter cant be zero")
            return
        log ("[DB] Sending results in database")

        response = requests.get('http://{0}/dashboard_set/'.format(DATABASE_IP),
                   params={"sportsmenid":"{0}".format(self.req_id[self.req_id_current]),
                           "competition":"{0}".format(COMPETITION),
                           "result":"{0}".format(self.num_count_last)})

        log(response.content)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    engine = QQmlApplicationEngine()
    exe = MainWindow()
    engine.rootContext().setContextProperty("exe", exe)
    engine.load("main.qml")
    engine.quit.connect(app.quit)
    sys.exit(app.exec_())	
