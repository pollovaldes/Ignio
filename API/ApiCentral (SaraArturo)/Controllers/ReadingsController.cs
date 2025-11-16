using Microsoft.AspNetCore.Mvc;
using ApiCentral.Models;
using MySql.Data.MySqlClient;

namespace ApiCentral.Controllers;

[ApiController]
[Route("[controller]")]
public class ReadingsController : ControllerBase
{
    private readonly IConfiguration _config;

    public ReadingsController(IConfiguration cfg)
    {
        _config = cfg;
    }

    [HttpGet("since/{timestamp}")]
    public IActionResult GetSince(DateTime timestamp)
    {
        string connStr = _config.GetConnectionString("DefaultConnection");

        using var conn = new MySqlConnection(connStr);
        conn.Open();

        var result = new Dictionary<string, List<UnifiedSensorReading>>();

        // Función auxiliar
        List<UnifiedSensorReading> Query(string sql, string type)
        {
            var list = new List<UnifiedSensorReading>();

            using var cmd = new MySqlCommand(sql, conn);
            cmd.Parameters.AddWithValue("@ts", timestamp);

            using var reader = cmd.ExecuteReader();
            while (reader.Read())
            {
                list.Add(new UnifiedSensorReading
                {
                    SensorType = type,
                    Id = reader.GetInt64(0),
                    IdDevice = reader.GetInt32(1),
                    Timestamp = reader.GetDateTime(2),
                    Value = reader.IsDBNull(3) ? null : reader.GetValue(3)
                });
            }
            return list;
        }

        result["smoke"] = Query("SELECT id, id_device, timestamp, value FROM smoke_reading WHERE timestamp >= @ts", "smoke");
        result["light"] = Query("SELECT id, id_device, timestamp, value FROM light_reading WHERE timestamp >= @ts", "light");
        result["dht11"] = Query("SELECT id, id_device, timestamp, temperature FROM dht11_reading WHERE timestamp >= @ts", "temperature");
        result["humidity"] = Query("SELECT id, id_device, timestamp, humidity FROM dht11_reading WHERE timestamp >= @ts", "humidity");
        result["pir"] = Query("SELECT id, id_device, timestamp, motion FROM pir_reading WHERE timestamp >= @ts", "pir");
        result["distance"] = Query("SELECT id, id_device, timestamp, distance_cm FROM distance_reading WHERE timestamp >= @ts", "distance");

        return Ok(result);
    }
}
