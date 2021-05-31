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
        
        // Sidebar variables
        self.interlock = ko.observable();
        self.interlockColor = ko.observable();
        self.tripBtnVisible = ko.observable();

        // Settings variables
        self.autoscrollEnabled = ko.observable(true);
        self.expertMode = ko.observable(false);
        self.availablePorts = ko.observableArray();
        self.terminalLines = ko.observableArray();
        self.countTerminalLines = 0;
        self.command = ko.observable();
        self.tabActive = false;
        self.oldSP = ["0","0","0","0","0","0","0","0"];
        self.tempMsgFilter = ko.observable(false);
        self.startedFilter = false;

        // Navbar variables
        self.navbarcolor = ko.observable("black");
        self.navbartitle = ko.observable("Safety Printer: Offline");

        // General variables
        self.connection = ko.observable("Offline");
        self.connectionColor = ko.observable("red");
        self.notConnected = ko.observable(true);
        self.connectedPort = ko.observable("None");
        self.connectionCaption = ko.observable("Connect");

        self.spSensors = ko.observableArray([
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false),
           new spSensorsType(false,"","offline","gray","0",false,false,"0","0",[],false)
        ]); 

        self.onStartupComplete = function() {
            // Update serial ports info. Also called when user clicks on "default Serial" combo box
            self.availablePorts.removeAll();
            self.availablePorts.push(new ItemViewModel("AUTO"));
            OctoPrint.simpleApiCommand("SafetyPrinter", "getPorts");
        };

        // ************* Functions to autoscrool the terminal:

        self.terminalScrollEvent = _.throttle(function () {
            // If user scrolls the terminal, it stops the autoscrolling
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

        self.gotoTerminalCommand = function () {
            // skip if user highlights text.
            var sel = getSelection().toString();
            if (sel) {
                self.autoscrollEnabled(false);
                return;
            }

            $("#SPterminal-command").focus();
        };

        self.onAfterTabChange = function (current, previous) {
            self.tabActive = current === "#term";
            self.updateOutput();
        };


        self.updateOutput = function () {
            if (
                self.tabActive &&
                OctoPrint.coreui.browserTabVisible &&
                self.autoscrollEnabled()
            ) {
                self.scrollToEnd();
            }
        };

        self.scrollToEnd = function () {
            var container = $("#SafetyPrinterTerminal");
            if (container.length) {
                container.scrollTop(container[0].scrollHeight);
            }
        };

        // ************* Functions for each button on Settings TAB:

        self.toggleAutoscrollBtn = function() {
            // Changes the setings terminal autoscroll to ON or OFF
            self.autoscrollEnabled(!self.autoscrollEnabled());

            if (self.autoscrollEnabled()) {
                self.updateOutput();
            }
        };

        self.toggleFilterBtn = function() {
            // enable or disable <R1> messages on terminal
            self.tempMsgFilter(!self.tempMsgFilter());
        }

        self.sendCommandBtn = function() {
            // Send generic user comands from command line to arduino
            OctoPrint.simpleApiCommand("SafetyPrinter", "sendCommand", {serialCommand: self.command()}); 
        }

        self.toggleEnabledBtn = function(item) {
            // Send a command to arduino to enable or disable any specific sensor
            if (self.spSensors()[self.spSensors.indexOf(item)].enabled()) {
                OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: self.spSensors.indexOf(item), onoff: "off"});
            } else {
                OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: self.spSensors.indexOf(item), onoff: "on"});
            }
        };

        self.changeSPSel = function(item) {
            // Send a command to arduino to change a specific sensor alarm set point
            if (self.oldSP[self.spSensors.indexOf(item)] != self.spSensors()[self.spSensors.indexOf(item)].SP()) {
                //Avoid to send the command if the user just clicks on the selector, but select the same SP.
                OctoPrint.simpleApiCommand("SafetyPrinter", "changeSP", {id: self.spSensors.indexOf(item), newSP: self.spSensors()[self.spSensors.indexOf(item)].SP()});
            }
        };

        self.saveEEPROMBtn = function() {
            // Send a command to arduino to write changes on "enable" and "set point" sensor properties on EEPROM
            OctoPrint.simpleApiCommand("SafetyPrinter", "saveEEPROM");  
        }

        // ************* Functions for each button on Sidebar:

        self.tripResetBtn = function() {
            // Send a command to arduino to reset all trips
            OctoPrint.simpleApiCommand("SafetyPrinter", "resetTrip");
        };

        self.statusBtn = function() {

        };

        // ************* Functions for general buttons:

        self.connectBtn = function() {
            // Connects or disconnects to the Safety Printer Arduino
            if (self.notConnected()) {
                self.connection("Connecting");
                self.connectionColor("black");
                OctoPrint.simpleApiCommand("SafetyPrinter", "reconnect");    
            } else {
                self.connection("Disconecting");
                self.connectionColor("black");
                self.notConnected(true);
                OctoPrint.simpleApiCommand("SafetyPrinter", "disconnect");    
            }

        };

        // ************* Function to manage plug-in messages

        self.onDataUpdaterPluginMessage = function(plugin, data) {
          
            if (plugin != "SafetyPrinter") {
                return;
            }
            //console.log(data.type);

            if ((data.type == "statusUpdate") && (self.notConnected() == false)) {
            // Update all sensors status
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
                        self.spSensors()[i].active(true);    
                    } else {
                        self.spSensors()[i].active(false); 
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
                    self.spSensors()[i].SP(data.sensorSP);
                    self.oldSP[i] = data.sensorSP; 
                    self.spSensors()[i].expertMode(self.expertMode);
                    //statusStr = data.sensorActualValue;
                    if (data.sensorType == "1") {
                        //NTC sensor
                        statusStr = data.sensorActualValue + "°C "
                    } else {
                        statusStr = ""
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
                            //statusStr += "OK";
                            colorStr = "green";
                        } else {
                            statusStr += "(Disabled)";
                            colorStr = "gray";
                        }                        
                    }
                    self.spSensors()[i].status(statusStr);
                    self.spSensors()[i].color(colorStr);
                }
            } 

            else if (data.type == "interlockUpdate") {
            // Update interlock (trip) status
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
            // Update connection status
                if (data.connectionStatus) {
                    self.connection("Online");
                    self.connectionColor("black");
                    self.connectionCaption("Disconnect");
                    self.notConnected(false);
                    self.connectedPort(data.port);
                    
                } else {
                    self.connection("Offline");
                    self.connectionColor("red");
                    self.connectionCaption("Connect");
                    self.notConnected(true);                    

                    self.navbarcolor("black");
                    self.navbartitle("Safety Printer: Offline");

                    var i;
                    for (i = 0; i < 8; i++) {                                         
                        self.spSensors()[i].visible(false);   
                    }
                }
            }

            else if (data.type == "serialPortsUI") {
            // Update list of serial ports available
                self.availablePorts.push(new ItemViewModel(data.port));
            }

            else if (data.type == "terminalUpdate") {
            // Update messages displayed on settings terminal
                
                data.line.replace(/[\n\r]+/g, '');
                if (!self.tempMsgFilter() || (data.line.search("R1") == -1)) {
                    self.startedFilter = false;
                    self.terminalLines.push(new TerminalViewModel(data.line,data.terminalType));
                    self.countTerminalLines++;

                } else if (!self.startedFilter) {
                    self.startedFilter = true;
                    self.terminalLines.push(new TerminalViewModel("[...]",""));
                    self.countTerminalLines++;

                }
                if (self.countTerminalLines > 3600) {
                    self.terminalLines.shift(); //removes the first line
                }
                if (self.autoscrollEnabled()) {
                    self.scrollToEnd();
                }
            }
        };
    }

    OCTOPRINT_VIEWMODELS.push({
        construct: SafetyprinterViewModel,
        // ViewModels your plugin depends on, e.g. loginStateViewModel, settingsViewModel, ...
        dependencies: ["settingsViewModel"],
        // Elements to bind to, e.g. #settings_plugin_SafetyPrinter, #tab_plugin_SafetyPrinter, ...
        elements: ["#settings_plugin_SafetyPrinter","#navbar_plugin_SafetyPrinter","#sidebar_plugin_SafetyPrinter"] //
    });
});
