import CoreBluetooth
import SwiftUI

class BLEManager: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var isSwitchedOn = false
    @Published var peripherals = [CBPeripheral]()
    @Published var isConnected = false
    @Published var showingDeviceList = false

    var centralManager: CBCentralManager!
    var selectedPeripheral: CBPeripheral?
    var motorControlCharacteristic: CBCharacteristic?
    var movementTimer: Timer?

    enum Direction {
        case up, down, left, right
    }

    let motorServiceUUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")  // Update UUIDs as necessary
    let motorControlCharacteristicUUID = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")

    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        if central.state == .poweredOn {
            isSwitchedOn = true
            scanForDevices()
        } else {
            isSwitchedOn = false
        }
    }

    func scanForDevices() {
        centralManager.scanForPeripherals(withServices: [motorServiceUUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: NSNumber(value: false)])
    }

    func connect(to peripheral: CBPeripheral) {
        selectedPeripheral = peripheral
        selectedPeripheral?.delegate = self
        centralManager.connect(selectedPeripheral!)
        centralManager.stopScan()
    }

    func disconnect() {
        if let peripheral = selectedPeripheral {
            centralManager.cancelPeripheralConnection(peripheral)
        }
        isConnected = false
        selectedPeripheral = nil
        sendCommand("APP_DISCONNECTED")
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        isConnected = true
        peripheral.discoverServices([motorServiceUUID])
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String: Any], rssi RSSI: NSNumber) {
        if !peripherals.contains(where: { $0.identifier == peripheral.identifier }) {
            peripherals.append(peripheral)
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        
        for service in services where service.uuid == motorServiceUUID {
            peripheral.discoverCharacteristics([motorControlCharacteristicUUID], for: service)
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        
        for characteristic in characteristics where characteristic.uuid == motorControlCharacteristicUUID {
            motorControlCharacteristic = characteristic
            sendCommand("APP_CONNECTED")
        }
    }

    // New functions for continuous movement
    func startMoving(direction: Direction) {
        movementTimer?.invalidate()
        sendDirectionCommand(direction: direction)
        movementTimer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { [weak self] _ in
            self?.sendDirectionCommand(direction: direction)
        }
    }

    func stopMoving() {
        movementTimer?.invalidate()
        sendCommand("STOP")
    }

    private func sendDirectionCommand(direction: Direction) {
        let command: String
        switch direction {
        case .up:
            command = "MOTOR1:1:100"  // Adjust speed value as necessary
        case .down:
            command = "MOTOR1:-1:100"  // Adjust speed value as necessary
        case .left:
            command = "MOTOR2:1:100"  // Adjust speed value as necessary
        case .right:
            command = "MOTOR2:-1:100"  // Adjust speed value as necessary
        }
        sendCommand(command)
    }

    private func sendCommand(_ command: String) {
        if let characteristic = motorControlCharacteristic, let peripheral = selectedPeripheral {
            let data = command.data(using: .utf8)
            peripheral.writeValue(data!, for: characteristic, type: .withoutResponse)
            print("Sent command: \(command)")
        } else {
            print("Could not send command: Bluetooth characteristic not found")
        }
    }
    
    // New function to send "RESET" command
        func sendResetCommand() {
            sendCommand("RESET")
        }

        // New function to send "AUTOMODE" command
        func sendAutoModeCommand() {
            sendCommand("AUTOMODE")
        }
}
