//
//  ContentView.swift
//  EEProm Programmer
//
//  Created by Paolo Godino on 13/06/23.
//

import SwiftUI
import ORSSerial

extension Unicode.Scalar {
  var isPrintable: Bool {
    switch properties.generalCategory {
    case .uppercaseLetter, .lowercaseLetter, .titlecaseLetter,
      /* ... */
      .mathSymbol, .currencySymbol, .modifierSymbol, .otherSymbol:
        return true
    default:
      return false
    }
  }
}

func getFile(_ fileUrl: URL) -> [UInt8]? {
    do {
        let rawData: Data = try Data(contentsOf: fileUrl)

        // Return the raw data as an array of bytes.
        return [UInt8](rawData)
    } catch {
        // Couldn't read the file.
        return nil
    }
}

class DataArray {
    var dataArray = Array(repeating: Array<UInt8>(repeating: 0, count: 16), count: 2048)
    var dataAsciiArray = Array(repeating: Array(repeating: "", count: 16), count: 2048)
    
    func createArrays(_ bytes: [UInt8]) {
        
    }
}

struct ContentView: View {
    @StateObject private var serialHandler = SerialHandler()
    
    @State private var selectedPortIndex = 0
    @State private var selectedPortName = ""
    
    @State var filename: String? = nil
    
    @State var isLoading = false
    
    @State var dataStringArray = Array<String>()
    
    @State private var numReadBanks: String = "20"
  
    var body: some View {
        VStack {
            HStack {
                Picker("Scegli la porta", selection: $selectedPortIndex) {
                    ForEach(0 ..< self.serialHandler.availablePortsPath.count, id: \.self) { index in
                        Text(self.serialHandler.availablePortsPath[index]).tag(index)
                    }
                }
                .frame(width: 300)
                .onChange(of: selectedPortIndex) { _ in
                    selectedPortName = self.serialHandler.availablePortsPath[selectedPortIndex]
                }
                
                Button(self.serialHandler.serialIsOpen ? "Chiudi" : "Apri") {
                    self.serialHandler.openCloseSerialPort(index: selectedPortIndex)
                }
                
                Button("Leggi") {
                    if (self.serialHandler.serialIsOpen) {
                        dataStringArray.removeAll()
                        
                        filename = nil
                        isLoading = true
                       
                        self.serialHandler.readData(readSize: UInt16((Int(numReadBanks) ?? 20)*16))
                    }
                }
                .padding(.leading, 20)
                
                Text("Banchi da leggere")
                TextField("", text: $numReadBanks)
                    .frame(width: 40)
                    .fixedSize()
                
                Button("Carica...") {
                  let panel = NSOpenPanel()
                  panel.allowsMultipleSelection = false
                  panel.canChooseDirectories = false
                  if panel.runModal() == .OK {
                      isLoading = true
                      self.filename = panel.url?.lastPathComponent ?? "<none>"
                      
                      if let url=panel.url, let bytes: [UInt8] = getFile(url) {
                          createArray(bytes: bytes, performCopy: true)
                          isLoading = false
                      }
                   }
                }
                .padding(.leading, 30)
                
                Button("Scrivi") {
                    if (self.serialHandler.serialIsOpen) {
                        dataStringArray.removeAll()
                        
                        self.serialHandler.writeData()
                    }
                }
                .padding(.leading, 10)
                .disabled(filename == nil)
            }
            .frame(maxWidth: .infinity, alignment: .topLeading)
            
            GeometryReader { geometry in
                ZStack(alignment: .center) {
                    ScrollView(.vertical) {
                        VStack {
                            ForEach (0..<dataStringArray.count, id: \.self) { bank in
                                Text(dataStringArray[bank])
                                    .font(.system(size: 14, design: .monospaced))
                            }
                        }
                    }
                    .frame(maxWidth: .infinity, alignment: .topLeading)
                    
                    if (isLoading) {
                        ProgressView()
                    }
                }
            }
        }
        .padding()
        .onReceive(serialHandler.$loadFinished) { finished in
            if (finished) {
                self.createArray(bytes: serialHandler.dataArray)
                self.isLoading = false
            }
        }
        .onReceive(serialHandler.$endWriting) { endWrite in
            if (endWrite) {
                filename = nil
            }
        }
        .onReceive(serialHandler.$availablePortsPath) { availables in
            selectedPortIndex = 0
            for name in availables {
                if (name == selectedPortName) {
                    selectedPortIndex = availables.firstIndex(of: name)!
                    break
                }
            }
        }
        
    }

    func createArray(bytes: [UInt8], performCopy:Bool = false) {
        var shouldStop = false
        
        dataStringArray.removeAll()
        
        if performCopy {
            serialHandler.dataArray.removeAll()
        }
        
        var buildLine = ""
        for bank in 0..<2048 {
          //  dataStringArray[bank] = String(format: "%04X   ",  bank*16)
            buildLine = String(format: "%04X   ",  bank*16)
            for i in 0..<16 {
                if bank*16 + i < bytes.count {
                  //  dataStringArray[bank] += String(format: "%02X ",  bytes[bank*16 + i])
                    buildLine += String(format: "%02X ",  bytes[bank*16 + i])
                    
                    if performCopy {
                        serialHandler.dataArray.append(bytes[bank*16 + i])
                    }
                } /*else {
                    dataStringArray[bank] += "xx "
                } */
            }
            
            for i in 0..<16 {
                if bank*16 + i < bytes.count {
                    if UnicodeScalar(bytes[bank*16 + i]).isPrintable {
                       // dataStringArray[bank] += String(Character(UnicodeScalar(bytes[bank*16 + i])))
                        buildLine += String(Character(UnicodeScalar(bytes[bank*16 + i])))
                    } else {
                      //  dataStringArray[bank] += "."
                        buildLine += "."
                    }
                } else {
                    shouldStop = true
                  /*  if !performCopy {
                     //   dataStringArray[bank] = ""
                        buildLine.append("")
                    }
                    return */
                   // dataStringArray[bank] += "."
                }
            }
            
            if buildLine.count > 7 {
                dataStringArray.append(buildLine)
            }
            
            if (shouldStop) {
                break
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
       
    }
}
