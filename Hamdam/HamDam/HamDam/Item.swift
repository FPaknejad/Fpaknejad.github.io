//
//  Item.swift
//  HamDam
//
//  Created by Fatemeh Paknejad on 03.11.25.
//

import Foundation
import SwiftData

@Model
final class Item {
    var timestamp: Date
    
    init(timestamp: Date) {
        self.timestamp = timestamp
    }
}
