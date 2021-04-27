import os
import re
import sys
import glob
import threading
import serial
import serial.tools.list_ports
import time
import termios

class Connection():
    def __init__(self, plugin):
        self.sensorIndex = []
        self.sensorLabel = []
        self.sensorEnabled = []
        self.sensorActive = []
        self.sensorNewAlarm = []
        self.sensorActualValue = []
        self.sensorType = []

        self.lastStatus = []
        self.totalSensorsInitial = 0
        self.totalSensors = 0

        self._logger = plugin._logger
        self._printer = plugin._printer
        self._printer_profile_manager = plugin._printer_profile_manager
        self._plugin_manager = plugin._plugin_manager
        self._identifier = plugin._identifier
        #self._settings = plugin._settings

        self.ports = []
        self._logger.info("**** Iniciando o connection")
        self._connected = False
        self.serialConn = None
        self.connect()

    def connect(self):
        self._logger.info("Connecting...")

        self.ports = self.getAllPorts()
        self._logger.info("Potential ports: %s" % self.ports)
        if len(self.ports) > 0:
            for port in self.ports:
                if not self._connected:
                    if self.isPrinterPort(port):
                        self._logger.info("Skipping Printer Port:" + port)
                    else:
                        try:
                            self.serialConn = serial.Serial(port, 115200, timeout=0.5)
                            self._connected = True
                        except serial.SerialException:
                            self.update_ui_error("Connection failed!")
            if not self._connected:
                self.update_ui_error("Couldn't connect on any port.")
            else:
                time.sleep(2.00) # wait for arduino boot
                self.data = self.serialConn.readline()
                self._logger.info("Safety Printer MCU connected! %s" % self.data)
        else:
            msg = "NO SERIAL PORTS FOUND!"
            self.update_ui_error(msg)

    def closeConnection(self):
        if self._connected:
            self.serialConn.close()
            self.serialConn.__del__()
            self._connected = False
            self._logger.info("Safety Printer MCU connection closed.")
        else :
            self._logger.info("Safety Printer MCU not connected.")

    def update_ui_status(self):
        if self._connected:
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": True})
            if (len(self.sensorLabel) == 0):
                self.update_ui_labels()
            responseStr = self.send_command("<R1>") 
            if responseStr == "error":
                return
            numhash = responseStr.count('#')
            totalSensors = int(numhash)
            if totalSensors != self.totalSensorsInitial:
                self.sensorLabel = []
                return
            self.interlockStatus = responseStr[3] 
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "interlockUpdate", "interlockStatus": self.interlockStatus})
            vpos1 = 4
            for x in range(totalSensors):
                vpos1 = responseStr.find('#',vpos1)
                vpos2 = responseStr.find(',',vpos1)
                self.sensorIndex[x] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorEnabled[x] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorActive[x] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorNewAlarm[x] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                vpos2 = responseStr.find(',',vpos1)
                self.sensorActualValue[x] = responseStr[vpos1:vpos2]
                vpos1 = vpos2 + 1
                self.lastStatus[x] = self.sensorActive[x]
                #self._logger.info("Index: " + str(x) + " Label:" + str(self.sensorLabel[x]) + " Enabled:" + str(self.sensorEnabled[x]) + " Active:" + str(self.sensorActive[x]) + " NewAlarm:" + str(self.sensorNewAlarm[x]) +  " ActualValue:" + str(self.sensorActualValue[x]))
                self._plugin_manager.send_plugin_message(self._identifier, {"type": "statusUpdate", "sensorIndex": x, "sensorLabel": self.sensorLabel[x], "sensorEnabled": self.sensorEnabled[x], "sensorActive": self.sensorActive[x], "sensorNewAlarm": self.sensorNewAlarm[x], "sensorActualValue": self.sensorActualValue[x], "sensorType": self.sensorType[x]})
        else :
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": False})    

    def update_ui_labels(self):

        responseStr = self.send_command("<R2>")
        if responseStr == "error":
                return
        numhash = responseStr.count('#')
        vpos1 = 0
        vpos2 = 0
        self.totalSensorsInitial = int(numhash)
        
        for x in range(self.totalSensorsInitial):
            vpos1 = responseStr.find('#',vpos1)
            vpos2 = responseStr.find(',',vpos1)
            self.sensorIndex.append(responseStr[vpos1:vpos2])
            vpos1 = vpos2 + 1
            vpos2 = responseStr.find(',',vpos1)
            self.sensorLabel.append(responseStr[vpos1:vpos2])
            vpos1 = vpos2 + 1
            vpos2 = responseStr.find(',',vpos1)
            self.sensorType.append(responseStr[vpos1:vpos2])
            vpos1 = vpos2 + 1
            self.lastStatus.append("2")
            self.sensorEnabled.append("2")
            self.sensorActive.append("2")
            self.sensorNewAlarm.append("2")
            self.sensorActualValue.append("")

    def update_ui_prompt(self, prompt):
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "prompt", "data": prompt})

    def update_ui_error(self, error):
        self._plugin_manager.send_plugin_message(self._identifier, {"type": "error", "data": error})

    def send_command(self, command):

        if self.is_connected():
            tries = 0

            for x in range(3):
                try:
                    tries += 1
                    self.serialConn.flush()
                    self._logger.info("Sending: %s" % command)
                    self.serialConn.write(command.encode())
                    time.sleep(0.05)
                    data = self.serialConn.readline()
                    if data: 
                        data = data.decode()
                        self._logger.info("Received: %s" % data)
                        if str(data[0:len(command)-2]) == command[1:-1]:
                            return str(data)
                        break
                except (serial.SerialException, termios.error):
                    self._logger.error("Safety Printer communication error.")
                    self.closeConnection()
                    return "error"
                if tries >= 3:
                    self._logger.error("Safety Printer communication error.")
                    self.closeConnection()
                    return "error"
        else:
            return "error"

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
            '/dev/*usbmodem*') + glob.glob('/dev/serial/by-id/*USB_Serial*') + glob.glob('/dev/serial/by-id/usb-Arduino*')
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