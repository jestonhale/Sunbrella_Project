import SwiftUI

struct ContentView: View {
    @ObservedObject var bleManager = BLEManager()
    @State private var isAuthenticated = false

    var body: some View {
        NavigationView {
            if isAuthenticated {
                if bleManager.isConnected {
                    VStack {
                        // Main control view with directional buttons
                        MainBLEView(bleManager: bleManager)

                        // New Reset Button (Orange)
                        Button(action: {
                            bleManager.sendResetCommand()
                        }) {
                            Text("Reset")
                                .frame(width: 200, height: 50)
                                .background(Color.orange)
                                .foregroundColor(.white)
                                .cornerRadius(10)
                                .padding()
                        }

                        // New Automode Button (Purple)
                        Button(action: {
                            bleManager.sendAutoModeCommand()
                        }) {
                            Text("Automode")
                                .frame(width: 200, height: 50)
                                .background(Color.purple)
                                .foregroundColor(.white)
                                .cornerRadius(10)
                                .padding()
                        }
                        // New ShadeLock Mode Button (Green)
                                  /*    Button(action: {
                                          bleManager.toggleShadeLockMode()
                                      }) {
                                          Text("S-L")
                                              .frame(width: 50, height: 50)
                                              .background(Color.green)
                                              .foregroundColor(.white)
                                              .cornerRadius(25) // Rounded edges
                                              .padding()
                                      }*/


                        // Existing Disconnect Button
                        Button("Disconnect and Select Another Device") {
                            bleManager.disconnect()
                        }
                        .padding()
                    }
                } else {
                    DeviceListView(bleManager: bleManager)
                }
            } else {
                LoginView(isAuthenticated: $isAuthenticated)
            }
        }
    }
}

struct MainBLEView: View {
    @ObservedObject var bleManager: BLEManager

    var body: some View {
        VStack {
            Text("Control the Stepper Motors")
                .font(.headline)
                .padding()

            // Control Buttons for Up, Down, Left, Right
            HStack {
                Spacer()
                Button(action: {}, label: {
                    Text("Up")
                        .frame(width: 100, height: 50)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(10)
                })
                .simultaneousGesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { _ in
                            bleManager.startMoving(direction: .up)
                        }
                        .onEnded { _ in
                            bleManager.stopMoving()
                        }
                )
                Spacer()
            }

            HStack {
                Button(action: {}, label: {
                    Text("Left")
                        .frame(width: 100, height: 50)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(10)
                })
                .simultaneousGesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { _ in
                            bleManager.startMoving(direction: .left)
                        }
                        .onEnded { _ in
                            bleManager.stopMoving()
                        }
                )

                Spacer()

                Button(action: {}, label: {
                    Text("Right")
                        .frame(width: 100, height: 50)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(10)
                })
                .simultaneousGesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { _ in
                            bleManager.startMoving(direction: .right)
                        }
                        .onEnded { _ in
                            bleManager.stopMoving()
                        }
                )
            }

            HStack {
                Spacer()
                Button(action: {}, label: {
                    Text("Down")
                        .frame(width: 100, height: 50)
                        .background(Color.blue)
                        .foregroundColor(.white)
                        .cornerRadius(10)
                })
                .simultaneousGesture(
                    DragGesture(minimumDistance: 0)
                        .onChanged { _ in
                            bleManager.startMoving(direction: .down)
                        }
                        .onEnded { _ in
                            bleManager.stopMoving()
                        }
                )
                Spacer()
            }
        }
        .padding()
    }
}
