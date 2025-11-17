#ifndef SENSOR_DATA_MANAGER_H
#define SENSOR_DATA_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>

struct SensorReading
{
    String timestamp;
    float value;
    bool isNull;

    SensorReading() : value(0.0), isNull(true) {}
    SensorReading(String ts, float val) : timestamp(ts), value(val), isNull(false) {}
};

class SensorDataManager
{
private:
    std::vector<SensorReading> smokeReadings;
    std::vector<SensorReading> lightReadings;
    std::vector<SensorReading> temperatureReadings;
    std::vector<SensorReading> humidityReadings;
    std::vector<SensorReading> pirReadings;
    std::vector<SensorReading> distanceReadings;

    void addReading(std::vector<SensorReading> &vec, const String &timestamp, float value, bool isNull);
    void pruneOldReadings(std::vector<SensorReading> &vec, unsigned long maxAge);
    float getAverage(const std::vector<SensorReading> &vec, int lastN);
    int countValid(const std::vector<SensorReading> &vec);

public:
    SensorDataManager();
    void init();
    void clearAll();
    void loadFromJson(const String &jsonResponse);

    // Acceso a vectores
    const std::vector<SensorReading> &getSmokeReadings() const;
    const std::vector<SensorReading> &getLightReadings() const;
    const std::vector<SensorReading> &getTemperatureReadings() const;
    const std::vector<SensorReading> &getHumidityReadings() const;
    const std::vector<SensorReading> &getPirReadings() const;
    const std::vector<SensorReading> &getDistanceReadings() const;

    // Analisis
    float getAverageTemperature(int lastN = 10);
    float getAverageHumidity(int lastN = 10);
    float getAverageSmoke(int lastN = 10);
    float getAverageLight(int lastN = 10);
    bool hasRecentMotion(int lastN = 5);
    float getAverageDistance(int lastN = 5);
    int getTotalValidReadings();
};

#endif