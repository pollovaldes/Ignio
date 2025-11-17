-- Crear base de datos
CREATE DATABASE IF NOT EXISTS ignio;
USE ignio;

-- Tabla: Zonas del bosque
CREATE TABLE location (
  id_location INT PRIMARY KEY AUTO_INCREMENT,
  name VARCHAR(150),
  address VARCHAR(200),
  postal_code VARCHAR(10),
  latitude DECIMAL(10,8),
  longitude DECIMAL(11,8)
);

-- Tabla: Dispositivos (Central y sensores remotos)
CREATE TABLE device (
  id_device INT PRIMARY KEY AUTO_INCREMENT,
  id_location INT,
  name VARCHAR(100),
  mac_address VARCHAR(17),
  device_type ENUM('central_station', 'remote_sensor'),
  FOREIGN KEY (id_location) REFERENCES location(id_location)
);

-- Tabla: Lecturas DHT11 (Temperatura y Humedad)
CREATE TABLE dht11_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  temperature DECIMAL(5,2) NULL,
  humidity DECIMAL(5,2) NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Tabla: Lecturas de humo MQ-2
CREATE TABLE smoke_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  value INT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Tabla: Movimiento PIR
CREATE TABLE pir_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  duration_seconds DECIMAL(8,2) NULL,
  event_number INT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device),
  INDEX idx_device_timestamp (id_device, timestamp)
);

-- Tabla: Distancia Ultrasonido
CREATE TABLE distance_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  distance_cm DECIMAL(8,2) NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Tabla: Fotoresistencia (Luz)
CREATE TABLE light_reading (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT,
  timestamp DATETIME(3),
  value INT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device)
);

-- Tabla: Alertas (Incendios automáticos o manuales)
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

-- Tabla: warning_event (Advertencias de fallos HTTP, anomalías de sensores, etc)
CREATE TABLE warning_event (
  id BIGINT PRIMARY KEY AUTO_INCREMENT,
  id_device INT NOT NULL,
  timestamp DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  warning_type ENUM(
    'http_get_failed',
    'http_post_failed', 
    'http_put_failed',
    'connection_lost',
    'sensor_anomaly',
    'motion_detected',
    'distance_anomaly'
  ) NOT NULL,
  http_method VARCHAR(10) NULL,
  http_endpoint VARCHAR(255) NULL,
  message VARCHAR(500) NOT NULL,
  FOREIGN KEY (id_device) REFERENCES device(id_device),
  INDEX idx_device_timestamp (id_device, timestamp),
  INDEX idx_warning_type (warning_type)
);


-- Insertar ubicación inicial
INSERT INTO location (name, address, postal_code, latitude, longitude) 
VALUES ('Zona Forestal Norte', 'Reserva Natural km 15', '64000', 25.6867, -100.3161);

-- Insertar dispositivos
INSERT INTO device (id_location, name, mac_address, device_type) 
VALUES 
  (1, 'Central_FireStation', 'AA:BB:CC:DD:EE:FF', 'central_station'),
  (1, 'SensorHumo_Nodo1',    'AA:BB:CC:DD:EE:01', 'remote_sensor'),
  (1, 'SensorLuz_Nodo2',     'AA:BB:CC:DD:EE:02', 'remote_sensor'),
  (1, 'SensorDHT11_Nodo3',   'AA:BB:CC:DD:EE:03', 'remote_sensor'),
  (1, 'SensorUltrasonico_N4','AA:BB:CC:DD:EE:04', 'remote_sensor'),
  (1, 'SensorPIR_Nodo5',     'AA:BB:CC:DD:EE:05', 'remote_sensor');
