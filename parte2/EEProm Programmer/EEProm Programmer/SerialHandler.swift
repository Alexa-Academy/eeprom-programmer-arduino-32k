//
//  SerialHandler.swift
//  EEProm Programmer
//
//  Created by Paolo Godino on 13/06/23.
//

// Ricordarsi di aggiungere nell'Entitlement File la riga:
// com.apple.security.device.serial   BOOLEAN  true

import Foundation
import ORSSerial

enum SmState {
    case STATE_START
    case STATE_GOT_AA
    case STATE_GOT_COMMAND
    case STATE_GOT_LENGHT_HIGH
    case STATE_GOT_LENGHT_LOW
}

final class SerialHandler: NSObject, ORSSerialPortDelegate, ObservableObject {
    @Published var serialIsOpen = false
    @Published var availablePortsPath = [String]()
    
    private  let serialPortManager = ORSSerialPortManager.shared()
    
    private var currentSelectedPortIndex = 0

    var serialPort: ORSSerialPort?
    
    private var smState = SmState.STATE_START
    
    var cmdLenghtHigh: Int = 0
    
    //var dataArray = Array<UInt8>(repeating: 0, count: 32768)
    var dataArray = Array<UInt8>()
    
    @Published var loadFinished = false
    @Published var isLoading = false
    @Published var endWriting = false
   
    var command: UInt8 = 0
    
    var cmdLenght: Int = 0

    
    override init() {
        super.init()
        
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(serialPortsWereConnected(_:)), name: NSNotification.Name.ORSSerialPortsWereConnected, object: nil)
        nc.addObserver(self, selector: #selector(serialPortsWereDisconnected(_:)), name: NSNotification.Name.ORSSerialPortsWereDisconnected, object: nil)
        
        loadAvailablePorts()
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
    
    @objc func serialPortsWereConnected(_ notification: Notification) {
        loadAvailablePorts()
    }
    
    @objc func serialPortsWereDisconnected(_ notification: Notification) {
        loadAvailablePorts()
    }
    
    func smProgress(_ byteRead: UInt8) {
      //  print( String(format:"%02X", byteRead))
        switch (smState) {
            case .STATE_START:
                cmdLenght = 0
               // commandRead.removeAll()
                dataArray.removeAll()
                loadFinished = false
                isLoading = true
                if (byteRead == 0xAA) {
                    smState = .STATE_GOT_AA
                }

            case .STATE_GOT_AA:
                smState = .STATE_GOT_COMMAND
                switch (byteRead) {
                    case 0x30:
                        command = byteRead
                    case 0x31:
                        endWriting = false
                        command = byteRead
                    default:
                        smState = .STATE_START
                        cmdLenght = 0
                        dataArray.removeAll()
                       // commandRead.removeAll()
                }
            
            case .STATE_GOT_COMMAND:
                cmdLenghtHigh = Int(byteRead)
                smState = .STATE_GOT_LENGHT_HIGH
            
            case .STATE_GOT_LENGHT_HIGH:
              //  arrayIdx = 0
                cmdLenght = cmdLenghtHigh<<8 + Int(byteRead)
                smState = .STATE_GOT_LENGHT_LOW
                if command == 0x31 {
                    smState = .STATE_START
                    cmdLenght = 0
                    endWriting = true
                    isLoading = false
                    print("Scrittura eseguita")
                }
            
            case .STATE_GOT_LENGHT_LOW:
              //  if (arrayIdx < cmdLenght) {
                if (dataArray.count < cmdLenght) {
                    dataArray.append(byteRead)
                }
                
                if (dataArray.count == cmdLenght) {
                    loadFinished = true
                    smState = .STATE_START
                    cmdLenght = 0
                   // dataArray.removeAll()
                    isLoading = false
                }
        }
    }
    
    func serialPortWasRemovedFromSystem(_ serialPort: ORSSerialPort) {
        if serialPort.path == availablePortsPath[currentSelectedPortIndex] {
            serialPort.close()
            serialIsOpen = false
        }
      
        loadAvailablePorts()
    }
    
    func openCloseSerialPort(index: Int) {
        currentSelectedPortIndex = index
        
        isLoading = false
        
        if serialIsOpen {
            serialPort?.close()
            serialIsOpen = false
        } else {
            if let sp = ORSSerialPort(path: availablePortsPath[index]) {
                self.serialPort = sp
                
                sp.baudRate = 115200
                sp.numberOfDataBits = 8
                sp.parity = .none
                sp.numberOfStopBits = 1
                sp.usesRTSCTSFlowControl = false
                //sp.usesDTRDSRFlowControl = false
               // sp.usesDCDOutputFlowControl = false
                
               // sp.rts = false
              //  sp.dtr = false
                
                sp.open()
                sp.delegate = self
                
                serialIsOpen = sp.isOpen
                
                if sp.isOpen {
                    print("openSerialPort open")
                } else {
                    print("openSerialPort not open")
                }
            } else {
                serialIsOpen = false
                
                print("openSerialPort ERROR")
            }
        }
    }
    
    private func loadAvailablePorts() {
        let availablePorts = serialPortManager.availablePorts
        
        self.availablePortsPath.removeAll()
        
        for port in availablePorts {
            availablePortsPath.append(port.path)
        }
    }
    
    func serialPort(_ serialPort: ORSSerialPort, didEncounterError error: Error) {
        isLoading = false
        print(error.localizedDescription)
    }
    
    func serialPortWasOpened(_ serialPort: ORSSerialPort) {
           /* let descriptor = ORSSerialPacketDescriptor(prefixString: "I", suffixString: "\n", maximumPacketLength: 8, userInfo: nil)
            serialPort.startListeningForPackets(matching: descriptor) */
        }
    
    func serialPortWasClosed(_ serialPort: ORSSerialPort) {
        print("serialPortWasClosed")
        serialIsOpen = false
    }
    
    func serialPort(_ serialPort: ORSSerialPort, didReceive data: Data) {
        let recBytes = data.bytes
    
        recBytes.forEach() { recByte in
            smProgress(recByte)
         //   print( String(format:"%02X", recByte))
        }
    }
    
    private func sendCommand(_ bytes: [UInt8]) {
        let dataToSend = Data(bytes)
        
        if let sp = serialPort, sp.isOpen {
            print("Comando inviato")
            sp.send(dataToSend)
        } else {
            print("Seriale non connessa")
        }
    }
    
    func readData(readSize: UInt16) {
        let hiSize = UInt8((readSize & 0xFF00)>>8)
        let loSize = UInt8(readSize & 0x00FF)
        
        let bytesToSend: [UInt8] = [0xAA, 0x30, hiSize, loSize]
        sendCommand(bytesToSend)
    }
    
    func writeData() {
        var bytesToSend: [UInt8] = [0xAA, 0x31, 0x00, 0x00]
        for dataByte in dataArray {
            bytesToSend.append(dataByte)
        }
        
        let lenght = dataArray.count
        bytesToSend[2] = UInt8((lenght & 0xFF00)>>8)
        bytesToSend[3] = UInt8(lenght & 0x00FF)
        
        sendCommand(bytesToSend)
    }
}

extension Data {
    var bytes: [UInt8] {
        return [UInt8](self)
    }
    
    struct HexEncodingOptions: OptionSet {
        let rawValue: Int
        static let upperCase = HexEncodingOptions(rawValue: 1 << 0)
    }

    func hexEncodedString(options: HexEncodingOptions = []) -> String {
        let format = options.contains(.upperCase) ? "%02hhX" : "%02hhx"
        return self.map { String(format: format, $0) }.joined()
    }
}
