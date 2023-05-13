//
//  ContentView.swift
//  AirHockey
//
//  Created by Lyam Maes on 10/05/2023.
//

import SwiftUI

struct ContentView: View {
    @ObservedObject public var bluetooth = BLEAirHockey();
    var points = 0x14;
    var body: some View {
        if(bluetooth.isPoweredOn) {
            if(bluetooth.connected) {
                    
                    ScoreboardView(bluetooth: bluetooth)
                    
//                } else {
//                    VStack {
//                        Button(action: {bluetooth.launchGame()}) {
//                            Text("Start Game")
//                                .font(.system(size: 32.0))
//                        }
//                    }
                }
            } else {
                VStack {
                    Text("En recherche")
                        .fontWeight(.bold)
                        .font(.system(size: 26.0))
                    Text("nous cherchons un appareil disponible")
                }
            }
        } else {
            VStack {
                Text("Bluetooth désactivé")
                    .fontWeight(.bold)
                    .font(.system(size: 26.0))
                Text("Veuillez activer le Bluetooth")
            }
        }
        
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
