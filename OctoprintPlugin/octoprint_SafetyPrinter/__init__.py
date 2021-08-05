# coding=utf-8
from __future__ import absolute_import
import logging
import logging.handlers
import octoprint.plugin
import serial
import time
import flask
from . import Connection
from octoprint.util import RepeatedTimer
from octoprint.events import eventManager, Events
from octoprint.server import user_permission

totalSensors = 0

class SafetyPrinterPlugin(
    octoprint.plugin.StartupPlugin,
    octoprint.plugin.SettingsPlugin,
    octoprint.plugin.AssetPlugin,
    octoprint.plugin.SimpleApiPlugin,
    octoprint.plugin.EventHandlerPlugin,
    octoprint.plugin.TemplatePlugin,
    octoprint.plugin.ShutdownPlugin):

    def __init__(self):
        self.abortTimeout = 0
        self.rememberCheckBox = False
        self.lastCheckBoxValue = False
        self.turnOffPrinter = True
        self._automatic_shutdown_enabled = False        
        self._timeout_value = None
        self._abort_timer = None
        self._wait_for_timelapse_timer = None
        self.loggingLevel = 0
        #self.FWVersion = "" 
        #self.FWReleaseDate = "" 
        #self.FWEEPROM = "" 
        #self.FWCommProtocol = ""
        #self.FWValidVersion = False

    def initialize(self):
        self._console_logger = logging.getLogger("octoprint.plugins.safetyprinter")

    def new_connection(self):
        self._console_logger.info("Attempting to connect to Safety Printer MCU ...")
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
    def on_startup(self, host, port):
        console_logging_handler = logging.handlers.RotatingFileHandler(self._settings.get_plugin_logfile_path(postfix="console"), maxBytes=2 * 1024 * 1024)
        console_logging_handler.setFormatter(logging.Formatter("%(asctime)s [%(levelname)s] %(message)s"))
        #console_logging_handler.setLevel(logging.DEBUG)

        self._console_logger.addHandler(console_logging_handler)
        self.loggingLevel = self._settings.get_int(["loggingLevel"])
        self._console_logger.setLevel(self.loggingLevel)
        #self.console_setlevel(self.loggingLevel)
        self._console_logger.propagate = False

    '''def console_setlevel(self, level):
    #    if level == "DEBUG":
    #        self._logger.info("********************** DEBUG") #Octoprint logger
            self._console_logger.setLevel(logging.DEBUG)
        elif level == "INFO":
            self._logger.info("********************** INFO") #Octoprint logger
            self._console_logger.setLevel(logging.INFO)
        elif level == "WARNING":
            self._console_logger.setLevel(logging.WARNING)
        elif level == "ERROR":
            self._console_logger.setLevel(logging.ERROR)
        elif level == "CRITICAL":
            self._console_logger.setLevel(logging.CRITICAL)
'''
    def on_after_startup(self):
        self._logger.info("Safety Printer Plugin started.") #Octoprint logger 
        self._console_logger.info("******************* Starting Safety Printer Plug-in ***************************")
        self._console_logger.info("Default Serial Port:" + str(self._settings.get(["serialport"])))
        self.new_connection()

        self.abortTimeout = self._settings.get_int(["abortTimeout"])
        self._console_logger.debug("abortTimeout: %s" % self.abortTimeout)

        self.rememberCheckBox = self._settings.get_boolean(["rememberCheckBox"])
        self._console_logger.debug("rememberCheckBox: %s" % self.rememberCheckBox)

        self.lastCheckBoxValue = self._settings.get_boolean(["lastCheckBoxValue"])
        self._console_logger.debug("lastCheckBoxValue: %s" % self.lastCheckBoxValue)
        if self.rememberCheckBox:
            self._automatic_shutdown_enabled = self.lastCheckBoxValue

        self.showTerminal = self._settings.get_boolean(["showTerminal"])
        self._console_logger.debug("showTerminal: %s" % self.showTerminal)

        self.loggingLevel = self._settings.get(["loggingLevel"])
        self._console_logger.debug("loggingLevel: %s" % self.loggingLevel)

    # ~~ ShutdonwPlugin mixin
    def on_shutdown(self):
        self._console_logger.info("Disconnectig Safety Printer MCU...")
        self._commTimer.cancel()
        self.conn.closeConnection()
                            
    ##~~ SettingsPlugin mixin    
    def get_settings_defaults(self):
        return dict(
            serialport="AUTO",
            abortTimeout = 30,
            rememberCheckBox = False,
            lastCheckBoxValue = False,
            turnOffPrinter = True,
            showTerminal = False,
            loggingLevel = "INFO",
        )

    def on_settings_save(self, data):
        octoprint.plugin.SettingsPlugin.on_settings_save(self, data)
        self.abortTimeout = self._settings.get_int(["abortTimeout"])
        self.rememberCheckBox = self._settings.get_boolean(["rememberCheckBox"])
        self.lastCheckBoxValue = self._settings.get_boolean(["lastCheckBoxValue"])
        self.turnOffPrinter = self._settings.get_boolean(["turnOffPrinter"])
        self.showTerminal = self._settings.get_boolean(["showTerminal"])
        self.loggingLevel = self._settings.get_int(["loggingLevel"]) 
        #self._console_logger.setLevel(self.loggingLevel)
        
        self._console_logger.info("User changed settings.")
        self._console_logger.debug("serialport: %s" % self._settings.get(["serialport"]))
        self._console_logger.debug("abortTimeout: %s" % self.abortTimeout)
        self._console_logger.debug("rememberCheckBox: %s" % self.rememberCheckBox)
        self._console_logger.debug("lastCheckBoxValue: %s" % self.lastCheckBoxValue)
        self._console_logger.debug("showTerminal: %s" % self.showTerminal)
        self._console_logger.debug("loggingLevel: %s" % self.loggingLevel)

    def get_template_vars(self):
        return dict(
            serialport=self._settings.get(["serialport"]),
            loggingLevel=self._settings.get(["loggingLevel"]),
            #FWVersion=self.FWVersion, 
            #FWReleaseDate=self.FWReleaseDate, 
            #FWEEPROM=self.FWEEPROM, 
            #FWCommProtocol=self.FWCommProtocol,
            #FWValidVersion=self.FWValidVersion 
        )

    ##~~ AssetPlugin mixin
    def get_template_configs(self):
        return [
            dict(type="settings", custom_bindings=True),
            #dict(type="navbar", custom_bindings=False),
            dict(type="sidebar", name="Safety Printer", custom_bindings=False, icon="fire"),
            dict(type="tab", name="Safety Printer", custom_bindings=False, icon="fire"),
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
                #pip="https://github.com/SinisterRj/SafetyPrinter/archive/{target_version}.zip"
                pip="https://downgit.github.io/#/home?url=https://github.com/SinisterRj/SafetyPrinter/tree/main/OctoprintPlugin"
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
            changeTimer=["id", "newTimer"],
            sendCommand=["serialCommand"],
            resetSettings=["id"],
            saveEEPROM=[],
            enableShutdown=[],
            disableShutdown=[],
            abortShutdown=[]
        )

    def on_api_command(self, command, data):
        try:
            if command == "reconnect":
                self.new_connection()
            elif command == "disconnect":
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
            elif command == "changeTimer":
                self.changeTimer(int(data["id"]), str(data["newTimer"]))
            elif command == "sendCommand":
                self.sendCommand(str(data["serialCommand"]))
            elif command == "resetSettings":
                self.resetSettings(int(data["id"]))    
            elif command == "saveEEPROM":
                self.saveEEPROM()
            elif command == "enableShutdown":
                self.enableShutdown()
            elif command == "disableShutdown":
                self.disableShutdown()
            elif command == "abortShutdown":
                self.abortShutdown()
            response = "POST request (%s) successful" % command
            return flask.jsonify(response=response, data=data, status=200), 200
        except Exception as e:
            error = str(e)
            self._console_logger.info("Exception message: %s" % str(e))
            return flask.jsonify(error=error, status=500), 500

    def resetTrip(self):
        if self.conn.is_connected():
            self._console_logger.info("Resseting ALL trips.")
            self.conn.newSerialCommand("<C1>")
            
    def sendTrip(self):
        if self.conn.is_connected():
            self._console_logger.info("Virtual Emergency Button pressed.")            
            self.conn.newSerialCommand("<C2>")

    def toggleEnabled(self, index, status):
        if self.conn.is_connected():
            if status == "on":
                self._console_logger.info("Enabling sensor #" + str(index))
            else:
                self._console_logger.info("Disabling sensor #" + str(index))
            self.conn.newSerialCommand("<C3 " + str(index) + " " + status + ">")

    def changeSP(self, index, newSP):
        if self.conn.is_connected():
            self._console_logger.info("Changing sensor #" + str(index) + " setpoint to:" + newSP)
            self.conn.newSerialCommand("<C4 " + str(index) + " " + newSP + ">")

    def changeTimer(self, index, newTimer):
        if self.conn.is_connected():
            self._console_logger.info("Changing sensor #" + str(index) + " timer to:" + newTimer)
            self.conn.newSerialCommand("<C7 " + str(index) + " " + newTimer + ">")

    def sendCommand(self,newCommand):
        if self.conn.is_connected():
            self._console_logger.info("Sending terminal command: " + newCommand)
            self.conn.newSerialCommand(newCommand)
    
    def resetSettings(self, index):
        if self.conn.is_connected():
            self._console_logger.info("Loading sensor #" + str(index) + " default configurations.")
            self.conn.newSerialCommand("<C8 " + str(index) + ">")

    def saveEEPROM(self):
        if self.conn.is_connected():
            self._console_logger.info("Saving configuration to EEPROM.")
            self.conn.newSerialCommand("<C5>")

    def toggleShutdown(self):
        self.lastCheckBoxValue = self._automatic_shutdown_enabled
        if self.rememberCheckBox:
            self._settings.set_boolean(["lastCheckBoxValue"], self.lastCheckBoxValue)
            self._settings.save()
            eventManager().fire(Events.SETTINGS_UPDATED)
        self._plugin_manager.send_plugin_message(self._identifier, {"type":"shutdown","automaticShutdownEnabled": self._automatic_shutdown_enabled, "timeout_value":self._timeout_value})
    
    def enableShutdown(self):
        self._automatic_shutdown_enabled = True
        self.toggleShutdown()

    def disableShutdown(self):
        self._automatic_shutdown_enabled = False
        self.toggleShutdown()        

    def abortShutdown(self):
        if self._wait_for_timelapse_timer is not None:
                self._wait_for_timelapse_timer.cancel()
                self._wait_for_timelapse_timer = None
        if self._abort_timer is not None:
                self._abort_timer.cancel()
                self._abort_timer = None
        self._timeout_value = None
        self._plugin_manager.send_plugin_message(self._identifier, {"type":"shutdown","automaticShutdownEnabled": self._automatic_shutdown_enabled, "timeout_value":self._timeout_value})
        self._console_logger.info("Shutdown aborted.")

    def on_event(self, event, payload):

        if event == Events.CLIENT_OPENED:
            self._plugin_manager.send_plugin_message(self._identifier, {"type":"shutdown","automaticShutdownEnabled": self._automatic_shutdown_enabled, "timeout_value":self._timeout_value})
            return
        
        if not self._automatic_shutdown_enabled:
            return
        
        if not self._settings.global_get(["server", "commands", "systemShutdownCommand"]):
            self._console_logger.warning("systemShutdownCommand is not defined. Aborting shutdown...")
            return

        if event not in [Events.PRINT_DONE, Events.PRINT_FAILED]:
            return

        if event == Events.PRINT_FAILED and not self._printer.is_closed_or_error():
            #Cancelled job
            return
        
        if event in [Events.PRINT_DONE, Events.PRINT_FAILED]:
            webcam_config = self._settings.global_get(["webcam", "timelapse"], merged=True)
            timelapse_type = webcam_config["type"]
            if (timelapse_type is not None and timelapse_type != "off"):
                    self._wait_for_timelapse_start()
            else:
                    self._timer_start()
            return

    def _wait_for_timelapse_start(self):
        if self._wait_for_timelapse_timer is not None:
                return

        self._wait_for_timelapse_timer = RepeatedTimer(5, self._wait_for_timelapse)
        self._wait_for_timelapse_timer.start()

    def _wait_for_timelapse(self):
        c = len(octoprint.timelapse.get_unrendered_timelapses())

        if c > 0:
                self._console_logger.info("Waiting for %s timelapse(s) to finish rendering before starting shutdown timer..." % c)
        else:
                self._timer_start()

    def _timer_start(self):
        if self._abort_timer is not None:
                return

        if self._wait_for_timelapse_timer is not None:
                self._wait_for_timelapse_timer.cancel()

        self._console_logger.info("Starting abort shutdown timer.")
        
        self._timeout_value = self.abortTimeout
        self._abort_timer = RepeatedTimer(1, self._timer_task)
        self._abort_timer.start()

    def _timer_task(self):
        if self._timeout_value is None:
                return

        self._timeout_value -= 1
        self._plugin_manager.send_plugin_message(self._identifier, {"type":"shutdown", "automaticShutdownEnabled": self._automatic_shutdown_enabled, "timeout_value":self._timeout_value})
        if self._timeout_value <= 0:
                if self._wait_for_timelapse_timer is not None:
                        self._wait_for_timelapse_timer.cancel()
                        self._wait_for_timelapse_timer = None
                if self._abort_timer is not None:
                        self._abort_timer.cancel()
                        self._abort_timer = None
                self._shutdown_system()

    def _shutdown_system(self):
        if self.turnOffPrinter:
            if self.conn.is_connected():
                self._console_logger.info("Turning off the printer.")
                self.conn.newSerialCommand("<C6 off>")           

        shutdown_command = self._settings.global_get(["server", "commands", "systemShutdownCommand"])
        self._console_logger.info("Shutting down system with command: {command}".format(command=shutdown_command))
        try:
            import sarge            
            self._console_logger.info("**************** SHUT DOWN ******************")
            p = sarge.run(shutdown_command, async_=True)
        except Exception as e:
            self._console_logger.exception("Error when shutting down: {error}".format(error=e))
            return

__plugin_name__ = "Safety Printer"
__plugin_pythoncompat__ = ">=2.7,<4" # python 2 and 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = SafetyPrinterPlugin()

    global __plugin_hooks__
    __plugin_hooks__ = {"octoprint.plugin.softwareupdate.check_config":__plugin_implementation__.get_update_information}


