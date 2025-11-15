using System;
using System.Collections.Generic;
using SistemaIoT.Model;
using Microsoft.AspNetCore.Mvc;
using MySql.Data.MySqlClient;

namespace SistemaIoT.Controllers
{
    // ============================================
    // CONTROLLER: DHT11 (Temperature + Humidity)
    // ============================================
    [Route("[controller]")]
    public class Dht11Controller : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpPost]
        public void Post([FromBody] Dht11Reading reading)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"INSERT INTO dht11_reading 
                               (id_device, timestamp, temperature, humidity) 
                               VALUES (@ID_DEVICE, NOW(), @TEMPERATURE, @HUMIDITY)";

            cmd.Parameters.AddWithValue("@ID_DEVICE", reading.IdDevice);
            cmd.Parameters.AddWithValue("@TEMPERATURE", reading.Temperature ?? (object)DBNull.Value);
            cmd.Parameters.AddWithValue("@HUMIDITY", reading.Humidity ?? (object)DBNull.Value);

            cmd.Prepare();
            cmd.ExecuteNonQuery();
            conexion.Close();
        }
    }

    // ============================================
    // CONTROLLER: Smoke
    // ============================================
    [Route("[controller]")]
    public class SmokeController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpPost]
        public void Post([FromBody] SmokeReading reading)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"INSERT INTO smoke_reading 
                               (id_device, timestamp, value) 
                               VALUES (@ID_DEVICE, NOW(), @VALUE)";

            cmd.Parameters.AddWithValue("@ID_DEVICE", reading.IdDevice);
            cmd.Parameters.AddWithValue("@VALUE", reading.Value ?? (object)DBNull.Value);

            cmd.Prepare();
            cmd.ExecuteNonQuery();
            conexion.Close();

            Console.WriteLine($"✓ Smoke guardado - Device: {reading.IdDevice}, Value: {reading.Value}");
        }
    }

    // ============================================
    // CONTROLLER: PIR
    // ============================================
    [Route("[controller]")]
    public class PirController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpPost]
        public void Post([FromBody] PirReading reading)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"INSERT INTO pir_reading 
                               (id_device, timestamp, motion) 
                               VALUES (@ID_DEVICE, NOW(), @MOTION)";

            cmd.Parameters.AddWithValue("@ID_DEVICE", reading.IdDevice);
            cmd.Parameters.AddWithValue("@MOTION", reading.Motion ?? (object)DBNull.Value);

            cmd.Prepare();
            cmd.ExecuteNonQuery();
            conexion.Close();

            Console.WriteLine($"✓ PIR guardado - Device: {reading.IdDevice}, Motion: {reading.Motion}");
        }
    }

    // ============================================
    // CONTROLLER: Distance
    // ============================================
    [Route("[controller]")]
    public class DistanceController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpPost]
        public void Post([FromBody] DistanceReading reading)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"INSERT INTO distance_reading 
                               (id_device, timestamp, distance_cm) 
                               VALUES (@ID_DEVICE, NOW(), @DISTANCE_CM)";

            cmd.Parameters.AddWithValue("@ID_DEVICE", reading.IdDevice);
            cmd.Parameters.AddWithValue("@DISTANCE_CM", reading.DistanceCm ?? (object)DBNull.Value);

            cmd.Prepare();
            cmd.ExecuteNonQuery();
            conexion.Close();

            Console.WriteLine($"✓ Distance guardado - Device: {reading.IdDevice}, Distance: {reading.DistanceCm}");
        }
    }

    // ============================================
    // CONTROLLER: Button
    // ============================================
    [Route("[controller]")]
    public class ButtonController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpPost]
        public void Post([FromBody] ButtonReading reading)
        {
            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"INSERT INTO button_reading 
                               (id_device, timestamp, pressed) 
                               VALUES (@ID_DEVICE, NOW(), @PRESSED)";

            cmd.Parameters.AddWithValue("@ID_DEVICE", reading.IdDevice);
            cmd.Parameters.AddWithValue("@PRESSED", reading.Pressed ?? (object)DBNull.Value);

            cmd.Prepare();
            cmd.ExecuteNonQuery();
            conexion.Close();

            Console.WriteLine($"✓ Button guardado - Device: {reading.IdDevice}, Pressed: {reading.Pressed}");
        }
    }

    // ============================================
    // CONTROLLER: Readings (GET SINCE)
    // ============================================
    [Route("[controller]")]
    public class ReadingsController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        [HttpGet("since/{timestamp}")]
        public List<UnifiedSensorReading> GetReadingsSince(string timestamp)
        {
            List<UnifiedSensorReading> allReadings = new List<UnifiedSensorReading>();

            try
            {
                DateTime parsedTimestamp = DateTime.Parse(timestamp);

                MySqlConnection conexion = new MySqlConnection(connectionString);
                conexion.Open();

                // ============================================
                // DHT11 Readings
                // ============================================
                MySqlCommand cmd = new MySqlCommand();
                cmd.Connection = conexion;
                cmd.CommandText = @"SELECT id, id_device, timestamp, temperature, humidity 
                                    FROM dht11_reading 
                                    WHERE timestamp >= @TIMESTAMP 
                                    ORDER BY timestamp DESC";
                cmd.Parameters.AddWithValue("@TIMESTAMP", parsedTimestamp);

                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var reading = new UnifiedSensorReading
                        {
                            SensorType = "dht11",
                            Id = Convert.ToInt64(reader["id"]),
                            IdDevice = Convert.ToInt32(reader["id_device"]),
                            Timestamp = Convert.ToDateTime(reader["timestamp"]),
                            Value = new
                            {
                                // Añadido (decimal?) en ambas líneas:
                                temperature = reader["temperature"] == DBNull.Value ? (decimal?)null : Convert.ToDecimal(reader["temperature"]),
                                humidity = reader["humidity"] == DBNull.Value ? (decimal?)null : Convert.ToDecimal(reader["humidity"])
                            }
                        };
                        allReadings.Add(reading);
                    }
                }

                // ============================================
                // Smoke Readings
                // ============================================
                cmd.CommandText = @"SELECT id, id_device, timestamp, value 
                                    FROM smoke_reading 
                                    WHERE timestamp >= @TIMESTAMP 
                                    ORDER BY timestamp DESC";
                cmd.Parameters.Clear();
                cmd.Parameters.AddWithValue("@TIMESTAMP", parsedTimestamp);

                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var reading = new UnifiedSensorReading
                        {
                            SensorType = "smoke",
                            Id = Convert.ToInt64(reader["id"]),
                            IdDevice = Convert.ToInt32(reader["id_device"]),
                            Timestamp = Convert.ToDateTime(reader["timestamp"]),
                            Value = reader["value"] == DBNull.Value ? null : Convert.ToInt32(reader["value"])
                        };
                        allReadings.Add(reading);
                    }
                }

                // ============================================
                // PIR Readings
                // ============================================
                cmd.CommandText = @"SELECT id, id_device, timestamp, motion 
                                    FROM pir_reading 
                                    WHERE timestamp >= @TIMESTAMP 
                                    ORDER BY timestamp DESC";
                cmd.Parameters.Clear();
                cmd.Parameters.AddWithValue("@TIMESTAMP", parsedTimestamp);

                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var reading = new UnifiedSensorReading
                        {
                            SensorType = "pir",
                            Id = Convert.ToInt64(reader["id"]),
                            IdDevice = Convert.ToInt32(reader["id_device"]),
                            Timestamp = Convert.ToDateTime(reader["timestamp"]),
                            Value = reader["motion"] == DBNull.Value ? null : Convert.ToBoolean(reader["motion"])
                        };
                        allReadings.Add(reading);
                    }
                }

                // ============================================
                // Distance Readings
                // ============================================
                cmd.CommandText = @"SELECT id, id_device, timestamp, distance_cm 
                                    FROM distance_reading 
                                    WHERE timestamp >= @TIMESTAMP 
                                    ORDER BY timestamp DESC";
                cmd.Parameters.Clear();
                cmd.Parameters.AddWithValue("@TIMESTAMP", parsedTimestamp);

                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var reading = new UnifiedSensorReading
                        {
                            SensorType = "distance",
                            Id = Convert.ToInt64(reader["id"]),
                            IdDevice = Convert.ToInt32(reader["id_device"]),
                            Timestamp = Convert.ToDateTime(reader["timestamp"]),
                            Value = reader["distance_cm"] == DBNull.Value ? null : Convert.ToDecimal(reader["distance_cm"])
                        };
                        allReadings.Add(reading);
                    }
                }

                // ============================================
                // Button Readings
                // ============================================
                cmd.CommandText = @"SELECT id, id_device, timestamp, pressed 
                                    FROM button_reading 
                                    WHERE timestamp >= @TIMESTAMP 
                                    ORDER BY timestamp DESC";
                cmd.Parameters.Clear();
                cmd.Parameters.AddWithValue("@TIMESTAMP", parsedTimestamp);

                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var reading = new UnifiedSensorReading
                        {
                            SensorType = "button",
                            Id = Convert.ToInt64(reader["id"]),
                            IdDevice = Convert.ToInt32(reader["id_device"]),
                            Timestamp = Convert.ToDateTime(reader["timestamp"]),
                            Value = reader["pressed"] == DBNull.Value ? null : Convert.ToBoolean(reader["pressed"])
                        };
                        allReadings.Add(reading);
                    }
                }

                conexion.Close();

                // Ordenar por timestamp descendente
                allReadings.Sort((a, b) => b.Timestamp.CompareTo(a.Timestamp));

                Console.WriteLine($"✓ GET /readings/since - {allReadings.Count} lecturas obtenidas desde {parsedTimestamp}");

                return allReadings;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"✗ Error en GET /readings/since: {ex.Message}");
                return new List<UnifiedSensorReading>();
            }
        }
    }

    // ============================================
    // CONTROLLER: Alert
    // ============================================
    [Route("[controller]")]
    public class AlertController : Controller
    {
        private string connectionString = "Server=mysql-95caaf-tec-fdb5.k.aivencloud.com;Port=25096;Database=ignio;Uid=avnadmin;Password=AVNS_HHx9FJdxWHsM1S3p2lc;SslMode=Required;";

        // GET: /alert (obtener todas las alertas)
        [HttpGet]
        public List<Alert> Get()
        {
            List<Alert> alertas = new List<Alert>();

            MySqlConnection conexion = new MySqlConnection(connectionString);
            conexion.Open();
            MySqlCommand cmd = new MySqlCommand();
            cmd.Connection = conexion;

            cmd.CommandText = @"SELECT alert_uuid, id_device, timestamp_started, timestamp_ended, 
                                       is_real, num_sensors_triggered, responded, response_time_seconds, alert_type
                                FROM alert 
                                ORDER BY timestamp_started DESC 
                                LIMIT 100";

            using (var reader = cmd.ExecuteReader())
            {
                while (reader.Read())
                {
                    Alert alerta = new Alert();
                    alerta.AlertUuid = Convert.ToString(reader["alert_uuid"]);
                    alerta.IdDevice = Convert.ToInt32(reader["id_device"]);
                    alerta.TimestampStarted = Convert.ToDateTime(reader["timestamp_started"]);
                    alerta.TimestampEnded = reader["timestamp_ended"] == DBNull.Value ? null : Convert.ToDateTime(reader["timestamp_ended"]);
                    alerta.IsReal = reader["is_real"] == DBNull.Value ? null : Convert.ToBoolean(reader["is_real"]);
                    alerta.NumSensorsTriggered = reader["num_sensors_triggered"] == DBNull.Value ? null : Convert.ToInt32(reader["num_sensors_triggered"]);
                    alerta.Responded = Convert.ToBoolean(reader["responded"]);
                    alerta.ResponseTimeSeconds = reader["response_time_seconds"] == DBNull.Value ? null : Convert.ToInt32(reader["response_time_seconds"]);
                    alerta.AlertType = Convert.ToString(reader["alert_type"]);

                    alertas.Add(alerta);
                }
            }

            conexion.Close();
            return alertas;
        }

        // POST: /alert (crear nueva alerta)
        [HttpPost]
        public IActionResult Post([FromBody] AlertCreateRequest alertRequest)
        {
            try
            {
                MySqlConnection conexion = new MySqlConnection(connectionString);
                conexion.Open();
                MySqlCommand cmd = new MySqlCommand();
                cmd.Connection = conexion;

                cmd.CommandText = @"INSERT INTO alert 
                                   (alert_uuid, id_device, timestamp_started, timestamp_ended, 
                                    is_real, num_sensors_triggered, responded, response_time_seconds, alert_type)
                                   VALUES (@UUID, @ID_DEVICE, @START, NULL, NULL, @NUM_SENSORS, FALSE, NULL, @TYPE)";

                cmd.Parameters.AddWithValue("@UUID", alertRequest.AlertUuid);
                cmd.Parameters.AddWithValue("@ID_DEVICE", alertRequest.IdDevice);
                cmd.Parameters.AddWithValue("@START", alertRequest.TimestampStarted);
                cmd.Parameters.AddWithValue("@NUM_SENSORS", alertRequest.NumSensorsTriggered ?? (object)DBNull.Value);
                cmd.Parameters.AddWithValue("@TYPE", alertRequest.AlertType);

                cmd.Prepare();
                cmd.ExecuteNonQuery();
                conexion.Close();

                Console.WriteLine($"✓ Alerta creada - UUID: {alertRequest.AlertUuid}, Type: {alertRequest.AlertType}");
                return Ok(new { message = "Alert created", uuid = alertRequest.AlertUuid });
            }
            catch (Exception ex)
            {
                Console.WriteLine($"✗ Error creando alerta: {ex.Message}");
                return BadRequest(new { error = ex.Message });
            }
        }

        // PUT: /alert/{uuid} (actualizar alerta)
        [HttpPut("{uuid}")]
        public IActionResult Put(string uuid, [FromBody] AlertUpdateRequest updateRequest)
        {
            try
            {
                MySqlConnection conexion = new MySqlConnection(connectionString);
                conexion.Open();
                MySqlCommand cmd = new MySqlCommand();
                cmd.Connection = conexion;

                cmd.CommandText = @"UPDATE alert 
                                   SET timestamp_ended = @END, 
                                       is_real = @IS_REAL, 
                                       responded = TRUE, 
                                       response_time_seconds = @RESPONSE_TIME
                                   WHERE alert_uuid = @UUID";

                cmd.Parameters.AddWithValue("@UUID", uuid);
                cmd.Parameters.AddWithValue("@END", updateRequest.TimestampEnded);
                cmd.Parameters.AddWithValue("@IS_REAL", updateRequest.IsReal);
                cmd.Parameters.AddWithValue("@RESPONSE_TIME", updateRequest.ResponseTimeSeconds);

                cmd.Prepare();
                int rowsAffected = cmd.ExecuteNonQuery();
                conexion.Close();

                if (rowsAffected > 0)
                {
                    Console.WriteLine($"✓ Alerta actualizada - UUID: {uuid}, IsReal: {updateRequest.IsReal}, ResponseTime: {updateRequest.ResponseTimeSeconds}s");
                    return Ok(new { message = "Alert updated", uuid = uuid });
                }
                else
                {
                    return NotFound(new { error = "Alert not found" });
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"✗ Error actualizando alerta: {ex.Message}");
                return BadRequest(new { error = ex.Message });
            }
        }
    }
}