/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "InsiBox", "index.html", [
    [ "InsuBox Documentation", "index.html", "index" ],
    [ "InsuBox Bluetooth Interface Specification", "BleInterfaceSpec.html", [
      [ "Introduction", "BleInterfaceSpec.html#autotoc_md0", null ],
      [ "Bluetooth Device Profile", "BleInterfaceSpec.html#autotoc_md1", null ],
      [ "Insulin Delivery Service", "BleInterfaceSpec.html#insulin-delivery-service", null ],
      [ "Device Information service", "BleInterfaceSpec.html#device-information-service", null ],
      [ "Current Time Service", "BleInterfaceSpec.html#current-time-service", null ],
      [ "Insulin Delivery Automation Service", "BleInterfaceSpec.html#insulin-delivery-automation-service", [
        [ "IDD Automation Status", "BleInterfaceSpec.html#idas-idd-automation-status", [
          [ "AID State", "BleInterfaceSpec.html#idas-aid-state", null ],
          [ "AID Suspend Remaining Time", "BleInterfaceSpec.html#idas-aid-suspend-remaining-time", null ],
          [ "Last Insulin recommendation", "BleInterfaceSpec.html#idas-last-insulin-recommendation", null ],
          [ "Flags", "BleInterfaceSpec.html#idas-flags", null ]
        ] ],
        [ "IDD Automation Control Point", "BleInterfaceSpec.html#idas-idd-automation-control-point", [
          [ "IDD Automation Control Point procedure requirements", "BleInterfaceSpec.html#idas-idd-automation-control-point-procedure-requirements", null ],
          [ "Set AID State", "BleInterfaceSpec.html#idas-set-aid-state", [
            [ "Op Code", "BleInterfaceSpec.html#idas-set-aid-state-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#idas-set-aid-state-operand", null ]
          ] ],
          [ "Set Algorithm Mode", "BleInterfaceSpec.html#idas-set-algorithm-mode", [
            [ "Op Code", "BleInterfaceSpec.html#idas-set-algorithm-mode-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#idas-set-algorithm-mode-operand", null ]
          ] ],
          [ "Get AID State", "BleInterfaceSpec.html#idas-get-aid-state", [
            [ "Op Code", "BleInterfaceSpec.html#idas-get-aid-state-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#idas-get-aid-state-operand", null ]
          ] ],
          [ "Get Algorithm Mode", "BleInterfaceSpec.html#idas-get-algorithm-mode", [
            [ "Op Code", "BleInterfaceSpec.html#idas-get-algorithm-mode-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#idas-get-algorithm-mode-operand", null ]
          ] ]
        ] ],
        [ "IDD Automation Command Data", "BleInterfaceSpec.html#idas-idd-automation-command-data", [
          [ "Response Op Code", "BleInterfaceSpec.html#idas-response-op-code", null ],
          [ "Operand", "BleInterfaceSpec.html#idas-operand", [
            [ "Operand of Get AID State", "BleInterfaceSpec.html#idas-operand-get-aid-state", null ],
            [ "Operand of Get Algorithm Mode", "BleInterfaceSpec.html#idas-operand-get-algorithm-mode", null ]
          ] ]
        ] ]
      ] ],
      [ "InsuBox Settings Service", "BleInterfaceSpec.html#insubox-settings-service", [
        [ "InsuBox Settings Control Point", "BleInterfaceSpec.html#iss-insubox-settings-control-point", [
          [ "InsuBox Settings Control Point procedure requirements", "BleInterfaceSpec.html#iss-insubox-settings-control-point-procedure-requirements", null ],
          [ "Set Dexcom ID", "BleInterfaceSpec.html#iss-set-dexcom-id", [
            [ "Op Code", "BleInterfaceSpec.html#iss-set-dexcom-id-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#iss-set-dexcom-id-operand", null ]
          ] ],
          [ "Get Dexcom ID", "BleInterfaceSpec.html#iss-get-dexcom-id", [
            [ "Op Code", "BleInterfaceSpec.html#iss-get-dexcom-id-op-code", null ],
            [ "Operand", "BleInterfaceSpec.html#iss-get-dexcom-id-operand", null ]
          ] ]
        ] ],
        [ "InsuBox Settings Data", "BleInterfaceSpec.html#iss-insubox-settings-data", [
          [ "Response Op Code", "BleInterfaceSpec.html#iss-response-op-code", null ],
          [ "Operand", "BleInterfaceSpec.html#iss-operand", [
            [ "Operand of Get Dexcom ID", "BleInterfaceSpec.html#iss-operand-get-dexcom-id", null ]
          ] ]
        ] ]
      ] ],
      [ "Battery Service", "BleInterfaceSpec.html#battery-service", null ],
      [ "Bond Management Service", "BleInterfaceSpec.html#bond-management-service", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"AuthPacket_8cpp.html",
"classCommandType.html#acd440488cc03501502486fb0d465b015a8e76d9ad1535bf678d04635a14c1e77c",
"classSetTempBasalPacketTest.html#a47c08a227ad4dc9c9f37d56654282a57"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';