//
//  ScoreboardView.swift
//  AirHockey
//
//  Created by Lyam Maes on 12/05/2023.
//

import SwiftUI

struct ScoreboardView: View {
    @ObservedObject var bluetooth: BLEAirHockey;
    var fontSize: CGFloat = 200;
    var body: some View {
        @State var s1 = bluetooth.score[0];
        @State var s2 = bluetooth.score[1];
        if(s1 == 0xFE) {
            ZStack {
                s2 == 0x00 ? Color.blue : Color.red
                VStack {
                    Text("Victoire")
                        .fontWeight(.bold)
                        .font(.system(size: 100.0))
                        .foregroundColor(.white)
                }
            }
            .ignoresSafeArea()
        } else {
            HStack {
                
                Spacer()
                
                Text("\(s1)")
                    .font(.system(size: fontSize))
                    .foregroundColor(.blue)
                
                Spacer()
                Divider()
                    .frame(height: fontSize*1.5)
                Spacer()
                
                Text("\(s2)")
                    .font(.system(size: fontSize))
                    .foregroundColor(.red)
                
                Spacer()
                
                
            }
        }
    }
}

struct ScoreboardView_Previews: PreviewProvider {
    static var bluetooth = BLEAirHockey();
    static var previews: some View {
        ScoreboardView(bluetooth: bluetooth);
    }
}
