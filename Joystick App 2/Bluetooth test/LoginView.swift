import SwiftUI

struct LoginView: View {
    @Binding var isAuthenticated: Bool
    
    @State private var username: String = ""
    @State private var password: String = ""
    @State private var isRegistering = false
    @State private var showingAlert = false
    @State private var alertMessage = "Incorrect username or password. Please try again."

    var body: some View {
        NavigationView {
            VStack(spacing: 20) {
                TextField("Username", text: $username)
                    .autocapitalization(.none)
                    .padding()
                    .background(Color(.secondarySystemBackground))

                SecureField("Password", text: $password)
                    .padding()
                    .background(Color(.secondarySystemBackground))

                if isRegistering {
                    Button("Register") {
                        registerUser(username: username, password:  password)
                    }
                    .foregroundColor(.white)
                    .padding()
                    .background(Color.green)
                    .cornerRadius(8)

                    Button("Already Register? Login in here") {
                        isRegistering = false
                    }
                    .padding()
                } else {
                    Button("Log In") {
                        // Attempt to log in with provided credentials
                        if !loginUser(username: username, password: password) {
                            showingAlert = true // Only show alert if login fails
                        } else {
                            isAuthenticated = true // Set authenticated to true if login is successful
                        }
                    }
                    .foregroundColor(.white)
                    .padding()
                    .background(Color.blue)
                    .cornerRadius(8)
                    
                    Button( "No Account? | Sign Up") {
                        isRegistering = true
                    }
                    .padding()
                }
            }
            .padding()
            .navigationTitle(isRegistering ? "Create new Account" : "Sunbrella")
            .alert(isPresented: $showingAlert) {
                Alert(title: Text("Login Failed"), message: Text(alertMessage), dismissButton: .default(Text("OK")))
            }
        }
    }
    
    private func registerUser(username: String, password: String) {
        UserDefaults.standard.set(password, forKey: username)
        isAuthenticated = true // Log in the user immediately after registration
    }
    
    private func loginUser(username: String, password: String) -> Bool {
        let storedPassword = UserDefaults.standard.string(forKey: username)
        return password == storedPassword
    }
}
