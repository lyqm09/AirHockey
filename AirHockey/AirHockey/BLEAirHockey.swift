//
//  BLEAirHockey.swift
//  AirHockey
//
//  Created by Lyam Maes on 12/05/2023.
//

//https://github.com/robkerr/BlogProjects/blob/main/BLECalculator/BLECalculator/View%20Models/CalculatorViewModel.swift
//https://github.com/robkerr/BlogProjects/blob/main/BLECalculator/basic_ble_peripheral.ino
//https://robkerr.com/posts/swift/ble-peripheral-ios-development-part1/
//https://pastebin.com/v2Qmp1vr

import Foundation
import CoreBluetooth

class BLEAirHockey: NSObject, ObservableObject, Identifiable {
    var uuid: UUID = UUID();
    
    //MARK: - Interface
    @Published var isPoweredOn = false;
    @Published var connected: Bool = false;
    @Published var score: [UInt8] = [0xFF,0xFF];
    
    func send(_ args: [UInt8]) {
        guard let peripheral = self.peripheral,
              let inputChar = self.inputChar
        else {return;}
        
        peripheral.writeValue(Data(args), for: inputChar, type: .withoutResponse);
    }
    
    //MARK: - BLE
    private var centralQueue: DispatchQueue?
    
    private let serviceUUID: CBUUID = CBUUID(string: "A71120B3-6254-44BD-A27F-42A66F1732BA");
    private let inputCharUUID: CBUUID = CBUUID(string: "E3B3CADD-710C-4A3D-9030-CB99823CB738");
    private var inputChar: CBCharacteristic?
    private let outputCharUUID: CBUUID = CBUUID(string: "32FE11C3-1807-4D05-81B3-4A301BDC3928");
    private var outputChar: CBCharacteristic?
    
    private var centralManager: CBCentralManager?
    private var peripheral: CBPeripheral?
    
    override init() {
        super.init();
        self.centralQueue = DispatchQueue(label: "test Discovery");
        self.centralManager = CBCentralManager(delegate: self, queue: centralQueue);
    }
    
    func disconnectAirHockey() {
        guard let manager = self.centralManager,
              let peripheral = self.peripheral else {return;}
        
        manager.cancelPeripheralConnection(peripheral);
    }
    
}

extension BLEAirHockey: CBCentralManagerDelegate {
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("Central Manager state changed: \(central.state)");
        
        if(central.state == .poweredOn) {
            DispatchQueue.main.async {
                self.isPoweredOn = true;
            }
            central.scanForPeripherals(withServices: [self.serviceUUID], options: nil);
        } else {
            DispatchQueue.main.async {
                self.isPoweredOn = false;
            }
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        print("Discovered \(peripheral.name ?? "UNKNOWN")");
        
        central.stopScan();
        
        self.peripheral = peripheral;
        central.connect(peripheral, options: nil);
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to \(peripheral.name ?? "UNKNOWN")");
        
        peripheral.delegate = self;
        peripheral.discoverServices(nil);
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("Disconnected from \(peripheral.name ?? "UNKNOWN")");
        
        self.centralManager = nil;
        
        DispatchQueue.main.async {
            self.connected = false;
        }
    }
    
}

extension BLEAirHockey: CBPeripheralDelegate {
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        print("Discovered services for \(peripheral.name ?? "UNKNOWN")");
        
        guard let services = peripheral.services else {return;}
        
        for service in services {
            peripheral.discoverCharacteristics(nil, for: service);
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        print("Discovered characteristics for \(peripheral.name ?? "UNKNOWN")");

        guard let characteristics = service.characteristics else {return;}
        
        for characteristic in characteristics {
            switch characteristic.uuid {
            case self.inputCharUUID:
                inputChar = characteristic;
            case self.outputCharUUID:
                outputChar = characteristic;
                peripheral.setNotifyValue(true, for: characteristic)
            default:
                break;
            }
        }
        
        DispatchQueue.main.async {
            self.connected = true;
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        print("Notification state changed to \(characteristic.isNotifying)");
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        print("Characteristic updated: \(characteristic.uuid)");
        
        if(characteristic.uuid == self.outputCharUUID), let data = characteristic.value {
            let bytes:[UInt8] = data.map {$0};

            print(bytes);
            
            if(bytes.count >= 2) {
                DispatchQueue.main.async {
                    self.score = [bytes[0], bytes[1]];
                }
            }
        }
    }
    
}
