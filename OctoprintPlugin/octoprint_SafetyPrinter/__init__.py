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
    #octoprint.plugin.EventHandlerPlugin,
    octoprint.plugin.ShutdownPlugin):

    def new_connection(self):
        self._logger.info("******** Attempting to connect to Safety Printer MCU ...")
        self.conn = Connection.Connection(self)
        self.startTimer(1.0)

    # ~~ StartupPlugin mixin
    def on_after_startup(self):
        self._logger.info("******************* Iniciando o plugin ***************************")
        #self.flag = True
        self._logger.info("Default Serial Port:" + str(self._settings.get(["serialport"])))

        self.new_connection()

    # ~~ ShutdonwPlugin mixin
    def on_shutdown(self):
        self._logger.info("Disconnectig Safety Printer MCU...")
        self._commTimer.cancel()
        self.conn.closeConnection()
                            
    def startTimer(self, interval):
        self._commTimer = RepeatedTimer(interval, self.updateStatus, None, None, True)
        self._commTimer.start()

    def updateStatus(self):
        if self.conn.is_connected():
            self.conn.update_ui_status()
        else:
            self._commTimer.cancel()
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": False})

    ##~~ SettingsPlugin mixin
    
    def get_settings_defaults(self):
        return dict(
            serialport="AUTO"
            # put your plugin's default settings here
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
    # Define your plugin's asset files to automatically include in the
    # core UI here.
        return dict(
            js=["js/SafetyPrinter.js"],
            css=["css/SafetyPrinter.css"],
            #less=["less/SafetyPrinter.less"]
          )

    ##~~ Softwareupdate hook
    def get_update_information(self):
        # Define the configuration for your plugin to use with the Software Update
        # Plugin here. See https://docs.octoprint.org/en/master/bundledplugins/softwareupdate.html
        # for details.
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

    '''
    def on_event(self, event, payload):
        if event == Events.CLIENT_OPENED:
            self.update_ui()
        elif event == Events.PRINT_STARTED:
            self._logger.info("Got event: {}".format(event))
            self.print_started()
        elif event == Events.PRINT_DONE:
            self._logger.info("Got event: {}".format(event))
            self.print_stopped()
        elif event == Events.PRINT_FAILED:
            self._logger.info("Got event: {}".format(event))
            self.print_stopped()
        elif event == Events.PRINT_CANCELLING:
            self._logger.info("Got event: {}".format(event))
            #self.print_stopped()
        elif event == Events.PRINT_CANCELLED:
            self._logger.info("Got event: {}".format(event))
            #self.print_stopped()
        return
    '''
__plugin_name__ = "Safety Printer"
__plugin_pythoncompat__ = ">=2.7,<4" # python 2 and 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = SafetyPrinterPlugin()

    global __plugin_hooks__
    __plugin_hooks__ = {"octoprint.plugin.softwareupdate.check_config":__plugin_implementation__.get_update_information}


