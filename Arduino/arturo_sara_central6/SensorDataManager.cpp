#include "SensorDataManager.h"

SensorDataManager::SensorDataManager()
{
}

void SensorDataManager::init()
{
    clearAll();
    Serial.println("Sensor Data Manager inicializado");
}

void SensorDataManager::clearAll()
{
    smokeReadings.clear();
    lightReadings.clear();
    temperatureReadings.clear();
    humidityReadings.clear();
    pirReadings.clear();
    distanceReadings.clear();
}

void SensorDataManager::loadFromJson(const String& jsonResponse)
{
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error)
    {
        Serial.print("Error parseando lecturas: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Procesar smoke
    if (doc.containsKey("smoke"))
    {
        JsonArray smokeArray = doc["smoke"].as<JsonArray>();
        for (JsonObject reading : smokeArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(smokeReadings, ts, val, isNull);
        }
    }
    
    // Procesar light
    if (doc.containsKey("light"))
    {
        JsonArray lightArray = doc["light"].as<JsonArray>();
        for (JsonObject reading : lightArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(lightReadings, ts, val, isNull);
        }
    }
    
    // Procesar temperature (dht11)
    if (doc.containsKey("dht11"))
    {
        JsonArray tempArray = doc["dht11"].as<JsonArray>();
        for (JsonObject reading : tempArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(temperatureReadings, ts, val, isNull);
        }
    }
    
    // Procesar humidity
    if (doc.containsKey("humidity"))
    {
        JsonArray humArray = doc["humidity"].as<JsonArray>();
        for (JsonObject reading : humArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(humidityReadings, ts, val, isNull);
        }
    }
    
    // Procesar pir
    if (doc.containsKey("pir"))
    {
        JsonArray pirArray = doc["pir"].as<JsonArray>();
        for (JsonObject reading : pirArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(pirReadings, ts, val, isNull);
        }
    }
    
    // Procesar distance
    if (doc.containsKey("distance"))
    {
        JsonArray distArray = doc["distance"].as<JsonArray>();
        for (JsonObject reading : distArray)
        {
            String ts = reading["timestamp"].as<String>();
            bool isNull = reading["value"].isNull();
            float val = isNull ? 0.0 : reading["value"].as<float>();
            addReading(distanceReadings, ts, val, isNull);
        }
    }
    
    Serial.print("Lecturas cargadas - Smoke: ");
    Serial.print(smokeReadings.size());
    Serial.print(", Light: ");
    Serial.print(lightReadings.size());
    Serial.print(", Temp: ");
    Serial.print(temperatureReadings.size());
    Serial.print(", Humidity: ");
    Serial.print(humidityReadings.size());
    Serial.print(", PIR: ");
    Serial.print(pirReadings.size());
    Serial.print(", Distance: ");
    Serial.println(distanceReadings.size());
}

void SensorDataManager::addReading(std::vector<SensorReading>& vec, const String& timestamp, float value, bool isNull)
{
    if (isNull)
    {
        SensorReading reading;
        reading.timestamp = timestamp;
        reading.isNull = true;
        vec.push_back(reading);
    }
    else
    {
        vec.push_back(SensorReading(timestamp, value));
    }
}

void SensorDataManager::pruneOldReadings(std::vector<SensorReading>& vec, unsigned long maxAge)
{
    // Implementacion simple: mantener solo las ultimas maxAge lecturas
    while (vec.size() > maxAge)
    {
        vec.erase(vec.begin());
    }
}

float SensorDataManager::getAverage(const std::vector<SensorReading>& vec, int lastN)
{
    if (vec.empty())
    {
        return 0.0;
    }
    
    float sum = 0.0;
    int count = 0;
    int start = (int)vec.size() - lastN;
    if (start < 0)
    {
        start = 0;
    }
    
    for (size_t i = start; i < vec.size(); i++)
    {
        if (!vec[i].isNull)
        {
            sum += vec[i].value;
            count++;
        }
    }
    
    if (count == 0)
    {
        return 0.0;
    }
    
    return sum / count;
}

int SensorDataManager::countValid(const std::vector<SensorReading>& vec)
{
    int count = 0;
    for (const auto& reading : vec)
    {
        if (!reading.isNull)
        {
            count++;
        }
    }
    return count;
}

const std::vector<SensorReading>& SensorDataManager::getSmokeReadings() const
{
    return smokeReadings;
}

const std::vector<SensorReading>& SensorDataManager::getLightReadings() const
{
    return lightReadings;
}

const std::vector<SensorReading>& SensorDataManager::getTemperatureReadings() const
{
    return temperatureReadings;
}

const std::vector<SensorReading>& SensorDataManager::getHumidityReadings() const
{
    return humidityReadings;
}

const std::vector<SensorReading>& SensorDataManager::getPirReadings() const
{
    return pirReadings;
}

const std::vector<SensorReading>& SensorDataManager::getDistanceReadings() const
{
    return distanceReadings;
}

float SensorDataManager::getAverageTemperature(int lastN)
{
    return getAverage(temperatureReadings, lastN);
}

float SensorDataManager::getAverageHumidity(int lastN)
{
    return getAverage(humidityReadings, lastN);
}

float SensorDataManager::getAverageSmoke(int lastN)
{
    return getAverage(smokeReadings, lastN);
}

bool SensorDataManager::hasRecentMotion(int lastN)
{
    if (pirReadings.empty())
    {
        return false;
    }
    
    int start = (int)pirReadings.size() - lastN;
    if (start < 0)
    {
        start = 0;
    }
    
    for (size_t i = start; i < pirReadings.size(); i++)
    {
        if (!pirReadings[i].isNull && pirReadings[i].value > 0.5)
        {
            return true;
        }
    }
    
    return false;
}

int SensorDataManager::getTotalValidReadings()
{
    return countValid(smokeReadings) + 
           countValid(lightReadings) + 
           countValid(temperatureReadings) + 
           countValid(humidityReadings) + 
           countValid(pirReadings) + 
           countValid(distanceReadings);
}
