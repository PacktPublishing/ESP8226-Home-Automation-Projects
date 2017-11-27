
/*
bool settings_valid_old()
{
  if(uint32_t(sysCfg.cfg_holder) == (uint32_t)CFG_HOLDER)
   return true;
  else
   return false; 
}
*/
bool settings_valid()
{
  if (SPIFFS.exists("/config.json"))  
   return true;
  else
   return false;  
}


/********************************************************************
* FunctionName: settings_clear
* Description : clear settings data
* Parameters  : None
* Returns     : True/False                                                     
*********************************************************************/
void  settings_clear()
{
////  noInterrupts();
////  spi_flash_erase_sector(CFG_LOCATION + 0);
////  spi_flash_erase_sector(CFG_LOCATION + 1);
////  spi_flash_erase_sector(CFG_LOCATION + 2);
////  spi_flash_erase_sector(CFG_LOCATION + 3);
////  interrupts();
//
//  SECTOR3_DATA nullStruct; 
//  
//  nullStruct.flag =0;
//  nullStruct.cron_count = 0;
//  nullStruct.pad[0] = 0;
//  nullStruct.pad[1] = 0;  
//  //CLEAR THE SECTOR 1 WHERE THE cron_count is stored
//  noInterrupts(); 
//  spi_flash_erase_sector(CFG_LOCATION + 1);
//  spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE, (uint32 *)&nullStruct, sizeof(SECTOR3_DATA));
//  interrupts();
//
//  //CLEAR THE SECTOR 0 WHERE IS STORED THE CONNECTING 
//  SYSCFG nullSysCfg;
//  nullSysCfg.cfg_holder = 0;  
//  noInterrupts();
//  spi_flash_erase_sector(CFG_LOCATION + 0);
//  spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE, (uint32 *)&nullSysCfg, sizeof(SYSCFG));
//  interrupts();
  
}
/********************************************************************
* FunctionName: setting_load
* Description : Read settings from flash. If is a new non configured
*               module just read garbage from memory, if is a configured
*               module read the actual data. For data validity check
*               function settings_validate()
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
void settings_load()
{
  //JUST FOR TESTING.......
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println(F("mounting FS...settings_load"));

  if (SPIFFS.begin()) {
    //Serial.println("mounted file system settings_load");
    if (SPIFFS.exists("/config.json")) 
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        //Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
        {
          Serial.println("\nparsed json");
          strcpy(sysCfg.sta_ssid,   json["sta_ssid"]   );
          strcpy(sysCfg.sta_pwd,    json["sta_pass"]   );
          strcpy(sysCfg.mqtt_host,  json["mqtt_host"]  );
          strcpy(sysCfg.mqtt_port,  json["mqtt_port"]  );
          strcpy(sysCfg.mqtt_user,  json["mqtt_user"]  );
          strcpy(sysCfg.mqtt_pass,  json["mqtt_pass"]  );
          strcpy(sysCfg.time_zone,  json["timezone"]   );
          strcpy(sysCfg.alexa_name, json["alexa_name"] );
          strcpy(sysCfg.sw_version, json["sw_version"] ); //String( sysCfg.alexa_name).c_str()
        } else {
          Serial.println(F("failed to load json config 1"));
        }
      }
    }
    else
    {
      Serial.println(F("ERROR: /config.json doesn't exists in settings load!"));
    }
  } 
  else 
  {
    //Serial.println("ERROR:  failed to mount FS. ");
  }  
}
/*
void settings_load_old()
{
  INFO("\n\n============= settings_load ===============\n");
   noInterrupts();
   spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE, (uint32 *)&sector3Data, sizeof(SECTOR3_DATA));
   spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE, (uint32 *)&sysCfg, sizeof(SYSCFG));

//    File sector3DataFile = SPIFFS.open("/sector3Data.json", "r");
//    if(!sector3DataFile)
//    {
//      INFO("Fail to open sector3Data.json file");
//      return;
//    }
//
//    size_t sizeSector3DataFile = sector3DataFile.size();
//    if(sizeSector3DataFile > 1024) {
//        Serial.println("Config file size is too large");
//        return;      
//    }
//    // Allocate a buffer to store contents of the file.
//    std::unique_ptr<char[]> buf(new char[sizeSector3DataFile]);
//    // We don't use String here because ArduinoJson library requires the input
//    // buffer to be mutable. If you don't use ArduinoJson, you may as well
//    // use configFile.readString instead.
//    sector3DataFile.readBytes(buf.get(), sizeSector3DataFile);
//    StaticJsonBuffer<200> jsonBuffer;
//    JsonObject& json = jsonBuffer.parseObject(buf.get());
//
//    if (!json.success()) {
//       Serial.println("Failed to parse config file");
//       return;
//    }
//
//    const int cron_count = json["cron_count"];
//    sector3Data.cron_count = cron_count;
//    File sysCfgFile = SPIFFS.open("/sysCfg.json", "r");
//    if(!sysCfgFile)
//    {
//      INFO("Fail to open sysCfg.json file for reading");
//      return;
//    }
//
//    size_t sizesysCfgFile = sysCfgFile.size();
//    if(sizesysCfgFile > 1024) {
//        Serial.println("Config file size is too large");
//        return;      
//    }
//    // Allocate a buffer to store contents of the file.
//    std::unique_ptr<char[]> buf_sysCfgFile(new char[sizesysCfgFile]);
//    // We don't use String here because ArduinoJson library requires the input
//    // buffer to be mutable. If you don't use ArduinoJson, you may as well
//    // use configFile.readString instead.
//    sysCfgFile.readBytes(buf_sysCfgFile.get(), sizesysCfgFile);
//    StaticJsonBuffer<200> jsonBuffer_syscfg;
//    JsonObject& json_sysCfgFile = jsonBuffer_syscfg.parseObject(buf_sysCfgFile.get());
//
//    if (!json_sysCfgFile.success()) {
//       Serial.println("Failed to parse config file");
//       return;
//    }

//     strcpy( sysCfg.device_id, json_sysCfgFile["device_id"] );
//     strcpy( sysCfg.sta_ssid, json_sysCfgFile["sta_ssid"] );
//     strcpy( sysCfg.sta_pwd, json_sysCfgFile["sta_pwd"] );
//     strcpy( sysCfg.mqtt_host, json_sysCfgFile["mqtt_host"] );
//     strcpy( sysCfg.mqtt_user, json_sysCfgFile["mqtt_user"] );
//     strcpy( sysCfg.mqtt_pass, json_sysCfgFile["mqtt_pass"] );
//     sysCfg.sw_version = json_sysCfgFile["sw_version"];
     
    interrupts();

//    INFO("%s\n",json_sysCfgFile["device_id"]);
//    INFO("%s\n",json_sysCfgFile["sta_ssid"]);
//    INFO("%s\n",json_sysCfgFile["sta_pwd"]);
//    INFO("%s\n",json_sysCfgFile["mqtt_host"]);
//    INFO("%s\n",json_sysCfgFile["mqtt_user"]);
//    INFO("%s\n",json_sysCfgFile["mqtt_pass"]);
//    INFO("%s\n",json_sysCfgFile["sw_version"]);
//    INFO("===================================\n");


//
//
//  StaticJsonBuffer<200> jsonBuffer;
//  JsonObject& json = jsonBuffer.createObject();
//  json["serverName"] = "api.example.com";
//  json["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";
//  json["my_num"]= 567;
//  
//  File configFile1 = SPIFFS.open("/config.json", "w");
//  if (!configFile1) {
//    Serial.println("Failed to open config file for writing");
//    return ;//false;
//  }
//
//  json.printTo(configFile1);
////-0-0-0-0-0-0-0-0-0-0
//
//  File configFile = SPIFFS.open("/config.json", "r");
//  if (!configFile) {
//    Serial.println("Failed to open config file");
//    return ;//false;
//  }
//
//  size_t size = configFile.size();
//  Serial.println(configFile.size());
//  if (size > 1024) {
//    Serial.println("Config file size is too large");
//    return ;//false;
//  }
//
//  // Allocate a buffer to store contents of the file.
//  std::unique_ptr<char[]> buf(new char[size]);
//
//  // We don't use String here because ArduinoJson library requires the input
//  // buffer to be mutable. If you don't use ArduinoJson, you may as well
//  // use configFile.readString instead.
//  configFile.readBytes(buf.get(), size);
//
//  StaticJsonBuffer<200> jsonBuffer1;
//  JsonObject& json1 = jsonBuffer1.parseObject(buf.get());
//
//  if (!json1.success()) {
//    Serial.println("Failed to parse config file");
//    return;// false;
//  }
//
//  const char* serverName = json1["serverName"];
//  const char* accessToken = json1["accessToken"];
//  const int my_num = json["my_num"];
//
//  // Real world application would store these values in some variables for
//  // later use.
//
//  Serial.print("Loaded serverName: ");
//  Serial.println(serverName);
//  Serial.print("Loaded accessToken: ");
//  Serial.println(accessToken);
//  Serial.print("My num ");
//  Serial.println(my_num + 1);
//  
//
//
//
//
//



}
*/
/********************************************************************
* FunctionName: setting_save
* Description : Save data from sysCfg to flash. If flag from sector3Data
*               is 0 save in sector 1 if 1 then save in sector3
* Parameters  : None
* Returns     : None                                                     
*********************************************************************/
int settings_save()
{
    Serial.println("saving config");

    //SPIFFS.format(); Serial.println ("..............DELETE THIS LINE ........ Settings.h 287");
    
    //sysCfg.cfg_holder = CFG_HOLDER;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
   // json["cfg_holder"]=sysCfg.cfg_holder;
    json["sta_ssid"]    = sysCfg.sta_ssid;
    json["sta_pass"]    = sysCfg.sta_pwd;
    json["mqtt_host"]   = sysCfg.mqtt_host;
    json["mqtt_port"]   = sysCfg.mqtt_port;
    json["mqtt_user"]   = sysCfg.mqtt_user;
    json["mqtt_pass"]   = sysCfg.mqtt_pass;
    json["timezone"]    = sysCfg.time_zone;
    json["alexa_name"]  = sysCfg.alexa_name;
    json["sw_version"]  = sysCfg.sw_version;
    //json["device_id"] = sysCfg.device_id;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println(F("failed to open config file for writing"));
    }
    json.printTo(Serial); //TO BE DELETED
    json.printTo(configFile);    
    configFile.close();  

   /************************************************************************
    *  Create sector3.json file with initial values.
    ************************************************************************/
   //create here aditional file to keep sector3 (flag, cron_count, pad[2] )
    File sector3 = SPIFFS.open("/sector3.json", "w");
    if (!sector3) {
      Serial.println("failed to open sector3.json file for writing");
    }
   
    DynamicJsonBuffer jsonSector3;
    JsonObject& json3 = jsonSector3.createObject();
    json3["flag"]         = 1;
    json3["cron_count"]   = 0;
    json3.printTo(Serial); //TO BE DELETED
    json3.printTo(sector3);
    sector3.close();//new CC
//    Serial.println(F("[settings.c] settings_save: Done saving! restart the ESP\r\n"));
//    delay(300);
//    ESP.restart();
    return 1;  
    
}


void resetSettings()
{
  //Serial.println("CLEAR SETTINGS.....!");
  settings_clear();// temporary
  //writeCronData = true;
  //ota_update = true;
  Serial.println("CLEAR SETTINGS.....Done !");
}

