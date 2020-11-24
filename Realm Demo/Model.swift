//
//  Model.swift
//  Realm Demo
//
//  Created by Adwait Barkale on 24/11/20.
//  Copyright © 2020 Adwait Barkale. All rights reserved.
//

import Foundation
import RealmSwift

class MainModelClass: Object,Codable{
    
    @objc dynamic var id: String?
    @objc dynamic var name: String?
    @objc dynamic var gender: String?
    @objc dynamic var age: String?
    @objc dynamic var some_object:SomeObject?
    
}

class SomeObject: Object,Codable{
    
    @objc dynamic var id: String?
    @objc dynamic var title: String?
    @objc dynamic var some_integer: Int = 0
    @objc dynamic var some_double: Double = 0.0
    var some_array : List<SomeArray> = List<SomeArray>()
    
}

class SomeArray: Object,Codable{
    
    @objc dynamic var id: String?
    @objc dynamic var title: String?
    @objc dynamic var value: String?
    
}
