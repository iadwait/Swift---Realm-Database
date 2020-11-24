//
//  ViewController.swift
//  Realm Demo
//
//  Created by Adwait Barkale on 24/11/20.
//  Copyright © 2020 Adwait Barkale. All rights reserved.
//

import UIKit
import RealmSwift

class ViewController: UIViewController {
    
    var realm : Realm?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
        realm = try! Realm()
//        readData()
//        print("Update Data")
//        updateData()
//        print("Delete Object")
//        deleteObject()
    }
    
    @IBAction func btnSaveDataTapped(_ sender: UIButton) {
        
        let list = List<SomeArray>()
        var obj = SomeArray()
        obj.id = String(Date().timeIntervalSince1970)
        obj.title = "Array 1 Title"
        obj.value = "Array 1 Value"
        list.append(obj)
        
        obj = SomeArray()
        obj.id = String(Date().timeIntervalSince1970)
        obj.title = "Array 2 Title"
        obj.value = "Array 2 Value"
        list.append(obj)
        
        let someObject = SomeObject()
        someObject.id = String(Date().timeIntervalSince1970)
        someObject.title = "Some Title"
        someObject.some_integer = 12
        someObject.some_double = 12345.056
        someObject.some_array = list
        
        let mainModel = MainModelClass()
        mainModel.id = String(Date().timeIntervalSince1970)
        mainModel.name = "Adwait  Barkale"
        mainModel.age = "25"
        mainModel.some_object = someObject
        
        
        realm?.beginWrite()
        realm?.add(mainModel)
        try! realm?.commitWrite()
    }
    
    @IBAction func btnFetchDataTapped(_ sender: UIButton) {
        readData()
    }
    
    
    func readData()
    {
        realm?.beginWrite()
        let data = realm?.objects(MainModelClass.self)
        let jsonData = try! JSONEncoder().encode(data!)
        print(jsonData) //Data
        let strData = String(data: jsonData, encoding: .utf8)
        print(strData!)
        
        let obj = try! JSONDecoder().decode([MainModelClass].self, from: jsonData)
        if obj.count > 0{
            if let name = obj[0].name{
                print("Object 0 Name - \(name)")
                //print("Gender = \(obj[0].gender)")
            }
        }
        
        
        
        try! realm?.commitWrite()
    }
    
    @IBAction func btnUpdateTapped(_ sender: UIButton) {
        updateData()
    }
    
    
    func updateData()
    {
        let realm = try! Realm()
        let mainModel = realm.objects(MainModelClass.self).filter("age = '22'")
        if mainModel.count > 0 {
            try! realm.write{
                
                for user in 0..<mainModel.count{
                let someObject = mainModel[user].some_object ?? SomeObject()
                someObject.title = "This is New Title"
                mainModel[user].some_object = someObject
                }
                
                let jsonData = try! JSONEncoder().encode(mainModel)
                let strData = String(data: jsonData, encoding: .utf8)
                print(strData)
                
                //try! realm.commitWrite()
            }
        }
    }
    
    @IBAction func btnDeleteTapped(_ sender: UIButton) {
        deleteObject()
    }
    
    
    func deleteObject()
    {
        let realm = try! Realm()
        let obj = realm.objects(MainModelClass.self).filter("name = 'Adwait  Barkale'")
        if obj.count > 0 {
            try! realm.write{
                
                realm.delete(obj)
                
            }
        }
    }
    
}

