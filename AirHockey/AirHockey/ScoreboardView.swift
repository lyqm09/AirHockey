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
        HStack {
            
            Spacer()
            
            Text("\(bluetooth.score[0])")
                .font(.system(size: fontSize))
                .foregroundColor(.blue)
            
            Spacer()
            Divider()
                .frame(height: fontSize*1.5)
            Spacer()
            
            Text("\(bluetooth.score[1])")
                .font(.system(size: fontSize))
                .foregroundColor(.red)
            
            Spacer()
        }
    }
}

struct ScoreboardView_Previews: PreviewProvider {
    static var bluetooth = BLEAirHockey();
    static var previews: some View {
        ScoreboardView(bluetooth: bluetooth);
    }
}
