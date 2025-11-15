-- Crear base de datos ignio si no existe
CREATE DATABASE IF NOT EXISTS ignio;
USE ignio;

-- Crear tabla de ubicaciones de zonas forestales
CREATE TABLE location (
  id_location INT PRIMARY KEY AUTO_INCREMENT,
  name VARCHAR(150),
  address VARCHAR(200),
  postal_code VARCHAR(10),
  latitude DECIMAL(10,8),
  longitude DECIMAL(11,8)
);

-- Crear tabla de dispositivos (estación central y sensores remotos)
CREATE TABLE device (
  id_device INT PRIMARY KEY AUTO_INCREMENT,
  id_location INT,
  name VARCHAR(100),
  mac_address VARCHAR(17),
  device_type ENUM('central_station', 'remote_sensor'),
  FOREIGN KEY (id_location) REFERENCES location(id_location)
);

-- Crear tabla para lecturas DHT11 (temperatura y humedad)
CREATE TABLE dht11_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  temperature DECIMAL(5,2) NULL,
  humidity DECIMAL(5,2) NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla para lecturas de sensor MQ-2 (humo)
CREATE TABLE smoke_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  value INT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla para lecturas de sensor PIR (detección de movimiento)
CREATE TABLE pir_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  motion BOOLEAN NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla para lecturas de distancia con sensor ultrasónico
CREATE TABLE distance_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  distance_cm DECIMAL(8,2) NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla para lecturas de botón físico
CREATE TABLE button_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  pressed BOOLEAN NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla para lecturas de fotoresistencia (luz)
CREATE TABLE light_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  value INT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Crear tabla de alertas generadas por el sistema
CREATE TABLE alert (
  alert_uuid VARCHAR(36) PRIMARY KEY,
  id_device INT,
  timestamp_started DATETIME(3) NOT NULL,
  timestamp_ended DATETIME(3) NULL,
  is_real BOOLEAN NULL,
  num_sensors_triggered INT NULL,
  responded BOOLEAN DEFAULT FALSE,
  response_time_seconds INT NULL,
  alert_type ENUM('automatic', 'manual') NOT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);