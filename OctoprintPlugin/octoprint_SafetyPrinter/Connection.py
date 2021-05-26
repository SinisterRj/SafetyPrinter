import os
import re
import sys
import glob
import threading
import serial
import serial.tools.list_ports
import time
import termios
#import numpy as np  #precisa instalar o numpy (usar array com indice: sudo apt-get install python3-numpy)

class Connection():
    def __init__(self, plugin):

        self.complatibleFirmware = ["0.2.1"]

        self.sensorLabel = []
        self.sensorEnabled = []
        self.sensorActive = []
        self.sensorActualValue = []
        self.sensorType = []
        self.sensorSP = []
        self.sensorSpare1 = []
        self.sensorSpare2 = []
        self.sensorSpare3 = []
        self.sensorSpare4 = []

        self.lastStatus = []
        self.totalSensorsInitial = 0
        self.totalSensors = 0
        self.lastInterlockStatus = 0

        self._logger = plugin._logger
        self._printer = plugin._printer
        self._printer_profile_manager = plugin._printer_profile_manager
        self._plugin_manager = plugin._plugin_manager
        self._identifier = plugin._identifier
        self._settings = plugin._settings

        self.ports = []
        #self.UIports = []
        #self._logger.info("**** Iniciando o connection")
        self._connected = False
        self.serialConn = None
        self.connectedPort = ""
        self.waitingResponse = False

        self.connect()
        

    def connect(self):
        self._logger.info("Connecting...")

        self.ports = self.getAllPorts()
        #self._logger.info("Potential ports: %s" % self.ports)
        
        #self.UIports.append("AUTO")
        if len(self.ports) > 0:
            for port in self.ports:
                
                #self._logger.info(port)
                #self.availableSerialPorts.append(port)
                #i += 1
                #self.UIports.append(port)
                
                if ((not self._connected) and ((self._settings.get(["serialport"]) == "AUTO") or (self._settings.get(["serialport"]) == port))):
                    if self.isPrinterPort(port):
                        self._logger.info("Skipping Printer Port:" + port)
                    else:
                        try:
                            self.serialConn = serial.Serial(port, 115200, timeout=0.5)
                            self._connected = True
                            self.connectedPort = port
                        except serial.SerialException:
                            self.terminal("Connection failed!","ERROR",True)
                            self.update_connection_status()
            if not self._connected:
                self.terminal("Couldn't connect on any port.","ERROR",True)
                self.update_connection_status()
            else:
                time.sleep(2.00) # wait for arduino boot
                responseStr = self.serialConn.readline()
                responseStr = responseStr.decode()
                self.terminal("Safety Printer MCU connected: " + str(responseStr[0:-4]),"Info",True)
                
                responseStr = self.send_command("<R4>",True)
                vpos1 = responseStr.find(':',0)
                vpos2 = responseStr.find(',',vpos1)
                connectedVersion = responseStr[vpos1+1:vpos2]

                validVersion = False
                for version in self.complatibleFirmware:
                    if version == connectedVersion:
                        validVersion = True

                if validVersion:
                    self.update_connection_status()
                else:
                    self.terminal("Invalid firmware version: " + connectedVersion,"ERROR",True)
                    self.closeConnection()

        else:
            #msg = "NO SERIAL PORTS FOUND!"
            #self.update_ui_error(msg)
            self.terminal("NO SERIAL PORTS FOUND!","ERROR",True)
            self.update_connection_status()

    def update_ui_ports(self):
        for port in self.ports:
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "serialPortsUI", "port": port})

    def closeConnection(self):
        self._logger.error("********************** Fechando conex~ao")
        if self._connected:
            #self.serialConn.flush()  // n~ao usar pq da erro se der falha na comunicacao
            #self.UIports = []
            self.serialConn.close()
            self.serialConn.__del__()
            self._connected = False
            self.terminal("Safety Printer MCU connection closed.","Info",True)
            self.update_connection_status()
        else :
            self.terminal("Safety Printer MCU not connected.","Info",True)

    def update_ui_status(self):
        if self._connected:
            #self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": True})
            self.update_connection_status()
            if (len(self.sensorLabel) == 0):
                self.update_ui_labels()
            responseStr = self.send_command("<R1>",False) 
            if ((responseStr == "error") or (not(isinstance(responseStr, str)))):
                return
            numhash = responseStr.count('#')
            totalSensors = int(numhash)
            if totalSensors != self.totalSensorsInitial:
                self.sensorLabel = []
                return
            self.interlockStatus = responseStr[3] 
            if (self.interlockStatus != self.lastInterlockStatus) and (self.interlockStatus == "1"):
                self._logger.info("New INTERLOCK detected")
            
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
                    #self._logger.info(self.sensorEnabled[index])
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
                    self.sensorSpare1[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    vpos2 = responseStr.find(',',vpos1)
                    self.sensorSpare3[index] = responseStr[vpos1:vpos2]
                    vpos1 = vpos2 + 1
                    if (self.lastStatus[index] != self.sensorActive[index]) and (self.sensorActive[index] == "1"):
                        self._logger.info("New Alarm detected - Index: " + str(index) + " Label:" + str(self.sensorLabel[index]) + " Enabled:" + str(self.sensorEnabled[index]) + " Active:" + str(self.sensorActive[index]) + " ActualValue:" + str(self.sensorActualValue[index]))

                    self.lastStatus[index] = self.sensorActive[index]
                    #self._logger.info("Index: " + str(index) + " Label:" + str(self.sensorLabel[index]) + " Enabled:" + str(self.sensorEnabled[index]) + " Active:" + str(self.sensorActive[index]) + " ActualValue:" + str(self.sensorActualValue[index]))
                    self._plugin_manager.send_plugin_message(self._identifier, {"type": "statusUpdate", "sensorIndex": index, "sensorLabel": self.sensorLabel[index], "sensorEnabled": self.sensorEnabled[index], "sensorActive": self.sensorActive[index], "sensorActualValue": self.sensorActualValue[index], "sensorType": self.sensorType[index], "sensorSP": self.sensorSP[index]})
        else :
            #self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": False})    
            self.update_connection_status()

    def update_ui_labels(self):

        responseStr = self.send_command("<R2>",False)
        if ((responseStr == "error") or (not(isinstance(responseStr, str)))):
                return
        numhash = responseStr.count('#')
        vpos1 = 0
        vpos2 = 0
        self.totalSensorsInitial = int(numhash)
        
        for x in range(self.totalSensorsInitial):
            vpos1 = responseStr.find('#',vpos1)
            vpos2 = responseStr.find(',',vpos1)
            #index = int(responseStr[vpos1+1:vpos2])
            if responseStr[vpos1+1:vpos2].isdigit():
                index = int(responseStr[vpos1+1:vpos2])
            else :
                return
            #self._logger.info("indice :" + str(index) + str(len(self.sensorLabel)))
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
                self.lastStatus.append("2")
                self.sensorEnabled.append("2")
                self.sensorActive.append("2")
                self.sensorActualValue.append("")
                self.sensorSP.append("0")
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
                self.lastStatus[index] = "2"
                self.sensorEnabled[index] = "2"
                self.sensorActive[index] = "2"
                self.sensorActualValue[index] = ""
                self.sensorSP[index] = "0"
                self.sensorSpare1[index] = ""
                self.sensorSpare3[index] = ""

    def update_connection_status(self):
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": self._connected, "port": self.connectedPort})    

    def send_command(self, command, log):

        if self.is_connected():
            tries = 0

            for x in range(3):
                try:
                    tries += 1
                    self.serialConn.flush()

                    if log:
                        self._logger.info("Sending: %s" % command)
                    self._plugin_manager.send_plugin_message(self._identifier, {"type": "terminalUpdate", "line": command.strip(), "terminalType": "Send"})    
                    
                    if not self.waitingResponse and self.is_connected():
                        self.serialConn.write(command.encode())
                    else:
                        return "error"
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
                        self._plugin_manager.send_plugin_message(self._identifier, {"type": "terminalUpdate", "line": data.strip(), "terminalType": "Recv"})  
                        if log:
                            self._logger.info("Received: %s" % data.strip())
                        if str(data[0:len(command)-2]) == command[1:-1]:
                            self.waitingResponse = False
                            return str(data)
                        break
                except (serial.SerialException, termios.error):
                    self.waitingResponse = False
                    self._logger.error("Safety Printer communication error.")
                    self.closeConnection()
                    
                    return "error"
                if tries >= 3:
                    self.waitingResponse = False
                    self._logger.error("Safety Printer communication error.")
                    self.closeConnection()                    
                    return "error"
        else:
            self.waitingResponse = False
            return "error"
        self.waitingResponse = False

    # below code "stolen" from https://gitlab.com/mosaic-mfg/palette-2-plugin/blob/master/octoprint_palette2/Omega.py
    def getAllPorts(self):
        baselist = []

        if 'win32' in sys.platform:
            # use windows com stuff
            self._logger.info("Using a windows machine")
            for port in serial.tools.list_ports.grep('.*0403:6015.*'):
                self._logger.info("got port %s" % port.device)
                baselist.append(port.device)

        baselist = baselist + glob.glob('/dev/serial/by-id/*FTDI*') + glob.glob('/dev/*usbserial*') + glob.glob(
            '/dev/*usbmodem*') + glob.glob('/dev/serial/by-id/*USB_Serial*') + glob.glob('/dev/serial/by-id/usb-*')
        baselist = self.getRealPaths(baselist)
        # get unique values only
        baselist = list(set(baselist))
        return baselist

    def getRealPaths(self, ports):
        self._logger.info("Paths: %s" % ports)
        for index, port in enumerate(ports):
            port = os.path.realpath(port)
            ports[index] = port
        return ports

    def isPrinterPort(self, selected_port):
        selected_port = os.path.realpath(selected_port)
        printer_port = self._printer.get_current_connection()[1]
        self._logger.info("Trying port: %s" % selected_port)
        self._logger.info("Printer port: %s" % printer_port)
        # because ports usually have a second available one (.tty or .cu)
        printer_port_alt = ""
        if printer_port is None:
            return False
        else:
            if "tty." in printer_port:
                printer_port_alt = printer_port.replace("tty.", "cu.", 1)
            elif "cu." in printer_port:
                printer_port_alt = printer_port.replace("cu.", "tty.", 1)
            self._logger.info("Printer port alt: %s" % printer_port_alt)
            if selected_port == printer_port or selected_port == printer_port_alt:
                return True
            else:
                return False

    def is_connected(self):
        return self._connected

    def terminal(self,msg,ttype,log):
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "terminalUpdate", "line": msg, "terminalType": ttype})  
        if log:
            if ttype == "ERROR":
                self._logger.error(msg)
            else:
                self._logger.info(msg)