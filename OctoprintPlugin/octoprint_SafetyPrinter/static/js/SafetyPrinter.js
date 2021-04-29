/*
 * View model for SafetyPrinter
 *
 * Author: Rodrigo C. C. Silva
 * License: AGPLv3
 */
$(function() {
    function spSensorsType(visible, label, status, color, actualValue, newAlarm, enabled, active, type) {
        var self = this;
        self.visible = ko.observable(visible);
        self.label = ko.observable(label);
        self.status = ko.observable(status);
        self.color = ko.observable(color);
        self.actualValue = ko.observable(actualValue);
        self.newAlarm = ko.observable(newAlarm);
        self.enabled = ko.observable(enabled);
        self.active = ko.observable(active);
        self.type = ko.observable(type);
    }

    function SafetyprinterViewModel(parameters) {
        var self = this;

        //self.settingsViewModel = parameters[0];

        self.interlock = ko.observable();
        self.interlockColor = ko.observable();
        self.tripBtnVisible = ko.observable();

        self.connection = ko.observable("offline");
        self.connectionColor = ko.observable("red");
        self.notConnected = ko.observable(true);

        self.navbarcolor = ko.observable("black");

        self.spSensors = ko.observableArray([
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0),
           new spSensorsType(false,"","offline","gray",0,0,0)
        ]);

        self.tripResetBtn = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "resetTrip");
        };

        self.emergencyStopBtn = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "sendTrip");
        };

        self.connectBtn = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "reconnect");
        };

        self.onDataUpdaterPluginMessage = function(plugin, data) {
          
            if (plugin != "SafetyPrinter") {
                return;
            }
            //console.log(data.type);
            
            if (data.type == "statusUpdate") {

                // Incluir o tipo de sensor na msg R2 para que possa apresentar OK ou actual value (sensores analogicos)no status.
                //console.log("passando");
                if (data.sensorIndex >=0 || data.sensorIndex < 8) {
                    self.spSensors()[data.sensorIndex].visible(true);                    
                    self.spSensors()[data.sensorIndex].label(data.sensorLabel);
                    self.spSensors()[data.sensorIndex].enabled(data.sensorEnabled);
                    self.spSensors()[data.sensorIndex].newAlarm(data.sensorNewAlarm);
                    self.spSensors()[data.sensorIndex].actualValue(data.sensorActualValue);
                    self.spSensors()[data.sensorIndex].active(data.sensorActive);
                    self.spSensors()[data.sensorIndex].type(data.sensortype);

                    statusStr = "";
                    if (data.sensorType == "1") {
                        //NTC sensor
                        statusStr = data.sensorActualValue + "°C - "
                    }

                    if (data.sensorActive == "1") {
                        if(data.sensorEnabled == "1") {
                            
                            statusStr += "Alarm";
                            colorStr = "orange";
                        } else {
                            statusStr += "Alarm (disabled)";
                            colorStr = "gray";
                        }                        
                    } else {
                        if(data.sensorEnabled == "1") {
                            statusStr += "OK";
                            colorStr = "green";
                        } else {
                            statusStr += "OK (disabled)";
                            colorStr = "gray";
                        }                        
                    }
                    self.spSensors()[data.sensorIndex].status(statusStr);
                    self.spSensors()[data.sensorIndex].color(colorStr);

                }
            } 
            else if (data.type == "interlockUpdate") {
                if (data.interlockStatus == "0") {
                    self.interlock("Normal");
                    self.interlockColor("black");
                    self.tripBtnVisible(false)
                    self.navbarcolor("green");
                } else {
                    self.interlock("TRIP");
                    self.interlockColor("red");
                    self.tripBtnVisible(true)
                    self.navbarcolor("red");
                }
                
            }
            else if (data.type == "connectionUpdate") {
                if (data.connectionStatus) {
                    self.connection("online");
                    self.connectionColor("black");
                    self.notConnected(false);
                } else {
                    self.connection("offline");
                    self.connectionColor("red");
                    self.notConnected(true);
                    self.navbarcolor("black");
                    self.spSensors = [];
                }
            }
        };
    }

    /* view model class, parameters for constructor, container to bind to
     * Please see http://docs.octoprint.org/en/master/plugins/viewmodels.html#registering-custom-viewmodels for more details
     * and a full list of the available options.
     */
    OCTOPRINT_VIEWMODELS.push({
        construct: SafetyprinterViewModel,
        // ViewModels your plugin depends on, e.g. loginStateViewModel, settingsViewModel, ...
        // dependencies: ["settingsViewModel"],
        // Elements to bind to, e.g. #settings_plugin_SafetyPrinter, #tab_plugin_SafetyPrinter, ...
        elements: ["#navbar_plugin_SafetyPrinter","#sidebar_plugin_SafetyPrinter"] //"#settings_plugin_SafetyPrinter"
    });
});
