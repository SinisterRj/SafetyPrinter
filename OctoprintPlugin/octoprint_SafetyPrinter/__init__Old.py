# coding=utf-8
from __future__ import absolute_import
import octoprint.plugin
import serial

class SafetyPrinterPlugin(octoprint.plugin.SettingsPlugin,
  octoprint.plugin.AssetPlugin,
  octoprint.plugin.TemplatePlugin):

    ##~~ SettingsPlugin mixin

    def get_settings_defaults(self):
        return dict(
            # put your plugin's default settings here
          )

    ##~~ AssetPlugin mixin

    def get_template_configs(self):
        return [dict(type="sidebar",
            name="Safety Printer",
            custom_bindings=False,
            icon="power-off"),
        dict(type="settings", custom_bindings=False)]

    def get_assets(self):
    # Define your plugin's asset files to automatically include in the
    # core UI here.
        return dict(
            js=["js/SafetyPrinter.js"],
            css=["css/SafetyPrinter.css"],
            less=["less/SafetyPrinter.less"]
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

# -------------------------------------------------------------------------------
    def serial_factory_hook(self, comm_instance, port, baudrate, read_timeout, *args, **kwargs):
        self.create_serial_obj()
        self.sync_settings_with_serial_obj()

        self._serial_obj.timeout = read_timeout
        self._serial_obj.write_timeout = 0
        if baudrate == 0:
            self._serial_obj.baudrate = 9600
        else:
            self._serial_obj.baudrate = baudrate
            self._serial_obj.port = str(port)

            # Parity workaround needed for linux
            use_parity_workaround = settings().get(["serial", "useParityWorkaround"])
            needs_parity_workaround = get_os() == "linux" and os.path.exists("/etc/debian_version")  # See #673

            if use_parity_workaround == "always" or (needs_parity_workaround and use_parity_workaround == "detect"):
                self._serial_obj.parity = serial.PARITY_ODD
                self._serial_obj.open()
                self._serial_obj.close()
                self._serial_obj.parity = serial.PARITY_NONE

                self._serial_obj.open()

            # Set close_exec flag on serial handle, see #3212
            if hasattr(self._serial_obj, "fd"):
                # posix
                set_close_exec(self._serial_obj.fd)
            elif hasattr(self._serial_obj, "_port_handle"):
                # win32
                # noinspection PyProtectedMember
                set_close_exec(self._serial_obj._port_handle)

                self._serial_obj.query_config_state()

                return self._serial_obj

    # -------------------------------------------------------------------------------
    def on_shutdown(self):
        if self._serial_obj is not None:
            self._serial_obj.cleanup()

    # -------------------------------------------------------------------------------
    def create_serial_obj(self):
        if not self._serial_obj:
            self._serial_obj = PackingSerial(self._logger)

            self._serial_obj.statsUpdateCallback = self.serial_obj_stats_update_callback

            def serial_obj_stats_update_callback(self):
                self._plugin_manager.send_plugin_message(self._identifier, {"message": "update"})

    # -------------------------------------------------------------------------------
    def sync_settings_with_serial_obj(self):
        self._serial_obj.log_transmission_stats = self._settings.get_boolean(["logTransmissionStats"])
        self._serial_obj.play_song_on_print_complete = self._settings.get_boolean(["playSongOnPrintComplete"])
        self._serial_obj.packing_enabled = self._settings.get_boolean(["enableMeatPack"])
        self._serial_obj.omit_all_spaces = self._settings.get_boolean(["omitSpaces"])

# -------------------------------------------------------------------------------
def on_after_startup(self):
    self.create_serial_obj()
    self.sync_settings_with_serial_obj()
    self._logger.info("MeatPack version {} loaded... current state is {}"
      .format(self._plugin_version, "Enabled" if self._serial_obj.packing_enabled else "Disabled"))

__plugin_name__ = "Safety Printer"
__plugin_pythoncompat__ = ">=2.7,<4" # python 2 and 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = SafetyPrinterPlugin()

    global __plugin_hooks__
    __plugin_hooks__ = {
    "octoprint.comm.transport.serial.factory":__plugin_implementation__.serial_factory_hook,
    "octoprint.plugin.softwareupdate.check_config":__plugin_implementation__.get_update_information}


