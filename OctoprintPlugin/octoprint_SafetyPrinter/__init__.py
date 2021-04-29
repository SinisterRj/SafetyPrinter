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
    #octoprint.plugin.SettingsPlugin,
    octoprint.plugin.AssetPlugin,
    octoprint.plugin.SimpleApiPlugin,
    octoprint.plugin.TemplatePlugin,
    octoprint.plugin.ShutdownPlugin):

    def new_connection(self):
        self._logger.info("******** Attempting to connect to Safety Printer MCU ...")
        self.conn = Connection.Connection(self)
        self.startTimer(1.0)

    # ~~ StartupPlugin mixin
    def on_after_startup(self):
        self._logger.info("******************* Iniciando o plugin ***************************")
        #self.flag = True
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
        #self._logger.info(self.conn.is_connected())
        if self.conn.is_connected():
            self.conn.update_ui_status()
        else:
            self._commTimer.cancel()
            self._plugin_manager.send_plugin_message(self._identifier, {"type": "connectionUpdate", "connectionStatus": False})

    ##~~ SettingsPlugin mixin
    '''
    def get_settings_defaults(self):
        return dict(test="nada"   #Verificar se precisa
            # put your plugin's default settings here
        )
    '''

    ##~~ AssetPlugin mixin
    def get_template_configs(self):
        return [
        #dict(type="settings", 
        #    custom_bindings=False),
        dict(type="navbar", custom_bindings=True),
        dict(type="sidebar", name="Safety Printer", custom_bindings=False, icon="power-off"),
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
            resetTrip=[],
            sendTrip=[],
        )

    def on_api_command(self, command, data):
        try:
            data = None
            if command == "reconnect":
                self.new_connection()
            elif command == "resetTrip":
                self.resetTrip()
            elif command == "sendTrip":
                self.sendTrip()
            response = "POST request (%s) successful" % command
            return flask.jsonify(response=response, data=data, status=200), 200
        except Exception as e:
            error = str(e)
            self._logger.info("Exception message: %s" % str(e))
            return flask.jsonify(error=error, status=500), 500

    def resetTrip(self):
        if self.conn.is_connected():
            self.responseStr = self.conn.send_command("<C1>")
            
    def sendTrip(self):
        if self.conn.is_connected():
            self.responseStr = self.conn.send_command("<C2>")

__plugin_name__ = "Safety Printer"
__plugin_pythoncompat__ = ">=2.7,<4" # python 2 and 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = SafetyPrinterPlugin()

    global __plugin_hooks__
    __plugin_hooks__ = {"octoprint.plugin.softwareupdate.check_config":__plugin_implementation__.get_update_information}


