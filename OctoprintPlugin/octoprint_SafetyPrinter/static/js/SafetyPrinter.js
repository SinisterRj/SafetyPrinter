/*
 * View model for SafetyPrinter
 *
 * Author: Rodrigo C. C. Silva
 * License: AGPLv3
 */
$(function() {
    function spSensorsType(visible, label, status, color, enabled, active, type, SP, timer) {
        var self = this;
        self.visible = ko.observable(visible);
        self.label = ko.observable(label);
        self.status = ko.observable(status);
        self.color = ko.observable(color);
        self.enabled = ko.observable(enabled);
        self.active = ko.observable(active);
        self.type = ko.observable(type);
        self.SP = ko.observable(SP);
        self.timer = ko.observable(timer);

    }

    function spSensorsSettingsType(visible, checked, label, status, color, actualvalue, enabled, active, type, SP, timer, availableSP, expertMode) {
        var self = this;
        self.visible = ko.observable(visible);
        self.checked = ko.observable(checked);
        self.label = ko.observable(label);
        self.status = ko.observable(status);
        self.color = ko.observable(color);
        self.actualvalue = ko.observable(actualvalue);        
        self.enabled = ko.observable(enabled);
        self.active = ko.observable(active);
        self.type = ko.observable(type);
        self.SP = ko.observable(SP);
        self.timer = ko.observable(timer);
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
        self.interlock = ko.observable("Normal");
        self.interlockColor = ko.observable();
        self.tripBtnVisible = ko.observable();
        self.activeSensors = "";

        self.spSensors = ko.observableArray([
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0"),
           new spSensorsType(false,"","offline","gray",false,false,"0","0","0")
        ]);

        // Settings variables
        self.autoscrollEnabled = ko.observable(true);
        self.expertMode = ko.observable(false);
        self.availablePorts = ko.observableArray();
        self.terminalLines = ko.observableArray();
        self.countTerminalLines = 0;
        self.command = ko.observable();
        self.tabActive = false;
        self.tempMsgFilter = ko.observable(false);
        self.startedFilter = false;
        self.updateSettingsSensors = false;
        self.FWVersion = ko.observable("");
        self.FWReleaseDate = ko.observable("");
        self.FWEEPROM = ko.observable("");
        self.FWCommProtocol = ko.observable("");
        self.FWValidVersion = ko.observable(false);

        self.spSensorsSettings = ko.observableArray([
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false),
           new spSensorsSettingsType(false,false,"","offline","gray","0",false,false,"0","0","0",[],false)
        ]);

        // Navbar variables
        self.navbarcolor = ko.observable("black");
        self.navbartitle = ko.observable("Safety Printer: Offline");

        // General variables
        self.connection = ko.observable("Offline");
        self.connectionColor = ko.observable("red");
        self.notConnected = ko.observable(true);
        self.connectedPort = ko.observable("None");
        self.connectionCaption = ko.observable("Connect");
        self.automaticShutdownEnabled = ko.observable();
        self.newTrip = ko.observable(false);
        self.numOfSensors = 0;


        PNotify.prototype.options.confirm.buttons = [];

        self.tripPopupText = gettext('Printer Emergency Shutdown detected.');
        self.tripPopupOptions = {
            title: gettext('Shutdown'), 
            type: 'error',           
            icon: true,
            hide: false,
            confirm: {
                confirm: true,
                buttons: [{
                    text: 'Ok',
                    addClass: 'btn-block',
                    promptTrigger: true,
                    click: function(notice, value){
                        notice.remove();
                        notice.get().trigger("pnotify.cancel", [notice, value]);
                    }
                }]
            },
            buttons: {
                closer: false,
                sticker: false,
            },
            history: {
                history: false
            }
        };

        self.timeoutPopupText = gettext('Shutting down in ');
        self.timeoutPopupOptions = {
            title: gettext('System Shutdown'),
            type: 'notice',
            icon: true,
            hide: false,
            confirm: {
                confirm: true,
                buttons: [{
                    text: 'Abort Shutdown',
                    addClass: 'btn-block btn-danger',
                    promptTrigger: true,
                    click: function(notice, value){
                        notice.remove();
                        notice.get().trigger("pnotify.cancel", [notice, value]);
                    }
                }]
            },
            buttons: {
                closer: false,
                sticker: false,
            },
            history: {
                history: false
            }
        };

        self.errorPopupText = gettext('Safety Printer ERROR:');
        self.errorPopupOptions = {
            title: gettext('ERROR'), 
            type: 'error',           
            icon: true,
            hide: false,
            confirm: {
                confirm: true,
                buttons: [{
                    text: 'Ok',
                    addClass: 'btn-block',
                    promptTrigger: true,
                    click: function(notice, value){
                        notice.remove();
                        notice.get().trigger("pnotify.cancel", [notice, value]);
                    }
                }]
            },
            buttons: {
                closer: false,
                sticker: false,
            },
            history: {
                history: false
            }
        };

        self.onStartupComplete = function() {
            //Show or hide terminal TAB.
            self.showHideTab();
        };

        
        self.showHideTab = function() {
            // Shows or hides the terminal TAB on UI.
            if ((self.settingsViewModel.settings.plugins.SafetyPrinter.showTerminal() == true) && (!document.getElementById("tab_plugin_SafetyPrinter_link"))) {
                $("<li id='tab_plugin_SafetyPrinter_link' class='' data-bind='allowBindings: true'><a href='#tab_plugin_SafetyPrinter' data-toggle='tab'>Safety Printer</a></li>").appendTo("#tabs");
            } else if (self.settingsViewModel.settings.plugins.SafetyPrinter.showTerminal() == false) {
                $('#tab_plugin_SafetyPrinter_link').remove();
            }
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

        // ************* Update Settings TAB:

        self.onSettingsShown = function() {

            // Update serial ports info. Also called when user clicks on "default Serial" combo box
            self.availablePorts.removeAll();
            self.availablePorts.push(new ItemViewModel("AUTO"));
            OctoPrint.simpleApiCommand("SafetyPrinter", "getPorts");

            for (i = 0; i < self.numOfSensors; i++) {
                                                    
                self.spSensorsSettings()[i].visible(self.spSensors()[i].visible());
                self.spSensorsSettings()[i].visible.valueHasMutated();   //Force knockout to refresh View

                self.spSensorsSettings()[i].type(self.spSensors()[i].type());
                self.spSensorsSettings()[i].type.valueHasMutated();

                self.spSensorsSettings()[i].enabled(self.spSensors()[i].enabled());
                self.spSensorsSettings()[i].enabled.valueHasMutated();
                
                self.spSensorsSettings()[i].active(self.spSensors()[i].active());
                self.spSensorsSettings()[i].active.valueHasMutated();

                self.spSensorsSettings()[i].SP(self.spSensors()[i].SP());
                self.spSensorsSettings()[i].SP.valueHasMutated();

                self.spSensorsSettings()[i].timer(self.spSensors()[i].timer());
                self.spSensorsSettings()[i].timer.valueHasMutated();                
            }            
        };

        self.onSettingsBeforeSave = function() {
            changed = false;
            for (i = 0; i < self.numOfSensors; i++) {                                         

                if (self.spSensorsSettings()[i].enabled() != self.spSensors()[i].enabled()) {
                    if (self.spSensorsSettings()[i].enabled()) {
                        console.log("Enabling: " + self.spSensorsSettings()[i].label())
                        OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: i, onoff: "on"});
                        changed = true;
                    } else {
                        console.log("Disabling: " + self.spSensorsSettings()[i].label())
                        OctoPrint.simpleApiCommand("SafetyPrinter", "toggleEnabled", {id: i, onoff: "off"});
                        changed = true;
                    }                    
                }

                if (self.spSensorsSettings()[i].SP() != self.spSensors()[i].SP()) {
                    console.log("Changing SP: " + self.spSensorsSettings()[i].label())
                    OctoPrint.simpleApiCommand("SafetyPrinter", "changeSP", {id: i, newSP: self.spSensorsSettings()[i].SP()});
                    changed = true;
                }

                if (self.spSensorsSettings()[i].timer() != self.spSensors()[i].timer()) {
                    console.log("Changing Timer: " + self.spSensorsSettings()[i].label())
                    OctoPrint.simpleApiCommand("SafetyPrinter", "changeTimer", {id: i, newTimer: self.spSensorsSettings()[i].timer()});
                    changed = true;
                }
            }
            if (changed) {
                console.log("Writing EEPROM")
                OctoPrint.simpleApiCommand("SafetyPrinter", "saveEEPROM");  
            }
            self.showHideTab();

        }

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
        };

        self.resetBtn = function(item) {
            // Send a command to arduino restore sensor settings
            for (i = 0; i < self.numOfSensors; i++) {                                         
                if (self.spSensorsSettings()[i].checked()) {
                    console.log("Restoring " + self.spSensorsSettings()[i].label() + " default settings.");
                    OctoPrint.simpleApiCommand("SafetyPrinter", "resetSettings", {id: i}); 
                }
            }
            self.updateSettingsSensors = true;            
        };

        // ************* Functions for each button on Terminal TAB:

        self.sendCommandBtn = function() {
            // Send generic user comands from command line on terminal to arduino
            console.log("enviando comando");
            OctoPrint.simpleApiCommand("SafetyPrinter", "sendCommand", {serialCommand: self.command()}); 
        };


        // ************* Functions for each button on Sidebar:

        self.tripResetBtn = function() {
            // Send a command to arduino to reset all trips
            OctoPrint.simpleApiCommand("SafetyPrinter", "resetTrip");
            
            // Remove notification.
            if (typeof self.tripPopup != "undefined") {
                self.tripPopup.remove();
                self.tripPopup = undefined;
            }
        };

        self.onAutomaticShutdownEvent = function() {
            if (self.automaticShutdownEnabled()) {
                OctoPrint.simpleApiCommand("SafetyPrinter", "enableShutdown");
            } else {
                OctoPrint.simpleApiCommand("SafetyPrinter", "disableShutdown");
            }
        }
        self.automaticShutdownEnabled.subscribe(self.onAutomaticShutdownEvent, self);

        // ************* Functions for general buttons:

        self.connectBtn = function() {
            // Connects or disconnects to the Safety Printer Arduino
            if (self.notConnected()) {
                self.connection("Connecting");
                self.connectionColor("");
                OctoPrint.simpleApiCommand("SafetyPrinter", "reconnect");    
            } else {
                self.connection("Disconecting");
                self.connectionColor("");
                self.notConnected(true);
                OctoPrint.simpleApiCommand("SafetyPrinter", "disconnect");    
            }

        };

        // ************* Function to manage plug-in messages

        self.onDataUpdaterPluginMessage = function(plugin, data) {
          
            if (plugin != "SafetyPrinter") {
                return;
            }

            if ((data.type == "statusUpdate") && (self.notConnected() == false)) {
            // Update all sensors status
                var i = parseInt(data.sensorIndex);
                self.numOfSensors = parseInt(data.sensorNumber);
                if (i >=0 || i < 8) {
                                        
                    self.spSensors()[i].visible(true);
                    self.spSensorsSettings()[i].visible(true);                    
                    self.spSensors()[i].label(data.sensorLabel);
                    self.spSensorsSettings()[i].label(data.sensorLabel);
                    if (data.sensorEnabled == "1") {
                        self.spSensors()[i].enabled(true);    
                        //console.log(Math.floor(Math.random() * 101) + ":Index:" + i + " , Label:" + data.sensorLabel + " , Enabled: True - " + self.spSensors()[i].enabled());
                    } else {
                        self.spSensors()[i].enabled(false); 
                        //console.log(Math.floor(Math.random() * 101) + ":Index:" + i + " , Label:" + data.sensorLabel + " , Enabled: False - " + self.spSensors()[i].enabled());
                    }
                    self.spSensorsSettings()[i].actualvalue(data.sensorActualValue);
                    if (data.sensorActive == "1") {
                        self.spSensors()[i].active(true); 
                        self.spSensorsSettings()[i].active(true);   
                    } else {
                        self.spSensors()[i].active(false);
                        self.spSensorsSettings()[i].active(false);    
                    }
                    if (data.sensorType == "0") {
                        self.spSensors()[i].type("Digital");
                        if (self.spSensorsSettings()[i].availableSP().length == 0) {
                            self.spSensorsSettings()[i].availableSP.push(0,1);    
                        }                        
                    } else if (data.sensorType == "1") {
                        self.spSensors()[i].type("NTC Temperature"); 
                        if (self.spSensorsSettings()[i].availableSP().length == 0) {
                            self.spSensorsSettings()[i].availableSP.push(0,50,100,150,200,210,220,230,240,250,260,270,280,290,300);
                        }
                    }
                    self.spSensors()[i].SP(data.sensorSP);
                    self.spSensors()[i].timer(data.sensorTimer);
                    
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
                            colorStr = "green";
                        } else {
                            statusStr += "(Disabled)";
                            colorStr = "gray";
                        }                        
                    }
                    self.spSensors()[i].status(statusStr);
                    self.spSensors()[i].color(colorStr);
                }
                if (self.updateSettingsSensors) {
                    self.updateSettingsSensors = false;
                    self.onSettingsShown();
                } 
            } 

            else if (data.type == "interlockUpdate") {
            // Update interlock (trip) status
                if (data.interlockStatus == "0") {
                    self.interlock("Normal");
                    self.interlockColor("");
                    self.tripBtnVisible(false)

                    self.navbarcolor("green");
                    self.navbartitle("Safety Printer: Normal operation");
                    
                    self.activeSensors = "";
                } else {
                    
                    self.tripPopupOptions.text = self.tripPopupText;
                    for (i = 0; i < self.numOfSensors; i++) {                                         
                        if ((self.spSensors()[i].active()) && (self.spSensors()[i].enabled()) && (self.activeSensors.indexOf(String(self.spSensors()[i].label())) == -1)){
                            self.activeSensors = self.activeSensors + "\n" + String(self.spSensors()[i].label());                                                                       
                        }   
                    }
                    if (self.activeSensors != "") {
                        self.tripPopupOptions.text = self.tripPopupText + "\n\n Alarmed sensors:" + self.activeSensors;                      
                    }
                    if (typeof self.tripPopup != "undefined") {
                        self.tripPopup.update(self.tripPopupOptions);
                    } else {
                        self.tripPopup = new PNotify(self.tripPopupOptions);
                        self.tripPopup.get().on('pnotify.cancel');
                    }

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
                    self.connectionColor("");
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

                    self.FWVersion("");
                    self.FWReleaseDate("");
                    self.FWEEPROM("");
                    self.FWCommProtocol("");
                    self.FWValidVersion(false);

                    var i;
                    for (i = 0; i < self.numOfSensors; i++) {                                         
                        self.spSensors()[i].visible(false);
                        self.spSensorsSettings()[i].visible(false);   
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
            else if (data.type == "shutdown") {
                self.automaticShutdownEnabled(data.automaticShutdownEnabled);
                if ((data.timeout_value != null) && (data.timeout_value > 0)) {
                    self.timeoutPopupOptions.text = self.timeoutPopupText + data.timeout_value;
                    if (typeof self.timeoutPopup != "undefined") {
                        self.timeoutPopup.update(self.timeoutPopupOptions);
                    } else {
                        self.timeoutPopup = new PNotify(self.timeoutPopupOptions);
                        self.timeoutPopup.get().on('pnotify.cancel', function() {self.abortShutdown(true);});
                    }
                } else {
                    if (typeof self.timeoutPopup != "undefined") {
                        self.timeoutPopup.remove();
                        self.timeoutPopup = undefined;
                    }
                }
            }
            else if (data.type == "firmwareInfo") {
                self.FWVersion(data.version);
                self.FWReleaseDate(data.releaseDate);
                self.FWEEPROM(data.EEPROM);
                self.FWCommProtocol(data.CommProtocol);
                self.FWValidVersion(data.ValidVersion);
            }
            else if (data.type == "error") {

                self.errorPopupOptions.text = data.errorMsg;
                self.errorPopup = new PNotify(self.errorPopupOptions);
                self.errorPopup.get().on('pnotify.cancel');
            }

        };
        
        self.abortShutdown = function(abortShutdownValue) {
            self.timeoutPopup.remove();
            self.timeoutPopup = undefined;
            OctoPrint.simpleApiCommand("SafetyPrinter", "abortShutdown");
        }
    }

    OCTOPRINT_VIEWMODELS.push({
        construct: SafetyprinterViewModel,
        // ViewModels your plugin depends on, e.g. loginStateViewModel, settingsViewModel, ...
        dependencies: ["settingsViewModel"],
        // Elements to bind to, e.g. #settings_plugin_SafetyPrinter, #tab_plugin_SafetyPrinter, ...
        elements: ["#settings_plugin_SafetyPrinter","#navbar_plugin_SafetyPrinter","#sidebar_plugin_SafetyPrinter","#tab_plugin_SafetyPrinter"] //
    });
});
