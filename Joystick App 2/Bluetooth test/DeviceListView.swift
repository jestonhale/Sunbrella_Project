import SwiftUI

struct DeviceListView: View {
    @ObservedObject var bleManager: BLEManager
    
    var body: some View {
        List {
            ForEach(bleManager.peripherals, id: \.identifier) { peripheral in
                HStack {
                    Text(peripheral.name ?? "Unknown Device")
                    Spacer()
                    Button("Connect") {
                        bleManager.connect(to: peripheral)
                    }
                }
            }
        }
        .navigationBarTitle("Select a Device")
        .toolbar {
            ToolbarItem(placement: .navigationBarLeading) {
                Button("Refresh") {
                    bleManager.scanForDevices()
                }
            }
        }
    }
}
