/*
 * View model for SafetyPrinter
 *
 * Author: Rodrigo C. C. Silva
 * License: AGPLv3
 */
$(function() {
    function spSensorsType(label, status) {
        var self = this;
        self.label = label;
        self.status = ko.observable(status);
    }

    function SafetyprinterViewModel(parameters) {
        var self = this;

        // assign the injected parameters, e.g.:
        // self.loginStateViewModel = parameters[0];
        // self.settingsViewModel = parameters[1];

        self.flag = ko.observable();

        // TODO: Implement your plugin's view model here.

        self.spSensors = ko.observableArray([
            new spSensorsType("Flame1","ok"),
            new spSensorsType("Flame2","ok"),
            new spSensorsType("Smoke","ok"),
            new spSensorsType("Emergency Button","ok")
        ]);

        self.tripReset = function() {
            //  OctoPrint.simpleApiCommand("gpiocontrol", "turnGpioOn", {id: self.gpioButtons.indexOf(this)}).then(function () {
            //      self.updateGpioButtons();
            //  });
            self.flag("Tripreset");
        };

        self.emergencyStop = function() {
            self.flag("emergencyStop");
        };

        self.onDataUpdaterPluginMessage = function(plugin, data) {
            self.flag("recebi msg");

            if (plugin != "SafetyPrinter") {
                return;
            }
            
            self.flag("recebi msg depois if");
    
            //if (data.type == "statusupdate") {
            //    self.flag("E statusupdate");
                //self.spSensors.replace(self.spSensors()[data.sensorindex], new spSensorsType(data.sensorlabel,data.sensoractive));
            //}
        };
        
    }

    /* view model class, parameters for constructor, container to bind to
     * Please see http://docs.octoprint.org/en/master/plugins/viewmodels.html#registering-custom-viewmodels for more details
     * and a full list of the available options.
     */
    OCTOPRINT_VIEWMODELS.push({
        construct: SafetyprinterViewModel,
        // ViewModels your plugin depends on, e.g. loginStateViewModel, settingsViewModel, ...
        dependencies: ["settingsViewModel"],
        // Elements to bind to, e.g. #settings_plugin_SafetyPrinter, #tab_plugin_SafetyPrinter, ...
        elements: ["#settings_plugin_SafetyPrinter", "#sidebar_plugin_SafetyPrinter"]
    });
});
