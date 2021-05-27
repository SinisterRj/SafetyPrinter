# coding=utf-8
from __future__ import absolute_import
import octoprint.plugin
import serial
import time
import flask
from . import Connection
from octoprint.util import RepeatedTimer

totalSensors = 0

class SafetyPrinterPlugin(
    octoprint.plugin.StartupPlugin,
    octoprint.plugin.SettingsPlugin,
    octoprint.plugin.AssetPlugin,
    octoprint.plugin.SimpleApiPlugin,
    octoprint.plugin.TemplatePlugin,
    octoprint.plugin.ShutdownPlugin):

    def new_connection(self):
        self._logger.info("Attempting to connect to Safety Printer MCU ...")
        self.conn = Connection.Connection(self)
        self.startTimer(1.0)

    def startTimer(self, interval):
        # timer that updates UI with Arduino information
        self._commTimer = RepeatedTimer(interval, self.updateStatus, None, None, True)
        self._commTimer.start()

    def updateStatus(self):
        # Update UI status (connection, trip and sensors)
        self.conn.update_ui_connection_status()
        if self.conn.is_connected():
            self.conn.update_ui_status()
        else:
            self._commTimer.cancel()

    # ~~ StartupPlugin mixin
    def on_after_startup(self):
        self._logger.info("******************* Starting Safety Printer Plug-in ***************************")
        self._logger.info("Default Serial Port:" + str(self._settings.get(["serialport"])))
        self.new_connection()

    # ~~ ShutdonwPlugin mixin
    def on_shutdown(self):
        self._logger.info("Disconnectig Safety Printer MCU...")
        self._commTimer.cancel()
        self.conn.closeConnection()
                            
    ##~~ SettingsPlugin mixin    
    def get_settings_defaults(self):
        return dict(
            serialport="AUTO"
        )

    def get_template_vars(self):
        return dict(serialport=self._settings.get(["serialport"]))

    ##~~ AssetPlugin mixin
    def get_template_configs(self):
        return [
        dict(type="settings", custom_bindings=True),
        #dict(type="navbar", custom_bindings=False),
        dict(type="sidebar", name="Safety Printer", custom_bindings=False, icon="fire"),
        ]

    def get_assets(self):
        return dict(
            js=["js/SafetyPrinter.js"],
            css=["css/SafetyPrinter.css"],
          )

    ##~~ Softwareupdate hook
    def get_update_information(self):
        return dict(
            SafetyPrinter=dict(
                displayName="Safety Printer",
                displayVersion=self._plugin_version,

                # version check: github repository
                type="github_release",
                user="SinisterRj",
                repo="SafetyPrinter",
                current=self._plugin_version,

                # update method: pip
                pip="https://github.com/SinisterRj/SafetyPrinter/archive/{target_version}.zip"
             )
          )
    
    # Simple API commands. Deal with UI commands

    def get_api_commands(self):
        return dict(
            reconnect=[],
            disconnect=[],
            resetTrip=[],
            sendTrip=[],
            getPorts=[],
            toggleEnabled=["id", "onoff"],
            changeSP=["id", "newSP"],
            sendCommand=["serialCommand"],
            saveEEPROM=[]
        )

    def on_api_command(self, command, data):
        try:
            if command == "reconnect":
                self.new_connection()
            if command == "disconnect":
                self.on_shutdown()
            elif command == "resetTrip":
                self.resetTrip()
            elif command == "sendTrip":
                self.sendTrip()
            elif command == "getPorts":
                self.conn.update_ui_ports()
            elif command == "toggleEnabled":
                self.toggleEnabled(int(data["id"]), str(data["onoff"]))
            elif command == "changeSP":
                self.changeSP(int(data["id"]), str(data["newSP"]))
            elif command == "sendCommand":
                self.sendCommand(str(data["serialCommand"]))
            elif command == "saveEEPROM":
                self.saveEEPROM()
            response = "POST request (%s) successful" % command
            return flask.jsonify(response=response, data=data, status=200), 200
        except Exception as e:
            error = str(e)
            self._logger.info("Exception message: %s" % str(e))
            return flask.jsonify(error=error, status=500), 500

    def resetTrip(self):
        if self.conn.is_connected():
            self._logger.info("Resseting ALL trips.")
            self.newSerialCommand("<C1>")
            
    def sendTrip(self):
        if self.conn.is_connected():
            self._logger.info("Virtual Emergency Button pressed.")            
            self.newSerialCommand("<C2>")

    def toggleEnabled(self, index, status):
        if self.conn.is_connected():
            if status == "on":
                self._logger.info("Enabling sensor #" + str(index))
            else:
                self._logger.info("Disabling sensor #" + str(index))
            self.newSerialCommand("<C3 " + str(index) + " " + status + ">")

    def changeSP(self, index, newSP):
        if self.conn.is_connected():
            self._logger.info("Changing sensor #" + str(index) + " setpoint to:" + newSP)
            self.newSerialCommand("<C4 " + str(index) + " " + newSP + ">")

    def sendCommand(self,newCommand):
        if self.conn.is_connected():
            self._logger.info("Sending terminal command: " + newCommand)
            self.newSerialCommand(newCommand)
    
    def saveEEPROM(self):
        if self.conn.is_connected():
            self._logger.info("Saving configuration to EEPROM.")
            self.newSerialCommand("<C5>")

    def newSerialCommand(self,serialCommand):
        i = 0
        while self.conn.send_command(serialCommand,True) == "error":
            time.sleep(0.1)
            i += 1
            if i > 0:
                self._logger.info("Serial Port Busy")
            if i > 20:
                self.conn.closeConnection()
                break

__plugin_name__ = "Safety Printer"
__plugin_pythoncompat__ = ">=2.7,<4" # python 2 and 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = SafetyPrinterPlugin()

    global __plugin_hooks__
    __plugin_hooks__ = {"octoprint.plugin.softwareupdate.check_config":__plugin_implementation__.get_update_information}


