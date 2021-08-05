import os
import re
import sys
import glob
import threading
import serial
import serial.tools.list_ports
import time

if ((sys.platform == 'linux') or (sys.platform =='linux2')):
    import termios

class Connection():
    def __init__(self, plugin):

        self.compatibleFirmwareCommProtocol = ["1"]

        # Serial connection variables
        self.ports = []
        self._connected = False
        self.serialConn = None
        self.connectedPort = ""
        self.waitingResponse = False

        # Arrays for sensor status:
        self.sensorLabel = []
        self.sensorEnabled = []
        self.sensorActive = []
        self.sensorActualValue = []
        self.sensorType = []
        self.sensorSP = []
        self.sensorTimer = []
        self.sensorSpare1 = []
        self.sensorSpare2 = []
        self.sensorSpare3 = []
        self.sensorSpare4 = []
        self.lastStatus = []

        self.totalSensorsInitial = 0
        self.totalSensors = 0
        self.lastInterlockStatus = 0

        # Plug-in shortcuts
        #self._console_logger = plugin._logger #Change logger to octoprit.log - for debug only
        self._console_logger = plugin._console_logger
        self._logger = plugin._logger
        self._printer = plugin._printer
        self._printer_profile_manager = plugin._printer_profile_manager
        self._plugin_manager = plugin._plugin_manager
        self._identifier = plugin._identifier
        self._settings = plugin._settings

        #Firmware info
        self.FWVersion = ""
        self.FWReleaseDate = ""
        self.FWEEPROM = ""
        self.FWCommProtocol = ""
        self.FWValidVersion = False

        #Octopod Integration
        self.octopodDetected = False;
        helpers = self._plugin_manager.get_helpers("octopod", "apns_notification")
        if helpers and "apns_notification" in helpers:
            self.push_notification = helpers["apns_notification"]
            self.octopodDetected = True;

        self.connect()        

    # *******************************  Functions to deal with Serial connections

    def connect(self):
        # Connects to Safety Printer Arduino through serial port
        self._console_logger.info("Connecting...")

        self.ports = self.getAllPorts()
        self._console_logger.info("Potential ports: %s" % self.ports)
        
        if ((self._printer.get_current_connection()[1] == None) and (self._settings.get(["serialport"]) == "AUTO")):
            self.terminal("Can't connect on AUTO serial port if printer is not connected. Aborting Safety Printer MCU connection.","ERROR",True)
            return

        if len(self.ports) > 0:
            for port in self.ports:
                
                if ((not self._connected) and ((self._settings.get(["serialport"]) == "AUTO") or (self._settings.get(["serialport"]) == port))):
                    if self.isPrinterPort(port,True):
                        self._console_logger.info("Skipping Printer Port:" + port)
                    else:
                        try:
                            self.serialConn = serial.Serial(port, 115200, timeout=0.5)
                            self._connected = True
                            self.connectedPort = port
                        except serial.SerialException as e:
                            self.terminal("Connection failed! " + str(e),"ERROR",True)
                            self.update_ui_connection_status()

            if not self._connected:
                self.terminal("Couldn't connect on any port.","ERROR",True)
                self.update_ui_connection_status()
            else:
                self.terminal("Waiting Safety Printer MCU answer...","Info",True)
                responseStr = self.serialConn.readline().decode()
                self.terminal(responseStr,"Info",True)
                i = 0
                while responseStr.find("Safety Printer MCU") == -1: # Wait for arduino boot and answer
                    i += 1                    
                    time.sleep(0.50)
                    responseStr = self.serialConn.readline().decode() 
                    self.terminal(responseStr,"Info",True)   
                    if i > 20:
                        self.terminal("Safety Printer MCU connection error.","Error",True)
                        self.closeConnection()
                        return

                self.terminal("Safety Printer MCU connected: " + str(responseStr[0:-4]),"Info",True)
                
                responseStr = self.newSerialCommand("<R4>")
                if responseStr:
                    self.terminal(responseStr,"Info",True);
                    vpos1 = responseStr.find(':',0)
                    vpos2 = responseStr.find(',',vpos1)
                    self.FWVersion = responseStr[vpos1+1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.FWReleaseDate = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.FWEEPROM = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.FWCommProtocol = responseStr[vpos1:vpos2]

                    self.FWValidVersion = False
                    for version in self.compatibleFirmwareCommProtocol:
                        if version == self.FWCommProtocol:
                            self.FWValidVersion = True

                    if self.FWValidVersion:
                        self.update_ui_connection_status()
                    else:
                        self.terminal("Invalid firmware comunication protocol version: " + self.FWCommProtocol,"ERROR",True)
                        self.closeConnection()
                else:
                    self.terminal("Connected but no valid response","ERROR",True)
                    self.closeConnection()

        else:
            self.terminal("NO SERIAL PORTS FOUND!","ERROR",True)
            self.update_ui_connection_status()

    def closeConnection(self):
        # Disconnects Safety Printer Arduino
        if self._connected:
            self.serialConn.close()
            self.serialConn.__del__()
            self._connected = False
            self.terminal("Safety Printer MCU connection closed.","Info",True)
            self.update_ui_connection_status()
        else :
            self.terminal("Safety Printer MCU not connected.","Info",True)

    # below code "stolen" from https://gitlab.com/mosaic-mfg/palette-2-plugin/blob/master/octoprint_palette2/Omega.py
    #| Chip                | VID  | PID                      | Board                           | Link                                                                     | Note  
    #| Atmel ATMEGA16U2    | 2341 | 003D,003F,0042,0043,0044 | UNO, MEGA2560, ADK, DUE, clones | Included in Arduino software under drivers                               |  
    #| Atmel ATMEGA32U4    | 2341 | 8036 etc.                | Leonardo, micro, clones         | Included in Arduino software under drivers                               |  
    #| FTDI FT232RL        | 0403 | 6001                     | Nano, Duemilanove, MEGA, clones | http://www.ftdichip.com/Drivers/VCP.htm                                  | Included in Arduino software under drivers. Many fakes exist so avoid buying cheap Arduino compatible board with this chip.  
    #| WCH CH34X           | 1A86 | 5523, 7523 etc.          | Many clone boards               | http://www.wch.cn/download/CH341SER_ZIP.html                             | Supports win 10. Seems best choice for Arduino compatible boards.  
    #| Prolific PL2303     | 10CE |                          | Many clone boards               | http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41          | The company claims that many older PL2303 models on the market are fakes so it has stopped supplying drivers for win 7 and up on these models.  
    #| Silicon Labs CP210X | 11F6 |                          | Many clone boards               | https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx | The driver may not work with win 10 so if you just upgraded to win 10 and your board stops working, … 
    # Source: https://forum.arduino.cc/t/help-me-confirm-some-vid-pid-and-comments-for-an-intall-tutorial/339586

    def getAllPorts(self):
        baselist = []
        
        arduinoVIDPID = ['.*2341:003D.*','.*2341:003F.*','.*2341:0042.*','.*2341:0043.*','.*2341:0044.*','.*0403:6001.*','.*0403:6015.*','.*1A86:5523.*','.*1A86:7523.*']

        '''
        if 'win32' in sys.platform:
            # use windows com stuff
            self._console_logger.info("Using a Windows machine") 

            for arduino in arduinoVIDPID:
                for port in serial.tools.list_ports.grep(arduino):
                    self._console_logger.info("got port %s" % port.device)
                    baselist.append(port.device)
        else:
            baselist = baselist + glob.glob('/dev/serial/by-id/*FTDI*') + glob.glob('/dev/*usbserial*') + glob.glob(
                '/dev/*usbmodem*') + glob.glob('/dev/serial/by-id/*USB_Serial*') + glob.glob('/dev/serial/by-id/usb-*')
            baselist = self.getRealPaths(baselist)

            for arduino in arduinoVIDPID:
                for port in baselist:
                    if port == serial.tools.list_ports.ListPortInfo
                    baselistfiltered.append(port)'''
        
        #Works for linux and windows:
        for arduino in arduinoVIDPID:
            for port in serial.tools.list_ports.grep(arduino):
                self._console_logger.info("Arduino port: %s" % port.device)
                baselist.append(port.device)

        # get unique values only
        baselist = list(set(baselist))  
        return baselist

    def getRealPaths(self, ports):
        self._console_logger.info("Paths: %s" % ports)
        for index, port in enumerate(ports):
            port = os.path.realpath(port)
            ports[index] = port
        return ports

    def isPrinterPort(self, selected_port, loggin):
        selected_port = os.path.realpath(selected_port)
        #printer_port = self._printer.get_current_connection()[1]
        if self._printer.get_current_connection()[0] == "Closed":
            if loggin:
                self._console_logger.info("No printer connected.")
            return False

        printer_port = os.path.realpath(self._printer.get_current_connection()[1])
                
        if loggin:
            self._console_logger.info("Trying port: %s" % selected_port)
            self._console_logger.info("Printer port: %s" % self._printer.get_current_connection()[1])
            self._console_logger.info("Printer port path: %s" % printer_port)
        # because ports usually have a second available one (.tty or .cu)
        printer_port_alt = ""
        if printer_port is None:
            return False
        else:
            if "tty." in printer_port:
                printer_port_alt = printer_port.replace("tty.", "cu.", 1)
            elif "cu." in printer_port:
                printer_port_alt = printer_port.replace("cu.", "tty.", 1)
            if loggin:
                self._console_logger.info("Printer port alt: %s" % printer_port_alt)
            if selected_port == printer_port or selected_port == printer_port_alt:
                return True
            else:
                return False

    def is_connected(self):
        return self._connected

    # *******************************  Functions to update info on knockout interface

    def update_ui_ports(self):
        # Send one message for each serial port detected
        self.ports = self.getAllPorts()
        for port in self.ports:
            if not self.isPrinterPort(port,True):
                self._plugin_manager.send_plugin_message(self._identifier, {"type": "serialPortsUI", "port": port})

    def update_ui_status(self):
        # Send one message for each sensor with all status
        if self._connected:            
            
            if (len(self.sensorLabel) == 0):
                self.update_ui_sensor_labels()
            
            responseStr = self.send_command("<R1>",False) 

            if ((responseStr == "Error") or (not(isinstance(responseStr, str)))):
                return

            numhash = responseStr.count('#')
            totalSensors = int(numhash)

            if totalSensors != self.totalSensorsInitial:
                self.sensorLabel = []
                return

            self.interlockStatus = responseStr[3] 
            
            if (self.interlockStatus != self.lastInterlockStatus) and (self.interlockStatus == "1"):
                self._console_logger.info("WARNING: New INTERLOCK detected.")
                #Octopod notification
                try:
                    self.push_notification("\U0001F6D1 New INTERLOCK detected.")
                except AttributeError:
                    pass
            
            self.lastInterlockStatus = self.interlockStatus
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "interlockUpdate", "interlockStatus": self.interlockStatus})
            vpos1 = 4

            for x in range(totalSensors):
                vpos1 = responseStr.find('#',vpos1)
                vpos2 = responseStr.find(',',vpos1)
                
                if responseStr[vpos1+1:vpos2].isdigit():
                    index = int(responseStr[vpos1+1:vpos2])
                else :
                    return
                
                if (index >= 0 and index < totalSensors):
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorEnabled[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorActive[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorActualValue[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorSP[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorTimer[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorSpare1[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorSpare3[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    
                    if (self.lastStatus[index] != self.sensorActive[index]) and (self.sensorActive[index] == "1"):
                        self._console_logger.info("New Alarm detected - Index: " + str(index) + " Label:" + str(self.sensorLabel[index]) + " Enabled:" + str(self.sensorEnabled[index]) + " Active:" + str(self.sensorActive[index]) + " ActualValue:" + str(self.sensorActualValue[index]))
                        #Octopod notification
                        if (self.sensorEnabled[index] == "1"):
                            #Octopod notification
                            try:
                                self.push_notification("\U0001F514 " + str(self.sensorLabel[index]) + " (" + str(self.sensorActualValue[index]) + ")")
                            except AttributeError:
                                pass                            

                    self.lastStatus[index] = self.sensorActive[index]
                    #self._console_logger.info("Index: " + str(index) + " Label:" + str(self.sensorLabel[index]) + " Enabled:" + str(self.sensorEnabled[index]) + " Active:" + str(self.sensorActive[index]) + " ActualValue:" + str(self.sensorActualValue[index]))
                    self._plugin_manager.send_plugin_message(self._identifier, {"type": "statusUpdate", "sensorIndex": index, "sensorNumber": totalSensors, "sensorLabel": self.sensorLabel[index], "sensorEnabled": self.sensorEnabled[index], "sensorActive": self.sensorActive[index], "sensorActualValue": self.sensorActualValue[index], "sensorType": self.sensorType[index], "sensorSP": self.sensorSP[index], "sensorTimer": self.sensorTimer[index]})
        else :
            self.update_ui_connection_status()

    def update_ui_sensor_labels(self):
        # Update local arrays with sensor labels and type. create items for all the other properties. Should run just after connection, only one time or when the number of sensor status sended by arduino changes
        responseStr = self.send_command("<R2>",False)
        
        if ((responseStr == "Error") or (not(isinstance(responseStr, str)))):
                return
        
        numhash = responseStr.count('#')
        vpos1 = 0
        vpos2 = 0
        self.totalSensorsInitial = int(numhash)
        
        for x in range(self.totalSensorsInitial):
            vpos1 = responseStr.find('#',vpos1)
            vpos2 = responseStr.find(',',vpos1)
        
            if responseStr[vpos1+1:vpos2].isdigit():
                index = int(responseStr[vpos1+1:vpos2])
            else :
                return
        
            vpos1 = vpos2 + 1
            vpos2 = responseStr.find(',',vpos1)
        
            if index >= len(self.sensorLabel) :
                self.sensorLabel.append(responseStr[vpos1:vpos2])
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorType.append(responseStr[vpos1:vpos2])
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorSpare2.append(responseStr[vpos1:vpos2])
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorSpare4.append(responseStr[vpos1:vpos2])
                vpos1 = vpos2 + 1
                self.lastStatus.append("")
                self.sensorEnabled.append("")
                self.sensorActive.append("")
                self.sensorActualValue.append("")
                self.sensorSP.append("")
                self.sensorTimer.append("")
                self.sensorSpare1.append("")
                self.sensorSpare3.append("")
            else :
                self.sensorLabel[index] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorType[index] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorSpare2[index] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorSpare4[index] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                self.lastStatus[index] = ""
                self.sensorEnabled[index] = ""
                self.sensorActive[index] = ""
                self.sensorActualValue[index] = ""
                self.sensorSP[index] = ""
                self.sensorTimer[index] = ""
                self.sensorSpare1[index] = ""
                self.sensorSpare3[index] = ""

    def update_ui_connection_status(self):
        # Updates knockout connection status
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": self._connected, "port": self.connectedPort})    
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "firmwareInfo", "version": self.FWVersion, "releaseDate": self.FWReleaseDate, "EEPROM": self.FWEEPROM, "CommProtocol":self.FWCommProtocol, "ValidVersion": self.FWValidVersion})

    def terminal(self,msg,ttype,log):
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "terminalUpdate", "line": msg, "terminalType": ttype})  
        popup = "Safety Printer "
        hasError = False;
        # Pops up the error msg to user.
        if  ttype == "ERROR":
            popup = popup + "ERROR: " + msg
            hasError = True;

        elif ttype == "CRITICAL":
            popup = popup + "CRITICAL ERROR: " + msg
            hasError = True;

        if hasError:
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "error", "errorMsg": popup})

            popup = "\U000026A0 " + popup
            #Octopod notification
            try:
                self.push_notification(popup)
            except AttributeError:
                pass  

        if log:
            if ttype == "DEBUG":
                self._console_logger.debug(msg)
            elif ttype == "INFO":
                self._console_logger.info(msg)
            elif ttype == "WARNIG":
                self._console_logger.debug(msg)
            elif ttype == "ERROR":
                self._console_logger.error(msg)
            elif ttype == "CRITICAL":
                self._console_logger.debug(msg)        

    # ****************************************** Functions to interact with Arduino when connected

    def newSerialCommand(self,serialCommand):
        i = 0
        responseStr = self.send_command(serialCommand,True)
        while responseStr == "Error":
            time.sleep(0.2)
            i += 1
            responseStr = self.send_command(serialCommand,True)
            if i > 0:
                self._console_logger.info("Serial Port Busy")
            if i > 20:
                self.closeConnection()
                break
        return responseStr

    def send_command(self, command, log):
        # send serial commands to arduino and receives the answer

        if self.is_connected():
            try:
                self.serialConn.flush()
                self.terminal(command.strip(), "Send", False) 
                
                if not self.waitingResponse and self.is_connected():
                    self.serialConn.write(command.encode())
                else:
                    return "Error"
                
                
                self.waitingResponse = True                    
                data = ""
                keepReading = True

                while keepReading:
                    time.sleep(0.05)
                    if self.is_connected():
                        newline = self.serialConn.readline()                        
                    if not newline.strip():
                        keepReading = False
                    else:
                        data += newline.decode()
                
                if data: 
                    self.terminal(data.strip(), "Recv", False) 

                    if str(data[0:len(command)-2]) == command[1:-1]:
                        self.waitingResponse = False
                        return str(data)
            
            except (serial.SerialException, termios.error): #, termios.error):
                self.waitingResponse = False
                self.terminal("Safety Printer communication error.", "Error", True)
                self.closeConnection()                    
                return "Error"

        else:
            self.waitingResponse = False
            return "Error"

        self.waitingResponse = False