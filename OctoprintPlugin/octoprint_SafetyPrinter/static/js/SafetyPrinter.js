/*
 * View model for SafetyPrinter
 *
 * Author: Rodrigo C. C. Silva
 * License: AGPLv3
 */
$(function() {
    function spSensorsType(visible, label, status, color, actualvalue, enabled, active, type, SP, availableSP, expertMode) {
        var self = this;
        self.visible = ko.observable(visible);
        self.label = ko.observable(label);
        self.status = ko.observable(status);
        self.color = ko.observable(color);
        self.actualvalue = ko.observable(actualvalue);        
        self.enabled = ko.observable(enabled);
        self.active = ko.observable(active);
        self.type = ko.observable(type);
        self.SP = ko.observable(SP);
        self.availableSP = ko.observableArray(availableSP);
        self.expertMode = ko.observable(expertMode)
    }

    function ItemViewModel(val) {
        var self = this;
        self.name = ko.observable(val);
    }

    function TerminalViewModel(line,type) {
        var self = this;
        self.line = ko.observable(line);
        self.type = ko.observable(type);
    }

    function SafetyprinterViewModel(parameters) {
        var self = this;

        self.settingsViewModel = parameters[0];
        
        //settingsViewModel.settings.plugins.SafetyPrinter.serialport

        self.interlock = ko.observable();
        self.interlockColor = ko.observable();
        self.tripBtnVisible = ko.observable();

        self.autoscrollEnabled = ko.observable(true);

        self.expertMode = ko.observable(false);

        self.connection = ko.observable("Offline");
        self.connectionColor = ko.observable("red");
        self.notConnected = ko.observable(true);
        self.connectedPort = ko.observable("None");
        self.connectionCaption = ko.observable("Connect");

        self.navbarcolor = ko.observable("black");
        self.navbartitle = ko.observable("Safety Printer: Offline");

        self.availablePorts = ko.observableArray([new ItemViewModel("AUTO")]);

        self.terminalLines = ko.observableArray();

        self.teste = ko.observable("nao iniciada");

        self.countTerminalLines = 0;

        self.command = ko.observable();

        self.spSensors = ko.observableArray([
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,"0","0","0",[],false)
        ]);       

        self.onBeforeBinding = function() {
        
        
            //self.settings = self.settingsViewModel.settings;
            //self.ownSettings = self.settings.plugins.SafetyPrinter;
            //self.serialport = self.ownSettings.serialport();
            
            self.serialport = self.settingsViewModel.settings.plugins.SafetyPrinter.serialport();

 
            //settingsViewModel.settings.plugins.SafetyPrinter.serialport
        
            console.log(self.serialport);
            //self.teste(settingsViewModel.settings.plugins.SafetyPrinter.serialport());
        }

        self.onStartupComplete = function() {
            // Update serial ports info
            OctoPrint.simpleApiCommand("SafetyPrinter", "getPorts");
        };

        self.toggleAutoscroll = function() {
            self.autoscrollEnabled(!self.autoscrollEnabled());

            if (self.autoscrollEnabled()) {
                //self.updateOutput();
                self.scrollToEnd();
            }
        };

        self.terminalScrollEvent = _.throttle(function () {
            var container = $("#SafetyPrinterTerminal");
            var pos = container.scrollTop();
            var scrollingUp =
                self.previousScroll !== undefined && pos < self.previousScroll;

            if (self.autoscrollEnabled() && scrollingUp) {
                var maxScroll = container[0].scrollHeight - container[0].offsetHeight;

                if (pos <= maxScroll) {
                    self.autoscrollEnabled(false);
                }
            }

            self.previousScroll = pos;
        }, 250);

        self.stopAutoscroll = function() {
            self.autoscrollEnabled(false);
        };

        self.scrollToEnd = function () {
            var container = $("#SafetyPrinterTerminal");
            if (container.length) {
                container.scrollTop(container[0].scrollHeight);
            }
        };

        self.sendCommand = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "sendCommand", {serialCommand: self.command()}); 
        }

        self.tripResetBtn = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "resetTrip");
        };

        self.toggleEnabledBtn = function(item) {
            //var index = self.spSensors.indexOf(item);
            console.log("Mudando enabled " + self.spSensors.indexOf(item));
            //self.spSensors()[index].enabled = !self.spSensors()[index].enabled;
            if (self.spSensors()[self.spSensors.indexOf(item)].enabled()) {
                OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: self.spSensors.indexOf(item), onoff: "off"});
            } else {
                OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: self.spSensors.indexOf(item), onoff: "on"});
            }
        };

        self.changeSPSel = function(item) {
            //var index = self.spSensors.indexOf(item);
            console.log("Mudando Set Point " + self.spSensors.indexOf(item));
            OctoPrint.simpleApiCommand("SafetyPrinter", "changeSP", {id: self.spSensors.indexOf(item), newSP: self.spSensors()[self.spSensors.indexOf(item)].SP()});
        };

        self.connectBtn = function() {
            if (self.notConnected()) {
                self.connection("Connecting");
                self.connectionColor("black");
                self.notConnected(false);
                OctoPrint.simpleApiCommand("SafetyPrinter", "reconnect");    
            } else {
                self.connection("Disconecting");
                self.connectionColor("black");
                self.notConnected(true);
                OctoPrint.simpleApiCommand("SafetyPrinter", "disconnect");    
            }

        };

        self.saveEEPROMBtn = function() {
            OctoPrint.simpleApiCommand("SafetyPrinter", "saveEEPROM");  
        }

        self.onDataUpdaterPluginMessage = function(plugin, data) {
          
            if (plugin != "SafetyPrinter") {
                return;
            }
            //console.log(data.type);
            if ((data.type == "statusUpdate") && (self.notConnected() == false)) {

                var i = parseInt(data.sensorIndex);
                if (i >=0 || i < 8) {
                                        
                    self.spSensors()[i].visible(true);                    
                    self.spSensors()[i].label(data.sensorLabel);
                    if (data.sensorEnabled == "1") {
                        self.spSensors()[i].enabled(true);    
                        //console.log(Math.floor(Math.random() * 101) + ":Index:" + i + " , Label:" + data.sensorLabel + " , Enabled: True - " + self.spSensors()[i].enabled());
                    } else {
                        self.spSensors()[i].enabled(false); 
                        //console.log(Math.floor(Math.random() * 101) + ":Index:" + i + " , Label:" + data.sensorLabel + " , Enabled: False - " + self.spSensors()[i].enabled());
                    }
                    self.spSensors()[i].actualvalue(data.sensorActualValue);
                    if (data.sensorActive == "1") {
                        self.spSensors()[i].active("Alarm");    
                    } else {
                        self.spSensors()[i].active("Normal"); 
                    }
                    if (data.sensorType == "0") {
                        self.spSensors()[i].type("Digital");
                        if (self.spSensors()[i].availableSP().length == 0) {
                            self.spSensors()[i].availableSP.push(0,1);    
                        }                        
                    } else if (data.sensorType == "1") {
                        self.spSensors()[i].type("NTC Temperature"); 
                        if (self.spSensors()[i].availableSP().length == 0) {
                            self.spSensors()[i].availableSP.push(0,50,100,150,200,210,220,230,240,250,260,270,280,290,300);
                        }
                    }
                    //console.log(self.spSensors()[i].availableSP());
                    self.spSensors()[i].SP(data.sensorSP);

                    self.spSensors()[i].expertMode(self.expertMode);

                    statusStr = data.sensorActualValue;
                    if (data.sensorType == "1") {
                        //NTC sensor
                        statusStr += "°C - "
                    } else {
                        statusStr += " - "
                    }

                    if (data.sensorActive == "1") {
                        if(data.sensorEnabled == "1") {
                            
                            statusStr += "Alarm";
                            colorStr = "orange";
                        } else {
                            statusStr += "Alarm (Disabled)";
                            colorStr = "gray";
                        }                        
                    } else {
                        if(data.sensorEnabled == "1") {
                            statusStr += "OK";
                            colorStr = "green";
                        } else {
                            statusStr += "OK (Disabled)";
                            colorStr = "gray";
                        }                        
                    }
                    self.spSensors()[i].status(statusStr);
                    self.spSensors()[i].color(colorStr);

                }
            } 
            else if (data.type == "interlockUpdate") {
                if (data.interlockStatus == "0") {
                    self.interlock("Normal");
                    self.interlockColor("black");
                    self.tripBtnVisible(false)
                    self.navbarcolor("green");
                    self.navbartitle("Safety Printer: Normal operation");
                } else {
                    self.interlock("TRIP");
                    self.interlockColor("red");
                    self.tripBtnVisible(true);
                    self.navbarcolor("red");
                    self.navbartitle("Safety Printer: TRIP!");
                }
                
            }
            else if (data.type == "connectionUpdate") {
                if (data.connectionStatus) {
                    self.connection("Online");
                    self.connectionColor("black");
                    self.notConnected(false);
                    self.connectedPort(data.port);
                    self.connectionCaption("Disconnect");
                } else {
                    self.connection("Offline");
                    self.connectionColor("red");
                    self.notConnected(true);
                    self.navbarcolor("black");
                    self.navbartitle("Safety Printer: Offline");
                    self.connectionCaption("Connect");
                    var i;
                    for (i = 0; i < 8; i++) {                                         
                        self.spSensors()[i].visible(false);   
                    }
                }
            }
            else if (data.type == "serialPortsUI") {
                self.availablePorts.push(new ItemViewModel(data.port));
                //console.log(self.availablePorts());
            }
            else if (data.type == "terminalUpdate") {

                data.line.replace(/[\n\r]+/g, '');
                self.terminalLines.push(new TerminalViewModel(data.line,data.terminalType));
                self.countTerminalLines++;
                if (self.countTerminalLines > 3600) {
                    self.terminalLines.shift(); //removes the first line
                }

                if (self.autoscrollEnabled()) {
                    self.scrollToEnd();
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
        dependencies: ["settingsViewModel"],
        // Elements to bind to, e.g. #settings_plugin_SafetyPrinter, #tab_plugin_SafetyPrinter, ...
        elements: ["#settings_plugin_SafetyPrinter","#navbar_plugin_SafetyPrinter","#sidebar_plugin_SafetyPrinter"] //
    });
});
